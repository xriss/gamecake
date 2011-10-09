/*
 * THREADING.C   	                    Copyright (c) 2007-08, Asko Kauppi
 *
 * Lua Lanes OS threading specific code.
 *
 * Defines:
 *  TBB_SCALABLE_ALLOC  Use TBB (Intel Threading Building Blocks) scalable
 *                      allocator (EXPERIMENTAL!!!)
 *  ...
*/

/*
===============================================================================

Copyright (C) 2007-08 Asko Kauppi <akauppi@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "threading.h"
#include "lua.h"


/* Linux needs to check, whether it's been run as root
*/
#ifdef PLATFORM_LINUX
# include <sys/types.h>
# include <unistd.h>
  volatile bool_t sudo;
#endif

#ifdef _MSC_VER
// ".. selected for automatic inline expansion" (/O2 option)
# pragma warning( disable : 4711 )
// ".. type cast from function pointer ... to data pointer"
# pragma warning( disable : 4054 )
#endif

//#define THREAD_CREATE_RETRIES_MAX 20
    // loops (maybe retry forever?)


/*---=== Threading ===---*/

//---
// It may be meaningful to explicitly limit the new threads' C stack size.
// We should know how much Lua needs in the C stack, all Lua side allocations
// are done in heap so they don't count.
//
// Consequence of _not_ limiting the stack is running out of virtual memory
// with 1000-5000 threads on 32-bit systems.
//
// Note: using external C modules may be affected by the stack size check.
//       if having problems, set back to '0' (default stack size of the system).
// 
// Win32:       64K (?)
// Win64:       xxx
//
// Linux x86:   2MB     Ubuntu 7.04 via 'pthread_getstacksize()'
// Linux x64:   xxx
// Linux ARM:   xxx
//
// OS X 10.4.9: 512K    <http://developer.apple.com/qa/qa2005/qa1419.html>
//                      valid values N * 4KB
//
#ifndef _THREAD_STACK_SIZE
# if (defined PLATFORM_WIN32) || (defined PLATFORM_WINCE)
#  define _THREAD_STACK_SIZE 0
      // TBD: does it work with less?
# elif (defined PLATFORM_OSX)
#  define _THREAD_STACK_SIZE (524288/2)   // 262144
      // OS X: "make test" works on 65536 and even below
      //       "make perftest" works on >= 4*65536 == 262144 (not 3*65536)
# elif (defined PLATFORM_LINUX) && (defined __i386)
#  define _THREAD_STACK_SIZE (2097152/16)  // 131072
      // Linux x86 (Ubuntu 7.04): "make perftest" works on /16 (not on /32)
# elif (defined PLATFORM_BSD) && (defined __i386)
#  define _THREAD_STACK_SIZE (1048576/8)  // 131072
      // FreeBSD 6.2 SMP i386: ("gmake perftest" works on /8 (not on /16)
# endif
#endif

#if (defined PLATFORM_WIN32) || (defined PLATFORM_WINCE)
  /* FAIL is for unexpected API return values - essentially programming 
   * error in _this_ code. 
   */
  static void FAIL( const char *funcname, int rc ) {
    fprintf( stderr, "%s() failed! (%d)\n", funcname, rc );
    abort();
  }
  //
    /* MSDN: "If you would like to use the CRT in ThreadProc, use the
              _beginthreadex function instead (of CreateThread)."
       MSDN: "you can create at most 2028 threads"
    */
  void
  THREAD_CREATE( THREAD_T *ref,
                 THREAD_RETURN_T (__stdcall *func)( void * ),
                     // Note: Visual C++ requires '__stdcall' where it is
                 void *data, int prio /* -3..+3 */ ) {

    HANDLE h= (HANDLE)_beginthreadex( NULL, // security
                              _THREAD_STACK_SIZE,
                              func,
                              data,
                              0,    // flags (0/CREATE_SUSPENDED)
                              NULL  // thread id (not used)
                            );    

    if (h == INVALID_HANDLE_VALUE) FAIL( "CreateThread", GetLastError() );

    if (prio!= 0) {
        int win_prio= (prio == +3) ? THREAD_PRIORITY_TIME_CRITICAL :
                      (prio == +2) ? THREAD_PRIORITY_HIGHEST :
                      (prio == +1) ? THREAD_PRIORITY_ABOVE_NORMAL :
                      (prio == -1) ? THREAD_PRIORITY_BELOW_NORMAL :
                      (prio == -2) ? THREAD_PRIORITY_LOWEST :
                                     THREAD_PRIORITY_IDLE;  // -3

        if (!SetThreadPriority( h, win_prio )) 
            FAIL( "SetThreadPriority", GetLastError() );
    }
    *ref= h;
  }
  //
  void THREAD_FREE( THREAD_T *ref ) { *(ref)= NULL; }
  //
  bool_t THREAD_WAIT( THREAD_T *ref, long ms ) {
    DWORD rc= WaitForSingleObject( *ref, ms<0 ? INFINITE:ms /*timeout*/ );
        //
        // (WAIT_ABANDONED)
        // WAIT_OBJECT_0    success (0)
        // WAIT_TIMEOUT
        // WAIT_FAILED      more info via GetLastError()
    
    if (rc == WAIT_TIMEOUT) return FALSE;
    if (rc != 0) FAIL( "WaitForSingleObject", rc );
    return TRUE;
  }
  //
  void THREAD_EXIT( THREAD_T *ignore ) { (void)ignore; ExitThread(0); }
  //
  void THREAD_KILL( THREAD_T *ref ) {
    if (!TerminateThread( *ref, 0 )) FAIL("TerminateThread", GetLastError());
  }
  //
  void SIGNAL_INIT( SIGNAL_T *ref ) {
    HANDLE h= CreateEvent( NULL,    // security attributes
                           FALSE,   // TRUE: requires manual reset (ResetEvent())
                                    // FALSE: autoreset (after single waiting thread has been released)
                           FALSE,   // Initial state
                           NULL );  // name

    if (h == NULL) FAIL( "CreateEvent", GetLastError() );
    *ref= h;
  }
  void SIGNAL_FREE( SIGNAL_T *ref ) {
    if (!CloseHandle(*ref)) FAIL( "CloseHandle (event)", GetLastError() );
    *ref= NULL;
  }
  bool_t SIGNAL_WAIT( SIGNAL_T *ref, LOCK_T *lock_ref, double secs ) {
    DWORD rc;
    long ms= secs<0.0 ? INFINITE : (long)((secs*1000.0) +0.5);    // smoothen floating point inaccuracy issues

    // Wait for the event to become "signalled", with lock opened.
    //
    LOCK_END(lock_ref);
    {
        // Multiple threads can end here, but since we're using autoreset mode, 
        // only one is let to pass, and event is placed back to "non-signalled".
        //
        rc= WaitForSingleObject( *ref, ms );
        
        // Unlike in PThreads, the wakeup & lock acquiry is not atomic.
        // What can happen is a new "signal" while we're stuck here, letting
        // a second thread come and compete for the lock with us. What this
        // causes in practise is "false wakeups" where a thread is woken, but
        // it won't find any data coming to it.
    }
    LOCK_START(lock_ref);

    if (rc==WAIT_TIMEOUT) return FALSE;
    if (rc!=0) FAIL( "WaitForSingleObject", rc );
    return TRUE;
  }
  void SIGNAL_SET( SIGNAL_T *ref ) {
    // Sets the event to "signalled", releasing a single thread that is waiting
    // for it (or remaining signalled until a thread comes to ask).
    //
    if (!SetEvent( *ref )) FAIL( "SetEvent", GetLastError() );
  }
#else
  // PThread (Linux, OS X, ...)
  //
  // On OS X, user processes seem to be able to change priorities.
  // On Linux, SCHED_RR and su privileges are required..  !-(
  //
  #include <errno.h>
  #include <sys/time.h>
  //
  static void _PT_FAIL( int rc, const char *name, const char *file, uint_t line ) {
    const char *why= (rc==EINVAL) ? "EINVAL" : 
                     (rc==EBUSY) ? "EBUSY" : 
                     (rc==EPERM) ? "EPERM" :
                     (rc==ENOMEM) ? "ENOMEM" :
                     //...
                     "";
    fprintf( stderr, "%s %d: %s failed, %d %s\n", file, line, name, rc, why );
    abort();
  }
  #define PT_CALL( call ) { int rc= call; if (rc!=0) _PT_FAIL( rc, #call, __FILE__, __LINE__ ); }
  //
  static void set_abstime( struct timespec *ts, unsigned long ms ) {
    //
    // Linux has 'clock_gettime()' that works directly with 'struct timespec'.
    // OS X does not.
    //
    struct timeval tv;
        // long    tv_sec;  // seconds since Jan. 1, 1970
        // long    tv_usec; // microseconds
    PT_CALL( gettimeofday( &tv, NULL ) );   // timezone not used any more, says 'man'

    // 'tv_nsec' is long, having a range at least 2147483648 (2.1 secs) so
    // we can overflow temporarily
    //
    ts->tv_sec = tv.tv_sec + (ms/1000);
    ts->tv_nsec = tv.tv_usec * 1000L + (ms%1000) * 1000000L;    // ms == 1000us == 1000000ns
    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L; ts->tv_sec++;
    }
  }
  //
  void SIGNAL_INIT( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_init(ref,NULL /*attr*/) );
    }
  void SIGNAL_FREE( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_destroy(ref) );
  }
  bool_t SIGNAL_WAIT( SIGNAL_T *ref, pthread_mutex_t *mu, double secs ) {
    if (secs<0.0) {
        PT_CALL( pthread_cond_wait( ref, mu ) );  // infinite
    } else {
        int rc;
        struct timespec ts;
        long ms= (long)((secs*1000.0) +0.5);    // smoothen floating point inaccuracy issues

        set_abstime( &ts, ms );

        rc= pthread_cond_timedwait( ref, mu, &ts );
            // 0: success (condition reached)
            // EINVAL: some parameter is invalid
            // ETIMEDOUT: timeout

        if (rc==ETIMEDOUT) return FALSE;
        if (rc) { _PT_FAIL( rc, "pthread_cond_timedwait()", __FILE__, __LINE__ ); }
    }
    return TRUE;
  }
  void SIGNAL_SET( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_signal(ref) );
  }
  //
  void THREAD_CREATE( THREAD_T* ref, 
                      THREAD_RETURN_T (*func)( void * ),
                      void *data, int prio /* -2..+2 */ ) {
    pthread_attr_t _a;
    pthread_attr_t *a= &_a;
    struct sched_param sp;

    PT_CALL( pthread_attr_init(a) );

    // We create a NON-JOINABLE thread. This is mainly due to the lack of
    // 'pthread_timedjoin()', but does offer other benefits (s.a. earlier 
    // freeing of the thread's resources).
    //
    PT_CALL( pthread_attr_setdetachstate(a,PTHREAD_CREATE_DETACHED) );

    // Use this to find a system's default stack size (DEBUG)
#if 0
  { size_t n; pthread_attr_getstacksize( a, &n );
    fprintf( stderr, "Getstack: %u\n", (unsigned int)n ); }
    	//  524288 on OS X
    	// 2097152 on Linux x86 (Ubuntu 7.04)
    	// 1048576 on FreeBSD 6.2 SMP i386
#endif

#if (defined _THREAD_STACK_SIZE) && (_THREAD_STACK_SIZE > 0)
    PT_CALL( pthread_attr_setstacksize( a, _THREAD_STACK_SIZE ) );
#endif
    
    bool_t normal= 
#if defined(PLATFORM_LINUX) && defined(LINUX_SCHED_RR)
        !sudo;          // with sudo, even normal thread must use SCHED_RR
#else
        prio == 0;      // create a default thread if
#endif
    if (!normal) {
        // NB: PThreads priority handling is about as twisty as one can get it
        //     (and then some). DON*T TRUST ANYTHING YOU READ ON THE NET!!!

        // "The specified scheduling parameters are only used if the scheduling
        //  parameter inheritance attribute is PTHREAD_EXPLICIT_SCHED."
        //
        PT_CALL( pthread_attr_setinheritsched( a, PTHREAD_EXPLICIT_SCHED ) );

        //---
        // "Select the scheduling policy for the thread: one of SCHED_OTHER 
        // (regular, non-real-time scheduling), SCHED_RR (real-time, 
        // round-robin) or SCHED_FIFO (real-time, first-in first-out)."
        //
        // "Using the RR policy ensures that all threads having the same
        // priority level will be scheduled equally, regardless of their activity."
        //
        // "For SCHED_FIFO and SCHED_RR, the only required member of the
        // sched_param structure is the priority sched_priority. For SCHED_OTHER,
        // the affected scheduling parameters are implementation-defined."
        //
        // "The priority of a thread is specified as a delta which is added to 
        // the priority of the process."
        //
        // ".. priority is an integer value, in the range from 1 to 127. 
        //  1 is the least-favored priority, 127 is the most-favored."
        //
        // "Priority level 0 cannot be used: it is reserved for the system."
        //
        // "When you use specify a priority of -99 in a call to 
        // pthread_setschedparam(), the priority of the target thread is 
        // lowered to the lowest possible value."
        //
        // ...

        // ** CONCLUSION **
        //
        // PThread priorities are _hugely_ system specific, and we need at
        // least OS specific settings. Hopefully, Linuxes and OS X versions
        // are uniform enough, among each other...
        //
#ifdef PLATFORM_OSX
        // AK 10-Apr-07 (OS X PowerPC 10.4.9):
        //
        // With SCHED_RR, 26 seems to be the "normal" priority, where setting
        // it does not seem to affect the order of threads processed.
        //
        // With SCHED_OTHER, the range 25..32 is normal (maybe the same 26,
        // but the difference is not so clear with OTHER).
        //
        // 'sched_get_priority_min()' and '..max()' give 15, 47 as the 
        // priority limits. This could imply, user mode applications won't
        // be able to use values outside of that range.
        //
        #define _PRIO_MODE SCHED_OTHER
        
        // OS X 10.4.9 (PowerPC) gives ENOTSUP for process scope
        //#define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS

        #define _PRIO_HI  32    // seems to work (_carefully_ picked!)
        #define _PRIO_0   26    // detected
        #define _PRIO_LO   1    // seems to work (tested)

#elif defined(PLATFORM_LINUX)
        // (based on Ubuntu Linux 2.6.15 kernel)
        //
        // SCHED_OTHER is the default policy, but does not allow for priorities.
        // SCHED_RR allows priorities, all of which (1..99) are higher than
        // a thread with SCHED_OTHER policy.
        //
        // <http://kerneltrap.org/node/6080>
        // <http://en.wikipedia.org/wiki/Native_POSIX_Thread_Library>
        // <http://www.net.in.tum.de/~gregor/docs/pthread-scheduling.html>
        //
        // Manuals suggest checking #ifdef _POSIX_THREAD_PRIORITY_SCHEDULING,
        // but even Ubuntu does not seem to define it.
        //
        #define _PRIO_MODE SCHED_RR
        
        // NTLP 2.5: only system scope allowed (being the basic reason why
        //           root privileges are required..)
        //#define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS

        #define _PRIO_HI 99
        #define _PRIO_0  50
        #define _PRIO_LO 1

#elif defined(PLATFORM_BSD)
        //
        // <http://www.net.in.tum.de/~gregor/docs/pthread-scheduling.html>
        //
        // "When control over the thread scheduling is desired, then FreeBSD
        //  with the libpthread implementation is by far the best choice .."
        //
        #define _PRIO_MODE SCHED_OTHER
        #define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS
        #define _PRIO_HI 31
        #define _PRIO_0  15
        #define _PRIO_LO 1
#else
        #error "Unknown OS: not implemented!"
#endif

#ifdef _PRIO_SCOPE
        PT_CALL( pthread_attr_setscope( a, _PRIO_SCOPE ) );
#endif
        PT_CALL( pthread_attr_setschedpolicy( a, _PRIO_MODE ) );

#define _PRIO_AN (_PRIO_0 + ((_PRIO_HI-_PRIO_0)/2) )
#define _PRIO_BN (_PRIO_LO + ((_PRIO_0-_PRIO_LO)/2) )

        sp.sched_priority= 
            (prio == +2) ? _PRIO_HI :
            (prio == +1) ? _PRIO_AN :
#if defined(PLATFORM_LINUX) && defined(LINUX_SCHED_RR)
            (prio == 0) ? _PRIO_0 :
#endif
            (prio == -1) ? _PRIO_BN : _PRIO_LO;

        PT_CALL( pthread_attr_setschedparam( a, &sp ) );
    }

    ref->done= FALSE;
    PT_CALL( pthread_mutex_init( &ref->mu, NULL ) );
    PT_CALL( pthread_mutex_lock( &ref->mu ) );
    PT_CALL( pthread_cond_init( &ref->cv, NULL ) );

    //---
    // Thread creation might fail, if ~1000 threads are already in use.
    //
    // Seems on OS X, _POSIX_THREAD_THREADS_MAX is some kind of system
    // thread limit (not userland thread). Actual limit for us is way higher.
    // PTHREAD_THREADS_MAX is not defined (even though man page refers to it!)
    //
# ifndef THREAD_CREATE_RETRIES_MAX
    // Don't bother with retries; a failure is a failure
    //
    { int rc= pthread_create( &ref->pt, a, func, data );
      if (rc) _PT_FAIL( rc, "pthread_create()", __FILE__, __LINE__ );
    }
# else
    // Wait slightly if thread creation has exchausted the system
    //
    { uint_t retries;
    for( retries=0; retries<THREAD_CREATE_RETRIES_MAX; retries++ ) {

        int rc= pthread_create( &ref->pt, a, func, data );
            //
            // OS X / Linux:
            //    EAGAIN: ".. lacked the necessary resources to create
            //             another thread, or the system-imposed limit on the
            //             total number of threads in a process 
            //             [PTHREAD_THREADS_MAX] would be exceeded."
            //    EINVAL: attr is invalid
            // Linux:
            //    EPERM: no rights for given parameters or scheduling (no sudo)
            //    ENOMEM: (known to fail with this code, too - not listed in man)
            
        if (rc==0) break;   // ok!

        // In practise, exhaustion seems to be coming from memory, not a
        // maximum number of threads. Keep tuning... ;)
        //
        if (rc==EAGAIN) {
//fprintf( stderr, "Looping (retries=%d) ", retries );    // DEBUG

            // Try again, later.  We can simply busy loop on the 'pthread_create'
            // but better to use yielding the thread, if the OS allows.

            // Yield is non-portable:
            //
            //    OS X 10.4.8/9 has pthread_yield_np()
            //    Linux 2.4   has pthread_yield() if _GNU_SOURCE is #defined
            //    FreeBSD 6.2 has pthread_yield()
            //    ...
            //
#  ifdef PLATFORM_OSX
            pthread_yield_np();
#  else
            pthread_yield();
#  endif
        } else {
            _PT_FAIL( rc, "pthread_create()", __FILE__, __LINE__ );
        }
    }
    }
# endif

    if (a) {
        PT_CALL( pthread_attr_destroy(a) );
    }
  }
  //
  void THREAD_FREE( THREAD_T *ref ) {
    PT_CALL( pthread_mutex_unlock( &ref->mu ) );
    PT_CALL( pthread_mutex_destroy( &ref->mu ) );
    PT_CALL( pthread_cond_destroy( &ref->cv ) );
  }
  //
  void THREAD_EXIT( THREAD_T *ref ) {
    ref->done= TRUE;
    PT_CALL( pthread_cond_signal( &ref->cv ) );    // wake up 'THREAD_WAIT'
    pthread_exit( 0 /*ignored*/ );  // no return
  }
  //
  // Returns TRUE for succesful wait, FALSE for timed out
  //
  bool_t THREAD_WAIT( THREAD_T *ref, long ms ) {

    if (ms!=0) {
        // Important that we set the absolute timeout here, so even if there's
        // multiple loops the caller gets what (s)he wants.
        //
        struct timespec ts;
        if (ms>0) set_abstime( &ts, ms );

        while( !ref->done ) {
            if (ms<0) {
                PT_CALL( pthread_cond_wait( &ref->cv, &ref->mu ) );
            } else {
                int rc= pthread_cond_timedwait( &ref->cv, &ref->mu, &ts );
                if (rc==ETIMEDOUT) break;
                if (rc!=0) _PT_FAIL( rc, "pthread_cond_timedwait", __FILE__, __LINE__ );
            }
        }
    }
    return ref->done;
  }    
  //
  void THREAD_KILL( THREAD_T *ref ) {
    pthread_cancel( (ref)->pt );
  }
#endif

/*
 * Memory allocator used for states created via '_glua_new()'
 *
 * Ref: <http://devworld.apple.com/documentation/Performance/Conceptual/ManagingMemory/Articles/MemoryAlloc.html>
 *
 * Note: OS X 'malloc_zone_realloc()' was tried, but did not give speed benefits
 *       (on 10.5.2).
 */
    // TBD: measurements, measurements!!! Do we get any speed benefits?
    //
#ifdef TBB_SCALABLE_ALLOC
    // Threading Building Blocks - scalable allocator
    //
#include "tbb/scalable_allocator.h"
 static void *alloc_f (void *ignore, void *ptr, size_t osize, size_t nsize) {
  (void)ignore;
  (void)osize;
  if (nsize == 0) {
    scalable_free(ptr);
    return NULL;
  }
  return scalable_realloc(ptr, nsize);
 }
#else
 static const lua_Alloc alloc_f= 0;
#endif


/*
* Returns millisecond timing (in seconds) for the current time.
*/
double now_secs(void) {

#if (defined PLATFORM_WIN32) || (defined PLATFORM_WINCE)
    /*
    * Windows FILETIME values are "100-nanosecond intervals since 
    * January 1, 1601 (UTC)" (MSDN). Well, we'd want Unix Epoch as
    * the offset and it seems, so would they:
    *
    * <http://msdn.microsoft.com/en-us/library/ms724928(VS.85).aspx>
    */
    SYSTEMTIME st;
    FILETIME ft;
    ULARGE_INTEGER uli;

#if 1
    // TBD: Move this calculation to be done just once, at initialization.
    //
    ULARGE_INTEGER uli_epoch;   // Jan 1st 1970 0:0:0

    st.wYear= 1970;
    st.wMonth= 1;   // Jan
    st.wDay= 1;
    st.wHour= st.wMinute= st.wSecond= st.wMilliseconds= 0;
    //
    st.wDayOfWeek= 0;  // ignored

    if (!SystemTimeToFileTime( &st, &ft ))
        FAIL( "SystemTimeToFileTime", GetLastError() );

    uli_epoch.LowPart= ft.dwLowDateTime;
    uli_epoch.HighPart= ft.dwHighDateTime;
#endif

    GetSystemTime( &st );	// current system date/time in UTC
    if (!SystemTimeToFileTime( &st, &ft ))
        FAIL( "SystemTimeToFileTime", GetLastError() );

    uli.LowPart= ft.dwLowDateTime;
    uli.HighPart= ft.dwHighDateTime;

// seem to be getting negative numbers from the old version, probably number conversion clipping, this fixes it and maintains MS resolution
    return ( (double) (LONGLONG) ( ( uli.QuadPart/( (LONGLONG) 10000) ) - ( uli_epoch.QuadPart/( (LONGLONG) 10000) ) ) ) / 1000.0;

#else
    struct timeval tv;
        // {
        //   time_t       tv_sec;   /* seconds since Jan. 1, 1970 */
        //   suseconds_t  tv_usec;  /* and microseconds */
        // };

    int rc= gettimeofday( &tv, NULL /*time zone not used any more (in Linux)*/ );
    assert( rc==0 );

    return ((double)tv.tv_sec) + ((tv.tv_usec)/1000) / 1000.0;
#endif
}


/*
* THREADING.H
*/
#ifndef THREADING_H
#define THREADING_H

/* Platform detection
*/
#ifdef _WIN32_WCE
  #define PLATFORM_POCKETPC
#elif (defined _WIN32)
  #define PLATFORM_WIN32
#elif (defined __linux__)
  #define PLATFORM_LINUX
#elif (defined __APPLE__) && (defined __MACH__)
  #define PLATFORM_OSX
#elif (defined __NetBSD__) || (defined __FreeBSD__) || (defined BSD)
  #define PLATFORM_BSD
#elif (defined __QNX__)
  #define PLATFORM_QNX
#elif 0
  //..more platforms here..
#else
  #error "Unknown platform!"
#endif

typedef int bool_t;
#ifndef FALSE
# define FALSE 0
# define TRUE 1
#endif

typedef unsigned int uint_t;


/*---=== Generic ===---
*/
typedef double time_d;  // ms since Epoch
time_d now_secs(void);


/*---=== Locks & Signals ===---
*/

#if (defined PLATFORM_WIN32) || (defined PLATFORM_WINCE)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <process.h>

  // MSDN: http://msdn2.microsoft.com/en-us/library/ms684254.aspx
  //
  #define LOCK_T            CRITICAL_SECTION
  #define LOCK_INIT(ref)    InitializeCriticalSection(ref)
  #define LOCK_RECURSIVE_INIT(ref)  LOCK_INIT(ref)  /* always recursive in Win32 */
  #define LOCK_FREE(ref)    /* nothing */
  #define LOCK_START(ref)   EnterCriticalSection(ref)
  #define LOCK_END(ref)     LeaveCriticalSection(ref)

  typedef unsigned THREAD_RETURN_T;

  #define SIGNAL_T HANDLE
#else
  // PThread (Linux, OS X, ...)
  //
  #include <pthread.h>

  #ifdef PLATFORM_LINUX
  # define _MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
  #else
    /* OS X */
  # define _MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE
  #endif

  #define LOCK_T            pthread_mutex_t
  #define LOCK_INIT(ref)    pthread_mutex_init(ref,NULL)
  #define LOCK_RECURSIVE_INIT(ref) \
      { pthread_mutexattr_t a; pthread_mutexattr_init( &a ); \
        pthread_mutexattr_settype( &a, _MUTEX_RECURSIVE ); \
        pthread_mutex_init(ref,&a); pthread_mutexattr_destroy( &a ); \
      }
  #define LOCK_FREE(ref)    pthread_mutex_destroy(ref)
  #define LOCK_START(ref)   pthread_mutex_lock(ref)
  #define LOCK_END(ref)     pthread_mutex_unlock(ref)

  typedef void * THREAD_RETURN_T;

  typedef pthread_cond_t SIGNAL_T;
#endif

void SIGNAL_INIT( SIGNAL_T *ref );
void SIGNAL_FREE( SIGNAL_T *ref );
void SIGNAL_SET( SIGNAL_T *ref );
bool_t SIGNAL_WAIT( SIGNAL_T *ref, LOCK_T *mu, double secs );


/*---=== Threading ===---
*/

#if (defined PLATFORM_WIN32) || (defined PLATFORM_WINCE)

  typedef HANDLE THREAD_T;
  //
  void
  THREAD_CREATE( THREAD_T *ref,
                 THREAD_RETURN_T (__stdcall *func)( void * ),
                 void *data, int prio /* -3..+3 */ );
                 
# define THREAD_PRIO_MIN (-3)
# define THREAD_PRIO_MAX (+3)

#else
  // We cannot use 'pthread_t' directly, due to making a manual 'join'
  //
  typedef struct {
        pthread_t pt;
        pthread_mutex_t mu;     // mainly to keep pthread happy (keep locked)
        pthread_cond_t cv;      // signalled when thread has finished
        volatile bool_t done;   // we need some 'value' to change (to guard
                                // against false wakeups); could use 's->state'
                                // but we don't have 's_lane' here.
        } THREAD_T;

  void THREAD_CREATE( THREAD_T *ref, 
                      THREAD_RETURN_T (*func)( void * ),
                      void *data, int prio /* -2..+2 */ );
                      
# ifdef PLATFORM_LINUX
  volatile bool_t sudo;
#  ifdef LINUX_SCHED_RR
#   define THREAD_PRIO_MIN (sudo ? -2 : 0)
#  else
#   define THREAD_PRIO_MIN (0)
#  endif
# define THREAD_PRIO_MAX (sudo ? +2 : 0)
# else
#  define THREAD_PRIO_MIN (-2)
#  define THREAD_PRIO_MAX (+2)
# endif
#endif

void THREAD_FREE( THREAD_T *ref );
bool_t THREAD_WAIT( THREAD_T *ref, long ms );
void THREAD_EXIT( THREAD_T *ref );
void THREAD_KILL( THREAD_T *ref );

#endif
    // THREADING_H

/* IMLIB - jas_config.h for IM */

/* Avoid problems due to multiple inclusion. */
#ifndef JAS_CONFIG_H
#define JAS_CONFIG_H

/* This preprocessor symbol identifies the version of JasPer. */
#define	JAS_VERSION "1.900.1"

#define HAVE_FCNTL_H     1
#define HAVE_LIMITS_H    1
#define HAVE_STDLIB_H    1
#define HAVE_STDDEF_H    1
#define HAVE_STRING_H    1
#define HAVE_MEMORY_H    1
#define HAVE_SYS_TYPES_H 1

/* #define HAVE_UNISTD_H 1 (must control this in the makefile) */

/* #define JAS_CONFIGURE 0  to include some definitions in "jas_types.h" ifdef WIN32 */

#ifdef JAS_TYPES
typedef unsigned long ulong;
typedef unsigned char uchar;
#endif

#if !defined(WIN32)     /* These will be defined in "jas_types.h" ifdef WIN32 */
#define longlong long long
#define ulonglong unsigned long long
#endif

#endif

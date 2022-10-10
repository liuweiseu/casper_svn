/****************************************************
 * edt_threads.h
 * defines to try and have OS neutral thread code
 * plus some miscellaneous cross-defines
 ****************************************************/

#if !defined(EDT_THREADS_H)
#define EDT_THREADS_H


#ifndef NULL

#define NULL (0)

#endif


/* for using a dll */

#define DLLEXPORT __declspec(dllexport)

/*  OS-Specific defines */
/****************************************************/
/*  Windows                                         */
/****************************************************/

#ifdef WIN32

#include <process.h>

#ifndef ASSERT

#include <assert.h>

#ifdef _DEBUG

#define ASSERT(f) assert(f)

#else

#define ASSERT(f) 

#endif

#endif

#ifndef TRACE


#ifdef _DEBUG

#include <windows.h>

#define TRACE printf

#else

#define TRACE

#endif

#endif

#define LaunchThread(pThread,func,pointer) { \
	unsigned int thrdaddr; \
    pThread = (thread_t) _beginthreadex(NULL, 0, func, pointer, CREATE_SUSPENDED, &thrdaddr);\
	ResumeThread(pThread); }

#define WaitForThread(pThread,timeout) \
	WaitForSingleObject(pThread,timeout)


typedef unsigned int  (__stdcall *edt_thread_function)(void *);

typedef thread_t thread_type;

#define THREAD_RETURN UINT
#define THREAD_FUNC_DECLARE  UINT __stdcall 
#else

/****************************************************/
/*  Unix                                            */
/****************************************************/

#include <stdlib.h>

#include <pthread.h>
#include <sys/user.h>

typedef pthread_t thread_type;

#define THREAD_RETURN void *
#define THREAD_FUNC_MODIFIER
#define THREAD_FUNC_DECLARE THREAD_RETURN
typedef void * (*edt_thread_function)(void *);

#ifdef __sun


#define mutable 

#include <sys/param.h>
#include <limits.h>

#define PAGE_SIZE PAGESIZE

#endif


#define ASSERT(f) ((void)0)

/* utility helpers */

#ifndef TRUE

#define TRUE (1)
#define FALSE (0)

#endif

/* These show up in code written for windows */

typedef unsigned char BOOL;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long ULONG;


#ifndef max

#define max(a,b) ((a > b) ? (a):(b))

#endif

#ifndef min
#define min(a,b) ((a < b) ? (a):(b))
#endif

#define INFINITE 0

#define SECTOR_SIZE PAGE_SIZE

#define _MAX_PATH _POSIX_PATH_MAX


#if defined(__sun)


#define LaunchThread(pThread,func,pointer) \
               thr_create(NULL,0,func, pointer, THR_NEW_LWP|THR_BOUND, &pThread)

#define WaitForThread(pThread,timeout) \
	 thr_join(pThread, NULL,NULL)

#define CRITICAL_SECTION mutex_t

#define EnterCriticalSection(lock) mutex_lock(lock)
#define LeaveCriticalSection(lock) mutex_unlock(lock)
#define DeleteCriticalSection(lock) mutex_destroy(lock)
#define InitializeCriticalSection(lock) mutex_init(lock, USYNC_THREAD, NULL)

#define THREAD_RETURN void *

#else


#define LaunchThread(pThread,func,pointer) \
               pthread_create(&pThread,NULL,func,pointer);

#define WaitForThread(pThread,timeout) \
{ int return; pthread_join(pThread, &return;}

#define CRITICAL_SECTION pthread_mutex_t

#define EnterCriticalSection(lock) pthread_mutex_lock(lock)
#define LeaveCriticalSection(lock) pthread_mutex_unlock(lock)
#define DeleteCriticalSection(lock) pthread_mutex_destroy(lock)
#define InitializeCriticalSection(lock) pthread_mutex_init(lock, NULL)


#endif


#ifdef _DEBUG

#define TRACE printf

#else

#define TRACE

#endif


#endif

#endif

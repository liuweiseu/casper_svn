
#ifndef _EDT_OS_LNX_H_
#define _EDT_OS_LNX_H_


#define HANDLE int


typedef unsigned int uint_t;
typedef unsigned char uchar_t;
typedef unsigned short ushort_t;

#ifndef __KERNEL__

#include <sys/types.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <errno.h>

#include <linux/ioctl.h>

#include <pthread.h>

typedef pthread_t thread_t;

#else

typedef HANDLE thread_t;

#endif

#endif


/*
 * file: mem2mir.h
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-11-15
 */

#ifndef _MEM2MIR_H_
#define _MEM2MIR_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug_macros.h"
#include "integration_buffer.h"
#include "libc2m/c2m.h"

/*
 * Function Declarations
 */

void corner_turn_and_cast(INT_BUF_METADATA *metadata, INTEGRATION_ITEM *integration, double *matrix);

void cleanup(int signal);

#endif /* _MEM2MIR_H_ */

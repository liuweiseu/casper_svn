/*
 * file: net2mem.h
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-04-02
 */

#ifndef _NET2MEM_H_
#define _NET2MEM_H_

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

#include "caspern_packet.h"
#include "debug_macros.h"
#include "integration_buffer.h"
#include "ring_buffer.h"
#include "libc2m/c2m.h"

#define TIMESTAMP_TRUNC 8   // in bits

/*
 * Useful typedefs
 */

typedef int socket_t;
typedef struct sockaddr_in SA_in;
typedef struct sockaddr SA;

/*
 * Structure Definitions
 */

typedef struct
{
	int n_ants; // specified number of antennas in the system
	int n_xeng; // specified number of X engines in the system
	int n_chans; // specified number of channels in the system
	int n_stokes; // specified number of stokes parameters
	int n_bls; // calculated number of baselines

	size_t vecs_per_int; // number of vectors per integration
	size_t vecs_per_xeng; // number of vectors per X engine
	size_t vec_size; // size of a single data vector (in bytes)

	size_t chans_per_xeng;
	size_t vecs_per_chan;
	size_t table_offset;

	int rx_buffer_size;
	int int_buffer_size;
	int max_payload_len;

	int port;
} CORRELATOR_PARAMS;

typedef struct {
	RING_BUFFER *pkt_buffer;
} NET_THREAD_ARGS;

typedef struct {
	RING_BUFFER *pkt_buffer;
	INTEGRATION_BUFFER *int_buffer;
} ASM_THREAD_ARGS;

/*
 * Function Declarations
 */

void *network_thread_function(void *arg);
socket_t setup_network_listener();

void *assemble_thread_function(void *arg);
void assemble_integration(RING_ITEM *cur_pkt, INTEGRATION_BUFFER *int_buf);

void calculate_integration_parameters();

void cleanup(int signal);

/*
 * Global Variables
 */

static CORRELATOR_PARAMS corr;

#endif /* _NET2MEM_H_ */

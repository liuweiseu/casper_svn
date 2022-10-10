#ifndef _UDP_FUNCTIONS_H_
#define _UDP_FUNCTIONS_H_

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PAYLOADSIZE 4096
#define NUM 1000000 
#define RINGBUFFSIZE 4
#define RECVPORT 33001

#define BEE_CONFIG_RECV_PORT 31337
#define BEE_CONFIG_PACKET_SIZE 512

#define RECORD_SIZE_IN_BYTES 8
#define HEADER_SIZE_IN_RECORDS (2+64)
#define HEADER_SIZE_IN_BYTES HEADER_SIZE_IN_RECORDS*RECORD_SIZE_IN_BYTES


typedef struct packet_buffer{
    void *data;
    int full;
    pthread_mutex_t buffer_mutex;
} packet_buffer;

typedef struct spectra_buffer{
    void *data;
    unsigned int buffer_size;
    int full;
    pthread_cond_t buffer_filled_cond;
    pthread_mutex_t buffer_mutex;
    pthread_mutex_t sizeofbuf_mutex;
    pthread_mutex_t full_mutex;
} spectra_buffer;

struct data_vals{
    unsigned int raw_data;
    unsigned int overflow_cntr;
};

typedef struct udp_data{
    int sockfd;
    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr; 
    socklen_t addr_len;
} udp_data;

void initialize_buffers();
void close_buffers();

int udp_open();
int udp_close();

void *packet_buf( void *num );
void *fill_buf( void *num );
void *print_buf( void *num );
void *get_beam( void *num );
void *get_bee_config( void *num );

void launch_buf_threads();
void join_buf_threads();

extern int run_print_buf;
extern int run_fill_buf;
extern int run_packet_buf;
extern int run_get_beam;
extern int run_get_bee_config;

extern unsigned long total_spectra;

extern unsigned long int beam_number;
extern pthread_mutex_t beam_number_mutex;

#endif /*_UDP_FUNCTIONS_H_*/

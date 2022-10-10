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

#include <signal.h>

#include "udp_functions.h"

static void cleanup(int signal);

int main()
{
    int *val = 0;
    struct sigaction newact;
    pthread_t print_buf_thread;
    
    //set up the signal handler
    newact.sa_handler = cleanup;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    
    //start listening for Ctrl-C
    sigaction(SIGINT, &newact, NULL);
    
    
    initialize_buffers();

    udp_open();

    launch_buf_threads();
    pthread_create( &print_buf_thread, NULL, print_buf, (void *)val);
    
    pthread_join( print_buf_thread, NULL );

    join_buf_threads();
    udp_close();
    close_buffers();
    
    return 0;
}

static void cleanup(int signal)
{
    fprintf(stderr, "Ctrl-C received... cleaning up\n");
    run_print_buf=0;
}

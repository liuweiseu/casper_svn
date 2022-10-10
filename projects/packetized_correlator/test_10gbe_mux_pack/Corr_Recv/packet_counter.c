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

#include "debug_macros.h"
#include "corr_packet.h"

#define MYPORT 8888                 //the port users will be connecting to
#define MAXBUFLEN 9020              //maximum packet size
#define INTEGRATION_BUFFER_LEN 4    //the number of integrations to hold in a circular queue
#define TIME_RANGE 10               //if current time falls within integration_counter +/- TIME_RANGE, considered same integration

#define ERRSOCKFD "ERR Unable to set socket descriptor\n"
#define ERRSOCKBND "ERR Unable to bind to socket\n"
#define ERRRX "ERR Unable to receive packet\n"

typedef struct{
    uint64_t time;
    uint32_t x_engine;    
    int data_counter;
} integration_counter;

int main(int argc, char ** argv)
{
    //loop variables
    int i;
    
    //variable definitions to initialize socket and receive packets
    int sockfd;
    struct sockaddr_in my_addr;    // server's address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;  
    int num_bytes;
    char buf[MAXBUFLEN];
    
    //variable definitions for correlator size
    int n_ants = 8;         //specified number of antennas in the system
    int n_xeng = 8;         //specified number of x engines in the system (usually equal to n_ants)
    int n_chans = 2048;     //specified number of channels in the system
    int n_stokes = 4;       //specified number of stokes parameters
    int n_bls;               //calculated number of baselines
    int int_len;            //calculated data length per integration (in bytes)
    
    //variables for capturing packets and integrations
    corr_packet corr_pkt;
    integration_counter integration_buffer[INTEGRATION_BUFFER_LEN];
    int found_index=-1;
    int kill_ptr=0;
    for(i=0; i<INTEGRATION_BUFFER_LEN;i++)
    {
        integration_buffer[i].time = 0;
        integration_buffer[i].x_engine = -1;
    }
    
    n_bls = (n_ants * (n_ants+1))/2;
    int_len = (8)*n_stokes*n_bls*n_chans/n_xeng;
    debug_fprintf(stderr,"Number of antennas:%d. Number of baselines: %d. integration length:%d.\n", n_ants, n_bls, int_len);
    
    // obtain a socket descriptor
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {        
        perror(ERRSOCKFD);
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP  ... doesn't matter if we use hton on INASSR_ANY or not
    // since it's value is zero
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
    
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
        perror(ERRSOCKBND);
        exit(1);
    }
    
    debug_fprintf(stderr,"Listening on IP address %s on port %i\n",inet_ntoa(my_addr.sin_addr),MYPORT);
    
    int yes=1;
    
    // lose the pesky "Address already in use" error message
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    } 
    
    addr_len = sizeof their_addr;
    
    while (1) {
        // read a packet in, exit if there is an error reading
        if ((num_bytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror(ERRRX);
            exit(1);
        }
        
        //read in the packet header and data
        read_packet(corr_pkt,buf);
        
        found_index=-1;
        
        //search the integration queue for this xengine/timestamp
        for(i=0; i<INTEGRATION_BUFFER_LEN;i++)
        {
            if(corr_pkt.x_engine == integration_buffer[i].x_engine && 
               (corr_pkt.time < (integration_buffer[i].time+TIME_RANGE) && (corr_pkt.time > integration_buffer[i].time-TIME_RANGE)) )
            {
                found_index = i;
                break;
            }
        }
        
        //integration not found, replace the oldest one in the buffer with this one
        if(found_index == -1)
        {
            //write out the expired integration, check if kill_ptr is valid
            if(integration_buffer[kill_ptr].x_engine == -1)
            {
                printf("Integration expired x_engine = %u, timestamp = %llu, received %d bytes of data",
                       integration_buffer[kill_ptr].x_engine,integration_buffer[kill_ptr].time,integration_buffer[kill_ptr].data_counter);
            }
            integration_buffer[kill_ptr].x_engine = corr_pkt.x_engine;
            integration_buffer[kill_ptr].time = corr_pkt.time;
            integration_buffer[kill_ptr].data_counter = corr_pkt.data_length;
                
            kill_ptr = (kill_ptr+1) % MAXBUFLEN;
        }
        else
        {
            integration_buffer[found_index].data_counter += corr_pkt.data_length;
        }
        
    }
}




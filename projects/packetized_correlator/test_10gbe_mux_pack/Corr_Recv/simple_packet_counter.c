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


#define MYPORT 8888                 //the port users will be connecting to
#define MAXBUFLEN 9500              //maximum packet size

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))

#define ERRSOCKFD "ERR Unable to set socket descriptor\n"
#define ERRSOCKBND "ERR Unable to bind to socket\n"
#define ERRRX "ERR Unable to receive packet\n"


int main(int argc, char ** argv)
{
    //variable definitions to initialize socket and receive packets
    int sockfd;
    struct sockaddr_in my_addr;    // server's address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t addr_len;  
    int num_bytes;
    char buf[MAXBUFLEN];
    uint64_t next=0;
    int received = 0;
    uint dropped = 0;
    uint packet_size=1000;
    uint packets_to_receive=1000000;
    char c;
    
    //read in command line args
    while((c = getopt(argc, argv, "p:s:h")) != -1){
        switch(c) {
            case 'p':   
                packets_to_receive=atoi(optarg);
                //debug_fprintf(stderr,"Command line arg packet number: %d\n", fpga_number);
                break;
            case 's':   
                packet_size=atoi(optarg);
                //debug_fprintf(stderr,"Command line arg FPGA number: %d\n", fpga_number);
                break;
            //display help
            case 'h':  
                printf("Usage: %s [-s packet size] [-p number of packets to receive]\n", argv[0]);
                return 0;
        } // end switch option
    } //end while(get options)
    
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
    
    fprintf(stderr,"Listening on IP address %s on port %i\n",inet_ntoa(my_addr.sin_addr),MYPORT);
    fprintf(stderr,"Expecting %i-byte packets. Stop after %i packets.\n",packet_size,packets_to_receive);
    
    int yes=1;
    
    // lose the pesky "Address already in use" error message
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    } 
    
    addr_len = sizeof their_addr;
    
    while (next<(packets_to_receive*packet_size)) {
        // read a packet in, exit if there is an error reading
        if ((num_bytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror(ERRRX);
            exit(1);
        }
        received++;
        
        //check if this packet comes after the last packet received
        if( ntohll(*((uint64_t *) buf)) != (next) )
        {
            dropped+=(ntohll(*((uint64_t *) buf))-(next))/packet_size;
            fprintf(stderr,"Packet beginning with %llu dropped, new packet begins with %llu\n",next,ntohll(*((uint64_t *) buf)));
            fprintf(stderr,"Dropped %llu elements\n",ntohll(*((uint64_t *) buf))-(next));
            fprintf(stderr,"%d packets received %d drops detected\n",received, dropped);
        }
        next = ntohll(*((uint64_t *) (buf+num_bytes-8)))+1;
    }
    fprintf(stderr,"%d packets received %d packets dropped\n",received, dropped);
    return 0;
    
}




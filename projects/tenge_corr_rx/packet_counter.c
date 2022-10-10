/* file: packet_counter.c
 * auth: Billy Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-04-11
 * desc: Counts packets.
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RECV_PORT 8888
#define DEFAULT_PACKET_SIZE 8192 // in bytes
#define DEFAULT_PACKETS_TO_RECEIVE 1000000 // in bytes

//#define MYPORT 8888                 //the port users will be connecting to
//#define MAXBUFLEN 9500              //maximum packet size

typedef int socket_t;
typedef struct sockaddr_in SA_in;
typedef struct sockaddr SA;
typedef unsigned char U8;
typedef unsigned int U32;
typedef unsigned long long int U64;

#define htonll(x) ((((uint64_t)htonl(x)) << 32) | htonl(x >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) | ntohl(x >> 32))

#define ERRSOCKFD "ERR Unable to set socket descriptor\n"
#define ERRSOCKBND "ERR Unable to bind to socket\n"
#define ERRRX "ERR Unable to receive packet\n"

socket_t setupNetworkListener();

int main(int argc, char ** argv)
{
    socket_t sock = -1;
    int num_bytes = 0;
    U8 *buffer = NULL;
    
    // set default parameters
    int packets_to_receive = DEFAULT_PACKETS_TO_RECEIVE;
    int packet_size = DEFAULT_PACKET_SIZE;
    
    // parse command line args, overriding defaults as needed
    char c;
    while((c = getopt(argc, argv, "p:s:h")) != -1)
    {
        switch(c)
        {
            case 'p':   
                packets_to_receive = atoi(optarg);
                //debug_fprintf(stderr,"Command line arg packet number: %d\n", fpga_number);
                break;
            case 's':   
                packet_size = atoi(optarg);
                //debug_fprintf(stderr,"Command line arg FPGA number: %d\n", fpga_number);
                break;
            //display help
            case 'h':  
                printf("Usage: %s [-p num_packets] [-s packet_size]\n", argv[0]);
                return 0;
        } // end switch option
    } //end while(get options)

    // initialize network variables
    sock = setupNetworkListener();
    buffer = (U8*)calloc(packet_size, sizeof(U8));
    
    // initialize loop variables
    uint64_t last_element_received = 0;
    int total_received_packets = 0;
    int total_dropped_packets = 0;

    // count packets!
    while (total_received_packets < packets_to_receive)
    {
        // receive next packet
    	num_bytes = recv(sock, buffer, packet_size, 0);
    	
    	// verify successful receipt
        if (num_bytes == -1)
        {
            perror(ERRRX);
            exit(1);
        }
    	
    	// increment packet counter
        total_received_packets++;

        // calculate useful statistics
        U64 received_element = ntohll(*((U64 *)buffer));
        U64 expected_element = last_element_received + 1;
        U64 dropped_elements = expected_element - received_element;
        U64 dropped_packets = dropped_elements / packet_size;
        
        // check if this packet comes after the last packet received
        if (dropped_elements != 0)
        {
            total_dropped_packets += dropped_packets;
            fprintf(stderr, "Packet expected: %llu\n", expected_element);
            fprintf(stderr, "Packet received: %llu\n", received_element);
            fprintf(stderr, "Dropped: %llu elements\n", dropped_elements);
            fprintf(stderr, "         %llu packets\n", dropped_packets);
            fprintf(stderr, "Total packets received: %d\n", total_received_packets);
            fprintf(stderr, "Total packets dropped:  %d\n", total_dropped_packets);
            fprintf(stderr, "\n");
        }

        // store this element as the new last element
        last_element_received = ntohll(*((uint64_t *) (buffer + num_bytes - 8)));
    }

    fprintf(stderr,"%d packets received %d packets dropped\n", total_received_packets, total_dropped_packets);

    return 0;
}

/*
 *  Bind to a port and listen for incoming data.
 */
int setupNetworkListener()
{
	int sock = -1;
	struct sockaddr_in my_addr;    // server's address information
	int ret = 0;

	// create a new UDP socket descriptor
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror(ERRSOCKFD);
		exit(1);
	}

	// initialize local address struct
	my_addr.sin_family = AF_INET;                // host byte order
	my_addr.sin_port = htons(RECV_PORT);         // short, network byte order
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP
	memset(my_addr.sin_zero, 0, sizeof(my_addr.sin_zero));

	// bind socket to local address
	ret = bind(sock, (SA *)&my_addr, sizeof(my_addr)); 
	if (ret == -1)
	{
		perror(ERRSOCKBND);
		exit(1);
	}

	// prevent EADDRINUSE on subsequent calls to bind()
	const int on = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
	if (ret == -1)
	{
		perror("setsockopt");
		exit(1);
	} 

	fprintf(stderr, "Listening on IP address %s on port %i\n", inet_ntoa(my_addr.sin_addr), RECV_PORT);

	return sock;
}

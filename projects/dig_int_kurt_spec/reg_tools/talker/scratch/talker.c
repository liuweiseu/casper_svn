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

#include "borph.h"

#define RECV_IP "192.168.1.209"
#define SERVERPORT "4950"	// the port users will be connecting to

int main()
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	int value0;
	int value1;
	int value2;
	int value3;
	int value4;
	int value5;
	char *value6;

	// open files (registers) to be read
	
	FILE *fd;
	char info[512];

	// read registers
	
	char fft_proc[50] = "23258";
	char pfb_proc[50] = "23270";
	char thr_proc[50] = "23256";

	//char fft_proc[50] = argv[1];
	//char pfb_proc[50] = argv[2];
	//char thr_proc[50] = argv[3];

	// concat all registers into info buffer
	
	value0 = read_addr(fft_proc,"fft_shift",fd);
	value1 = read_addr(pfb_proc,"fft_shift",fd);
	value2 = read_addr(thr_proc,"thr_comp1_thr_lim",fd);
	value3 = read_addr(thr_proc,"thr_scale_p1_scale",fd);
	value4 = read_addr(thr_proc,"rec_reg_10GbE_destport0",fd);
	value5 = read_addr(thr_proc,"rec_reg_ip",fd);
	value6 = timeo();
	
	// final buffer to be sent

    sprintf(info,"BEE TIME: %s\nPFB SHIFT: %d\nFFT SHIFT: %d\nTHRESH LIMIT: %d\nTHRESH SCALE: %d\nTENGE PORT: %d\nTENGEIP: %d\n",value6,value0,value1,value2,value3,value4,value5);

	// send data 	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(RECV_IP, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, info, strlen(info), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, RECV_IP);
	close(sockfd);

	return 0;
}

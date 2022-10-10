// seti_send.c

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

#define RECVPORT 4950	// the port users will be connecting to
#define RECV_IP "128.32.62.213"

int main(void)
{
	int sockfd;
	struct sockaddr_in recv_addr; // connector's address information
	struct hostent *he;
	int numbytes;

	int *num,x;
	num = &x;
	*num = 5;
			

	if ((he=gethostbyname(RECV_IP)) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	recv_addr.sin_family = AF_INET;	 // host byte order
	recv_addr.sin_port = htons(RECVPORT); // short, network byte order
	recv_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(recv_addr.sin_zero, '\0', sizeof recv_addr.sin_zero);


	if ((numbytes = sendto(sockfd, num, sizeof (int), 0,
			 (struct sockaddr *)&recv_addr, sizeof recv_addr)) == -1) {
		perror("sendto");
		exit(1);
	}

	printf("sent %d bytes to %s\n", numbytes, inet_ntoa(recv_addr.sin_addr));
	printf("value is %d\n",*num);
	close(sockfd);

	return 0;
}





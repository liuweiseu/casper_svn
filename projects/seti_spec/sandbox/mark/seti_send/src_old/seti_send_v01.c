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

#define RECVPORT 33000	// the port users will be connecting to
#define RECV_IP "192.168.0.1"
#define PAYLOADSIZE 1024
#define DATAFILESIZE 2048
#define U32 unsigned int


int main(int argc,char *argv[])
{

    FILE *fifo_file;
    char fifo[200]; signed int fifo_buffer;size_t fifo_bytesread;
	int numbytes;
	int i,j,z;
	int num_vals = PAYLOADSIZE/4;
	int num_payloads = DATAFILESIZE/PAYLOADSIZE;

	U32 *ptr,*ptr2;
	U32 *buf = (U32 *)malloc(DATAFILESIZE);
	ptr = buf;
	ptr2 = buf;
	int sockfd;
	struct sockaddr_in recv_addr; // connector's address information
	struct hostent *he;

	int *num,x;
	num = &x;
	*num = 5;
			
    sprintf(fifo,"%s/%s",argv[1],argv[2]);
    fifo_file=fopen(fifo,"rb"); 

    if(fifo_file == NULL)
    {
        printf("Error: couldn't openfile '%s'\n",fifo);
        return 1;
    }

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

//  start 

	for(j=0; j< num_payloads;j++)
	{
		for(i=0;i< num_vals; i++)
		{
            fifo_bytesread = fread(&fifo_buffer,sizeof(int),1,fifo_file);
            if(fifo_bytesread != 0)
            {
			    z=fifo_buffer;
			    memcpy((void *)ptr,(const void *)&z,sizeof(int));
			    ptr++;
            }
		}
	}

	if ((numbytes = sendto(sockfd,buf ,PAYLOADSIZE, 0,
			 (struct sockaddr *)&recv_addr, sizeof recv_addr)) == -1) {
		perror("sendto");
		exit(1);
	}

//    FILE *fp;
//	fp=fopen("test.bin", "wb");
//	U32 a=5;
//	fwrite(buf,1024,1,fp);
//	fclose(fp);

	printf("sent %d bytes to %s\n", numbytes, inet_ntoa(recv_addr.sin_addr));
	close(sockfd);

	return 0;
}





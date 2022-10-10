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
#define RECV_IP "192.168.0.101"
#define PAYLOADSIZE 1024
#define DATAFILESIZE 2048
#define U32 unsigned int 
#define U8 unsigned char

int main(int argc,char *argv[])
{
    if(argv[1] == NULL)
    {
	printf("Usage: seti_send </path/to/fifo/dir> <fifofile>");
    }
    else
    {
	FILE *fifo_file;
	char fifo[200];size_t fifo_bytesread;

	void *fifo_buffer=(void *)malloc(16);
//	void *z=(void *)malloc(16);
	int numbytes;
	int i;
//	int j;
	int num_vals = PAYLOADSIZE/16;
//	int num_payloads = DATAFILESIZE/PAYLOADSIZE;

	void *ptr=0; 
///	void *ptr2=0; 
	unsigned char buf[DATAFILESIZE];
//	void *buf = (void *)malloc(DATAFILESIZE);

//	ptr = &buf;
//	ptr2 = &buf;

	int sockfd;
	struct sockaddr_in recv_addr; // connector's address information
	struct hostent *he;

	//    int *num,x;
	//   num = &x;
	//  *num = 5;
			
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
int j;
int z;
//	for(j=0; j< 5;j++)
	while(1)
	{   
	    z=0;
	    ptr=&buf;

	    for(i=0;i< num_vals; i++)
	    {
		fifo_bytesread = fread(fifo_buffer,4*sizeof(int),1,fifo_file);
		if(fifo_bytesread != 0)
		{
		    // printf("%x \n",ptr);
		    // z=fifo_buffer;
		    memcpy((void *)ptr,(const void *)fifo_buffer,4*sizeof(int));
		    /*
                    printf("raw:  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X\n",
                        *((U8 *)ptr+0), *((U8 *)ptr+1), *((U8 *)ptr+2), *((U8 *)ptr+3),
                        *((U8 *)ptr+4), *((U8 *)ptr+5), *((U8 *)ptr+6), *((U8 *)ptr+7),
                        *((U8 *)ptr+8), *((U8 *)ptr+9), *((U8 *)ptr+10), *((U8 *)ptr+11),
                        *((U8 *)ptr+12), *((U8 *)ptr+13), *((U8 *)ptr+14), *((U8 *)ptr+15));
			*/
		    ptr+=16;
		   // printf("%s\n",(char *)fifo_buffer);
		   z++;
		}
			 
	    }
	
	    if(z == num_vals)
	    {
		if ((numbytes = sendto(sockfd,(const void *)buf,PAYLOADSIZE, 0,
		    (struct sockaddr *)&recv_addr, sizeof recv_addr)) == -1) {
		    perror("sendto");
		    exit(1);
		}
	    }
//	printf("sent %d bytes to %s\n", numbytes, inet_ntoa(recv_addr.sin_addr));

	}
    
// uncomment to write buffer to a file
	FILE *fp;
	fp=fopen("test.bin", "wb");
	fwrite(buf,1024,1,fp);
	fflush(fp);
	fclose(fp);
//	

	close(sockfd);

	}

	return 0;
}





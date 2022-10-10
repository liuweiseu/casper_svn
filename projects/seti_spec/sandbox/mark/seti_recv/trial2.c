#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "setispec.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define NUM 100000000

int main()
{
    int pfb_bin_prev = 0;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    int pfb_fft_power = 0;
    long int i;
    int j = 0;
    void *buf1;
    long int *ptr1;
    int ctr = 0;
    FILE *datafile;
    FILE *cntfile;
    cntfile = fopen("test.dat","wb");
    U32 *buf = (U32 *)malloc(PAYLOADSIZE);

    //create 2 buffers
    buf1 = calloc(NUM,8);
    ptr1 = buf1;

    int sockfd;
    int numbytes;
    struct sockaddr_in recv_addr;       // recv address information
    struct sockaddr_in send_addr; // connector's address information
    struct data_vals *data_ptr;
    data_ptr = (struct data_vals *)buf;
    socklen_t addr_len;

    const char datafile_str[200];

 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    recv_addr.sin_family = AF_INET;              // host byte order
    recv_addr.sin_port = htons(RECVPORT);        // short, network byte order
    recv_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with recv IP
    memset(recv_addr.sin_zero, '\0', sizeof recv_addr.sin_zero);

    //bind socket file descriptor to local addr and port

    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof recv_addr) == -1)
    {
        perror("bind");
        exit(1);
    }

    addr_len = sizeof send_addr;

    while(1)
    {
	    if ((numbytes = recvfrom(sockfd, buf, PAYLOADSIZE , 1,
            (struct sockaddr *)&send_addr, &addr_len)) == -1)
            {
                perror("recvfrom");
                exit(1);
            }
    
	    data_ptr = (struct data_vals *)buf;

	    for(i=0;i<PAYLOADSIZE/8;i++)
	    {
		U32 fields = ntohl(data_ptr->raw_data);
		fft_bin = slice(fields,15,0);
		pfb_bin = slice(fields,12,15);
		over_thresh = slice(fields,1,27);
		blank = slice(fields,3,28);
		event = slice(fields,1,31);
		pfb_fft_power = ntohl(data_ptr->overflow_cntr); //32.0

		
		if(pfb_bin > 2047)
		{
		    pfb_bin = pfb_bin-2048;
		}
		else
		{
		    pfb_bin = pfb_bin+2048;
		}

		//correct for pfb bin being off
		if(fft_bin == 0)
		{
		    pfb_bin=pfb_bin-1;
		}

	    
		fprintf((FILE *)cntfile,"%d %d %d\n",pfb_bin,pfb_bin_prev,pfb_fft_power);

		if(pfb_bin < pfb_bin_prev)
		{
		    printf("===============\n");
		    sprintf((char *)datafile_str,(const char *)"datafiles/spectra/data%d.dat",j);  
		    datafile = fopen(datafile_str,"wb"); 
		     
		    fwrite((const void *)buf1,ctr*8,1,(FILE *)datafile);
		    printf("ctr %d\n",ctr*2);
		    fclose(datafile);
		    ptr1=buf1;
		    //header is the size of packet in bytes
		    //add header memcpy and advance ptr
		    printf("===============\n");
		    j++;	     
		    //add 'size' of header to ctr
		    ctr = 0;
		    fprintf((FILE *)cntfile,"a\n",pfb_bin);
	    
		}
		
		memcpy(ptr1,data_ptr,8);
		pfb_bin_prev = pfb_bin;
		data_ptr++;
		ctr++;		
		ptr1++;
	    }
	    	
    }
    fclose(cntfile);
    //free buffers 
    free(buf);
    free(buf1);
    close(sockfd);
    return 0;
}


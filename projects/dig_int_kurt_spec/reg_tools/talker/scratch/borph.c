#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "borph.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <time.h>


int read_addr(char *proc_number,char *shared_mem_name,FILE *shared_mem_fd)
{
    int value;
    char shared_mem_str[200];
    sprintf(shared_mem_str,"/proc/%s/hw/ioreg/%s",proc_number,shared_mem_name);

    shared_mem_fd = fopen(shared_mem_str,"rb");

    if( shared_mem_fd == NULL );
    {
	//printf("Error: errno %d \n",errno);
	//return 1;
    }

    fread(&value,sizeof(int),1,shared_mem_fd);
    return value;
}

int write_addr(char *proc_number,char *shared_mem_name,FILE *shared_mem_fd,int value)
{
    char shared_mem_str[200];
    sprintf(shared_mem_str,"/proc/%s/hw/ioreg/%s",proc_number,shared_mem_name);

    shared_mem_fd = fopen(shared_mem_str,"wb");

    if( shared_mem_fd == NULL );
    {
	//printf("Error: errno %d \n",errno);
	//return 1;
    }

    fwrite(&value,sizeof(int),1,shared_mem_fd);
    return 0;
}

char *timeo()
{
    static char thetime[100];
    char buffer[30];
    struct timeval tv;
    time_t curtime;

    gettimeofday(&tv, NULL);
    curtime=tv.tv_sec;

    strftime(buffer,30,"%m-%d-%Y  %T.",localtime(&curtime));
    sprintf(thetime,"%s%ld",buffer,tv.tv_usec);
    	
    return thetime;	
}

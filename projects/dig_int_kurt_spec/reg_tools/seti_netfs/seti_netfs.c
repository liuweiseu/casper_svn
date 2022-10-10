// seti_send.c

#include <stdio.h>
#include <stdlib.h>

#define PAYLOADSIZE 1024
#define DATAFILESIZE 2048
#define U32 unsigned int 
#define U8 unsigned char

int main(int argc,char *argv[])
{
    if(argv[1] == NULL)
    {
	printf("Usage: seti_send </path/to/fifo/dir> <fifofile>\n");
    }
    else
    {
	FILE *fp;
	FILE *fifo_file;
	char fifo[200];size_t fifo_bytesread;
	int bin_prev = 0,bin_cur=0;	
	void *fifo_buffer=(void *)malloc(16); 
	int i;
//	int j;
	int z;
	int num_vals = PAYLOADSIZE/16;

	void *ptr=0; 
	unsigned char buf[DATAFILESIZE];
			
	sprintf(fifo,"%s/%s",argv[1],argv[2]);
	fifo_file=fopen(fifo,"rb"); 

	if(fifo_file == NULL)
	{
	    printf("Error: couldn't openfile '%s'\n",fifo);
	    return 1;
	}

	fp=fopen("/mnt/netfs/test.bin", "wb");
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

		//	memcpy((void *)ptr,(const void *)fifo_buffer,4*sizeof(int));
			bin_cur = (*((U32 *)fifo_buffer+1) << 5) >> 20;
				
		//	bin_cur = (*((U32 *)ptr+1) << 5) >> 20;

			if(((bin_cur - bin_prev) > 1) && (bin_cur - bin_prev != -4095))
			{
			    printf("skipped %d\n",bin_cur-bin_prev);
			} 
//                    printf("raw:  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X\n",
 //                       *((U8 *)ptr+0), *((U8 *)ptr+1), *((U8 *)ptr+2), *((U8 *)ptr+3),
  //                      *((U8 *)ptr+4), *((U8 *)ptr+5), *((U8 *)ptr+6), *((U8 *)ptr+7),
   //                     *((U8 *)ptr+8), *((U8 *)ptr+9), *((U8 *)ptr+10), *((U8 *)ptr+11),
    //                    *((U8 *)ptr+12), *((U8 *)ptr+13), *((U8 *)ptr+14), *((U8 *)ptr+15));
		       bin_prev=bin_cur;
//		    fwrite((const void *)fifo_buffer,16,1,(FILE *)fp);

		    ptr+=16;
		   z++;
		}
		fflush(fp);
			 
	    }
	
        

	}

	fclose(fp);
    
	}

	return 0;
}





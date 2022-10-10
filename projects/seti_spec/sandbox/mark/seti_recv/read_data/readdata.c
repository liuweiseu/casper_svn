#include <math.h>
#include <stdio.h>
#include <grace_np.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../setispec.h"

#define NUM 100000000
#define U32 unsigned int 

struct spectra_data{
    unsigned int coarse_spectra[4096];
    unsigned int fine_spectra[409600][2]; //power,bin
};

unsigned int slice(unsigned int value,unsigned int width,unsigned int offset){
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}

int main()
{
    int finebin = 0 ;
    int bytesread = 1;
    double value;
    long int size_of_spectra = 1;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    int pfb_fft_power = 0;
    long int i = 0;
    int j = 0;
    void *buf1;
    long int *ptr1;
    int ctr = 0;

    FILE *datafile;

    unsigned int *buf = (unsigned int *)malloc(NUM);

    //create 2 buffers
    buf1 = calloc(NUM,8);
    ptr1 = buf1;

    struct data_vals *data_ptr;
    data_ptr = (struct data_vals *)buf;

    struct spectra_data spectra;

    fopen("datafile","rb");


    while(1)
    {
	    //fread and grab buffer
	    //set buf to spectra 
	    //set header	 
	    
	    data_ptr = (struct data_vals *)buf;
	    
	    for(i=0;i<size_of_spectra;i++)
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


		finebin = fft_bin+32768*pfb_bin;
		value = (float) (pfb_fft_power);

		if(value < 1)
		{
		    value = 1;
		}

		//populate finebin 
		if(fft_bin != 0)
		{
		    spectra.fine_spectra[i][1] = value;  
		    spectra.fine_spectra[i][2] = finebin;  
		}

		//populate coarse bin power
		if(fft_bin == 0)
		{
		    spectra.coarse_spectra[i] = finebin;  
		}

		ptr1=buf1;
		//header is the size of packet in bytes
		//add header memcpy and advance ptr
		
		memcpy(ptr1,data_ptr,8);
		data_ptr++;
		ctr++;		
		ptr1++;
	    }
    }

    //free buffers 
    fclose(datafile);
    free(buf);
    free(buf1);

    return 0;

}




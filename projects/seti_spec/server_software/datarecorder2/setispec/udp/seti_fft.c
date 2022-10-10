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
#include "gracie.h"
#include "casper.h"
#include "setispec.h"

#define MAXBUFLEN 500
#define MAXSIZE 409600 

int main(void)
{
    int tone_bin;
    float maxvalue=0;
    int maxbin=0;	
    int xmax = BINS;
    int fft_bin = 0;	//15.0
    int pfb_bin = 0;	//12.0
    int over_thresh = 0;	//1.0	
    int blank = 0;		//3.0
    int event = 0;		//1.0
    int pfb_fft_power = 0;	//32.0
    int pfb_tmp;
    int max_pfb_bin;
    int max_fft_bin;
    int finebin;
    U32 *buf = (U32 *)malloc(8);
    
    FILE *data_file;	

    if((data_file = fopen("../datafiles/spectra/data3.dat","rb")) == NULL)
    {
	printf("Couldn't open file\n");
	return 1;
    }
   
    int ctr=0;  
    int bytesread=1;
    double value; 

    grace_open(8192);	
    grace_init(maxbin,maxvalue,xmax);

    struct data_vals *data_ptr;
    data_ptr=(struct data_vals *)buf;

    tone=250.00;
    tone_bin = freq_bin(tone,BANDBEGIN,BANDEND,BINS);

    while((bytesread = fread(data_ptr,8,1,data_file)) != 0)
    {
	    U32 fields = ntohl(data_ptr->raw_data);
	    fft_bin = slice(fields,15,0);
	    pfb_bin = slice(fields,12,15);
	    over_thresh = slice(fields,1,27);
	    blank = slice(fields,3,28);
	    event = slice(fields,1,31);
	    pfb_fft_power = ntohl(data_ptr->overflow_cntr); //32.0

	    printf("%d %d\n",pfb_bin,pfb_fft_power);

	    if(fft_bin > 16384.)
	    {
		fft_bin = fft_bin - 32768;
	    }

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

	    pfb_tmp=pfb_bin;

	    if(value < 1)
	    {
		value = 1;
	    }
	    //get max bin                                       

	    if(value > maxvalue)
	    { 
		maxvalue=value; 
		maxbin=fft_bin;
		max_pfb_bin=pfb_bin;
		max_fft_bin=finebin;
		pfb_tmp=pfb_bin;
	    }
		    
	    if(fft_bin != 0)
	    {	
		//black
		GracePrintf("g0.s0 point %d, %f",finebin,value);
		GracePrintf("subtitle \"Test signal at %f MHz\"",tone);
	    }

	    if(fft_bin == 0)
	    {

		//white
		GracePrintf("g0.s1 point %d, %f",finebin,value);

		if(((tone_bin + 2) > finebin) && (finebin > (tone_bin - 2)) && value > 12)
		{
		    GracePrintf("g0.s1 point %d, %f",finebin,value);
		}
	    }
   
	    if(abs(tone_bin-max_fft_bin) < 64)
	    {
		printf("maxfftbin  %d  maxpfbbin: %d actmaxfft %d deltafft %d\n",max_fft_bin,max_pfb_bin,maxbin,tone_bin-max_fft_bin);
	    }
	    ctr++;

	    if(ctr == 4096*50)
	    {
		//    GracePrintf("autoscale");
		GracePrintf("autoticks");
		GracePrintf("redraw");
		GracePrintf("updateall");
		GracePrintf("saveall \"plots/sample%d.agr\"",tone);
	    	
//		GracePrintf("hardcopy device \"POSTSCRIPT\" ");
	//	GracePrintf("print to \"sample.ps\" ");
		GracePrintf("hardcopy device \"PNG\" ");
		GracePrintf("print to \"plots/sample%d.png\" ",tone);
		GracePrintf("print");
//		break;
//
		//GracePrintf("kill g0.s0");
	//	GracePrintf("kill g0.s1");
		grace_init(maxbin,maxvalue,xmax);
		ctr=0;
	    } 
    }

//    GraceClose();	
    fclose(data_file);
    return 0;
}



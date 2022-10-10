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

#define RECVPORT 33000	// the port users will be connecting to
#define MAXBUFLEN 500
#define U32 unsigned int
#define U8 unsigned char
#define PAYLOADSIZE 4096        //pay load size in bytes
#define DATAFILESIZE 536870912       //data file size in bytes
#define COARSEBINS 4096
#define BINS 4096
#define TONE 150 

int main(void)
{
    int tone_bin=0;
    int pfb_bin = 0;	//12.0
    int pfb_power = 0;		//32.0
    int real = 0;
    int imag = 0;
    int maxbin=0;
    int maxvalue=0; 
    double value; 
    int xmax=BINS;
    int i;
    FILE *data_file;	

    if((data_file = fopen("datafiles/pfb_fft.txt","rb")) == NULL)
    {
	printf("Couldn't open file\n");
	return 1;
    }
   

    grace_open(8192);	
    grace_init(maxbin,maxvalue,xmax);

  //  tone_bin = (TONE)*40.96-2048.;
    tone_bin = (TONE)*20.48-2048.;
    
    for(i=0;i<BINS;i++)
    { 

	fscanf(data_file," %d %*d %*d %d %d %*f %*f ",&pfb_bin,&real,&imag);
	pfb_power=pow((real),2)+pow((imag),2);

	if(pfb_bin > 2047)
	{
	    pfb_bin = pfb_bin-2048;
	}
	else
	{
	    pfb_bin = pfb_bin+2048;
	}

	value = (float) (pfb_power);
	if(value < 1)
	{
	    value=1;
	}

	//get max bin                                       

	if(value > maxvalue)
	{ 
	    maxvalue=value; maxbin=pfb_bin;
	}
	 
//	printf("%5d %5d\n",pfb_bin,pfb_power);
	GracePrintf("g0.s0 point %d, %f",pfb_bin,value);
    }

    grace_init(maxbin,maxvalue,xmax);
    GracePrintf("saveall \"plots/pfb-sample%dMHz.agr\"",TONE);
    GracePrintf("hardcopy device \"PNG\" ");
    GracePrintf("print to \"plots/pfb-sample%dMHz.png\" ",TONE);

    GracePrintf("print");
    
    printf("maxvalue: %d maxbin: %d expected bin: %d\n",maxvalue,maxbin,tone_bin);
    fclose(data_file);
    return 0;
}

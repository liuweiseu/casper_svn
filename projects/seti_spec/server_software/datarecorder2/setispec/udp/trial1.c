#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "setispec.h"

int main()
{
    int rstcnt=0;
    float scale_rand=1./2147483648.;
    float increment;
    float increment1=.01;//200./4096.;
    int j=0;
    FILE *fp;
    float i=250.;
    int pfbbin;
    int thrbin;
    int expbin;    
    
    int fftbin;
    int fftexpbin;
    int deltafft;
   // int scale=1000;
    float freq=1.0;

  //  int skip_scale=1;

    fp=fopen("datafiles/testtrial4.dat","wb");

    setamp(-15.0);

//    ddc_conf();
//    restart_bee();

    while( i < 300 )
    {
	printf("=======================================================================\n");
	increment=increment1*scale_rand*rand();
	printf("rand inc %f\n",increment);
//	sleep(1);

	tone=i;

	setfreq(tone);
	freq=setfreq(tone);
	write_pfb_fft();
	pfbbin=read_pfb_fft_compare(); 

	recv_data_write();

	thrbin=compare_tones(1);
	fftbin=compare_tones(0);
		
	fftexpbin=freq_bin(i,BANDBEGIN,BANDEND,BINS);
//	fftexpbin=fftexpbin+16384;
	expbin=freq_bin(i,BANDBEGIN,BANDEND,COARSEBINS);

	deltafft=fftbin-fftexpbin;

	//tvg_test(scale,freq,skip_scale);
	fprintf(fp,"%f exp: %d pfb: %d thr: %d fft: %d fftexp: %d deltafft: %d\n",tone,expbin,pfbbin,thrbin,fftbin,fftexpbin,deltafft);
	printf("%3.7f exp: %4d pfb: %4d thr: %4d fft: %9d fftexp: %9d deltafft: %6d\n",tone,expbin,pfbbin,thrbin,fftbin,fftexpbin,deltafft);

	//If PFB bin is off restart CT
	if( (pfbbin - expbin) != 0 && j > 5 )
	{
	    fprintf(fp,"pfb exp bin mismatch %f %d %d %d \n",tone,expbin,pfbbin,thrbin);
	    if( pfbbin - expbin > 5 )
	    {
		printf("PFB bin is off, reconfiguring PFB...\n");
		fprintf(fp,"reconfiguring PFB...\n");
		pfb_conf();
	    }
	}

		printf("%d\n",rstcnt);
	//If PFB bin out of THR is off restart CT
	if( (thrbin - expbin) != 0 && j > 5 )
	{
	    fprintf(fp,"thr exp bin mismatch %f %d %d %d \n",tone,expbin,thrbin,expbin);

	}

	//restart CT if FFT bin is off    
	if( abs(deltafft) > 128000 )
	{
//		    printf("THR PFB is off, restarting ct...\n");
	//	    fprintf(fp,"restarting ct...\n");
		//    restart_ct();
		 //   ct_conf();
		    if( i >= 100 + increment )
		    {
			i=i-increment;	
		    }

	}

	i=i+increment;
	fflush(fp);
	j++;
    }
    
	
   fclose(fp); 

    //write_pfb_fft(); 
/*
    for(i=256;i<260;i++)
    { 
	freq=(float)(i)*(1.0);
//	tvg_test(scale,freq,skip_scale);
	pfbbin=read_pfb_fft_compare(); 
	printf("%f %d\n",freq,pfbbin);
    }
*/
    return 0;
}


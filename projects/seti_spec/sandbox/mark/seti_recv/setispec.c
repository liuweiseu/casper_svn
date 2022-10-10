#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "setispec.h"
#include "casper.h"
#include "gracie.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void kill_ct(void)
{
    system("echo '/home/mwagner/bin/seti_kill_ct.sh' | ssh beecourageous");
}

void kill_pfb(void)
{
    system("echo '/home/mwagner/bin/seti_kill_pfb.sh' | ssh beecourageous");
}

void kill_fft(void)
{
    system("echo '/home/mwagner/bin/seti_kill_fft.sh' | ssh beecourageous");
}

void kill_thr(void)
{
    system("echo '/home/mwagner/bin/seti_kill_thr.sh' | ssh beecourageous");
}

void kill_bee(void)
{
    //kill all processes on the bee
    kill_ct();
    kill_pfb();
    kill_fft();
    kill_thr();
}

void ddc_conf(void)
{
    system("echo 'regwrite ddc_en 1' | nc -u -q1 192.168.1.6 7");
}

void ct_conf(void)
{
    //configure corner turner
    system("ssh beecourageous '/home/mwagner/bin/conf_chip_ct.sh&'");
}

void pfb_conf(void)
{
    //configure pfb 
    system("ssh beecourageous '/home/mwagner/bin/conf_chip_pfb.sh&'");
}

void fft_conf(void)
{
    //configure fft 
    system("ssh beecourageous '/home/mwagner/bin/conf_chip_fft.sh&'");
}

void thr_conf(void)
{
    //configure thresholder 
    system("ssh beecourageous '/home/mwagner/bin/conf_chip_thr.sh&'");
}

void restart_ct(void)
{
    //kill ct proc 
    kill_ct();
    //restart corner turner
    system("ssh beecourageous '/home/mwagner/bin/seti_start_ct.sh&' > /dev/null &");
    sleep(3);
    //kill local ssh process
    system("tmp=$(ps aux | grep seti_start_ct | awk '{print $2}'); kill -9 $tmp");
    ct_conf();
}

void restart_pfb(void)
{
    //kill pfb proc
    kill_pfb();
    //restart corner turner
    system("ssh beecourageous '/home/mwagner/bin/seti_start_pfb.sh&' > /dev/null &");
    sleep(3);
    //kill local ssh process
    system("tmp=$(ps aux | grep seti_start_pfb | awk '{print $2}'); kill -9 $tmp");
    pfb_conf();
}

void restart_thr(void)
{
    //kill thr all proc 
    kill_thr();
    system("ssh beecourageous '/home/mwagner/bin/seti_start_thr.sh&' > /dev/null &");
    sleep(2);
    system("tmp=$(ps aux | grep seti_start_thr | awk '{print $2}'); kill -9 $tmp");
    thr_conf();
}

void restart_fft(void)
{
    //kill all procs 
    kill_fft();
    system("ssh beecourageous '/home/mwagner/bin/seti_start_fft.sh&' > /dev/null &");
    sleep(3);
    //kill local ssh process
    system("tmp=$(ps aux | grep seti_start_fft | awk '{print $2}'); kill -9 $tmp");
    fft_conf();
}


void restart_bee(void)
{
    //restart all chips
    restart_thr();
    restart_fft();
    restart_ct();
    restart_pfb();
}


void write_pfb_fft(void)
{
    //char cmd1[200];
    //char cmd2[200];	
    //sprintf((char *)cmd1,(const char *)"ssh %s '/home/mwagner/bin/write_pfb_fft.sh'","beecourageous");
    //system(cmd1);
    system("ssh beecourageous '/home/mwagner/bin/write_pfb_fft.sh'");
//    system("ssh beecourageous 'scp pfb_fft.txt paper1:/home/mwagner/workspace/seti_spec/src/mark/seti_recv/datafiles/pfb_fft.txt'");
/*
    system("ssh beecourageous 'echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_we'");
    system("ssh beecourageous 'echo 1 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl'");
    system("ssh beecourageous 'echo 0 > /proc/$pfbPN/hw/ioreg/snap_fft_ctrl'");
    system("ssh beecourageous 'sleep 2'");
    system("ssh beecourageous 'echo 0 > /proc/$pfbPN/hw/ioreg/snap_fft_we'");
    system("ssh beecourageous 'echo 1 > /proc/$pfbPN/hw/ioreg_mode'");
    //echo "Reading SNAP_PFB_XAUI"
    system("ssh beecourageous 'read_bram12 $pfbPN snap_fft c2c > pfb_fft.txt'"); 
    system("ssh beecourageous 'echo 1 > /proc/$pfbPN/hw/ioreg/snap_xaui_we'");
*/
}

int read_pfb_fft_compare(void)
{
   
    int tone_bin=0;
    int pfb_bin = 0;	//12.0
    int pfb_power = 0;		//32.0
    int real = 0;
    int imag = 0;
    int maxbin=0;
    int maxvalue=0; 
    double value; 
    int i;
    FILE *data_file;	

    if((data_file = fopen("datafiles/pfb_fft.txt","rb")) == NULL)
    {
	printf("Couldn't open file\n");
	return 1;
    }
   
    tone_bin = freq_bin(tone,100,300,4096);
    
    for(i=0;i<COARSEBINS;i++)
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
	 
    }

     
    fclose(data_file);
    return maxbin;
}


int recv_data_write(void)
{
    //get packet and write to file
    int i,j=0;

    int sockfd;
    struct sockaddr_in recv_addr;       // recv address information
    struct sockaddr_in send_addr; // connector's address information
    socklen_t addr_len;

    int numbytes;
    FILE *data_file;
    const char data_file_str[200];

    U32 *buf = (U32 *)malloc(PAYLOADSIZE);

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
	
    while(j<NUMFILES)
    {
	sprintf((char *)data_file_str,(const char *)"datafiles/data%d.dat",j);
	//printf("\nwriting %s ...\n",data_file_str);

	data_file = fopen(data_file_str,"wb");	
	
	for(i=0;i<DATAFILESIZE/PAYLOADSIZE;i++)
	{	
			
	    if ((numbytes = recvfrom(sockfd, buf, PAYLOADSIZE , 1,
		    (struct sockaddr *)&send_addr, &addr_len)) == -1) 
	    {
		perror("recvfrom");
		exit(1);
	    }

	    fwrite((const void *)buf,PAYLOADSIZE,1,(FILE *)data_file);
	    fflush((FILE *)data_file);
	}

	fclose(data_file);
	free(buf);
	j++;
    }

    close(sockfd);
    return 0;
}

int compare_tones(int sel)
{
    float sigma=0.0;
    int tone_bin;
    float maxvalue=0;
    int maxbin=0;	
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

    if((data_file = fopen("datafiles/data0.dat","rb")) == NULL)
    {
	printf("Couldn't open file\n");
	return 1;
    }
   
    int ctr=0;  
    int bytesread=1;
    double value; 

    struct data_vals *data_ptr;
    data_ptr=(struct data_vals *)buf;

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

	    if(fft_bin > 16384)
	    {
		fft_bin = fft_bin-32768;
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
		    
	    if(fft_bin != 0 && sel == 0)
	    {	
		//FFT bin
		if(value > 4*sigma && abs(tone_bin-finebin) < 66000)
		{
		    //max_fft_bin=fft_bin;
		  //  finebin = max_fft_bin+32768*max_pfb_bin; 
		   // pfb_tmp=max_pfb_bin;
		   // max_pfb_bin=finebin;
		   // max_pfb_bin=;
		}
	    }

	   // if(fft_bin == 0 && sel == 1)
	    if(sel == 1)
	    {
		//PFB bin
	//	if(value > 4*sigma && abs(tone_bin-finebin) < 66000)
	//	{
		    //max_fft_bin=fft_bin;
		 //   finebin = max_fft_bin+32768*max_pfb_bin; 
		    //pfb_tmp=max_pfb_bin;
		   // max_pfb_bin=finebin;
		    //max_pfb_bin=maxbin;
		    max_fft_bin=max_pfb_bin;
//	    	    printf("=== %d ===\n",max_pfb_bin);
	//	}

	    }
	    ctr++;

    }

 //   printf("%d\n",max_pfb_bin);
    printf("maxfftbin  %d  maxpfbbin: %d actmaxfft %d \n",max_fft_bin,max_pfb_bin,maxbin);
    fclose(data_file);
    return max_fft_bin;

}

int freq_bin(float freq,float band_begin,float band_end,float channels)
{
    int tone_bin;    
    float a,b;
    b=(channels*band_begin)/(band_begin-band_end); 
    a=-1*(b/band_begin);
    tone_bin=rnd(a*freq+b);
    return tone_bin;
}

void compare_pfb_tvg(void)
{
    fill_tvg();
    set_tvg(tone);
    write_pfb_fft();
    read_pfb_fft_compare();
}

void fill_tvg(void)
{
    system("ssh beecourageous '/home/mwagner/bin/fill_tvg.sh 100'");
}

void set_tvg(float freq)
{
    int bin;
    int skip;
    const char command[200];
    bin = freq_bin(freq,BANDBEGIN,BANDEND,COARSEBINS);
    skip = rnd(((float)bin)/8.0);
    printf("\n %d \n",skip);
    sprintf((char *)command,(const char *)"ssh beecourageous '/home/mwagner/bin/set_tvg.sh 512 %d 1'",skip);
}

void tvg_test(int scale,float freq,int skip_scale)
{
    //scale=1000;
    //freq=1;
    //skip_scale=128;

//    const char command1[200];
//    const char command2[200];
    const char command3[200];

    
//    sprintf((char *)command1,(const char *)"ssh beecourageous '/home/mwagner/bin/tvg_create %f %s > /home/mwagner/data/tvg_%s.dat'",freq,(char *)"sin");
 //   printf(command1);
 //   sprintf((char *)command2,(const char *)"ssh beecourageous '/home/mwagner/bin/tvg_create %f %s > /home/mwagner/data/tvg_%s.dat'",freq,(char *)'cos');

    sprintf((char *)command3,(const char *)"ssh beecourageous '/home/mwagner/bin/tvg_test.sh %f %d %d'",freq,scale,skip_scale);

    //printf(command3);
    //printf("\n"); 

   // printf("%s\n",command);
//    system(command1);
//    system(command2);
    system(command3);
}


int setfreq(float freq)
{
    //set freq on signal generator
    const char command1[200];
    const char command2[200];

    sprintf((char *)command1,(const char *)"echo '0%f MHz' | nc -q 1 128.32.63.144 4000",freq);
    system(command1);
    sprintf((char *)command2,(const char *)"echo '1' | nc -q 1 128.32.63.144 4000");
    return 0;
}

int setamp(float amp)
{
    //set amplitude on signal generator
    const char command1[200];
    const char command2[200];

    sprintf((char *)command1,(const char *)"echo '3%f dBm' | nc -q 1 128.32.63.144 4000",amp);
    system(command1);
    sprintf((char *)command2,(const char *)"echo '4' | nc -q 1 128.32.63.144 4000");
    system(command2);
    printf("\n");
    return 0;
}

int rnd(float x)
{
    int y;

    if(x > 0)
    {
	y=(int)(x+.5f);
    }
    else
    {
	y=(int)(x-.5f);
    }

    return y;
}


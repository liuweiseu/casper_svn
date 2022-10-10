//Catch-all program for data capture
//Command line options:
//   For udp catpure: > udp | udp64 (5 Dec - works)
//   Input text file: > txtfile filename
//   BORPH capture: > proc_num bram_name_prefix
//   Format 2's compliment data: c2c (works)
//   Do complex FFT: fft (not implimented)
//   Unsort adc data: adc
//   Split concatinated 16-bit data from a 32-bit BRAM: split

//OUTPUT - stdout
//COL 1         COL2         COL3       COL4         COL5    
//BRAM address  LSB unsigned LSB signed MSB unsigned MSB signed
//
//Things to do:
//Expand text file option
//Make setting IP addresses easier

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
//#include <fftw3.h>
#include "two_comp.h"

#define BRAM_LEN 2048
#define SHIFT 4

/* Prototypes */

int format2comp(unsigned int u_int, int bitwidth);
int calc_bw(unsigned int buffer[][2], int bram_len); //Calculate bitwidth

int main(int argc,char **argv)
{
    int borph, udpdata, txtfile = 0;    //input type flags
    int dofft, dof2c, doadc, dosplit = 0;	//dof2c = do format 2 compliment  
    int i, j, k= 0;			//looping int's
    int bram_bw = 32;			//bram bitwidth
    int bitwidth, snap_loop = 0; 
/* For BORPH capture */
    FILE *bram_file, *bram_file_l, *bram_file_m;
    char bram[200], bramlsb[200], brammsb[200];
    unsigned int dum;

/*For text file input*/
    FILE *txt_file;

/*For UPD capture */
    unsigned char **buffer; //Pointer to array of pointers
    unsigned char *buff_hold;
    int s,c,cl;
    int loop_num;
    struct sockaddr_in sock_loc, sock_rem;

    double pwr;
    unsigned int ubuffer[BRAM_LEN][2]; 
    int sbuffer[BRAM_LEN][2]; //index 0 = lsb / snap32, index 1 = msb
    double fft[BRAM_LEN][2];

/*Check for input format*/    
    if(!strcmp(argv[1], "txtfile"))
	txtfile = 1;
    else if(!strncmp(argv[1], "udp", 3))
	udpdata = 1;
    else
	borph = 1;

/*Check for desired processing*/

    for(i = 2; i < argc; i++)
    {
    	if(!strcmp(argv[i], "fft"))
		dofft = 1;
	else if(!strcmp(argv[i], "c2c"))
		dof2c = 1;
    	else if(!strcmp(argv[i], "adc"))
		doadc = 1; 
	else if(!strcmp(argv[i], "split"))
		dosplit = 1;
    }
//printf("argc: %d\n", argc);
//printf("c2c flag: %d\n", dof2c);
//printf("udp flag: %d\n", udpdata);
//printf("adc flag: %d\n", doadc);

/*If BORPH == 1, open register files*/

    if(borph == 1)
    {
        sprintf(bram,"/proc/%s/hw/ioreg/%s",argv[1],argv[2]);
    	bram_file=fopen(bram,"rb");
    
    	sprintf(bramlsb,"/proc/%s/hw/ioreg/%s_bram_lsb",argv[1],argv[2]);
    	bram_file_l=fopen(bramlsb,"rb");

    	sprintf(brammsb,"/proc/%s/hw/ioreg/%s_bram_msb",argv[1],argv[2]);
    	bram_file_m=fopen(brammsb,"rb");

//Check for snap or snap64

    	if(bram_file != NULL)
        	bram_bw = 32;
    	else if(bram_file_m != NULL && bram_file_l != NULL)
        	bram_bw = 64;
    	else if(bram_file == NULL && bram_file_l == NULL && bram_file_m == NULL)
        	printf("Error: couldn't open file \n"); 
    
//If BRAM snap32, set file descriptor for bram_file = lsb for looping

    	if(bram_bw == 32)
        	bram_file_l = bram_file;
    
//Read in BRAMs
    
    	for(i=0; i<BRAM_LEN; i++)
    	{
            if(bram_bw == 32)
            {
                fread(&ubuffer[i][0],sizeof(int),1,bram_file_l);
            	ubuffer[i][1] = 0;
                sbuffer[i][0] = 0; sbuffer[i][1] = 0;
        	}
            else
            {
                fread(&ubuffer[i][0], sizeof(int),1,bram_file_l);
                fread(&ubuffer[i][1], sizeof(int),1,bram_file_m);
            }
    	}
   
       if(bram_bw == 32) 
        fclose(bram_file);
       else{
        fclose(bram_file_l);
        fclose(bram_file_m);
       }
        if(dosplit == 1 && bram_bw == 32)
        {   bram_bw = 64;
            for(i = 0; i < BRAM_LEN; i++)
            {   
                dum = ubuffer[i][0];
                ubuffer[i][0] = (unsigned int) 0x0000FFFF & dum;
                ubuffer[i][1] = (unsigned int) (0xFFFF0000 & dum) >> 16;
            }
        }
            
    } //(borph == 1)

/*If txtfile == 1. Currently assumes 32-bit bram and file has one column.*/

    if(txtfile == 1)
    {
	int sink;
    	sprintf(bram, "%s", argv[2]);
    	txt_file = fopen(bram, "rb");

	bram_bw = 32;
/* Assumes input format same as output of this program */	
	for(i = 0; i < BRAM_LEN; i++)
		fscanf(txt_file,"%d %u %u %d %d %lf %lf\n", &sink, &ubuffer[i][0], &ubuffer[i][1], &sbuffer[i][0], &sbuffer[i][1], &fft[i][0], &fft[i][1]);

    	fclose(txt_file);
    } //txtfile == 1

/*  If udpdata == 1.  */

    if(udpdata == 1)
    {
/*Determine if SNAP or SNAP64*/

        if(!strcmp(argv[1], "udp64"))
	{
                snap_loop = (2*BRAM_LEN) / 256; //Call recv() 256 * 16 = 2 * 2048
        	bram_bw = 64;
	}
	else if(!strcmp(argv[1], "udp"))
	{        
        	snap_loop = BRAM_LEN / 256; //Call recv() 256 * 8 = 2048
        	bram_bw = 32;
	}
	else
                snap_loop = 0;
/* Open socket */

        s = socket(PF_INET, SOCK_DGRAM, 0);     //SOCK_DGRAM for UDP
        	if(s < 0)
                	printf("A socket is not open.\n");

/* Define protocol, port, and IP address */
	sock_rem.sin_family = AF_INET; //Remote socket
        sock_rem.sin_port = htons(14850);
        inet_pton(AF_INET, "132.236.7.214", &(sock_rem.sin_addr)); //Set iBOB IP
        
        sock_loc.sin_family = AF_INET;  //Local socket        
	sock_loc.sin_port = htons(14850);
        inet_pton(AF_INET, "132.236.7.238", &(sock_loc.sin_addr)); //Set local IP

/*Bind socket */
        c = bind(s, (struct sockaddr *)&sock_loc, sizeof(sock_loc) );
        	if(c<0)
                	printf("Bind failed.\n");

/*Create receive buffers*/	
	buffer = malloc(snap_loop * sizeof(char *));   //Create array of pointers
        
        if(buffer == NULL)
                printf("Pointer array not allocated\n");

        for(i = 0; i < snap_loop; i++)
        {
                buffer[i] = malloc(1200 * sizeof(char));
                
                if(buffer[i] == NULL)
                        printf("Memory for buffer %i not allocated\n", i);
	}

/* Receive data loop */

        for(i = 0; i < snap_loop; i++)
                recv(s, buffer[i], 1200,0);
        
/*Rearrange pointers so buffer[0] corresponds to header = 0 */

       loop_num = buffer[0][0]; //Get loop number from header
	
       for(i = 0; i < (snap_loop - loop_num); i++)
        {
                buff_hold = buffer[0];
                
                for(j = 0; j < (snap_loop-1); j++)
                {
                        buffer[j] = buffer[j+1];
                }
                
                buffer[snap_loop - 1] = buff_hold;
        }
/*
	for(j = 0; j < 8; j++)
	{
		for(i = 0; i < 256; i++)
			printf("%u\n", buffer[j][i]);
	}
*/
/*Sort out bytes appropriately and turn them into integers*/
   if(doadc == 1)  //Unsort ADC data
   {

	for(j = 0; j < 7; j++)
	{
		for(i = 0; i < 256; i++)
		{
			ubuffer[(i*2) + (j*256)][0] = buffer[j][(i*4)+3+SHIFT];
			ubuffer[(i*2) +1 +  (j*256)][0] = buffer[j][(i*4)+1+SHIFT];
			
			if(snap_loop == 16)
				ubuffer[i + j*1024][1] = buffer[j+8][i+SHIFT];   //This makes no sense
		}
	}
    }
    else if(dosplit == 1) //Split 16-bit data from 32-bit BRAM
    {
	for(j = 0; j < 8; j++)
	{
		for(i = 0; i < 256; i++)
		{
			ubuffer[i+j*256][0] = (buffer[j][(i*4)+SHIFT+2] << 8) | buffer[j][(i*4)+SHIFT+3];   //Least significant bytes
			ubuffer[i+j*256][1] = (buffer[j][(i*4)+SHIFT] << 8) | buffer[j][(i*4)+SHIFT+1];
            sbuffer[i+j*256][0] = 0; 
            sbuffer[i+j*256][1] = 0;
		}
	}
     }
    else
    {
        for(j = 0; j < 8; j++)
        {
                for(i = 0; i < 256; i++)
                {
                        ubuffer[i+j*256][0] = (buffer[j][(i*4)+SHIFT] << 24) | (buffer[j][(i*4)+SHIFT+1] << 16) | (buffer[j][(i*4)+SHIFT+2] << 8) | buffer[j][(i*4)+SHIFT+3];
                	sbuffer[i+j*256][0] = 0;

                        if(snap_loop == 16)
                        {        ubuffer[i+j*256][1] = (buffer[j+8][(i*4)+SHIFT] << 24) | (buffer[j+8][(i*4)+SHIFT+1] << 16) | (buffer[j+8][(i*4)+SHIFT+2] << 8) | buffer[j+8][(i*4)+SHIFT+3];
				sbuffer[i+j*256][1] = 0;
                        }
			else
			{
                                ubuffer[i+j*256][1] = 0;
				sbuffer[i+j*256][1] = 0;
                	}
		}
        }
    }

/*Free up memory*/

        for(i = 0; i < snap_loop; i++)
                free(buffer[i]);
        free(buffer);

/*Close socket*/        
        close(s);
    } //udp == 1

/*Format 2 compliment data*/
    if(dof2c == 1)
    {
        bitwidth = calc_bw(ubuffer, BRAM_LEN);	//Find bitwidth	
	
    for(j = 0; j < (bram_bw / 32); j++)
	{
		for(i = 0; i < BRAM_LEN; i++)
			sbuffer[i][j] = format2comp(ubuffer[i][j], bitwidth);
	}
    }

/*FFT*/

    if(dofft == 1)
    {
/*	fftw_complex *datain, *dataout;
	fftw_plan cfft;

	datain = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*BRAM_LEN);
	dataout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*BRAM_LEN);

	for(i = 0; i < BRAM_LEN; i++)
	{
		datain[i][0] = (double) sbuffer[i][1];   //Real
		datain[i][1] = (double) sbuffer[i][0];   //Imag  
	}

	cfft = fftw_plan_dft_1d(BRAM_LEN, datain, dataout, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(cfft);
	fftw_destroy_plan(cfft);
	
	for(i = 0; i < BRAM_LEN; i++)
	{
		fft[i][0] = dataout[i][0];
		fft[i][1] = dataout[i][1];
	}
    	
	fftw_free(datain); fftw_free(dataout);
*/    }

    else{
        for(i = 0; i < BRAM_LEN; i++){
            fft[i][0] = 0.0;
            fft[i][1] = 0.0;
        }
    }
           
/* Print data */        
	for(i = 0; i < BRAM_LEN; i++)
		printf("%4d %11u %11u %11d %11d %lf %lf\n", i, ubuffer[i][0], ubuffer[i][1], sbuffer[i][0], sbuffer[i][1], fft[i][0], fft[i][1]);
	
	return 0;
}

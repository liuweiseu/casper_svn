// seti_recv.c
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
#include "setispec.h"
#include "casper.h"
#include "gracie.h"

int main(void)
{
	
    int tmp;
    int fft_bin = 0;	//15.0
    int pfb_bin = 0;	//12.0
    int over_thresh = 0;	//1.0	
    int blank = 0;		//3.0
    int event = 0;		//1.0
    int pfb_fft_power = 0;	//32.0

    int grace_buf_size=8192;    
    float maxvalue=0;
    float tone = 150;
    int maxbin=0;	

    int i,j=0,k=0;
    int l=0;

    int sockfd;
    struct sockaddr_in recv_addr;	// recv address information
    struct sockaddr_in send_addr; // connector's address information
    socklen_t addr_len;

    int numbytes;
    FILE *data_file;	
    const char data_file_str[200];
	
    int spectra[COARSEBINS];
    int spectra_ctr=0;
    int channels[COARSEBINS];

    double value;
    //populate channels array
    for(l=0;l<COARSEBINS;l++)
    {
	channels[l]=l;
    }

    U32 *ptr,*ptr2;
    U32 *buf = (U32 *)malloc(PAYLOADSIZE);
    ptr = buf;
    ptr2 = buf;

    unsigned int prev_cnt = 0;
    unsigned int num_jump = 0;
    unsigned int num_good = 0;

    setfreq(250.0);

    GraceRegisterErrorFunction(my_error_function);

    if(GraceOpenVA("xmgrace", grace_buf_size,"-barebones", "-free","-nosafe", "-noask", NULL))
    {
        fprintf(stderr, "Can't run Grace. \n");
        exit(EXIT_FAILURE);
    }
	
    grace_init(maxbin,maxvalue,COARSEBINS);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
	perror("socket");
	exit(1);
    }

    //get local address info and populate sockaddr_in struct
	
    recv_addr.sin_family = AF_INET;		 // host byte order
    recv_addr.sin_port = htons(RECVPORT);	 // short, network byte order
    recv_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with recv IP
    memset(recv_addr.sin_zero, '\0', sizeof recv_addr.sin_zero);

    //bind socket file descriptor to local addr and port
	
    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof recv_addr) == -1) 
    {
	perror("bind");
	exit(1);
    }

    addr_len = sizeof send_addr;
	
   // setfreq(250.0);
    while(1)
    {
	sprintf((char *)data_file_str,(const char *)"../datafiles/spectra/data_file%d.dat",j);
	printf("%s\n",data_file_str);

	data_file = fopen(data_file_str,"wb");	
	
	for(i=0;i<DATAFILESIZE/PAYLOADSIZE;i++)
	{	
	    maxvalue=0;
			
	    if ((numbytes = recvfrom(sockfd, buf, PAYLOADSIZE , 1,
		    (struct sockaddr *)&send_addr, &addr_len)) == -1) 
	    {
		perror("recvfrom");
		exit(1);
	    }

	    struct data_vals *data_ptr;
	    data_ptr=(struct data_vals *)buf; 

	    for(k=0;k<PAYLOADSIZE;k+=8)
	    {

		U32 fields = ntohl(data_ptr->raw_data);

		fft_bin = slice(fields,15,0);
		pfb_bin = slice(fields,12,15);
		over_thresh = slice(fields,1,27);
		blank = slice(fields,3,28);
		event = slice(fields,1,31);
		pfb_fft_power = ntohl(data_ptr->overflow_cntr);	//32.0
			    
		if(fft_bin == 0)
		{
		    //populate array spectra
		    if(pfb_bin == spectra_ctr)
		    {
			spectra[spectra_ctr]=pfb_fft_power;
			spectra_ctr++;
		    }

		    //if spectra is full, plot it
		    if(spectra_ctr == COARSEBINS)
		    {
			//rearange array

			for(l=0;l<COARSEBINS/2;l++)
			{
			    tmp=spectra[l];
			    spectra[l]=spectra[l+2048];
			    spectra[l+2048]=tmp;
			}
			//plot
			for(l=0;l<COARSEBINS && GraceIsOpen();l++)
			{
			    //make sure not to get -inf values for log 
			    value = (double) spectra[l];			    
					    
			    if(value < 1)
			    {
				value = 1;
			    } 
			    //get max bin 					    
			    if(value > maxvalue)
			    {
				maxvalue=value;	
				maxbin=channels[l];
			    }

			    GracePrintf("g0.s0 point %d, %f",channels[l],(value));
			}

			 //reset counter
			spectra_ctr=0;
   //                                 GracePrintf("autoscale yaxes");
			GracePrintf("autoticks");
			GracePrintf("redraw");
			GracePrintf("updateall");
			GracePrintf("saveall \"plots/sample.agr\"");
			GracePrintf("hardcopy device \"POSTSCRIPT\" ");
			GracePrintf("print to \"plots/sample.ps\" ");
			GracePrintf("print");
			GracePrintf("kill g0.s0");
					
			grace_init(maxbin,log10(maxvalue),COARSEBINS);
		    }

		    
		}

		if( (pfb_bin - prev_cnt) >1 && (pfb_bin - prev_cnt)!=-4095)
		    num_jump++;

		prev_cnt = pfb_bin;
		data_ptr++;

	    } 

	    fwrite((const void *)buf,PAYLOADSIZE,1,(FILE *)data_file);
	    fflush((FILE *)data_file);
			    
	    if(num_jump == 0)
		num_good++;
	    else
		printf("Number of jumps:%d\n", num_jump);

	    num_jump = 0;

	}

	fclose(data_file);
	j++;
    }

    GraceClose();	
    close(sockfd);

    return 0;
}

/*
unsigned int slice(unsigned int value,unsigned int width,unsigned int offset){
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}   
*/
/*
void grace_init(int maxbin,float maxvalue)
{
//    GracePrintf("world xmax %d",COARSEBINS);
//    GracePrintf("world ymax 10");
//    GracePrintf("world ymin .25");
    //map fonts
    GracePrintf("map font 0 to \"Times-Roman\", \"Times-Roman\"");
    GracePrintf("map font 1 to \"Times-Italic\", \"Times-Italic\"");
    GracePrintf("map font 2 to \"Times-Bold\", \"Times-Bold\"");
    GracePrintf("map font 3 to \"Times-BoldItalic\", \"Times-BoldItalic\"");
    GracePrintf("map font 4 to \"Helvetica\", \"Helvetica\"");
    GracePrintf("map font 5 to \"Helvetica-Oblique\", \"Helvetica-Oblique\"");
    GracePrintf("map font 6 to \"Helvetica-Bold\", \"Helvetica-Bold\"");
    GracePrintf("map font 7 to \"Helvetica-BoldOblique\", \"Helvetica-BoldOblique\"");
    GracePrintf("map font 8 to \"Courier\", \"Courier\"");
    GracePrintf("map font 9 to \"Courier-Oblique\", \"Courier-Oblique\"");
    GracePrintf("map font 10 to \"Courier-Bold\", \"Courier-Bold\"");
    GracePrintf("map font 11 to \"Courier-BoldOblique\", \"Courier-BoldOblique\"");
    GracePrintf("map font 12 to \"Symbol\", \"Symbol\"");
    GracePrintf("map font 13 to \"ZapfDingbats\", \"ZapfDingbats\"");

    //map colors
    GracePrintf("map color 0 to (255,255,255),\"white\" ");
    GracePrintf("map color 1 to (0,0,0),\"black\" ");
    GracePrintf("map color 2 to (255,0,0),\"red\" ");
    GracePrintf("map color 3 to (0, 255, 0), \"green\"");
    GracePrintf("map color 4 to (0, 0, 255), \"blue\"");
    GracePrintf("map color 5 to (255, 255, 0), \"yellow\"");
    GracePrintf("map color 6 to (188, 143, 143), \"brown\"");
    GracePrintf("map color 7 to (220, 220, 220), \"grey\"");
    GracePrintf("map color 8 to (148, 0, 211), \"violet\"");
    GracePrintf("map color 9 to (0, 255, 255), \"cyan\"");
    GracePrintf("map color 10 to (255, 0, 255), \"magenta\"");
    GracePrintf("map color 11 to (255, 165, 0), \"orange\"");
    GracePrintf("map color 12 to (114, 33, 188), \"indigo\"");
    GracePrintf("map color 13 to (103, 7, 72), \"maroon\"");
    GracePrintf("map color 14 to (64, 224, 208), \"turquoise\"");
    GracePrintf("map color 15 to (0, 139, 0), \"green4\"");

    //defaults
    GracePrintf("default linewidth 1.0");
    GracePrintf("default linestyle 1");
    GracePrintf("default color 1");
    GracePrintf("default pattern 1");
    GracePrintf("default font 8");
    GracePrintf("default char size 1.000000");
    GracePrintf("default symbol size 1.000000");
    GracePrintf("default sformat \"%16.8g\"");

    //background color
    GracePrintf("background color %d",7);

    //g0
    GracePrintf("g0 on");
  //  GracePrintf("title \"\"");

    GracePrintf("g0 hidden false");
    GracePrintf("g0 type XY");
    GracePrintf("g0 stacked false");
    GracePrintf("g0 bar hgap 0.000000");
    GracePrintf("g0 fixedpoint off");
    GracePrintf("g0 fixedpoint type 0");
    GracePrintf("g0 fixedpoint xy 0.000000, 0.000000");
    GracePrintf("g0 fixedpoint format general general");
    GracePrintf("g0 fixedpoint prec 6, 6");
    GracePrintf("with g0");
    GracePrintf("view xmin 0.10000");
    GracePrintf("view xmax 0.9000");
    GracePrintf("view ymin 0.200000");
    GracePrintf("view ymax .80000");
  
    //g0 xaxis 
    GracePrintf("world xmax %d",4098);
    GracePrintf("world xmin %d",0);
    GracePrintf("world ymax %d",100000000);
    GracePrintf("world ymin %d",1);
    GracePrintf("stack world 0, 0, 0, 0 tick 0, 0, 0, 0");
    GracePrintf("yaxes scale Logarithmic");

    GracePrintf("xaxis tick major 1024");
    GracePrintf("xaxis tick minor 100");

    //g0 xaxis label 
    GracePrintf("xaxis label \"bins\"");
    GracePrintf("xaxis label font \"Courier\"");
    GracePrintf("xaxis label char size .7");
    GracePrintf("xaxis ticklabel font \"Courier\"");
    GracePrintf("xaxis ticklabel char size .7");
    GracePrintf("xaxis label color %d",1);

    //g0 yaxis 
  //  GracePrintf("yaxis tick major 1");
  //  GracePrintf("yaxis tick minor 1");
 //   GracePrintf("yaxis tick log ON");

    //g0 yaxis label 
    GracePrintf("yaxis label \"amplitude\"");
    GracePrintf("yaxis label font \"Courier\"");
    GracePrintf("yaxis label char size .7");
    GracePrintf("yaxis ticklabel font \"Courier\"");
    GracePrintf("yaxis ticklabel char size .7");
    GracePrintf("yaxis ticklabel format power");
    GracePrintf("yaxis tick minor ticks 5");

    GracePrintf("yaxis ticklabel prec 0");

    GracePrintf("yaxis label color %d",1);
    //g0 title properties
    GracePrintf("title \"Seti Spectrometer\"");
    GracePrintf("title color %d",1);
    GracePrintf("title font \"Courier\"");
    GracePrintf("title size .9");

    GracePrintf("subtitle \"max value %f in bin %d \"",maxvalue,maxbin);
    GracePrintf("subtitle color %d",1);
    GracePrintf("subtitle font 8");
    GracePrintf("subtitle size .8");

    //create set s0 and propoerties for graph g0 
    GracePrintf("s0 on");
    GracePrintf("s0 hidden false");
    GracePrintf("s0 type xy");
    GracePrintf("s0 symbol 2");
    GracePrintf("s0 symbol color 1");
    GracePrintf("s0 symbol size .3");
    GracePrintf("s0 symbol char 65");
    GracePrintf("s0 symbol fill pattern 1");
    GracePrintf("s0 symbol fill color 0");
    GracePrintf("s0 line linestyle 5");
    GracePrintf("s0 line color 1");
    GracePrintf("s0 fill type 2");
    GracePrintf("s0 fill rule 5");
    GracePrintf("s0 fill color 0");
    GracePrintf("s0 fill pattern 1");
    
}
int max_value(int *p_array,unsigned int values_in_array,int *p_max_value)
{
    int position;
    position = 0; 
    *p_max_value = p_array[position];

    for (position = 1; position < values_in_array; ++position)
    {
	if (p_array[position] > *p_max_value)
	{
	    *p_max_value = p_array[position];
	    break;
	}
    }
    return position;
}

*/

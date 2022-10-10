#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "setiread.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int64_t read_data(char * data, int datasize)
{
    int64_t record_count = ((int64_t *) data)[0];
    int64_t beam_no = ((int64_t *) data)[1];

    //fprintf(stderr, "record ct %lld beam no %lld\n", record_count, beam_no);
    //fprintf(stderr, &data[16]);
    return record_count;
}

int read_beam(char * data, int datasize)
{
    int64_t beam_no = ((int64_t *) data)[1];
    switch((int)beam_no) 
    {
	case 238:
	    beam_no = 0;
	    break;
	case 237:
	    beam_no = 0;
	    break;
	case 235:
	    beam_no = 1;
	    break;
	case 231:
	    beam_no = 2;
	    break;
	case 222:
	    beam_no = 3;
	    break;
	case 221:
	    beam_no = 4;
	    break;
	case 219:
	    beam_no = 5;
	    break;
	case 215:
	    beam_no = 6;
	    break;
	case 190:
	    beam_no = 7;
	    break;
	case 189:
	    beam_no = 8;
	    break;
	case 187:
	    beam_no = 9;
	    break;
	case 183:
	    beam_no = 10;
	    break;
	case 126:
	    beam_no = 11; 
	    break;
	case 125:
	    beam_no = 12;
	    break;
	case 123:
	    beam_no = 13; 
	    break;
	case 119:
	    beam_no = 14;
	    break;
    } 

    return beam_no;
}

//moves the fd to the beginning of the first header in the file
int find_first_header(int fd)
{
    int i;
    int header_not_found=1;
    int header_location=0;

    //we should be able to find the first header within DR_HEAER_SIZE+MAX_DATA_SIZE+strlen(HEADER_STRING)
    char *buffer = (char *) malloc(MAX_DATA_SIZE+DR_HEAER_SIZE+strlen(HEADER_STRING));
    
    read(fd, (void *) buffer, MAX_DATA_SIZE+DR_HEAER_SIZE+strlen(HEADER_STRING));
          
    for(i=0; header_not_found && i<DR_HEAER_SIZE+MAX_DATA_SIZE; i++)
    {
        header_not_found=strncmp(buffer+i, HEADER_STRING, strlen(HEADER_STRING));
        if(!header_not_found)
        {
            // need to match HEADER_SIZE also then are other instances where HEADER will occur in the header
            header_not_found = strncmp(buffer+i+strlen(HEADER_STRING)+1, HEADER_SIZE_STRING, strlen(HEADER_SIZE_STRING));
       //     fprintf(stderr, buffer+i);
        //    fprintf(stderr, buffer+i+strlen(HEADER_STRING)+1);
        }

        header_location=i;
    }

    if(header_not_found)
    {
        fprintf(stderr, "Error finding header in %s\n", FILENAME);
        return -1;
    }
    else
    {
        //fprintf(stderr, "header found at %d\n", header_location);
        lseek(fd, header_location, SEEK_SET);
        
        return 0;
    }

}
/*
void read_data(char * data, int datasize)
{
    int64_t record_count = ((int64_t *) data)[0];
    int64_t beam_no = ((int64_t *) data)[1];
    
    //fprintf(stderr, "record ct %ld beam no %ld\n", record_count, beam_no);
    //fprintf(stderr, &data[16]);
}
*/
int read_header(char * header)
{
    //fprintf(stderr, header);
    int i;
    int datasize;
    for(i=0; i<DR_HEAER_SIZE; )
    {
        //fprintf(stderr, header+i);
        if(strncmp(header+i, DATA_SIZE_STRING, strlen(DATA_SIZE_STRING))==0)
        {
            sscanf(header+i, "DATA_SIZE %d\n", &datasize);
           // fprintf(stderr, "found data size %d at %d\n", datasize, i);
        }
        i+=strlen(header+i)+1;
    }
    return datasize;
}

/* Initialize GRACE plotting window */
int grace_init()
{

	float viewx[9]; //GRACE graph x coordinates
	float viewy[9]; //GRACE graph y coordinates

	int i=0,j=0,k=0,x=0; //Counters


	/* Assign Viewgraph locations (3 x 3, with 2 shrunk down for the legend) */

    viewx[8] = 0.06;
    viewy[8] = 0.04;

    viewx[7] = 0.39;
    viewy[7] = 0.04;

    viewx[6] = 0.72;
    viewy[6] = 0.04;

    viewx[5] = 0.39;
    viewy[5] = 0.04;

    viewx[4] = 0.06;
    viewy[4] = 0.04;

    viewx[3] = 0.72;
    viewy[3] = 0.36;

    viewx[2] = 0.39;
    viewy[2] = 0.36;

    viewx[1] = 0.06;
    viewy[1] = 0.36;

    viewx[0] = 0.06;
    viewy[0] = 0.68;


	/* Register GRACE Error Function */
    GraceRegisterErrorFunction(my_error_function);

    /* Start Grace with a buffer size of 16k and open the pipe */
    if (GraceOpen(16768) == -1) {
        fprintf(stderr, "Can't run Grace. \n");
        exit(EXIT_FAILURE);
    }
    
    GracePrintf("DEVICE \"X11\" PAGE SIZE 1000, 1000");

    GracePrintf("arrange(3, 3, 0.1, 0.15, 0.5)");

    GracePrintf("with g8"); 
    GracePrintf("BACKGROUND COLOR 1");	

    GracePrintf("Legend ON");
    GracePrintf("Legend loctype view");
    GracePrintf("Legend 0.39, .88");
    GracePrintf("LEGEND COLOR 0"); 	
    GracePrintf("LEGEND BOX COLOR 0"); 
    GracePrintf("LEGEND BOX FILL COLOR 1"); 

    GracePrintf("with g8");
    GracePrintf("view ymax 0.002");
    GracePrintf("view ymin 0.001");
    GracePrintf("view xmax 0.002");
    GracePrintf("view xmin 0.001");
    GracePrintf("xaxis tick off");
    GracePrintf("yaxis tick off");    

    GracePrintf("with g7"); 
    GracePrintf("view ymax 0.002");
    GracePrintf("view ymin 0.001");
    GracePrintf("view xmax 0.002");
    GracePrintf("view xmin 0.001");
    GracePrintf("xaxis tick off");
    GracePrintf("yaxis tick off");    
	
	
	for(j=0;j<7;j++){
	   GracePrintf("with g%d", j);
	   		   	 	
	    GracePrintf("BACKGROUND COLOR 1");	

 	   GracePrintf("xaxis tick minor grid on");
 	   GracePrintf("yaxis tick minor grid on");
 	   GracePrintf("xaxis tick major grid on");
 	   GracePrintf("yaxis tick major grid on");

 	   GracePrintf("yaxis tick major color 0");
 	   GracePrintf("xaxis tick major color 0");
 	   GracePrintf("yaxis tick minor color 0");
 	   GracePrintf("xaxis tick minor color 0");

 	   GracePrintf("yaxis tick major linewidth 0");
 	   GracePrintf("xaxis tick major linewidth 1");
 	   GracePrintf("yaxis tick minor linewidth 0");
 	   GracePrintf("xaxis tick minor linewidth 0");

 	   GracePrintf("yaxis tick major linestyle 2");
 	   GracePrintf("xaxis tick major linestyle 2");
 	   GracePrintf("yaxis tick minor linestyle 2");
 	   GracePrintf("xaxis tick minor linestyle 2");

 	   GracePrintf("yaxis tick color 0");
 	   GracePrintf("xaxis tick color 0");

 	   GracePrintf("yaxis tick default 5");
 	   GracePrintf("xaxis tick default 5");


 	   GracePrintf("yaxis bar on");
 	   GracePrintf("xaxis bar on");

 	   GracePrintf("yaxis bar color 0");
 	   GracePrintf("xaxis bar color 0");

 	   GracePrintf("yaxis label color 0");
 	   GracePrintf("xaxis label color 0");

 	   GracePrintf("yaxis bar linewidth 2");
 	   GracePrintf("xaxis bar linewidth 2");

 	   GracePrintf("yaxis ticklabel color 0");
 	   GracePrintf("xaxis ticklabel color 0");

 	   GracePrintf("yaxis ticklabel format power");
 	   GracePrintf("xaxis ticklabel format decimal");



		GracePrintf("view ymin %f", viewy[j]);
    	GracePrintf("view xmin %f", viewx[j]);
		GracePrintf("view xmax %f", viewx[j] + .25);
	   	GracePrintf("view ymax %f", viewy[j] + .23);

	    GracePrintf("xaxis tick major 50");
	    GracePrintf("yaxis tick major 100");

	   GracePrintf("yaxis tick minor off");
 	   GracePrintf("xaxis tick minor off");

 	   GracePrintf("xaxis tick minor size .20");
 	   GracePrintf("yaxis tick minor size .20");
 	   GracePrintf("xaxis tick major size .40");
 	   GracePrintf("yaxis tick major size .40");
 	   
 	   GracePrintf("xaxis ticklabel char size 0.55");
 	   GracePrintf("yaxis ticklabel char size 0.55");

 	   GracePrintf("xaxis ticklabel skip 2");
 	   GracePrintf("yaxis ticklabel skip 2");
 
 	   GracePrintf("xaxis ticklabel append \" MHz\"");

 	   GracePrintf("yaxis ticklabel prec 2");
 
	   GracePrintf("SUBTITLE FONT 10");
	   GracePrintf("SUBTITLE SIZE .70");
   	   GracePrintf("SUBTITLE \"ALFA BEAM %d\"", j);    
   	   GracePrintf("SUBTITLE COLOR 0");    


	   GracePrintf("world xmax 300");
	   GracePrintf("world xmin 100");
	   GracePrintf("world ymax 600");
	   GracePrintf("world ymin 1");	
	   GracePrintf("yaxes scale Logarithmic");

	  GracePrintf("s0 color 3");
	  GracePrintf("s1 color 14");
	  GracePrintf("s2 color 3");
	  GracePrintf("s2 symbol 4");
	  GracePrintf("s2 symbol size 0.28");
	  GracePrintf("s2 symbol fill pattern 1");
	  GracePrintf("s2 linestyle 0");
	  GracePrintf("s2 symbol color 3");
	  GracePrintf("s2 symbol linewidth 0");

	  GracePrintf("s3 color 14");
	  GracePrintf("s3 symbol 4");
	  GracePrintf("s3 symbol size 0.28");
	  GracePrintf("s3 symbol fill pattern 1");
	  GracePrintf("s3 linestyle 0");
	  GracePrintf("s3 symbol color 14");
	  GracePrintf("s3 symbol linewidth 0");


	}
	
	    /* set up legend */
         GracePrintf("g8.s0 color 3");
         GracePrintf("g8.s0 legend \"Left Circular\"");

         GracePrintf("g8.s1 color 14");
         GracePrintf("g8.s1 legend \"Right Circular\"");
       
      
         for (i = 0; i < 1024 && GraceIsOpen(); i++) {
             GracePrintf("g8.s0 point %d, %d", i, i);
         
         }

         for (i = 0; i < 1024 && GraceIsOpen(); i++) {
             GracePrintf("g8.s1 point %d, %d", i, i);
         
         }


         GracePrintf("redraw");

		//GracePrintf("FRAME BACKGROUND COLOR 5");	
		
}


void my_error_function(const char *msg)
{
    fprintf(stderr, "library message: \"%s\"\n", msg);
}

unsigned int slice(unsigned int value,unsigned int width,unsigned int offset)
{
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}



//print coarse spectra dump to stdout.
int print_coarse(spectral_data * spectra, int beam_number, double start_freq)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    
    //start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    
    int i=0;

    /* plot coarse bins */
    for(i=0;i<4096;i++)
    {   
		//GracePrintf("AUTOSCALE");
		 printf("%d  %f  %d\n", beam_number, start_freq - ((i*32768) * bin_freq), (*spectra).coarse_spectra[i]);
		 //printf("%d, %f, %d\n", beam_number, start_freq - (((i*32768) + 32767) * bin_freq), (*spectra).coarse_spectra[i]);		
    }
    
}

//print fine spectra dump to stdout.
int print_fine(spectral_data * spectra, int beam_number, double start_freq)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    
    //start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    
    int i=0;

    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
       //GracePrintf("AUTOSCALE");
	   printf("%d  %f  %f\n", beam_number, start_freq - ((double) (*spectra).hits[i][1] * bin_freq), (double)((*spectra).hits[i][0]));
    }
	
}

//print fine spectra dump to stdout.
int print_fine_normalized(spectral_data * spectra, int beam_number, int counter, double start_freq)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    
    //start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    
    int i=0;

    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
       //GracePrintf("AUTOSCALE");
	   printf("%d  %d  %f  %f  %f  %d\n", counter, beam_number, start_freq - ((double) (*spectra).hits[i][1] * bin_freq), 24.0 * (double) ( ((*spectra).hits[i][0]) /  (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]), (double)((*spectra).hits[i][0]), (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]  );
    }
	
}

//print fine spectra dump to stdout.
int print_fine_normalized_file(FILE *fp, spectral_data * spectra, int beam_number, int counter, double start_freq)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    
    //start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    
    int i=0;

    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
       //GracePrintf("AUTOSCALE");
	   fprintf(fp, "%d  %d  %f  %f  %f  %d\n", counter%500, beam_number, start_freq - ((double) (*spectra).hits[i][1] * bin_freq), 24.0 * (double) ( ((*spectra).hits[i][0]) /  (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]), (double)((*spectra).hits[i][0]), (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]  );
    }	
}

//print fine spectra dump to stdout.
int increment_histograms(gsl_histogram *sigma_histogram, gsl_histogram *freq_histogram, spectral_data * spectra, int beam_number, int counter, double start_freq)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
        
    //(Synth - (100.00 - (200.00 / 8192)) ;)
    
    int i=0;


    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
		if( (24.0 * (double) ( ((*spectra).hits[i][0]) /  (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768])) < 24.0) {
			printf ("ERROR %d %d\n\n", counter, beam_number);
		}
	   gsl_histogram_increment(sigma_histogram, log10(24.0 * (double) ( ((*spectra).hits[i][0]) /  (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]) ) );
	   gsl_histogram_increment(freq_histogram, start_freq - ((double) (*spectra).hits[i][1] * bin_freq));
	   
    }	
}





//print fine spectra dump to stdout.
int print_errors(spectral_data * spectra, int beam_number, int counter)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    double start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    int i=0;

    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
       //GracePrintf("AUTOSCALE");
         
	     if( ((*spectra).hits[i][0]) < (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768] ) {
	   		printf("CNT: %d BEAM: %d, FREQ: %f, NORM POWER: %f, %f, %d, %d, overthresh: %d blank: %d event: %d\n", counter, beam_number, start_freq + ((double) (*spectra).hits[i][1] * bin_freq), 24.0 * (double) ( ((*spectra).hits[i][0]) /  (double) (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768]), (double)((*spectra).hits[i][0]), (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768],   (int) (*spectra).hits[i][1] % 32768, (*spectra).flags[i][0], (*spectra).flags[i][1], (*spectra).flags[i][2]);
	   		printf("before: %d\n",  (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768  +1]);
	   		printf("after: %d\n", (*spectra).coarse_spectra[ (int) (*spectra).hits[i][1] / 32768 - 1] );

    	 }
    }
	
}





//int plot_beam(spectral_data * spectra, int beam_number);
int plot_beam(spectral_data * spectra, int beam_number)
{
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
    double start_freq = 100.00 - (200.00 / 8192);  //100 MHz - half a bin;
    int i=0;

    GracePrintf("kill g%d.s%d",(int) ( (double) beam_number / 2), beam_number % 2);
    GracePrintf("kill g%d.s%d", (int) ( (double) beam_number / 2), ((beam_number % 2) + 2));

    GracePrintf("with g%d",(int) ( (double) beam_number / 2));
    GracePrintf("s0 color 3");
    GracePrintf("s1 color 14");
    GracePrintf("s2 color 3");
    GracePrintf("s2 symbol 4");
    GracePrintf("s2 symbol size 0.28");
    GracePrintf("s2 symbol fill pattern 1");
    GracePrintf("s2 linestyle 0");
    GracePrintf("s2 symbol color 3");
    GracePrintf("s2 symbol linewidth 0");
    GracePrintf("s3 color 14");
    GracePrintf("s3 symbol 4");
    GracePrintf("s3 symbol size 0.28");
    GracePrintf("s3 symbol fill pattern 1");
    GracePrintf("s3 linestyle 0");
    GracePrintf("s3 symbol color 14");
    GracePrintf("s3 symbol linewidth 0");

    /* plot coarse bins */
    for(i=0;i<4096;i++)
    {   
	//GracePrintf("AUTOSCALE");

	GracePrintf("g%d.s%d point %f, %d", (int) ( (double) beam_number / 2), beam_number % 2, start_freq + ((i*32768) * bin_freq), (*spectra).coarse_spectra[i]);
	GracePrintf("g%d.s%d point %f, %d", (int) ( (double) beam_number / 2), beam_number % 2, start_freq + (((i*32768) + 32767) * bin_freq), (*spectra).coarse_spectra[i]);		
    }

    /* plot hits */
    for(i=0;i<(*spectra).numhits;i++)
    {   
    	//GracePrintf("AUTOSCALE");
	   	GracePrintf("g%d.s%d point %f, %f", (int) ( (double) beam_number / 2), ((beam_number % 2) + 2), start_freq + ((double) (*spectra).hits[i][1] * bin_freq), (double)((*spectra).hits[i][0]));
	   	//if(i==0) printf("g%d.s%d point %f, %f", (int) ( (double) beam_number / 2), ((beam_number % 2) + 2), start_freq + ((double) (*spectra).hits[i][1] * bin_freq), (double)((*spectra).hits[i][0]));
    }
	
}

void grace_open_deux(int bufsize)
{
    GraceRegisterErrorFunction(my_error_function);

    if(GraceOpenVA("xmgrace", bufsize, "-free","-nosafe", "-noask", NULL))
    {
        fprintf(stderr, "Can't run Grace. \n");
        exit(EXIT_FAILURE);
    }

}

void grace_init_deux(int maxbin,float maxvalue,int xmax)
{

//fonts
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

//colors

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

//g0 xaxis

    GracePrintf("world xmax %d",xmax);
    GracePrintf("world xmin %d",0);
    GracePrintf("world ymax %d",1000000000);
    GracePrintf("world ymin %d",1);
    GracePrintf("stack world 0, 0, 0, 0 tick 0, 0, 0, 0");
    GracePrintf("yaxes scale Logarithmic");
    GracePrintf("xaxis tick major 32000");
    GracePrintf("xaxis tick minor 100");

//g0 xaxis label 

    GracePrintf("xaxis label \"bins\"");
    GracePrintf("xaxis label font \"Courier\"");
    GracePrintf("xaxis label char size .7");
    GracePrintf("xaxis ticklabel font \"Courier\"");
    GracePrintf("xaxis ticklabel char size .7");
    GracePrintf("xaxis label color %d",1);

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

//create set s0 and properties for graph g0

    GracePrintf("s0 on");
    GracePrintf("s0 hidden false");
    GracePrintf("s0 type xy");
    GracePrintf("s0 symbol 2");
    GracePrintf("s0 symbol color 1");
    GracePrintf("s0 symbol size .3");
    GracePrintf("s0 symbol char 65");
    GracePrintf("s0 symbol fill pattern 0");
    GracePrintf("s0 symbol fill color 0");
    GracePrintf("s0 line linestyle 0");
    GracePrintf("s0 line color 1");
    GracePrintf("s0 fill type 0");
    GracePrintf("s0 fill rule 5");
    GracePrintf("s0 fill color 0");
    GracePrintf("s0 fill pattern 1");
  
//create set s1 and properties for graph g0

    GracePrintf("s1 on");
    GracePrintf("s1 hidden false");
    GracePrintf("s1 type xy");
    GracePrintf("s1 symbol 2");
    GracePrintf("s1 symbol color 0");
    GracePrintf("s1 symbol size .3");
    GracePrintf("s1 symbol char 65");
    GracePrintf("s1 symbol fill pattern 0");
    GracePrintf("s1 symbol fill color 9");
    GracePrintf("s1 line linestyle 0");
    GracePrintf("s1 line color 1");
    GracePrintf("s1 fill type 0");
    GracePrintf("s1 fill rule 5");
    GracePrintf("s1 fill color 0");
    GracePrintf("s1 fill pattern 1");

//create set s2 and properties for graph g0

    GracePrintf("s2 on");
    GracePrintf("s2 hidden false");
    GracePrintf("s2 type xy");
    GracePrintf("s2 symbol 2");
    GracePrintf("s2 symbol color 14");
    GracePrintf("s2 symbol size .3");
    GracePrintf("s2 symbol char 65");
    GracePrintf("s2 symbol fill pattern 0");
    GracePrintf("s2 symbol fill color 9");
    GracePrintf("s2 line linestyle 0");
    GracePrintf("s2 line color 1");
    GracePrintf("s2 fill type 0");
    GracePrintf("s2 fill rule 5");
    GracePrintf("s2 fill color 0");
    GracePrintf("s2 fill pattern 1");

}








 


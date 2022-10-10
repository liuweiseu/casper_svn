#include <stdio.h>
#include <tgmath.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <grace_np.h>

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#  define EXIT_FAILURE -1
#endif

int grace_init();

void my_error_function(const char *msg)
{
    fprintf(stderr, "library message: \"%s\"\n", msg);
}

/* structure for spectral data */
typedef struct spectral_data {
	int coarse_spectra[4096];
	double hits[409600][2];
	int flags[409600][3];
	int numhits;
} spectral_data;

/* this function plots the data in spectra on the graph specified by beam_number */
int plot_beam(spectral_data * spectra, int beam_number)
{
	double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor
	int i=0;

	/* plot coarse bins */
	for(i=0;i<4096;i++){   
		//GracePrintf("AUTOSCALE");
		GracePrintf("g%d.s%d point %f, %d", (int) ( (double) beam_number / 2), beam_number % 2, 100.00 + ((i*32768) * bin_freq), (*spectra).coarse_spectra[i]);
		GracePrintf("g%d.s%d point %f, %d", (int) ( (double) beam_number / 2), beam_number % 2, 100.00 + (((i*32768) + 32767) * bin_freq), (*spectra).coarse_spectra[i]);		
	}


	/* plot hits */

	for(i=0;i<(*spectra).numhits;i++){   
		//GracePrintf("AUTOSCALE");
		GracePrintf("g%d.s%d point %f, %f", (int) ( (double) beam_number / 2), ((beam_number % 2) + 2),(*spectra).hits[i][0], (*spectra).hits[i][1]);
	}


}



int main(int argc, char** argv)
{

    spectral_data spectra;
    double bin_freq = (200.0 / 134217728.0); // bin to freq conversion factor

    int b=0;
    int i=0,j=0,k=0,x=0;

    /* Init GRACE Plotting Windows */
    grace_init();

    /*generate fake data and plot 14 beams */
    for(j=0;j<14;j++) {

	//generate fake coarse data
	for(i = 0; i<4096; i++) {
		spectra.coarse_spectra[i] = (int) (((double) random() / ((double)(RAND_MAX)+(double)(1))) * (double) 500);
		//printf("%d\n", coarse_spectra[i]);
	}

    //generate 99 fake hits
    spectra.numhits = 99;

    for(i = 0; i < spectra.numhits; i++) {
		spectra.hits[i][1] = (((double) random() / ((double)(RAND_MAX)+(double)(1))) * (double) 200) + (double) spectra.coarse_spectra[i];
		spectra.hits[i][0] = 100.00 + ((i*32768) * bin_freq) + (((double) random() / ((double)(RAND_MAX)+(double)(1))) * (double) 0.02);
		//printf("%f %f\n", hits[i][0], hits[i][1]);
	}

	//plot this beam
	plot_beam(&spectra, j);
}

/* play with some text */
for(i=0; i<10; i++){
	 GracePrintf("WITH STRING %d", i);
	 GracePrintf("STRING COLOR 0");
	 GracePrintf("STRING FONT 8");
	 
	 GracePrintf("STRING DEF \"TEST\"");
	 GracePrintf("STRING ON");
	 
	 GracePrintf("STRING LOCTYPE view");
	 GracePrintf("STRING 0.70, %f", 0.95 - (((float) i) / 40.0) );
}

GracePrintf("redraw");

sleep(500);

    //output pdf via grace

   GracePrintf("HARDCOPY DEVICE \"PDF\"");
   GracePrintf("PAGE SIZE 400, 400");
   GracePrintf("PRINT TO \"%s\"", argv[3]);
   GracePrintf("PRINT");

	 if (GraceIsOpen()) {
		  /* Flush the output buffer and close Grace */
		  GraceClose();
		  /* We are done */
		  exit(EXIT_SUCCESS);
	 } else {
		 exit(EXIT_FAILURE);
	 }


return 0; 
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
	   GracePrintf("world ymin 0");	

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




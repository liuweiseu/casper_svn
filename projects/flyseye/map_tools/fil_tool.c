#include <stdio.h>
#include <tgmath.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

//usage is progname <input_fil_file> <output_fil_file> 


//get size of file
long fsize(const char *const name)
{
	struct stat stbuf;
	if(stat(name, &stbuf) == -1)
	return -1; // The file could not be accessed.
	return stbuf.st_size;
}

void error_message(char *message)
{
  fprintf(stderr,"ERROR: %s\n\n",message);
  exit(1);
}

void print_usage(char *message)
{
  fprintf(stderr, "Usage is %s <input.fil> -o <output.fil> [-b (band pass correction) -t (time domain correction)\n", message);    
}

int strings_equal (char *string1, char *string2)
{
  if (!strcmp(string1,string2)) {
    return 1;
  } else {
    return 0;
  }
}

int file_exists(char *filename) 
{
  if ((fopen(filename,"rb"))==NULL) {
	return(0);
  } else {
	return(1);
  }
}

int main(int argc, char** argv)
{
int i, j, k, x;

int decimate=1; //default decimation value

int timedomain=0;  //time domain normalization switch
int bandpass=0;    //band pass correction switch


FILE *inputfile;
char inputfilename[100];
char outputfilename[100];
char message[100];
FILE *outputfile;
FILE *ignorefile;
int elementsread;

  /* work out how many files are on the command line */

  if(!file_exists(argv[1])) {
	  print_usage(argv[0]);
	  error_message("no input files supplied on command line!");
  } else strcpy(inputfilename, argv[1]);
  
  /* now parse any remaining command line parameters */
  if (argc>1) {
    i=2;
    while (i<argc) {
	  if (strings_equal(argv[i],"-o")) {
		  /* get and open file for output */
		  printf("opening %s for writing new fil output\n", argv[i]);
		  strcpy(outputfilename,argv[++i]);
		  outputfile=fopen(outputfilename,"wb");
	  } else if (strings_equal(argv[i],"-d")) {
	   //set decimation value 
	     decimate = atoi(argv[++i]);
      } else if (strings_equal(argv[i],"-b")) {
	   /* perform bandpass correction */
		bandpass=1;		
      } else if (strings_equal(argv[i],"-t")) {
	/* perform time domain normalization */
		timedomain=1;
      } else {
	/* unknown argument passed down - stop! */
	print_usage(argv[0]);
	sprintf(message,"unknown argument (%s) passed to filterbank.",argv[i]);
	error_message(message);
      }
      i++;
    }
  }


int tot_spectra = 0;
int zerospectracntr = 0;


float spectra[128];
float summed_spectra[128];
float average_equalized_spectra[128];
double average_spectra[128];
float most_recent_spectra[128];
double summedvalue=0.0;


for(i=0;i<128;i++) average_spectra[i] = 0.0;
for(i=0;i<128;i++) average_equalized_spectra[i] = 0.0;
for(i=0;i<128;i++) most_recent_spectra[i] = 0.0;


printf("opening %s for reading for spectral averaging..\n", inputfilename);

printf("Size of file \"%s\" is: %i bytes\n", inputfilename, fsize(inputfilename));

if(outputfile) printf("Output file open..\n");

if(( (fsize(inputfilename) / (128 * 4)) )%decimate != 0) {
    error_message("mismatched decimation\n");
}

printf("Decimating by factor: %i\n", decimate);


inputfile = fopen(inputfilename, "rb");

printf("reading..\n");

while(1){  

    elementsread = fread(&spectra, sizeof(float), 128, inputfile); 		

    if(elementsread != 128){
        printf("reached end of file\n");
        break;
    } else {
            
        tot_spectra++;
        summedvalue = 0.0;

        //hack correction for misinterpretation of unsigned values in filterbank-ata
        for(i=0;i<128;i++) if(spectra[i] < 0.0) spectra[i] = spectra[i] + 255.0;

        for(i=0;i<128;i++) {    
             average_spectra[i] = average_spectra[i] + (double) spectra[i]; 
             summedvalue = summedvalue + (double) (spectra[i]);         
        }
        
    }
}

printf("Closing file...\n");
fclose(inputfile);



printf("Total spectra read is is: %d\n", tot_spectra);

for(i=0;i<128;i++) {
    average_spectra[i] = average_spectra[i] / (double) tot_spectra;
}




   

printf("opening %s for rereading figuration..\n", argv[1]);
inputfile = fopen(inputfilename, "rb");



//      for(j=0;j<128;j++) summed_spectra[j] = summed_spectra[j] + spectra[j];
//      if((i+1)%decimate == 0) {
//         for(j=0;j<128;j++) {
//            fprintf(outputfile, "%f ", summed_spectra[j]/(float) decimate);
//            summed_spectra[j] = 0;
//         }
//         fprintf(outputfile, "\n");
//      }



j=0;
for(i=0;i<128;i++) summed_spectra[i] = 0.0;

while(1){  
    elementsread = fread(&spectra, sizeof(float), 128, inputfile); 		
    if(elementsread != 128){
        printf("reached end of file\n");
        break;
    }
 
    tot_spectra++;
    summedvalue=0.0;

    for(i=0;i<128;i++){
        if(spectra[i] < 0.0) {
		   spectra[i] = spectra[i] + 255.0;
		   zerospectracntr++;
        }
    }

	if(bandpass){
		for(i=0;i<128;i++) {    
		   spectra[i] = spectra[i] / (float) average_spectra[i];
		}
	}
	
	if(timedomain){
		for(i=0;i<128;i++) {    
		   summedvalue = summedvalue + (double) spectra[i]; 
		}
	
		for(i=0;i<128;i++) {    
		   spectra[i] = (128.0 * spectra[i]) / ((float) summedvalue);
		}
	}


    for(i=0;i<128;i++) summed_spectra[i] = summed_spectra[i] + spectra[i];
        
        
    if((j+1)%decimate == 0) {
	    for(i=0;i<128;i++) summed_spectra[i] = summed_spectra[i] / (float) decimate;
		//output real spectra    
   		fwrite(&summed_spectra, sizeof(float), 128, outputfile); 		        
        for(i=0;i<128;i++) summed_spectra[i] = 0.0;
    }   
    
	j++;    

}


fclose(outputfile);
fclose(inputfile);



printf("Total spectra read is: %d\n", tot_spectra);
printf("\n");
printf("Number of spectra less than zero: %d\n", zerospectracntr);
printf("\n");
exit(0);
}



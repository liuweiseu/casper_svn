/* PocketSpec WalletChain v. 0.01                   */
/* One ET in the Pocket is worth two in the sky.    */
/* Daniel Chapman, Henry Chen, Christina DeJesus,   */ 
/* Pierre Yves-Droz, Aaron Parsons, Andrew Siemion  */
/* SSL, BWRC, CASPER Collaboration  -- October 2005 */
/* dependencies: gnuplot 4.0 (www.gnuplot.org)      */
/*               strong intestinal fortitude        */

#include <stdio.h>     /* Standard input/output definitions */
#include <string.h>    /* String function definitions */
#include <unistd.h>    /* UNIX standard function definitions */
#include <fcntl.h>     /* File control definitions */
#include <errno.h>     /* Error number definitions */
#include <termios.h>   /* POSIX terminal control definitions */
#include <signal.h>    /*Signal functions*/
#include <sys/time.h>  /* Time... is on my side... */
#include <time.h>      /* Yes it is */
#include <sys/timex.h> /* NTP time include */ 
#include <math.h>
#include "/root/walletchain/gnuplot_i/gnuplot_i.h" /* modify this line to include the correct path containing gnuplot_i.h */


/*   Globals    */
#define BUFFER_SIZE 100000
#define COMMAND_LENGTH 10
#define ARRAY_SIZE 4096

void quit();
char *itoa(int value);
void print_usage(const char** argv);
void parse_args(int argc, const char** argv);

// Required by ntptime
typedef struct ntptimeval_s{
	struct timeval time;
	long int maxerror;
	long int esterror;
}ntptimeval;



FILE *fp;
char fileheader[120];
time_t timestuff;
gnuplot_ctrl *h1;

/* Defaults */
char specport[100]="/dev/ttyS0";
int plotting = 0;
int writing = 0;
int specinfo = 0;
int numfilecounter = 1;
int crudeoutput = 0; 
int spectracount = 10000;
int verboseflag = 0;
int spectrum2 = 0;

int main(int argc, const char * argv[])
{
  int fd;      
  int result;
  char buffer[BUFFER_SIZE];
  double array[ARRAY_SIZE];
  char fileheader2[100]; 
  int bufferCounter;
  int i, j;
  int loopCounter;
  char quitCommand[100];
  int processID;

  /*Remove if works*/
  char something[100];
 

  long one, two, three;
  
  /* Begin Default Spectrometer Configuration Values */

  int NUMCHANNELS = 2048;    /* Max ARRAY_SIZE */
  int POWERLEN = 4;
  char SWREV[24];
  int NUMINPUTS = 1;
   
  /* End Default Spectrometer Configuration Values */

  ntptimeval times;

  h1 = gnuplot_init();

  processID = getpid();    
  sprintf(quitCommand, "bind Escape \"!kill -63 %i\"", processID);
  
  gnuplot_cmd(h1, quitCommand);


  //  gnuplot_cmd(h1, "set yrange [0.0:260.0]");
  gnuplot_cmd(h1, "set style line 6 lt 3 lw 1");
  gnuplot_setstyle(h1, "st loopCounter++;eps ls 6");
  gnuplot_cmd(h1, "set mouse");







  time(&timestuff);
  strcpy(fileheader, itoa(timestuff));
  if(argc >= 2){
        parse_args(argc, argv);
  }
  else if(argc == 1){
    time(&timestuff);
    strcpy(fileheader, itoa(timestuff));
  }
   

   
   if( writing ){
      strcpy(fileheader2,fileheader);
      fp = fopen( strcat(strcat( strcat(fileheader2, "_"), itoa(numfilecounter) ), ".txt"), "w");
    }


       

  system("stty -F /dev/ttyS0 ispeed 115200 ospeed 115200 rows 0 columns 0 line 0 intr ^C quit '^\' kill ^U eof ^D start ^Q stop ^S susp ^Z rprnt ^R werase ^W lnext ^V flush ^O min 1 time 5 -parenb -parodd cs8 hupcl -cstopb cread clocal -crtscts ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke");

 
  if(verboseflag){
    printf("Verbose Mode Engaged\n");
  }

  fd = open(specport, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
  if (fd == -1){
    printf("open_port: Unable to open port: %s\n", specport);
    quit();
  }
  
  if(verboseflag){
    printf("Port open, sending data to IBOB...\n");
  }

    
  write(fd, "\n", 1);
 

  do{
    result = read(fd, buffer, 1);
      
  }while(buffer[0] != '%');




  printf("Communicating with IBOB...\n\n");
 

if(specinfo) {

   write(fd, "spectroinfo\n", 12);
   i=0;    
   do{
      result = read(fd, buffer, 1);
      if (result == 1) {
	i++;
      }
   }while(i < 14);
    
    bufferCounter = 0;
     do{
      result = read(fd, &buffer[bufferCounter], 1);
      
      if (result == 1) {
	  
   	     bufferCounter++;
      }
     
     }while((buffer[bufferCounter-1]!='%') ||
	   (buffer[bufferCounter-2]!=' ') ||
	   (buffer[bufferCounter-3]!='B') ||
	   (buffer[bufferCounter-4]!='O') ||
	   (buffer[bufferCounter-5]!='B') ||
	   (buffer[bufferCounter-6]!='I'));
   
     if (verboseflag){
     printf ("Spectrometer Reports: \n");
     }
     for(i=0; i<bufferCounter-6; i++){  
       if (verboseflag){
         printf("%c", buffer[i]);
       }
       if (writing) {
	 fprintf(fp, "%c", buffer[i]);  
       }
     }
     

     
     
     sscanf(buffer, "%*s %*s %s %*s %*s %*s %i %*s %*s %*s %*s %i",SWREV,&NUMINPUTS, &NUMCHANNELS);
 
     printf("Read Configuration....\n\n  Inputs: %i Channels: %i\n", NUMINPUTS, NUMCHANNELS);
     printf("  SW Rev: %s\n", SWREV);

     usleep(100000);

    }

 
	if(!writing && !plotting){
	        printf("\nWhat should I do with this data?\n\n");
		print_usage(&argv[0]);
		
		quit();
	}

loopCounter=0;
 


 do{
   

    if((loopCounter%spectracount == 0)  && loopCounter != 0  && writing){
      fclose(fp);
      numfilecounter++;
      strcpy(fileheader2, fileheader);
      fp = fopen(strcat(strcat(strcat(fileheader2, "_"), itoa(numfilecounter)), ".txt"), "w");
       
    }
    
    if(verboseflag){
    printf("Requesting spectrodump...\n");
    }
    
    write(fd, "spectrodump\n", 12);
    i=0;
    do{
      result = read(fd, buffer, 1);
      if (result == 1) {
	i++;
	}
      }while(i < 14);
    
    bufferCounter = 0;
     do{
      result = read(fd, &buffer[bufferCounter], 1);
      if (result == 1) {
		bufferCounter++;
      }
     
//     if(verboseflag){
//    printf("counter: %i,%i\n", bufferCounter, buffer[bufferCounter-1]);
//    }
     }while((buffer[bufferCounter-1]!='%') ||
	   (buffer[bufferCounter-2]!=' ') ||
	   (buffer[bufferCounter-3]!='B') ||
	   (buffer[bufferCounter-4]!='O') ||
	   (buffer[bufferCounter-5]!='B') ||
	   (buffer[bufferCounter-6]!='I'));
if (verboseflag){
     printf("Spectrodump received, bufferCounter=%i\n", bufferCounter);
     printf("buffer:\n%s\n", buffer);
}

    for(i=0; i<NUMCHANNELS; i++){
      
      array[i] = 0;
      //array[i] += (double) (((unsigned int) buffer[POWERLEN*i+0])%256)*pow(2,0);
      //array[i] += (double) (((unsigned int) buffer[POWERLEN*i+1])%256)*pow(2,8);
      for(j=0; j<POWERLEN;j++){ 
		array[i] += (double) (((unsigned int) buffer[POWERLEN*i+j])%256)*pow(2,j*8);
      }
	  /*
      array[i] += (double) (((unsigned int) buffer[8*i+3])%256)*pow(2,24);
      array[i] += (double) (((unsigned int) buffer[8*i+4])%256)*pow(2,32);
      array[i] += (double) (((unsigned int) buffer[8*i+5])%256)*pow(2,40);
      array[i] += (double) (((unsigned int) buffer[8*i+6])%256)*pow(2,48);
      array[i] += (double) (((unsigned int) buffer[8*i+7])%256)*pow(2,56);
	  */
    }


/* Handle Plotting */
if (plotting) {
    gnuplot_resetplot(h1);
    gnuplot_plot_x(h1, array, NUMCHANNELS, "Spectrum");
    }


/* Handle File Output */    
if (writing) {
	
	  ntp_gettime((struct ntptimeval *) &times);  			
	  fprintf(fp, "%i.%06i\n", (int) times.time.tv_sec, (int) times.time.tv_usec);		  

	  for(i=0; i<NUMCHANNELS; i++){
        	fprintf(fp, "%f|",array[i]);
	  }	

	  fprintf(fp, "\n");				
				

       	}


/* Increment Counter */
   loopCounter++;

  
  }while(1);
  exit(0);

}




char *itoa(int value)

{
int count,                   /* number of characters in string       */
    i,                       /* loop control variable                */
    sign;                    /* determine if the value is negative   */
char *ptr,                   /* temporary pointer, index into string */
     *string,                /* return value                         */
     *temp;                  /* temporary string array               */

count = 0;
if ((sign = value) < 0)      /* assign value to sign, if negative    */
   {                         /* keep track and invert value          */
   value = -value;
   count++;                  /* increment count                      */
   }

/* allocate INTSIZE plus 2 bytes (sign and NULL)                     */
temp = (char *) malloc(sizeof(int) + 2);    
if (temp == NULL)
   {
   return(NULL);
   }
memset(temp,'\0', sizeof(int) + 2);

string = (char *) malloc(sizeof(int) + 2);
if (string == NULL)
   {
   return(NULL);
   }
memset(string,'\0', sizeof(int) + 2);
ptr = string;                /* set temporary ptr to string          */

/*--------------------------------------------------------------------+
| NOTE: This process reverses the order of an integer, ie:            |
|       value = -1234 equates to: char [4321-]                        |
|       Reorder the values using for {} loop below                    |
+--------------------------------------------------------------------*/
do {
   *temp++ = value % 10 + '0';   /* obtain modulus and or with '0'   */
   count++;                      /* increment count, track iterations*/
   }  while (( value /= 10) >0);

if (sign < 0)                /* add '-' when sign is negative        */
   *temp++ = '-';

*temp-- = '\0';              /* ensure null terminated and point     */
                             /* to last char in array                */
/*--------------------------------------------------------------------+
| reorder the resulting char *string:                                 |
| temp - points to the last char in the temporary array               |
| ptr  - points to the first element in the string array              |
+--------------------------------------------------------------------*/
for (i = 0; i < count; i++, temp--, ptr++)
   {
   memcpy(ptr,temp,sizeof(char));
   }

return(string);
}

void print_usage(const char** argv)
{
	printf("Usage: %s [options]\n", argv[0]);
	printf(" -g                     Plot PocketSpec data in real time via GNUPlot.\n");
	printf(" -w                     Write spectra to a text file.\n");
	printf(" -v                     Output greater quantities of program status information.\n");
	printf(" -x <number of spectra> Specify number of spectra per log file.\n");
	printf(" -f <file name / path>  Specify file prefix and -optionally- path where logfiles are \n");
        printf("                        generated.\n");
	printf(" -i                     Request PocketSpec configuration.\n");
        //printf(" -ft <file name / path> Specify file prefix and -optionally- path where logfiles are \n");
        //printf("                        generated, appending a time stamp to <file name / path>.\n");	
	//	printf(" -p </dev/interface>         Specify interface of PocketSpec device.\n");
	
	//	printf(" -t                     Test mode, self-generate packetized data and send to\n"); 
	//printf("                        localhost.\n");
	//printf(" -d1                    Graphs by PFB bin number.\n");
	//printf(" -d2 <center>           Graph by frequency, specify the center frequency.\n");
        //printf("                        (Default, default center 2275.)\n");
	//printf(" -N                     For use from within Matlab/Octave. Write 2nd spectrum\n");
	//printf("                        to file.\n");	
	printf(" -h (or any other garbage) -- Get this help.\n\n");
	//printf(" ** Use file name /dev/stdout for screen output.\n");
}  


void parse_args(int argc, const char** argv) {

	int i;
	for(i=1;i<argc;i++){
		
	        if(strcmp(argv[i], "-w") == 0){
		  writing = 1;
		}
		else if(strcmp(argv[i], "-N") == 0){
			spectrum2 = 1;
		}
		else if(strcmp(argv[i], "-g") == 0){
			plotting = 1;
		}
	
	        else if(strcmp(argv[i], "-i") == 0){
		  specinfo = 1;
		}
           	
		else if(strcmp(argv[i], "-p") == 0){
                        memset(specport,'\0', 100);
			strcpy(specport, argv[i+1]);
			i++;
                        
			if (strncmp(argv[i], "-", 1) == 0){
					printf("Device names ought naught begin with a hyphen.\n");           
					quit();
			}
			
		}
		else if(strcmp(argv[i], "-x") == 0){
			spectracount = atoi  (argv[i+1]);
			i++;
			if (spectracount >= 1000001){
				printf("Invalid number of spectra per file. \nspectra per file must be within the range 1 to 10^6\n");
				quit();
			}
			if (spectracount < 1){
				printf("Invalid number of spectra per file. \nspectra per file must be within the range 1 to 10^6\n");
				quit();
			}
		}
		else if(strcmp(argv[i], "-f") == 0){
			if(i < argc-1){
				if(strcmp(argv[i+ 1], "/dev/stdout") == 0){
					crudeoutput = 1; 
					i++;
				} 
				else if (strncmp(argv[i+1], "-", 1) == 0){
					printf("Filenames ought naught begin with a hyphen.\n");           
					quit();
				}
				else{
					
					strcpy(fileheader, argv[ i + 1 ]);					
					i++;
				}
			}
			else{
			  printf("Must specify filename.\n");
			}
		}
		else if(strcmp(argv[i], "-ft") == 0){
		
		  
		        if(i < argc-1){
			
                        	if(strcmp(argv[i+ 1], "/dev/stdout") == 0){
					crudeoutput = 1;
					i++;
				}
				else if (strncmp(argv[i+1], "-", 1) == 0){
					printf("Filenames ought naught begin with a hyphen.\n");           
					quit();
				}
				else{
					
				  time(&timestuff);
				  strcpy(fileheader, argv[ i + 1 ]);
				  strcat(fileheader, itoa(timestuff));
				  i++;
				}
			}



		}




		else if(strcmp(argv[i], "-v") == 0){
			verboseflag = 1;
		}
	
  
		else{
		        
			print_usage(argv);
			
			quit();
		}
	}   

}		


void quit()
{
	if(writing){
		fclose(fp);
	}
	exit(0);	
}

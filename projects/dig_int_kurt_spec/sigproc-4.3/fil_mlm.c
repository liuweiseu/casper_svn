#include <string.h>

/*
  fil_mlm  - convert raw data from a pulsar machine into "filterbank data"
  a stream of samples: [s1,c1] [s1,c2]...[s1,cn] [s2,c1]...[s2,cn]..... etc
  where the indices s1, s2.... are individual sample readouts and c1, c2.... 
  are the individual channels. For each readout, there are "n" channels.
  Before writing out a stream of samples, filterbank sends a header string
  which is intercepted by other programs (dedisperse, fold etc) downstream.
  (N.B. The header can be omitted using the -headerfile command-line option)
*/

#include "filterbank.h"

main (int argc, char *argv[])
{
  int   i,j,k,nfiles,fileidx,opened=0;
  char  message[80], chan_name[4]; 
  unsigned char fil_value;
  
  float fcentral, fpower;
  
  int          skip_packet_2=0;    /* skip packet number 2 for 1601 Hz problem */
  /* command-line changeable: */
  int          equalize_all=0;     /* equalize channels and telescopes */
  int          sumall=0;           /* sum all ibobs and channels to single output */
  char         file_prepend[120];


  int          ts_sec, ts_usec; /* packet timestamp seconds, microcseconds */
  
  /* set and clear arrays */
  strcpy(file_prepend, "");

  

  /* check number of command-line arguments */
  if (argc<2) {
    filterbank_help();
    exit(0);
  } else {
    print_version(argv[0],argv[1]);
  }
 
  /* print help if necessary */
  if (help_required(argv[1])) {
    filterbank_help();
    exit(0);
  }
 
  /* set up default global variables */
  hanning=hamming=zerolagdump=swapout=sumifs=headerless=headerfile=0;
  invert_band=clip_threshold=headeronly=0;
  time_offset=start_time=final_time=tstart=0.0;
  obits=-1;
  do_vanvleck=compute_spectra=1;
  strcpy(ifstream,"XXXX");

  /* work out how many files are on the command line */
  i=1;
  nfiles=0;
  while(file_exists(argv[i])) {
        nfiles++;
        i++;
  }
  if (!nfiles) error_message("no input files supplied on command line!");
  fileidx=1;

  /* now parse any remaining command line parameters */
  if (argc>nfiles) {
    i=nfiles+1;
    while (i<argc) {
      if (strings_equal(argv[i],"-o")) {
        /* get and open file for output */
        strcpy(outfile,argv[++i]);
        output=fopen(outfile,"wb");
        opened=1;
      /* ata filterbank options */
      } else if (strings_equal(argv[i],"-mjd")) {
          /* get the fractional start mjd */
          tstart=atof(argv[++i]);
      } else if (strings_equal(argv[i],"-eq")) {
        /* equalize channels and ibobs */
          equalize_all=1;
      } else if (strings_equal(argv[i],"-sumall")) {
        /* equalize channels and ibobs */
          sumall=1;
      } else if (strings_equal(argv[i],"-skip2")) {
        /* skip spectra 2 for funny 1pps stuff */
        skip_packet_2=1; 
      } else if (strings_equal(argv[i],"-prepend")) {
        strcpy(file_prepend,argv[++i]);
      } else {
        /* unknown argument passed down - stop! */
          filterbank_help();

        sprintf(message,"unknown argument (%s) passed to filterbank.",argv[i]);
        error_message(message);
      }
      i++;
    }
  }




  /* hard code some values for now */
  machine_id=9;    //-1
  telescope_id=9;  //-1
  data_type=1;
  nchans=1024;
  foff=104.8576/nchans * -1.0;        // XXX hack
  fcentral=1420.00;
  
  fch1=fcentral  - ((nchans/2)+0.5)*foff;    // A bit of a guess
  tsamp=0.00125;
  nifs=1;
  strcpy(ifstream,"YYYY");
  obits = 32;
  nbits = 32;

  /* open up input file */
  strcpy(inpfile,argv[1]);
  input=open_file(inpfile,"rb");


  /* broadcast the header */
  /* to the single output file */
  filterbank_header(output);

  
  while(fread(&fil_value, sizeof(char), 1, input)){ 
  	 fpower = (float) fil_value;
 	 fwrite(&fpower, sizeof(float), 1, output);	
	 //printf("%u %d %f \n", fil_value, fil_value, fpower);
  }

       

  update_log("finished");
  close_log();
  fclose(output);
  exit(0);
  fprintf(stderr,"Completed Successfully\n");

}



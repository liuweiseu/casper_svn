#include "setiread.h"


int main(int argc, char** argv)
{
    //reading data values 
    int fd;
    int ctr_numhits=0;
    int num_records=0;
    ssize_t numbytes;
    int compare = 1;
    off_t offset = 0;
    char error_buf[ERROR_BUF_SIZE];
    int headersize=DR_HEAER_SIZE;
    int next_buffer_size=1;
    int datasize=next_buffer_size;
    char *header = (char *) malloc(DR_HEAER_SIZE);
    char *data = (char *) malloc(MAX_DATA_SIZE);

    //plotting variables
    int maxbin = 0;
    int maxvalue = 0;
    int xmax = BINS;

    //data values
    int finebin = 0;
    int bytesread = 1;
    int value;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    unsigned int pfb_fft_power = 0;
	unsigned int fields;
    int64_t i = 0;
    int j = 0;
    int ctr = 0;
    int counter=0;
	int beamnum = 0;

	int64_t fileposition;   //position in input file
	int64_t filesize;   //position in input file

    //create buffers
    struct spectral_data spectra;

    struct data_vals *data_ptr;
    data_ptr = (struct data_vals *)data;

    // Initialize grace plotting windows
    grace_init();
//    grace_open_deux(8192);
//    grace_init_deux(maxbin,maxvalue,xmax);

    //check for error opening file
    fd = open(argv[1], O_RDONLY);

    if(fd==-1)
    {
        snprintf(error_buf, ERROR_BUF_SIZE, "Error opening file %s", argv[1]);
        perror(error_buf);
        return 0;
    }

    //if we can't find a header, nothing to be done
    if(find_first_header(fd)==-1) return 0;

    //read header
    //make sure we are reading as much data as expected, otherwise might be at eof
    while(headersize==DR_HEAER_SIZE && datasize==next_buffer_size)
    {
    

		/* make sure that we have at least DR_HEAER_SIZE before we read */
/*
    	do {
			 fileposition = lseek(fd, 0, SEEK_CUR);
			 //printf("position is %d\n", fileposition);
			 filesize = lseek(fd, 0, SEEK_END);
			 //printf("position is %d\n", filesize);		
			 lseek(fd, fileposition, SEEK_SET);
			 //printf("position is now %d\n", lseek(fd, fileposition, SEEK_SET) );				
			 if( (filesize - fileposition) < DR_HEAER_SIZE ) usleep(100000);    	
    	} while ( (filesize - fileposition) < DR_HEAER_SIZE );
*/
    	
        headersize = read(fd, (void *) header, DR_HEAER_SIZE);
	//get the size of spectra


        next_buffer_size = read_header(header);
        //in case we are at EOF

		/* make sure that we have at least next_buffer_size before we read */
/*
    	do {
			 fileposition = lseek(fd, 0, SEEK_CUR);
			 //printf("position is %d\n", fileposition);
			 filesize = lseek(fd, 0, SEEK_END);
			 //printf("position is %d\n", filesize);		
			 lseek(fd, fileposition, SEEK_SET);
			 //printf("position is now %d\n", lseek(fd, fileposition, SEEK_SET) );				
			 if( (filesize - fileposition) < next_buffer_size ) usleep(100000);    	
    	} while ( (filesize - fileposition) < next_buffer_size );
*/


        datasize = read(fd, (void *) data, next_buffer_size);
		
        //doesn't do any bounds checking yet...
        spectra.numhits = read_data(data, datasize) - 4096;
        beamnum = read_beam(data, datasize);


	//printf("size of spectra %d\n",spectra.numhits);
	//header,data
	data_ptr = (struct data_vals *) (data+SETI_HEADER_SIZE_IN_BYTES);

    //======================== TEST FILE DATA DUMP ================

/*
    FILE *datafile;
    char dataf[100];
    sprintf(dataf,"datafiles/datafile%d.dat",counter);
    datafile = fopen(dataf,"wb");
    fwrite(data,spectra.numhits,1,(FILE *)datafile);
    fflush(datafile);
    fclose(datafile);  
*/

		
    //==============================================

    num_records = read_data(data,datasize);
	ctr_numhits=0;
	
	for(i=0;i< num_records ;i++)
	{
	    
	    fields = ntohl(data_ptr->raw_data);
	    fft_bin = slice(fields,15,0);
	    pfb_bin = slice(fields,12,15);
	    over_thresh = slice(fields,1,27);
	    blank = slice(fields,3,28);
	    event = slice(fields,1,31);
	    
	    pfb_fft_power = ntohl(data_ptr->overflow_cntr); //32.0
//	    pfb_fft_power = data_ptr->overflow_cntr; //32.0


/*    
	    if(fft_bin > 16384.)
	    {
		fft_bin = fft_bin - 32768;
	    }
	    

	    if(pfb_bin > 2047)
	    {
		pfb_bin = pfb_bin-2048;
	    }
	    else
	    {
		pfb_bin = pfb_bin+2048;
	    }


	    finebin = fft_bin+32768*pfb_bin;
*/
	    //value = (float) (pfb_fft_power);

		//pfb_bin = endianSwap32(pfb_bin);
            pfb_bin = (pfb_bin + 2048) % 4096;
            fft_bin = ((fft_bin + 16384) % 32768) + pfb_bin*32768;
		    value = (int) (pfb_fft_power);
		//printf("%d %d %d %d %d %d\n", pfb_bin, fft_bin, pfb_fft_power, blank, event, over_thresh);



	    if(value < 1)
	    {
			value = 1;
	    }

	    if(value > maxvalue)
	    {
			maxvalue = value;
			maxbin = fft_bin;
	    }

	    //populate data structure
		
	    if(fft_bin%16384 != 0)
	    {
			spectra.hits[ctr_numhits][0] = value;  
			spectra.hits[ctr_numhits][1] = fft_bin;  
			//if(ctr_numhits == 0) printf("value %d fft_bin %d\n", spectra.hits[0][0], spectra.hits[0][1]);
			ctr_numhits++;
	    }
		
	    //populate coarse bin power
	    if(fft_bin%16384 == 0)
	    {
			spectra.coarse_spectra[pfb_bin] = value;
	    }


	    //header is the size of packet in bytes
		
	    data_ptr++;
	    ctr++;		
	}

//	GracePrintf("autoticks");
//	GracePrintf("redraw");
//	GracePrintf("updateall");
//	GracePrintf("kill g0.s0");
//	GracePrintf("saveall \"plots/sample%d.agr\"",counter);
//	grace_init_deux(maxbin,log10(maxvalue),xmax);	

	counter++;

	//HACK to fix power spectra off-by-one
	//for(i=0;i<4094;i++) spectra.coarse_spectra[i] = spectra.coarse_spectra[i+1];
	
    	plot_beam(&spectra,beamnum);
	
		//printf("num_records: %d spectra.nuhits %d beamnum: %d\n",num_records,spectra.numhits,beamnum);
		//usleep(200000);
    	GracePrintf("redraw");
	if(counter%14 == 0) {
	GracePrintf("updateall");
	printf("waiting...\n");
	usleep(10000000);
	}

	}


//=====================================================
/*
    // play with some text 
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

    sleep(5);

    //output pdf via grace

   GracePrintf("HARDCOPY DEVICE \"PDF\"");
   GracePrintf("PAGE SIZE 400, 400");
   GracePrintf("saveall \"sample.agr\"");
   GracePrintf("PRINT TO \"%s\"", "plot.pdf");
   GracePrintf("PRINT");

         if (GraceIsOpen()) {
                  //Flush the output buffer and close Grace 
                  GraceClose();
                  // We are done 
                  exit(EXIT_SUCCESS);
         } else {
                 exit(EXIT_FAILURE);
         }
   */ 
//===========================================================
    //close file

    close(fd);
    
    return 0;
}



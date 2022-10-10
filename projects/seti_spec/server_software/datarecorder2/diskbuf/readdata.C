#include "setiread.h"

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

int freq_bin(float freq,float band_begin,float band_end,float channels)
{
    int tone_bin;
    float a,b;
    b=(channels*band_begin)/(band_begin-band_end);
    a=-1*(b/band_begin);
    tone_bin=rnd(a*freq+b);
    return tone_bin;
}

int main()
{
    //reading data values 
    int fd;
    ssize_t numbytes;
    int compare = 1;
    off_t offset = 0;
    char error_buf[ERROR_BUF_SIZE];
    int headersize=DR_HEAER_SIZE;
    int next_buffer_size=1;
    int datasize=next_buffer_size;
    char *header = (char *) malloc(DR_HEAER_SIZE);
    char *data = (char *) malloc(MAX_DATA_SIZE);
    fd = open("./largefile0", O_RDONLY);

    //plotting variables
    int maxbin = 0;
    int maxvalue = 0;
    int xmax = BINS;
    int tone_bin = freq_bin(175.0,100.0,300.0,32768.*4096.);
    printf("%f\n",tone_bin);

    //data values
    int finebin = 0 ;
    int bytesread = 1;
    double value;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    int pfb_fft_power = 0;
    long int i = 0;
    int j = 0;
    int ctr = 0;

    int counter=0;

    //create buffers
    struct spectral_data spectra;

    struct data_vals *data_ptr;
    data_ptr = (struct data_vals *)data;

    // Initialize grace plotting windows
    //grace_init();
    grace_open_deux(8192);
    grace_init_deux(maxbin,maxvalue,xmax);

    //check for error opening file
    if(fd==-1)
    {
        snprintf(error_buf, ERROR_BUF_SIZE, "Error opening file %s", FILENAME);
        perror(error_buf);
        return 0;
    }

    //if we can't find a header, nothing to be done
    if(find_first_header(fd)==-1) return 0;

    //read header
    //make sure we are reading as much data as expected, otherwise might be at eof
    while(headersize==DR_HEAER_SIZE && datasize==next_buffer_size)
    {
        headersize = read(fd, (void *) header, DR_HEAER_SIZE);
	//get the size of spectra
        next_buffer_size = read_header(header);
        //in case we are at EOF
        datasize = read(fd, (void *) data, next_buffer_size);

        //doesn't do any bounds checking yet...
        spectra.numhits = read_data(data, datasize);

	//printf("size of spectra %d\n",spectra.numhits);
	//header,data
	data_ptr = (struct data_vals *) (data+SETI_HEADER_SIZE_IN_BYTES);

    //======================== TEST FILE DATA DUMP ================

    FILE *datafile;
    char dataf[100];
    sprintf(dataf,"datafiles/datafile%d",counter);
    datafile = fopen(dataf,"wb");
//    fwrite(data,spectra.numhits,1,(FILE *)datafile);
    fflush(datafile);
    fclose(datafile);  

    //==============================================

	for(i=0;i<spectra.numhits;i++)
	{
	    
	    U32 fields = ntohl(data_ptr->raw_data);
	    fft_bin = slice(fields,15,0);
	    pfb_bin = slice(fields,12,15);
	    over_thresh = slice(fields,1,27);
	    blank = slice(fields,3,28);
	    event = slice(fields,1,31);
	    pfb_fft_power = ntohl(data_ptr->overflow_cntr); //32.0
    
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
	    value = (float) (pfb_fft_power);
	    //value = (int) (pfb_fft_power);

	    if(value < 1)
	    {
		value = 1;
	    }

	    if(value > maxvalue)
	    {
		maxvalue =value;
		maxbin = fft_bin;
	    }

	    //populate data structure

	    if(fft_bin != 0)
	    {
		spectra.hits[i][0] = value;  
		spectra.hits[i][1] = finebin;  
	    }

	    //populate coarse bin power
	    if(fft_bin == 0)
	    {
		spectra.coarse_spectra[i] = finebin;
	    }

	    //test plotting ===================================


	    if(fft_bin != 0)
            {
                //black
                GracePrintf("g0.s0 point %d, %f",finebin,value);
                //GracePrintf("g0.s0 point %d, %d",spectra.hits[i][1],spectra.hits[i][0]);
                //GracePrintf("subtitle \"Test signal at %f MHz\"",tone);
            }

            if(fft_bin == 0)
            {

                //white
                GracePrintf("g0.s1 point %d, %f",finebin,value);
                //GracePrintf("g0.s1 point %d, %d",spectra.hits[i][1],spectra.coarse_spectra[i]);

            }

	    //=================================================

	    //header is the size of packet in bytes

	   //look at fine bins
	    if(abs(tone_bin-finebin) < 320200)
	    {
		printf("delta bin: %d\n",finebin-tone_bin);
		printf("fft bin: %d\n",fft_bin);
		printf("bin: %d\n",finebin);
		printf("freq: %3.10f power: %f\n",100.+200.0/(32768.*4096.)*finebin,value);
	    }
		
	    data_ptr++;
	    ctr++;		
	}

	printf("-----------------------------------------\n");
	GracePrintf("autoticks");
	GracePrintf("redraw");
	GracePrintf("updateall");
	GracePrintf("kill g0.s0");
	GracePrintf("kill g0.s1");
	grace_init_deux(maxbin,log10(maxvalue),xmax);	

	counter++;
//	plot_beam(&spectra,(i % 14));
	usleep(100000);


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

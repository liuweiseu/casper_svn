#include "setiread.h"




int finish_up(gsl_histogram *sigma_histogram, gsl_histogram *freq_histogram) {

	FILE *fp;
	int k=0;
	
	fp = fopen("out_sigma.txt", "w");
	for(k=0;k<10000;k++) {
			fprintf(fp, "%f %f\n", (1.0 +  (((double) k) * 4.0/10000.0) + (4.0/20000)), gsl_histogram_get(sigma_histogram, k));
	}
	fclose(fp);


	fp = fopen("out_freq.txt", "w");
	for(k=0;k<8192;k++) {
			fprintf(fp, "%f %f\n", (100.0 +  (((double) k) * 200.0/8192) + (200.0/16384)), gsl_histogram_get(freq_histogram, k));
	}
	fclose(fp);



}


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
	char *temp;
	const char nullchar = 0;
    //plotting variables
    int maxbin = 0;
    int maxvalue = 0;
    int xmax = BINS;

    //data values
    int finebin = 0 ;
    int bytesread = 1;
    int value;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    unsigned int pfb_fft_power = 0;
	unsigned int fields;
    long int i = 0;
    int j = 0;
    int ctr = 0;
    int counter=0;
	int beamnum = 0;

	int agc_systime, agc_time;
	double agc_az, agc_za, agc_lst, agc_ra, agc_dec;
	
	int if1_systime; 
	double if1_syni_freqhz_0; 
	int if1_syni_ampdb_0; 
	double if1_rffreq; 
	double if1_iffrqmhz; 
	int if1_alfafb; //filter?
	

	/*record to start on, record to end on */
    int recstart =0;
    int recend=0;
    
	long int fileposition;   //position in input file
	long int filesize;   //position in input file

    //create buffers
    struct spectral_data spectra;

    struct data_vals *data_ptr;
    data_ptr = (struct data_vals *)data;


	float hit_image[4096];

	
	
	//output file
	FILE *fp;

	FILE *metafile;

	FILE *imagefile;
	
	gsl_histogram *sigma_histogram = gsl_histogram_alloc (10000);
	gsl_histogram_set_ranges_uniform (sigma_histogram, 1.0, 5.0);

	gsl_histogram *freq_histogram = gsl_histogram_alloc (12288);
	gsl_histogram_set_ranges_uniform (freq_histogram, 1225.0, 1525.0);



     //open output file

	fp = fopen(argv[4], "w"); 
     
    //check for error opening file
    fd = open(argv[1], O_RDONLY);


	metafile = fopen("metafile.txt", "w");
	imagefile = fopen("imagefile.txt","w");


    if(fd==-1)
    {
        snprintf(error_buf, ERROR_BUF_SIZE, "Error opening file %s", argv[1]);
        perror(error_buf);
        return 0;
    }

	recstart = atoi(argv[2]);
	recend = atoi(argv[3]);


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

/*
		// make sure that we have at least next_buffer_size before we read 
    	do {
			 fileposition = lseek(fd, 0, SEEK_CUR);
			 printf("position is %d\n", fileposition);
			 filesize = lseek(fd, 0, SEEK_END);
			 printf("position is %d\n", filesize);		
			 lseek(fd, fileposition, SEEK_SET);
			 printf("position is now %d\n", lseek(fd, fileposition, SEEK_SET) );				
			 if( (filesize - fileposition) < next_buffer_size ) usleep(100000);    	
    	} while ( (filesize - fileposition) < next_buffer_size );
*/

        
        datasize = read(fd, (void *) data, next_buffer_size);
		if(!(headersize==DR_HEAER_SIZE && datasize==next_buffer_size)) {
			
			printf("EOF Total Spectra: %d\n", counter);
			return 0;		
        }
        //doesn't do any bounds checking yet...
        spectra.numhits = read_data(data, datasize) - 4096;
        beamnum = read_beam(data, datasize);
		for(i=0;i<4096;i++) spectra.coarse_spectra[i] = 0;
		for(i=0;i<4096;i++) hit_image[i] = 0.0;

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
	if(counter >= recend) {
		finish_up(sigma_histogram, freq_histogram);
		return 0;
	}
	
	if(counter >= recstart && counter <= recend){
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
		
		
		
					pfb_bin = (pfb_bin + 2048) % 4096;
					value = (int) (pfb_fft_power);

					//populate coarse bin power
			
					if(fft_bin == 0)
					{
						spectra.coarse_spectra[pfb_bin] = value;
					}


				
						
				//populate data structure
				
				if(fft_bin != 0)
				{
					spectra.hits[ctr_numhits][0] = value;  
					spectra.hits[ctr_numhits][1] = ((fft_bin + 16384) % 32768) + pfb_bin*32768;
					spectra.flags[ctr_numhits][0] = over_thresh;
					spectra.flags[ctr_numhits][1] = blank;
					spectra.flags[ctr_numhits][2] = event;

					hit_image[pfb_bin] = hit_image[pfb_bin] + (24.0 * (double) ( (spectra.hits[ctr_numhits][0]) /  (double) spectra.coarse_spectra[ (int) spectra.hits[ctr_numhits][1] / 32768]));
					
					//if(ctr_numhits == 0) printf("value %d fft_bin %d\n", spectra.hits[0][0], spectra.hits[0][1]);
					ctr_numhits++;
				}
				
		
		
				//header is the size of packet in bytes
				
				data_ptr++;
				ctr++;		
			}

     
		   //HACK to fix power spectra off-by-one - not needed, fixed
		   //for(i=0;i<4094;i++) spectra.coarse_spectra[i] = spectra.coarse_spectra[i+1];
		   
	   
		   for(i=0;i<4094;i++) if(header[i] == 0) header[i] = 10;   //my strtok code didn't like nulls
	   
		   //for(i=0;i<4094;i++) printf("%c", header[i]);
	   
	   
			  temp = strtok(header, "\n");
			  while(temp != NULL)
			  {
				  i = sscanf(temp, "SCRAM AGC AGC_SysTime %d AGC_Az %lf AGC_Za %lf AGC_Time %d AGC_LST %lf AGC_Ra %lf AGC_Dec %lf", &agc_systime, &agc_az, &agc_za, &agc_time, &agc_lst, &agc_ra, &agc_dec);
				  i = sscanf(temp, "SCRAM IF1 IF1_SysTime %d IF1_synI_freqHz_0 %lf IF1_synI_ampDB_0 %d IF1_rfFreq %lf IF1_if1FrqMhz %lf IF1_alfaFb %d", &if1_systime, &if1_syni_freqhz_0, &if1_syni_ampdb_0, &if1_rffreq, &if1_iffrqmhz, &if1_alfafb);
	   
				  //if(strlen(temp) > 2) printf("%s\n", temp);
				  
				  temp = strtok(NULL, "\n"); 
			  }
	   			/*	 
				   printf("AGC_SysTime: %d", agc_systime);
				   printf("\n");
				   printf("AGC_Az: %f", agc_az);
				   printf("\n");
				   printf("AGC_Za: %f", agc_za);
				   printf("\n");
				   printf("AGC_Time: %d", agc_time);
				   printf("\n");
				   printf("AGC_LST: %f", agc_lst);
				   printf("\n");
				   printf("AGC_Ra: %f", agc_ra);
				   printf("\n");
				   printf("AGC_Dec: %f", agc_dec);
				   printf("\n");
	   			*/

			  // print_fine(&spectra,beamnum);
			  // print_coarse(&spectra,beamnum);
		       fprintf(metafile, "%d %lf %lf %d %lf %d\n", counter, agc_az, agc_za, agc_systime, if1_rffreq, if1_alfafb, if1_syni_freqhz_0);
	
				
	
				for(i=0;i<4096;i++) fprintf(imagefile, "%d %f %f\n", counter%500, ( (float) if1_syni_freqhz_0 - (100.00 - (200.00 / 8192))) - (200.0/8192) - ((float) i * 200.0 / 4096), hit_image[i]);
				fprintf(imagefile, "\n");
				
				print_errors(&spectra,beamnum, counter);
			   
			   //for(i=0;i<4096;i++) fits_spectra[i][counter] = spectra.coarse_spectra[i];
		
				//spectra_ctr++;
				//for(i=0;i<4096;i++) image_data[(4096 * spectra_ctr) + i] = spectra.coarse_spectra[i]
		
			   increment_histograms(sigma_histogram, freq_histogram, &spectra,beamnum,counter, (if1_syni_freqhz_0 - (100.00 - (200.00 / 8192))) );
			   print_fine_normalized_file(fp, &spectra,beamnum,counter, (if1_syni_freqhz_0 - (100.00 - (200.00 / 8192))) );
	   			//printf("counter: %d numhits: %d filter: %d headersize: %d\n", counter, ctr_numhits, if1_alfafb, headersize);
			      
	}


	counter++;


    }
    



    close(fd);
    
			printf("EOF Total Spectra: %d\n", counter);
    return 0;
}




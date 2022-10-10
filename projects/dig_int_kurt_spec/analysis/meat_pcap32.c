#include <stdio.h>
#include <string.h>

/*
Mars Lightning pcap processor - read in a mars lightning pcap capture and output usable data

syntax: svv_pcap_proc <filename> 
A. Siemion
siemion@berkeley.edu
*/

static inline unsigned short bswap_16(unsigned short x) {
  return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
  return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
  return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}


/* extract variable bit length values from a 32bit word */
unsigned int slice32(unsigned int value,unsigned int width,unsigned int offset)
{
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}

/* extract variable bit length values (32 bit max!) from a 64bit word */
unsigned int slice64(unsigned long int value,unsigned int width,unsigned int offset)
{
    value = value << (64 - (width + offset));
    value = value >> (64 - width);
    return value;
}


main (int argc, char *argv[])
{
  FILE *input, *xpowerfile, *ypowerfile, *xpowersqfile, *ypowersqfile;
  int   i,j,k,l;
  char file[120];


  char payload[100000];
  unsigned int bump, packet_len, packet_saved_len, ip, s;
  long int          ts_sec, ts_usec; /* packet timestamp seconds, microcseconds */

  /* number of clock cycles between accumulations */
  unsigned long int accum = 131072;

  int bytesread = 1;
  unsigned char xpolpower[1024];
  unsigned char ypolpower[1024];
  unsigned short xpolsquared[1024];
  unsigned short ypolsquared[1024];

  unsigned int value=0;
  signed char crossreal=0;
  signed char crossimag=0;  
  int dropped_packet_cnt = 0;
  int speccnt = 0;
  int err=0;
  unsigned long int fields = 0;
  unsigned long int pkt_index=0;
  unsigned long int pkt_index_prev=0;
  /* check number of command-line arguments */
  if (argc<3) {
	    fprintf(stderr, "usage is: %s <input.pcap> <output_prefix>\n", argv[0]);
	    return(0);
  } else {

  }
  
  

    /* open up input file */
    strcpy(file,argv[1]);
    input=fopen(file,"rb");

  
  	/* open up output files */
    
    sprintf(file,"%s_xpower.bin", argv[2]);
    xpowerfile=fopen(file,"wb");

    sprintf(file,"%s_ypower.bin", argv[2]);
    ypowerfile=fopen(file,"wb");

    sprintf(file,"%s_xpowersq.bin", argv[2]);
    xpowersqfile=fopen(file,"wb");

    sprintf(file,"%s_ypowersq.bin", argv[2]);
    ypowersqfile=fopen(file,"wb");
    




    /* bump past pcap global header */
    bump = fread(&payload, sizeof(char), 24, input);

	//printf("size of unsigned long int is %ld and should be 4\n\n", sizeof(unsigned long long int));

do {
            bump = fread(&payload, sizeof(char), 8, input); /* skip 8 bytes */

			/* these should be equal, but sometimes tcpdump/gulp/etc doesn't work quite right? */
            bump = fread(&packet_saved_len, sizeof(int), 1, input);           
            bump = fread(&packet_len, sizeof(int), 1, input);            
                 //printf("packet len is %d\n", packet_len);
                 //if we received a 4138 byte packet, its probably a bee2 UDP event dump
                 if((packet_len == packet_saved_len) && (packet_len == 8242) && (bump != 0)) {
 					
 					speccnt++;
 					bump = fread(&payload, sizeof(char), packet_len, input);
				
					 
					/* skip past ip and udp headers, read event records */ 
					memcpy(&pkt_index, payload+42, 8);

					/* first 64 bits is the clock cycle counter value */
					pkt_index = bswap_64(pkt_index);
					
					//fprintf(stderr, "packet index delta: %ld\n", (pkt_index - pkt_index_prev)/accum);
					if( (pkt_index_prev != 0) && ((pkt_index - pkt_index_prev) != accum)){
               				
					dropped_packet_cnt += ((pkt_index - pkt_index_prev)/accum - 1);
					err = ((pkt_index - pkt_index_prev)/accum - 1);
					for(l = 0; l < ((pkt_index - pkt_index_prev)/accum - 1);l++){
				                
						for(k=512;k<1024;k++){
                        	                          fwrite(xpolpower + k, sizeof(char), 1, xpowerfile);
                                	                  fwrite(ypolpower + k, sizeof(char), 1, ypowerfile);
                                        	          fwrite(xpolsquared + (k), sizeof(short), 1, xpowersqfile);
                                               		  fwrite(ypolsquared + (k), sizeof(short), 1, ypowersqfile);
                                         	}

                                         	for(k=0;k<512;k++){
                                                	  fwrite(xpolpower + k, sizeof(char), 1, xpowerfile);
                                                	  fwrite(ypolpower + k, sizeof(char), 1, ypowerfile);
                                                  	  fwrite(xpolsquared + (k), sizeof(short), 1, xpowersqfile);
                                                  	  fwrite(ypolsquared + (k), sizeof(short), 1, ypowersqfile);
                                         	}

					 }
					   printf("dropped pkts: %d at spec count (strts at 1): %d (corrected)\n", err,speccnt);
					
					 }
					
					pkt_index_prev = pkt_index;					
					for (j = (8242 - 8192);j<8242;j=j+8){
						  //copy one event recorc from the payload						 
						  memcpy(&fields, payload + j, 8);
						  
						  
						  xpolpower[(j - (8242 - 8192))/8] = (unsigned char) slice64(fields,8,0);
						  ypolpower[(j - (8242 - 8192))/8] = (unsigned char) slice64(fields,8,24);
						  xpolsquared[(j - (8242 - 8192))/8] = (unsigned short) bswap_16(slice64(fields,16,32));
 						  ypolsquared[(j - (8242 - 8192))/8] = (unsigned short) bswap_16(slice64(fields,16,48));

						  //extract record components
						  //event = slice(fields,1,31);
							
						  //pfb_fft_power = ntohl(pfb_fft_power);
						  
						  /*
						  hex dump
						  */
						  
						  
						  //fwrite(&xpolpower, sizeof(char), 1, xpowerfile);
						  //fwrite(&ypolpower, sizeof(char), 1, ypowerfile);
						  //fwrite(&xpolsquared, sizeof(short), 1, xpowersqfile);
						  //fwrite(&ypolsquared, sizeof(short), 1, ypowersqfile);

 			             //printf("X: %u Y: %u X2: %hu Y2: %hu \t\t", xpolpower[(j - (8242 - 8192))/8], ypolpower[(j - (8242 - 8192))/8], xpolsquared[(j - (8242 - 8192))/8], ypolsquared[(j - (8242 - 8192))/8]);

						  /*						  
						  printf("X: 0x%x Y: 0x%x X2: %hu Y2: %hu \t\t", xpolpower, ypolpower, xpolsquared, ypolsquared); 

						  printf("0x%02X", (unsigned char) payload[j]);
						  printf("%02X", (unsigned char) payload[j+1]);
						  printf("%02X", (unsigned char) payload[j+2]);
						  printf("%02X", (unsigned char) payload[j+3]);						  
						  printf("%02X", (unsigned char) payload[j+4]);
						  printf("%02X", (unsigned char) payload[j+5]);
						  printf("%02X", (unsigned char) payload[j+6]);
						  printf("%02X\n", (unsigned char) payload[j+7]);
						  */
						  
						  //printf("0x%16X\n", (unsigned long int) fields);
						  
						  /* CASPER ffts output positive frequencies first        	*/
						  /* after this transformation, bin indexes will be 	 	*/
						  /* sensible (bottom of IF = bin 0, top of IF = bin N-1 	*/
						  
					      //fft_bin = (fft_bin + 16384) % 32768;
						  //pfb_bin = (pfb_bin + 2048) % 4096;

						  
						  //printf("pfb_bin is %u - fft bin is %u - power is %u\n", pfb_bin, fft_bin, pfb_fft_power);

						  /* output comma delimited pfb_bin, fft_bin, power */
						  //printf("%u, %u, %u\n", pfb_bin, fft_bin, pfb_fft_power);					 	
					 }
					
					 //fft_bin = (fft_bin + 512) % 1024;
					 
					 //Reorder frequency bins in regular order (low frequencies first)
					 for(k=512;k<1024;k++){
						  fwrite(xpolpower + k, sizeof(char), 1, xpowerfile);
						  fwrite(ypolpower + k, sizeof(char), 1, ypowerfile);
						  fwrite(xpolsquared + (k), sizeof(short), 1, xpowersqfile);
						  fwrite(ypolsquared + (k), sizeof(short), 1, ypowersqfile);
					 }					
					 
					 for(k=0;k<512;k++){
						  fwrite(xpolpower + k, sizeof(char), 1, xpowerfile);
						  fwrite(ypolpower + k, sizeof(char), 1, ypowerfile);
						  fwrite(xpolsquared + (k), sizeof(short), 1, xpowersqfile);
						  fwrite(ypolsquared + (k), sizeof(short), 1, ypowersqfile);
					 }					


                 } else if ( bump !=0 ){
                    //skip this packet 
					//fprintf(stderr, "pcap file contains a non-event-record packet - trying to skip\n");
					bump = fread(&payload, sizeof(char), packet_saved_len, input);
					//fprintf(stderr, "got: %d - %d\n", packet_len, packet_saved_len);

                }   
} while (bump != 0);

/*
If you want to grab the packet time stamp, you might do something like this...

    bump = fread(&payload, sizeof(char), 4, input);
      ts_sec = (((unsigned int) payload[0])%256);
      ts_sec += (((unsigned int) payload[1])%256) * 256;
      ts_sec += (((unsigned int) payload[2])%256) * 65536;
      ts_sec += (((unsigned int) payload[3])%256) * 16777216;    

    bump = fread(&payload, sizeof(char), 4, input);
      ts_usec = (((unsigned int) payload[0])%256);
      ts_usec += (((unsigned int) payload[1])%256) * 256;
      ts_usec += (((unsigned int) payload[2])%256) * 65536;
      ts_usec += (((unsigned int) payload[3])%256) * 16777216;
*/




  /*fclose(output);*/
  fprintf(stderr,"Completed Successfully\n");
  fprintf(stderr, "Spectra count: %d\n", speccnt);
  fprintf(stderr, "Dropped packet count: %d\n", dropped_packet_cnt);

  
  fclose(xpowerfile);
  fclose(ypowerfile);
  fclose(xpowersqfile);
  fclose(ypowersqfile);

  return(0);

}

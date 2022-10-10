#include <stdio.h>
#include <string.h>

/*
SERENDIP V.v pcap processor - read in a serendip v.v pcap capture and output some potentially useful
diagnostics.

syntax: svv_pcap_proc <filename> 
A. Siemion
siemion@berkeley.edu
*/


/* extract variable bit length values from a 32bit word */
unsigned int slice(unsigned int value,unsigned int width,unsigned int offset)
{
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}


main (int argc, char *argv[])
{
  FILE *input;
  int   i,j,k;
  char inpfile[120];


  char payload[100000];
  unsigned int bump, packet_len, packet_saved_len, ip, s;
  int          ts_sec, ts_usec; /* packet timestamp seconds, microcseconds */
  

  int finebin = 0 ;
  int bytesread = 1;
  int value;
  int pfb_bin = 0;
  int fft_bin = 0;
  int over_thresh = 0;
  int blank = 0;
  int event = 0;
  unsigned int pfb_fft_power = 0;
  unsigned int fields = 0;


  /* check number of command-line arguments */
  if (argc<2) {
    return(0);
  } else {

  }
  
  


//  strcpy(inpfile,argv[fileidx]);
//  input=open_file(inpfile,"rb");


    /* open up input file */
    strcpy(inpfile,argv[1]);
    input=fopen(inpfile,"rb");


    /* bump past pcap global header */
    bump = fread(&payload, sizeof(char), 24, input);

	printf("size of unsigned int is %ld and should be 4\n\n", sizeof(unsigned int));

do {
            bump = fread(&payload, sizeof(char), 8, input); /* skip 12 bits */

			/* these should be equal, but sometimes tcpdump/gulp/etc doesn't work quite right? */
            bump = fread(&packet_saved_len, sizeof(int), 1, input);           
            bump = fread(&packet_len, sizeof(int), 1, input);            
                 
                 //if we received a 4138 byte packet, its probably a bee2 UDP event dump
                 if((packet_len == packet_saved_len) && (packet_len == 4138)) {
 
 					bump = fread(&payload, sizeof(char), packet_len, input);
				
					 
					/* skip past ip and udp headers, read event records */ 
					for (j = (4138 - 4096);j<4138;j=j+8){

						  //copy one event recorc from the payload						 
						  memcpy(&fields, payload + j, 4);
						  memcpy(&pfb_fft_power, payload + j + 4, 4);
						  
						  fields = ntohl(fields);
						  
						  //extract record components
						  fft_bin = slice(fields,15,0);
						  pfb_bin = slice(fields,12,15);
						  over_thresh = slice(fields,1,27);
						  blank = slice(fields,3,28);
						  event = slice(fields,1,31);
							
						  pfb_fft_power = ntohl(pfb_fft_power);
						  
						  /*
						  hex dump
						  printf("0x%02x\n", (unsigned int) payload[j+4]);
						  printf("0x%02x\n", (unsigned int) payload[j+5]);
						  printf("0x%02x\n", (unsigned int) payload[j+6]);
						  printf("0x%02x\n", (unsigned int) payload[j+7]);
						  */
						  
						  /* CASPER ffts output positive frequencies first        	*/
						  /* after this transformation, bin indexes will be 	 	*/
						  /* sensible (bottom of IF = bin 0, top of IF = bin N-1 	*/
						  
					      fft_bin = (fft_bin + 16384) % 32768;
						  pfb_bin = (pfb_bin + 2048) % 4096;

						  
						  //printf("pfb_bin is %u - fft bin is %u - power is %u\n", pfb_bin, fft_bin, pfb_fft_power);

						  /* output comma delimited pfb_bin, fft_bin, power */
						  printf("%u, %u, %u\n", pfb_bin, fft_bin, pfb_fft_power);
					 	
					 }
					

                 } else {
                    //skip this packet 
					printf("pcap file contains a non-event-record packet - trying to skip\n");
					bump = fread(&payload, sizeof(char), packet_saved_len, input);
					printf("got: %d - %d\n", packet_len, packet_saved_len);

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
  return(0);

}

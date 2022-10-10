/* #pragma ident "@(#)check_gap_ss.c	1.7 07/20/04 EDT" */
/*
 * Jenny Thai
 * 10/08/02
 * check_gap.c
 *
 * This program performs continuous input from edt dma device
 * and optionally reads in different size of memory buffer.
 * The program keeps track of the number of gaps from the input
 * and the result of throughput is printed out with the table
 * of frequency.
 *
 * INPUT: check_gap -l <loop_size> or check_gap -F <clock_speed>
 */

#include "edtinc.h"
#include <stdlib.h>
#include <fcntl.h>

float clock_speed = 66000000;

#define MAXFREQ 100000

#define MAXCHANNELS 4

#define PCITESTSS	0x02010010	/* zeros shifted data if bit set */


typedef struct {

	u_int freqs[MAXFREQ];
	u_int blocks[MAXFREQ];

	u_int last_index;
	double t_xferred;
	double total_clocks;

	int started;

} GapHistory;


void InitGaps(GapHistory *pgaps)
{

	if (pgaps)
	{
		memset(pgaps, 0, sizeof(*pgaps));

	}
}
/*
 * count_gap
 *
 * count_gap takes in an array of unsigned integer buffer and keep
 * track of the number of gaps in the array
 */
int 
count_gap(u_int *buf,
			  u_int size,
			  GapHistory *pgaps,
			  char *textout)

{
	/*Declaring variables*/
	unsigned int lastvalue, newvalue;
	unsigned int firstvalue;
		FILE *fout = NULL;
		unsigned int blockstart;

	int xferred = 0;
	int index = 0;
	int runlength = 1;
	int clock = 0, gaplength;

	int num_of_element = size;

		lastvalue = buf[0];

	firstvalue = lastvalue;
		blockstart = lastvalue;
	index = 1;

		if (textout)
			fout = fopen(textout, "w+");

		if (fout)
			fprintf(fout,"Start Value %d\n",firstvalue);

	/*loops through until end of buffer*/
	while (index < num_of_element)
	{
	   newvalue = buf[index];		    /*Read in new value*/
		   if (newvalue < lastvalue)
		   {
				/*wraparound */
				gaplength = 0xffffffff - lastvalue + newvalue;

		   }
		   else
		   {
			   gaplength = newvalue - lastvalue;
		   }

	   /*Out of range*/
	   if (gaplength < 0 || gaplength >= MAXFREQ)
		   {
			   gaplength = MAXFREQ-1;
		   }

	   /*Print out index,runlength, and gaplength*/
	   if (gaplength > 1)
	   {
		 xferred += (runlength * 4);	   /*Number of bytes transferred*/
				 pgaps->blocks[runlength]++;
				 pgaps->freqs[gaplength-1]++;

				 if (fout)
				 {
					 fprintf(fout, "Block %8d -> %8d size %8d\n",
								blockstart, lastvalue, runlength);
					 fprintf(fout, "Gap   %8d -> %8d size %8d\n",
						 lastvalue+1, newvalue-1,gaplength-1);
				 }

				 blockstart = newvalue;

		 runlength = 1;
				 clock += gaplength+1;

	   }
	   else if (gaplength == 1)
	   {
				runlength ++;
				clock++;
		   }

	   index++;
	   lastvalue = newvalue;
	}

		/* last block */
	 xferred += (runlength * 4);	   /*Number of bytes transferred*/
		 pgaps->blocks[runlength]++;

		 if (fout)
		 {
			 fprintf(fout, "Block %8d -> %8d size %8d\n",
						blockstart, lastvalue, runlength);
			 fprintf(fout, "Gap   %8d -> %8d size %8d\n",
				 lastvalue+1, newvalue-1,gaplength-1);
		 }

		clock += gaplength+1;

	pgaps->last_index += index;			/*Keep track of data*/
	pgaps->t_xferred += xferred;
	pgaps->total_clocks += clock;

		if (fout)
			fclose(fout);

	return 0;
}
/*
 * output_gap
 *
 * output_gap takes in total length, total bytes transferred, total clock,
 * and frequency to display the result to standard output
 */
int 
output_gap(GapHistory *pgaps)
{
	int i;
	double t;
	int totalgaps = 0;


	printf("Transferred:  %10f bytes\n", pgaps->t_xferred);
	printf("Clocks: %10f %d \n", pgaps->total_clocks);

	t = pgaps->total_clocks/clock_speed;

	printf("Bytes per second: %10f \n\n", (pgaps->t_xferred/t));

	/*Print frequency result table*/
	for (i = 0; i < MAXFREQ; i++)
	{
	   if (pgaps->blocks[i])
		  printf("\t Block of: %d \t Frequency: %d\n", i, pgaps->blocks[i]);
	}
	/*Print frequency result table*/

	for (i = 0; i < MAXFREQ; i++)
	{
	   if (pgaps->freqs[i])
		  printf("\t Gap of: %d \t Frequency: %d\n", i, pgaps->freqs[i]);
	   totalgaps += pgaps->freqs[i];
	}
	printf("Total # of gaps: %d\n", totalgaps);

	return 0;
}

/*Display default message*/
void command_error();


GapHistory gaps[MAXCHANNELS];

/* main program starts */
int 
main(int argc, char **argv)
{
  EdtDev *edt_p[MAXCHANNELS];			/*Pointer for device handle*/


  int t_index = 0;
  int t_xferred = 0;
  int t_clock = 0;
  int sunswap = 0;

  int bufsize = 1024*1024;
  int unit = 0, i, loops=1;
	int nchannels = 1;
	int channel;

	int finished_count[MAXCHANNELS];

	char *rawout = NULL;
	char *textout = NULL;
    int bitload = 1;
    char *bitfilename ;

  while (argc > 1 && argv[1][0] == '-')
  {
     switch (argv[1][1])
     {
	case 'u':			/*specified unit number*/
	    ++argv;
	    --argc;
	    unit = atoi(argv[1]);
	    break;

	case 'F':			/*specified clock speed*/
	    ++argv;
	    --argc;
	    clock_speed = (float) atof(argv[1]);
	    break;

	case 'l':			/*specified number of loops*/
	    ++argv;
	    --argc;
	    loops = atoi(argv[1]);
	    break;

	case 'n':
		++argv;
		--argc;
		nchannels = atoi(argv[1]);
		if (nchannels > MAXCHANNELS)
			nchannels = MAXCHANNELS;
		break;

	case 's':
		++argv;
		--argc;
		bufsize = strtol(argv[1],0,0);
		break;

	case 'S':
		sunswap = 1;
		break;

	case 'o':
		++argv;
		--argc;
		if (argc > 0)
			rawout = argv[1];
		break;

	case 't':
		++argv;
		--argc;
		if (argc > 0)
			textout = argv[1];
		break;

	case 'B':
		bitload = !bitload;
		break;

	default:
	    command_error();
	    exit(1);
	    break;
     }
	 --argc;
	 ++argv;
  }

  if (bitload)
  {
	char cmd[256];

	if ((edt_p[0] = edt_open(EDT_INTERFACE, unit)) == NULL)
	{
	    edt_perror("edt_open pcd0");
	    exit(1);
	}

	if (edt_device_id(edt_p[0]) == PSS4_ID)
	    bitfilename = "pciss1test.bit";
	else if (edt_device_id(edt_p[0]) == PSS16_ID)
	    bitfilename = "pciss16test.bit";
	else
	{
	    fprintf(stderr, "Illegal devid %d; must be PSS4_ID or PSS16_ID\n", edt_device_id(edt_p[0])) ;
	}
	edt_close(edt_p[0]) ;

	sprintf(cmd,"bitload -u %d %s\n", unit, bitfilename);
	system(cmd);
  }

  for (channel = 0; channel < nchannels ; channel++)
  {

	InitGaps(&gaps[channel]);
  }

  for (channel=0;channel < nchannels;channel++)
  {

	  if ((edt_p[channel] = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL){
		perror("edt_open");
		exit(1);
	  }

	  if (channel == 0)
	  {
	      u_char cmd = edt_reg_read(edt_p[0], PCD_CMD) ;
	      cmd |= 0x08 ;
	      edt_reg_write(edt_p[0], PCD_CMD, cmd) ;

	      if (edt_device_id(edt_p[0]) == PSS4_ID)
		  edt_reg_write(edt_p[0],PCITESTSS,0x7ff);
	      else
		  edt_reg_write(edt_p[0], SSD16_CHEN, 0xffff) ;
	  }

	  if (edt_configure_ring_buffers(edt_p[channel], bufsize, loops, EDT_READ, NULL) == -1){
		perror("edt_configure_ring_buffers");
		exit(1);
	  }

  }

  /* set byte swap and short swap if this is a Sun */
  if (sunswap)
	  edt_reg_write(edt_p[0], PCD_CONFIG, PCD_BYTESWAP | PCD_SHORTSWAP);

  edt_flush_fifo(edt_p[0]) ;

  for (channel=0;channel < nchannels;channel++)
  {
		edt_start_buffers(edt_p[channel], loops);
  }


  finished_count[0] = loops;

  for (i = 0; (i < loops); i++)
  {
		edt_wait_for_buffers(edt_p[0], 1);

  }

  for (channel=1;channel<nchannels;channel++)
	  finished_count[channel] = edt_done_count(edt_p[channel]);


  for (channel = 0;channel < nchannels;channel++)
  {
	  FILE *f = NULL;

	  if (rawout)
		f = fopen(rawout, "wb+");

	  for (i=0;i<finished_count[channel];i++)
	  {
		  count_gap((u_int *) edt_p[channel]->ring_buffers[i],
			bufsize/4,
			&gaps[channel], textout);

		  if (f)
			  fwrite(edt_p[channel]->ring_buffers[i],bufsize, 1, f);
	  }

	  if (f)
		  fclose(f);


	output_gap(&gaps[channel]);
	edt_close(edt_p[channel]);

  }

  /*Display result for throughput*/


  return 0;
}



/*
 * command_error displays the command line description for each command
 */
void 
command_error()
{
    printf("usage: simple_getdata\n") ;
    printf("	-u <unit>		- specifies edt board unit number\n") ;
    printf("	-F <clock speed>	- specifies the clock's speed\n") ;
    printf("	-l <loops size>		- specifies the loops size in Mbytes\n") ;
}






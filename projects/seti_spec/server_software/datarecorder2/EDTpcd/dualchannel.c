
/********************************************************************
 *
 * Dualchannel.c
 *
 * Uses the ss_exdma bit file to show an example of reading and writing
 * channels on the same board which can have different speeds.
 *
 * The program opens two channels, one for reading, one for writing.
 * Each channel is assigned to its own thread which read and write asynchronously.
 * The read channel will check that its data matches what the write channel sent
 * out, depending on the -C option.
 * 
 * The bitfile has two modes, depending on a bit in the PCD_FUNCT register. The bit will be set 
 * depending on the argument to the -C option:
 *
 * -	one in which the data that is written to the card
 *		is passed through unchanged. In this case the read speed and the write speed 
 *		will be the same. To see varying read and write speeds set the buffer
 *		sizes different. (the default is 10:1 ratio write buffer/ read buffer size)
 *		To use this mode use the argument -C 1
 *
 * -    One in which 10 16-bit words written to the card are XORed together in the Xilinx,
 *		then the result is read back. In this case the write speed is 10 X the read speed.
 *		Use -C 2 to use this mode.
 *
 *
 * Some other options:
 *
 *  -r : this will use random data instead of a 16-bit counter to the fill the buffers written to
 *		  the card.
 *
 *  -B : don't reload the bitfile
 *
 *  -l : number of buffers to write. -l 0 will go forever
 *
 *
 *
 *******************************************************************/

#include <stdlib.h>

#include <math.h>

#include "edtinc.h"

#include "edt_threads.h"

int verbose = 0;

int
initialize_ss_board(char *devname, 
					   int unit, 
						char *bitfile,
						u_char byteswap,
						u_char shortswap,
						int timeout);

/**
 * Defines a structure which holds some extra 
 * state data for the threads doing I/O
 **/


typedef struct _channel_thread_data {

	EdtDev *edt_p;

	int nbufs;
	int bufsize;

	thread_type thread;

	edt_thread_function func;

	u_char active;

	u_char stopping;

	int loops;

	u_char *last_data;

	void *pTarget;
	void (*notify)( void *, struct _channel_thread_data *);

} channel_thread_data;



/**
 *
 * Allocate and return an edt_trhead_data structure
 * Configure ring buffers 
 *
 **/

channel_thread_data *
initialize_channel_thread(char *devname, 
					int unit,
					int channel,
					int bufsize,
					int nbuffers,
					int direction,
					edt_thread_function f)

{

	channel_thread_data * pth = NULL;
	
	pth = (channel_thread_data *) calloc(1,sizeof(channel_thread_data));

	pth->edt_p = edt_open_channel(devname, unit, channel);

	edt_configure_block_buffers(pth->edt_p, bufsize, 
		nbuffers, direction, 
		0,0);

	pth->func = f;
	pth->nbufs = nbuffers;
	
	return pth;

}

/**
 *
 * Start the DMA thread for a thread_data structure
 *
 **/

void
start_channel_thread(channel_thread_data *pth)

{
	if (pth)
	{
		pth->stopping = 0;
		pth->active = 1;

		LaunchThread(pth->thread, pth->func, (void *) pth);
	}
}

/**
 *
 * Stop a thread by setting the stopping flag
 * Optionally block until the active flag goes to 0
 *
 **/

void
stop_channel_thread(channel_thread_data *pth, int blocking)

{
	if (pth)
	{
		pth->stopping = 1;

		if (blocking)
		{
			
			while (pth->active)
			{
				edt_msleep(10);
			}
		}

	}
}

/**
 * fill the write buffers with either counter data 
 * or random values depending on randomize
 *
 **/
 
void fill_counter_buffers(EdtDev *write_p, int randomize)

{
	unsigned short *wbuf_p ;
	unsigned short *wtmpp ;

	int tmpc;
	unsigned int loop;
	unsigned int i;


	tmpc = 0;
	/* initialize the write buffers to inc data */


	for(loop=0; loop< write_p->ring_buffer_numbufs; loop++)
	{
		wbuf_p = (unsigned short *) edt_next_writebuf(write_p) ;
		wtmpp = (unsigned short *) wbuf_p;
		for(i=0; i< write_p->ring_buffer_bufsize/2; i++)
		{
			*wtmpp = (randomize)? rand() : tmpc++;

			wtmpp++;
		}

	}



}

/* Globals */


channel_thread_data *write_thread = NULL;
channel_thread_data *read_thread = NULL;


/***********************************
 **
 *  display section 
 *
 **********************************/

int write_n, read_n, sleep_n;
int total_buffer_size ; /* for checking write buffer against read */

CRITICAL_SECTION cs;

int cs_initialized = 0;

double start_time = 0;
double write_size = 0;


void
initialize_info()

{

	if (!cs_initialized)
	{

		InitializeCriticalSection(&cs);
		cs_initialized = 1;

		start_time = edt_timestamp();

		printf("\n\n  Sec     Write  Read   Speed\n\n");

	}


}

void
update_info()

{
	
	double t = edt_timestamp() - start_time;

	EnterCriticalSection(&cs);

	printf("\r%5d: %6d %6d %10.3f", sleep_n/10, write_n, read_n,
									write_size /(t * 1000000.0));

	LeaveCriticalSection(&cs);

}

/**
 * 
 * Check exdma bit file when it is passing data straight through
 * by comparing write buffer with read buffer data
 *
 **/

void
check_read(void *target, channel_thread_data *channel)

{
	
	/* check that data read is the same as data written */
	/* at this place in stream */

	unsigned int i;
	u_short * wrp;
	u_short *readp;

	
	double fstream_index = (read_n - 1) * channel->edt_p->ring_buffer_bufsize;

	unsigned int stream_index;
	
	stream_index = (unsigned int ) fmod(fstream_index, (double) total_buffer_size);

	wrp = (u_short *) (write_thread->edt_p->base_buffer + stream_index);
	readp = (u_short *) channel->last_data;

	for (i=0;i<channel->edt_p->ring_buffer_bufsize / 2;i++)
	{
		if (wrp[i] != readp[i])
		{
			printf("\nfailed at %d, %x (%d) != %x (%d) delta = %x (%d) ",
				i, wrp[i], wrp[i],readp[i],readp[i], readp[i] - wrp[i], readp[i] - wrp[i]);

		}

	}


}

/**
 * 
 * Check exdma bit file when it is xoring 10 shorts at a time.
 *
 **/

void
check_read_xor10(void *target, channel_thread_data *channel)

{
	
	unsigned int i;
	u_short * wrp;
	u_short *readp;

	double fstream_index = 10.0 * (read_n - 1) * channel->edt_p->ring_buffer_bufsize;

	unsigned int stream_index;
	
	stream_index = (unsigned int ) fmod(fstream_index, (double) total_buffer_size);
	
	wrp = (u_short *) (write_thread->edt_p->base_buffer + stream_index);
	readp = (u_short *) channel->last_data;

	for (i=0;i<channel->edt_p->ring_buffer_bufsize / 2;i++)
	{
		int j;
		int b = i*10;

		/* compute xor of 10 shorts */
		u_short checkx = wrp[b];

		for (j=1;j< 10; j++)
			checkx ^= wrp[j+b];

		if (checkx != readp[i])
		{

			printf("\nfailed at  buffer %d,%7x %12.0f %5d, %5x (%5d) != %5x (%5d)\n",
				read_n,
				stream_index, fstream_index, i, checkx, checkx,readp[i],readp[i]);

		}

	}


}


/**
 * Simple write thread function that starts loop buffers and 
 * doesn't do much else 
 *
 **/

THREAD_FUNC_DECLARE wr_thread_function(void *ptr)

{
	channel_thread_data * channel = (channel_thread_data *)ptr;

	write_n = 0;

	edt_start_buffers(channel->edt_p, channel->loops);

	while (!channel->stopping && (!channel->loops || write_n < channel->loops)) {

	
		channel->last_data = edt_wait_for_buffers(channel->edt_p,1);

		write_n  = edt_done_count(channel->edt_p) ;

		write_size = write_n *
				(double) channel->edt_p->ring_buffer_bufsize;

		update_info();

		if (channel->notify)
			channel->notify(channel->pTarget, channel);
		
	}

	channel->active = 0;
		
	return 0;
}


THREAD_FUNC_DECLARE rd_thread_function(void *ptr)

{

	channel_thread_data * channel = (channel_thread_data *)ptr;
	int nstarted;


	read_n = 0;

	nstarted = (channel->loops)?channel->loops:channel->nbufs;

	edt_start_buffers(channel->edt_p, nstarted);


	while (!channel->stopping && (!channel->loops || read_n < channel->loops)) {

		channel->last_data  = edt_wait_for_buffers(channel->edt_p, 1);

		if (!channel->loops || (nstarted < channel->loops))
		{
			edt_start_buffers(channel->edt_p, 1);
			nstarted ++;

		}
		
		/* how many have we waited for ?*/
		read_n = channel->edt_p->donecount;
		
		update_info();

		if (channel->notify)
			channel->notify(channel->pTarget, channel);

	}

	channel->active = 0;

	return 0;
}

#define BUF_SIZE 1024*32

int
main(int argc, char **argv)
{
	int channel = 1;

	int rc = 0;

	char devname[128];

	unsigned char do_preload = 1;

	
	unsigned char byteswap = 0;
	unsigned char shortswap = 0;
	unsigned char shiftr = 0;
	unsigned char testout = 0;
	unsigned char checking = 1;
	unsigned char randomize = 0;

	int unit = 0 ;

	int rd_bufsize = BUF_SIZE;
	int wr_bufsize = BUF_SIZE * 10;

	int rd_nbuffers = 80;
	int wr_nbuffers = 8;

	int rd_channel = 0;
	int wr_channel = 1;

	int timeout = 5000 ;	/* default timeout 5 seconds */

	int loops = 1000;  /* how many write buffers */

	int oneshot = 0;

	double freq = 0;

	int backio=0;
	
	char *bitfile = "ss_exdma.bit";


	strcpy(devname, EDT_INTERFACE);

    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
		switch (argv[0][1])
		{

			case 'u':
			++argv;
			--argc;
			unit = atoi(argv[0]);
			break;

		  case 't':
			++argv;
			--argc;
			timeout = atoi(argv[0]);
			break;

		  case 'c':

			  /* select write, read channels - any number between 0 and 3 */

			++argv;
			--argc;
			wr_channel = atoi(argv[0]);
			++argv;
			--argc;
			rd_channel = atoi(argv[0]);
			break;

		  case 'b':

			  /* select read and write buffer size */
			  /* Note that the write buffer size should be a multiple of 10 times the
			     read size if the XOR checking is enabled (-C 2) */

			++argv;
			--argc;
			wr_bufsize = atoi(argv[0]);
			++argv;
			--argc;
			rd_bufsize = atoi(argv[0]);
			break;

		  case 'N':
			++argv;
			--argc;
			wr_nbuffers = atoi(argv[0]);
			++argv;
			--argc;
			rd_nbuffers = atoi(argv[0]);
			break;


		  case 'r':
			randomize = 1;
			break;

		  case 's':
			shortswap = 1;
			break;

		  case 'z':
			byteswap = 1;
			break;

		  case 'B':
			bitfile = NULL;
			break;

		  case 'v':
			verbose = 1;
			break;
	
		  case 'l':
			++argv;
			--argc;
			loops = atoi(argv[0]);
			break;


			case 'C':
			++argv;
			--argc;
			checking = atoi(argv[0]);
			break;

		  default:
			puts("Usage: dualchannel [-v[erbose]] [-u unit_no] [-t timeout] [-b buffer size] -B(don't load bitfile)\n") ;
			exit(0);
		}
		argc--;
		argv++;
    }

	/* set up the board - load the bitfile, set swap bits (which don't really matter here) */

	if (rc = initialize_ss_board(devname, 
						unit, 
						bitfile,
						byteswap,
						shortswap,
						timeout) != 0)
	{	
		/* fail */

		return (rc);
	}

	/* Create thread functions */

	write_thread = initialize_channel_thread(devname, unit, 
			wr_channel, wr_bufsize, wr_nbuffers, EDT_WRITE, 
			wr_thread_function);

	read_thread = initialize_channel_thread(devname, unit, 
			rd_channel, rd_bufsize, rd_nbuffers, EDT_READ, 
			rd_thread_function);

	/* these settings make sure that the RFIFO is enabled 
	   on the board without a reset for each channel */

	edt_set_firstflush(write_thread->edt_p,EDT_ACT_NEVER);
	edt_set_firstflush(read_thread->edt_p,EDT_ACT_NEVER);

	edt_set_autodir(write_thread->edt_p,0);
	edt_set_autodir(read_thread->edt_p,0);

	fill_counter_buffers(write_thread->edt_p, randomize);
	
	write_thread->loops = loops;

	/* Fill in the notify function on the read thread to check the 
	   data against what was written */

	switch (checking)
	{
	case 1:
		read_thread->notify = check_read;
	default:
		edt_reg_write(write_thread->edt_p, PCD_FUNCT, 1);
		read_thread->loops = loops*10;
		break;
	case 2:
		read_thread->notify = check_read_xor10;
		edt_reg_write(write_thread->edt_p, PCD_FUNCT, 0);
		read_thread->loops = loops;
		break;
        
	}

	total_buffer_size = wr_bufsize * wr_nbuffers;

	/* initialize counters */

	initialize_info();
	
	/* start the read first */

	start_channel_thread(read_thread);
	
	start_channel_thread(write_thread);

	/* Loop in the main thread, sleeping for a second and waiting for the write side to finish */


	while (write_thread->active) {

		edt_msleep(100);

		sleep_n += 1;

		update_info();

	}

	
	return(0) ;
}
/**
 * Initialize an ssdio board transfer 
 *
 */

int
initialize_ss_board(char *devname, 
					   int unit, 
						char *bitfile,
						u_char byteswap,
						u_char shortswap,
						int timeout)

{

	EdtDev *write_p;

	unsigned char conf;

	/* Set up bitfile */

	if (bitfile)
	{
		char cmd[128];

		if (verbose)
			  printf("Loading bitfile %s\n", bitfile);
		sprintf(cmd,"bitload -u %d %s",unit, bitfile) ;
		system(cmd) ;
	}

	if ((write_p = edt_open_channel(devname, unit, 0)) == NULL)
	{

		edt_perror(EDT_INTERFACE) ;
		puts("Hit return to exit") ;
		getchar() ;
		return(1) ;
	}
	
	if (write_p->devid != PSS4_ID)
	{
		printf("This program requires a PCI SS programmed with the PCISS1 pci bitfile\n");
		return(1);
	}

	(void)edt_set_wtimeout(write_p, timeout) ;

	edt_set_autodir(write_p, 0) ;

	/* set byte swap  if required */

	conf = conf & ~(PCD_BYTESWAP | PCD_SHORTSWAP);

	if ( byteswap )
		conf |= PCD_BYTESWAP;
	if ( shortswap )
		conf |= PCD_SHORTSWAP;

	edt_reg_write(write_p, PCD_CONFIG, conf) ;
	conf = edt_reg_read(write_p, PCD_CONFIG);

	if (verbose) printf("config register was %02x\n", conf);

	edt_flush_fifo(write_p);

	/* enable */

	edt_reg_or(write_p, PCD_CMD, 8);

	edt_close(write_p);

	return 0;
}

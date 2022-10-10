
/*
 * simple_putdata.c: 
 *	An example program to show use of library for ring buffer mode
 *	This program either generates data or reads it from a file or device
 *      and writes the data to the pcd device.
 *
 *	See writeblock if you just have a single write to do -
 *	that can be done with one write system call
 *
 * To compile:
 *	cc -O -DSYSV -o simple_putdata simple_putdata.c -L. -ledt -lthread
 *
 * To run:
 *
 *      On the SCD:
 *
 *	If your device is not hooked up or running, you can start data
 *	generation by typing "confidence" and hitting ^C while it is 
 *	running the speed test.
 *
 * 	Be sure to do an "scdload 0 scd.rbt" before trying to talk to
 * 	your device again after running confidence.
 *
 *	On the S16D and S11W:
 *
 *	An external data source must be provided on the S16D and S11W.
 *
 *	Run simple_putdata with no arguments to run a basic speed test.
 *      A dot will be printed each time another buffer is done.  Type 
 *      "putdata -h" to get usage.
 */

#include "edtinc.h"

#include <string.h>
#include <stdlib.h>
#ifndef _NT_
#include <unistd.h>
#endif

#define NUMBUFS 1

#include <math.h>

#include "edt_vco.h"
#include "edt_ss_vco.h"


int numbufs = NUMBUFS ;
int buffers_inuse ;

/* standard SS/CDA frequency */

double xtal = 10368100.0;

void
usage()
{
    puts("usage: putdata") ;
    puts("-b        - set byteswap on") ;
    puts("-s size	- specifies buffer size in bytes") ;
    puts("-k size	- specifies buffer size in kbytes") ;
    puts("-l loop	- specifies number of loops") ;
    puts("-i infile\t- specifies input filename (\"-i -\" uses stdandard input");
    puts("-h 		- prints usage (help)") ;
}

void init_buf(u_char *buf, int size) ;

edt_set_cda_frequency(EdtDev *edt_p, 
			double target,
			int divide_by_2,
			int verbose)

{

  edt_pll pll;
  double delta;
  double actual;

	if (divide_by_2 && target <= 100000000.0)
		actual = edt_find_vco_frequency_ics307(NULL,target,
			xtal, &pll, verbose);
	else
		actual = edt_find_vco_frequency_ics307_nodivide(NULL,target,
				xtal,&pll,verbose);

	delta = fabs(target  - actual);

	delta /= target;

	if (verbose)
	  {
		printf("Target = %f, actual = %f, delta = %f\n",
			   target, actual, delta);
		
	  }

	if (edt_p)
	  {
		edt_set_out_clk_ics307(edt_p, &pll, 0);
		
		/* turn on PLL */
		edt_reg_or(edt_p, PCD_FUNCT, EDT_FUNCT_PLLCLK);

	  }

	return 0;
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    int unit ;
    int i;
    int loop ;
	int loopcount ;
	int byteswap=0;
	double dtime ;
    unsigned int size ;
    u_char *buf ;
    u_char **buffers = 0;
    char *infile ;
    int ifd = -1 ;
	int lstcnt ;
	int curcnt ;
	int timeouts ;
	int channel = 0;
	int kernel_buffers = -1;
	int lead_value = -1;
	double frequency = 0.0;

    /* 
     * set defaults 
     */
    unit = 0 ;
    infile = NULL ;
    /* default to 100 writes of 1 Megabyte */
    size = 1024*1024 ;
    loopcount = 100 ;
    timeouts = 0 ;

    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
		switch (argv[0][1])
		{
		case 'u':
			++argv;
			--argc;
			unit = strtol(argv[0],0,0);
			break ;
		case 'b':
			byteswap = 1;
			break ;
		case 's':
			++argv;
			--argc;
			size = strtol(argv[0],0,0);
			break ;
		case 'k':
			++argv;
			--argc;
			kernel_buffers = strtol(argv[0],0,0);
			break ;
		case 'N':
			++argv;
			--argc;
			numbufs = strtol(argv[0],0,0); ;
		break ;
		case 'l':
			++argv;
			--argc;
			loopcount = strtol(argv[0],0,0); ;
			break ;
		case 'i':
			++argv ;
			--argc ;
			infile = strdup(argv[0]) ;
			if (strcmp(infile, "-") == 0)
				ifd = fileno(stdin) ;
			else
			{
#ifdef _NT_
				ifd =	(int)CreateFile(
					infile,
					GENERIC_READ | GENERIC_WRITE ,
					FILE_SHARE_READ	| FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if (ifd == (int)INVALID_HANDLE_VALUE)
				{
					fprintf(stderr,	"failed open %s\n",infile) ;
					exit(1) ;
				}
#else
				if ((ifd = open(infile, 0666)) < 0)
				{
					printf("create of %s failed\n", infile) ;
					perror("infile") ;
					exit(1) ;
				}
#endif
			}
			break ;
		case 'c':
			++argv;
			--argc;
			channel = strtol(argv[0],0,0);
			break;

		case 'F':
		  ++argv;
			--argc;
			frequency = atof(argv[0]);
			break;
	  
		case 'v':
			++argv;
			--argc;
			lead_value = strtol(argv[0],0,0);
			break;

		case 'h':
		case '?':
		default:
			usage() ;
			exit(1) ;
		}
		--argc;
		++argv;
    }

    printf("writing %d buffers of %d bytes from unit %d with %d bufs\n",
	loopcount, size, unit, numbufs) ;

    if ((edt_p = edt_open_channel(EDT_INTERFACE, unit,channel)) == NULL)
    {
		perror("edt_open") ;
		exit(1) ;
    }
	if (kernel_buffers != -1)
	{
		edt_ioctl(edt_p, EDTS_DRV_BUFFER, &kernel_buffers);
	}

	if (lead_value != -1)
	{
		edt_ioctl(edt_p, EDTS_DRV_BUFFER_LEAD, &lead_value);
	}

	if (edt_p->devid == PCDA_ID)
	{
	  if (frequency != 0.0)
		{
		  edt_set_cda_frequency(edt_p, frequency, TRUE, TRUE);
		}
	  else
		{
		  edt_reg_and(edt_p,PCD_FUNCT, ~EDT_FUNCT_PLLCLK);
		}
	}
					

	buffers =(u_char **)  edt_alloc(sizeof(u_char *) * NUMBUFS);

    for (i=0;i<NUMBUFS;i++)
	  {
		buffers[i] = edt_alloc(size);
		init_buf(buffers[i], size);
	  }

	(void)edt_dtime();
#ifdef PCD
	if (byteswap)
	    pcd_set_byteswap(edt_p, 1) ;
#endif

    buffers_inuse = 0 ;
	curcnt = edt_timeouts(edt_p) ;
	lstcnt = curcnt ;
	edt_flush_fifo(edt_p);

    for(loop = 0 ; (loop < loopcount || loopcount == 0) ; loop++)
    {
		u_int Length = size;
		/*
		 * Initialize and start ring buffers until they're all in use,
		 * i.e. don't wait for a buffer until they're all being used.
		 */

		/*
		 * Copy data into each ring buffer, then start DMA.
		 * Data could come from a file, a tape drive, etc,
		 * and is written to the external device connected to
		 * the EDT board continuously via the ring buffers.
		 */
		buf = buffers[i % numbufs];

		if (infile)
		{
#ifdef _NT_		
			ReadFile((void *)ifd, buf, size, &Length, NULL) ;
#else
			Length = read(ifd, buf, size) ;
#endif
		}

		edt_ref_tmstamp(edt_p,0xbeefbeef);


		edt_write(edt_p, buf, Length) ; /* Start the transfer on this buffer */

		printf("."); fflush(stdout);
    }

	dtime = edt_dtime() ;
	printf("\nput %d bytes in %f seconds for %f bytes/second\n",
			    size * loopcount,
			    dtime,
			    (size * loopcount)/dtime);
	edt_close(edt_p) ;
	exit(0) ;
	return(0) ;
}

/*
 * initialize buffer with 16-bit incremented data
 * could replace with read of data from file, other device, or ???
 */
void
init_buf(u_char *buf, int size)
{
    u_short word = 0;
    u_short *p, *endp;
    static int count = 0 ;

    if (++count > numbufs)
	return ;

    endp = (u_short *) (buf + size);
    p = (u_short *) buf;

    while (p < endp)
    {
    	*p++ = word++;
    }
}

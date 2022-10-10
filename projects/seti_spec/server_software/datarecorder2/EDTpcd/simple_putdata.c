
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

#define NUMBUFS 4   /* Four is the optimal number of buffers for EDT boards */
int numbufs = NUMBUFS ;
int buffers_inuse ;

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

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    int unit ;
    int loop ;
	int loopcount ;
	int byteswap=0;
	double dtime ;
    unsigned int size ;
    u_char *buf ;
    char *infile ;
    int ifd = -1 ;
	int lstcnt ;
	int curcnt ;
	int timeouts ;
	int channel = 0;
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
			unit = atoi(argv[0]);
			break ;
		case 'b':
			byteswap = 1;
			break ;
		case 's':
			++argv;
			--argc;
			size = atoi(argv[0]);
			break ;
		case 'k':
			++argv;
			--argc;
			size = atoi(argv[0]) * 1000;
			break ;
		case 'n':
			++argv;
			--argc;
			numbufs = atoi(argv[0]) ;
			if (numbufs < 2)
			{
				printf("numbufs should be at least two to take\n") ;
				printf("advantage of ring buffer mode\n") ;
				exit(1) ;
			}
			break ;
		case 'l':
			++argv;
			--argc;
			loopcount = atoi(argv[0]) ;
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
			channel = atoi(argv[0]);
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

    /*
     * Have edt_configure_ring_buffers allocate the ring buffers by
     * passing the last argument as NULL.  Otherwise pass in an array
     * of pointers to the buffers.
     */
	/* Why is this here???  At least say "Hit return to continue"
	 * when you hide getchar()'s in the code.
	 * printf("Before configure ring buffers\n"); getchar();
	 */
	
    if (edt_configure_ring_buffers(edt_p, size, numbufs, EDT_WRITE, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

	(void)edt_dtime();
#ifdef PCD
	if (byteswap)
	    pcd_set_byteswap(edt_p, 1) ;
#endif

    buffers_inuse = 0 ;
	curcnt = edt_timeouts(edt_p) ;
	lstcnt = curcnt ;
    for(loop = 0 ; (loop < loopcount || loopcount == 0) ; loop++)
    {
		/*
		 * Initialize and start ring buffers until they're all in use,
		 * i.e. don't wait for a buffer until they're all being used.
		 */
		if (buffers_inuse >= NUMBUFS)
		{
			/* All ring buffers are in use; wait for the next one to finish */
			edt_wait_for_buffers(edt_p, 1) ;
			--buffers_inuse ;
			curcnt = edt_timeouts(edt_p) ;
			if (lstcnt != curcnt) 
			{
				putchar('x') ;
				timeouts++ ;
				lstcnt = curcnt ;
				edt_reset_ring_buffers(edt_p,0) ;
				edt_flush_fifo(edt_p) ;
				edt_start_buffers(edt_p, 0) ; /* start the transfers in free running mode */
			}
			else putchar('.') ;
			fflush(stdout) ;
		}

		/*
		 * Copy data into each ring buffer, then start DMA.
		 * Data could come from a file, a tape drive, etc,
		 * and is written to the external device connected to
		 * the EDT board continuously via the ring buffers.
		 */
		buf = edt_next_writebuf(edt_p) ;

		if (infile)
		{
			u_int Length ;
#ifdef _NT_		
			ReadFile((void *)ifd, buf, size, &Length, NULL) ;
#else
			Length = read(ifd, buf, size) ;
#endif
			if (Length != size)
			{
			/* change size on last if not full buf */
			printf("setting last  short from %x to %x\n", size,Length) ;
			edt_set_sgbuf(edt_p, (buffers_inuse + 1) % numbufs,
			  Length, EDT_WRITE, 1);
			}
		}
		else 
			init_buf(buf, size);


		edt_start_buffers(edt_p, 1) ; /* Start the transfer on this buffer */
		++buffers_inuse ;
    }

    /* Now wait for remaining buffers to finish */
    while (buffers_inuse)
    {
		edt_wait_for_buffers(edt_p, 1) ;
		--buffers_inuse ;

		putchar('.') ;
		fflush(stdout) ;
    }

	dtime = edt_dtime() ;
	printf("\n%f bytes/sec %d timeouts\n",
		(double)(loopcount * size) / dtime,timeouts) ;
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

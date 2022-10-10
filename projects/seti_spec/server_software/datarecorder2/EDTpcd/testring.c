
/*
 * simple_getdata.c: 
 *
 *	An example program to show use of library for ring buffer mode
 *	This program reads from the edt device and optionally writes
 *	to a disk file or device.
 *
 *	Just use read(2) if you just have a simple read to do.
 *
 * To compile:
 *	cc -O -DSYSV -o simple_getdata simple_getdata.c -L. -ledt -lthread
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
 *	Run simple_getdata with no arguments to run a basic speed test.
 *      A dot will be printed each time another buffer is done.  Type 
 *      "getdata -h" to get usage.
 */

#include "edtinc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chkbuf.h"

void
usage()
{
    printf("usage: getdata\n") ;
    printf("-s size	- specifies buffer size in bytes\n") ;
    printf("-k size	- specifies buffer size in kbytes\n") ;
	printf("-n num  - specifies number of multiple buffers\n") ;
    printf("-l loop	- specifies number of loops\n") ;
    printf("-o outfile	- specifies output filename\n") ;
    printf("-c cnt	- to check on freerun\n") ;
    printf("-h 		- prints usage (help)\n") ;
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    int unit ;
    int loop ;
    int numbufs ;
    int loopcount ;
    int size ;
	double dtime ;
    extern char *optarg ;
    extern int optind ;
    u_char *buf ;
    char *outfile  ;
    int outfd = -1 ;
    int dochannel = 0 ;
	int started = 0;
	int doxtest = 1;
	int     doswap = 0;
	int     dowordswap = 0;
	int dobig = 0;
	int doinvert = 0;
	int dowrite = 0;

	unsigned short *bufs[8];
	
	char devname[80];
	char *testdev = NULL;
	int testunit;

	int channel = 0;
	int initbufs = 1;
    /* 
     * set defaults 
     */
    unit = 0 ;
    outfile = NULL ;
    /* default to 100 reads of 1 Megabyte */
    size = 0x10000 ;
    loopcount = 8 ;
    numbufs = 4 ;   /* Four is the optimal number of buffers for EDT boards */

	strcpy(devname,EDT_INTERFACE);
	testdev = devname;

	printf("devname %s\n",devname);

    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
		switch (argv[0][1])
		{
		case 'u':
			++argv;
			--argc;
			testdev = argv[0];
			
			break;
		case 's':
			++argv;
			--argc;
			size = atoi(argv[0]);
			break ;
		case 'k':
			++argv;
			--argc;
			size = atoi(argv[0]) * 1024;
			break ;
		case 'n':
			++argv;
			--argc;
			numbufs = atoi(argv[0]) ;
			break ;
		case 'l':
			++argv;
			--argc;
			loopcount = atoi(argv[0]) ;
			break ;
		case 'o':
			++argv ;
			--argc ;
			outfile = argv[0] ;
#ifdef _NT_
			outfd =	(int)CreateFile(
				outfile,
				GENERIC_READ | GENERIC_WRITE ,
				FILE_SHARE_READ	| FILE_SHARE_WRITE,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (outfd == (int)INVALID_HANDLE_VALUE)
			{
				fprintf(stderr,	"failed open %s\n",outfile) ;
				exit(1) ;
			}
#else
			if ((outfd = creat(outfile, 0666)) < 0)
			{
				printf("create of %s failed\n", outfile) ;
				perror("outfile") ;
				exit(1) ;
			}
#endif
			break ;

		case 'i':
			++argv;
			--argc;
			initbufs = atoi(argv[0]) ;
			break ;
			
		case 'c':
			++argv;
			--argc;
			dochannel = 1 ;
			channel = atoi(argv[0]) ;
			break ;
		case 'x':
		  doxtest = !doxtest;
		  break;


		case 'h':
		case '?':
		default:
			usage() ;
			exit(1) ;
		}
		--argc ;
		++argv ;
    }

	printf("devname %s\n",devname);

	if ((testunit = edt_parse_unit(devname, devname, EDT_INTERFACE)) != -1)
		unit = testunit;

    printf("%s: reading %d buffers of %d bytes from unit %d with %d bufs\n",
		   devname, loopcount, size, unit, numbufs) ;


    if ((edt_p = edt_open_channel(devname, unit,channel)) == NULL)
    {
		perror("edt_open") ;
		exit(1) ;
    }

    /*
     * Have edt_configure_ring_buffers allocate the ring buffers by
     * passing the last argument as NULL.  Otherwise pass in an array
     * of pointers to the buffers.
     */

	printf("calling configure ring buffers\n");
	getchar();
    if (edt_configure_ring_buffers(edt_p, size, numbufs, EDT_READ, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

	edt_flush_fifo(edt_p) ;

	if (edt_p->devid == PDV_ID
	 || edt_p->devid == PDVA_ID
	 || edt_p->devid == PDVA16_ID
	 || edt_p->devid == PDVFOX_ID
	 || edt_p->devid == PCD20_ID
	 || edt_p->devid == PCD40_ID
	 || edt_p->devid == PCD60_ID
	 || edt_p->devid == PCDA_ID
	 || edt_p->devid == PCDA16_ID)
	{

	  u_short pcd_cmd ;
	  printf("Starting xtest\n");

	getchar();

	  pcd_cmd = edt_intfc_read(edt_p, PCD_CMD) ;
		pcd_cmd |= PCD_BNR_ENABLE ;
		edt_intfc_write(edt_p, PCD_CMD, pcd_cmd) ;
		if (doxtest)
		  {
			int cmd = 0;
			if (doswap)
			  cmd |= XTEST_SWAPBYTES;
			if (dowordswap)
			  cmd |= XTEST_SWAPWORDS;
			if (doinvert)
			  cmd |= XTEST_INVERT;
			if (dobig)
			  cmd |= XTEST_BIT32;
			if (dowrite)
			  cmd |= XTEST_DEV_READ;
			edt_intfc_write_short(edt_p, XTEST_CMD, cmd);
			cmd |= XTEST_UN_RESET_FIFO;
			edt_intfc_write_short(edt_p, XTEST_CMD, cmd);
			cmd |= XTEST_EN_DATA;
			edt_intfc_write_short(edt_p, XTEST_CMD, cmd);
	
		  }
	}

	(void)edt_dtime();
 
    /*
     * The following lines puts the driver into free running mode and
     * starts DMA immediately.  The application must be fast enough to
     * keep up with no handshaking.
     */
    edt_start_buffers(edt_p, numbufs) ; /* start the transfers in free running mode */

	started = numbufs;

	if (edt_p->devid == PDV_ID
	 || edt_p->devid == PDVA_ID
	 || edt_p->devid == PDVA16_ID
	 || edt_p->devid == PDVFOX_ID
	 || edt_p->devid == PCD20_ID
	 || edt_p->devid == PCD40_ID
	 || edt_p->devid == PCD60_ID
	 || edt_p->devid == PCDA_ID
	 || edt_p->devid == PCDA16_ID)
	{
		u_short pcd_cmd ;
		pcd_cmd = edt_intfc_read(edt_p, PCD_CMD) ;
		pcd_cmd &= ~PCD_BNR_ENABLE ;
		edt_intfc_write(edt_p, PCD_CMD, pcd_cmd) ;


	}

	for(loop = 0 ; loop < loopcount ; loop++)
	{
		bufs[loop % numbufs] = (unsigned short *) buf = 
		  edt_wait_for_buffers(edt_p, 1) ;  /* Wait for the next buffer   */


		if (loop < loopcount - numbufs)
		  edt_start_buffers(edt_p,1);
		/*
		 * Write or process each buffer after starting dma on the next buffer
		 * could make this aio write or routine to write to raid.
		 */

		if (outfile)
		{
#ifdef _NT_		
			u_int Length ;
			WriteFile((void *)outfd, buf, size, &Length, NULL) ;
#else
			write(outfd, buf, size) ;
#endif
		}
		printf("%8p\n",buf);
		putchar('.') ;
		fflush(stdout) ;
    }

	dtime = edt_dtime() ;
	for (loop = 0;loop < numbufs; loop++)
	  {
		chkbuf_short(bufs[loop], size / 2, loop, 1);
	  }
 
	printf("\n%f bytes/sec\n", (double)(loopcount * size) / dtime) ;

	edt_close(edt_p) ;
	exit(0) ;
	return(0) ;
}





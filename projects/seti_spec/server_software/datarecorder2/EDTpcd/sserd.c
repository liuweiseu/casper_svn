/* #pragma ident "@(#)sserd.c	1.8 07/15/04 EDT" */

#include "edtinc.h"
#include <stdlib.h>


void
usage()
{
    puts("usage: sserd [-u unit] [-o filename] size");
    puts("-u sets pcd device number - default is 0");
    puts("-o specifies output file for data, default is none");
    puts("size is the size of read in bytes");
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p;
    FILE   *outfile;
    char   *outfilename = NULL ;
    u_char *buf_p;
    u_char  sse_bswap ;
    int     unit = 0;
    int     channel = 0;
    int     bcnt = 0;
    int     doshift = 0;
    int	    swapbits = 0 ;
    int	    swapbytes = 0 ;
    int	    swapshorts = 0 ;
    int	    zero_fifo = 0 ;
    int	    loop = 0 ;
    double  setFreq = 0.0 ;
    double  delta;

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'o':
	    ++argv;
	    --argc;
	    outfilename = argv[0] ;
	    break;

	case 'u':
	    ++argv;
	    --argc;
	    unit = atoi(argv[0]);
	    break;

	case 'c':
	    ++argv;
	    --argc;
	    channel = atoi(argv[0]);
	    break;

	case 'b':
	    ++argv;
	    --argc;
	    swapbits = atoi(argv[0]);
	    break;

	case 'B':
	    ++argv;
	    --argc;
	    swapbytes = atoi(argv[0]);
	    break;

	case 'S':
	    ++argv;
	    --argc;
	    swapshorts = atoi(argv[0]);
	    break;

	case 's':
	    ++argv;
	    --argc;
	    doshift = atoi(argv[0]);
	    break;

	case 'f':
	    ++argv;
	    --argc;
	    setFreq = atof(argv[0]);
	    printf("Freq: %f\n", setFreq);
	    break;

	case 'z':
	    ++argv;
	    --argc;
	    zero_fifo = atof(argv[0]);
	    break;

	case 'L':
	    ++argv;
	    --argc;
	    loop = atoi(argv[0]);
	    break;


	default:
	    usage();
	    exit(0);
	}
	argc--;
	argv++;
    }

    if (argc < 1)
    {
	usage();
	exit(0);
    }

    bcnt = atoi(argv[0]);

    if (bcnt <= 0)
    {
	fprintf(stderr,"last parameter must specify bytecount\n");
	exit(1);
    }

    printf("reading %d bytes\n", bcnt);


    edt_p = edt_open_channel("pcd", unit, channel);
    if (edt_p == NULL)
    {
	fprintf(stderr, "Could not open EDT device");
	exit(1);
    }

    /* Be sure to do this after setting the out_clk, not before. */
    if (doshift)
	sse_shift(edt_p, doshift) ;

    if (setFreq != 0.0)
	sse_set_out_clk(edt_p, setFreq) ;

    /*
     * Read the SSE_BYTESWAP register.  Set bit, byte, or short swap,
     * but also set the RSTA/RSTB bit to reset the daughterboard and flush
     * its fifo.  Later, after DMA is configured, set RSTA to 0 to
     * start data acquisition.
     */
    sse_bswap = edt_intfc_read(edt_p, SSE_BYTESWAP) ;
    if (!channel) sse_bswap |= SSE_RSTA ;
    else          sse_bswap |= SSE_RSTB ;

    if (swapbits)
	sse_bswap |= SSE_SWAPBITS ;
    else
	sse_bswap &= ~SSE_SWAPBITS ;

    if (swapbytes)
	sse_bswap |= SSE_SWAPBYTES ;
    else
	sse_bswap &= ~SSE_SWAPBYTES ;

    if (swapshorts)
	sse_bswap |= SSE_SWAPSHORTS ;
    else
	sse_bswap &= ~SSE_SWAPSHORTS ;

    edt_intfc_write(edt_p, SSE_BYTESWAP, sse_bswap) ;


    /* Disable timeouts */
    edt_set_rtimeout(edt_p, 0);



    /*
     * Configuring one ring buffer, starting it and then waiting
     * for it to finish is equivalent to edt_read() EXCEPT that
     * there is no latency in starting the DMA.  This prevents
     * the fifo from overflowing after edt_flush_fifo().
     */
    if (loop==0) 
        edt_configure_ring_buffers(edt_p, bcnt, 1, EDT_READ, NULL);
    else   /* set up 4 ring buffers if we want to loop forever */
        edt_configure_ring_buffers(edt_p, bcnt, 4, EDT_READ, NULL);
    

    if (zero_fifo)  {
	int bswap_val;
        bswap_val = edt_intfc_read(edt_p, SSE_BYTESWAP) ;
	printf("Flushing PCI DMA engine fifos, bswap:%02x\n", bswap_val);
	edt_flush_fifo(edt_p);	/* Flush PCI interface fifos */
    }

    if (loop==0) 
        edt_start_buffers(edt_p, 1); 		/* Start the DMA read */
    else	/* loop forever */
        edt_start_buffers(edt_p, 4); 		/* Start with 4 buffers */

    /* To start the data acquisition, turn off the channel reset bit */
    if (!channel) sse_bswap &= ~SSE_RSTA ;
    else          sse_bswap &= ~SSE_RSTB ;
    edt_intfc_write(edt_p, SSE_BYTESWAP, sse_bswap) ;

    edt_dtime();
    buf_p = edt_wait_for_buffers(edt_p, 1);	/* Wait till DMA complete */
    delta = edt_dtime();
    printf("%d bits in %lf seconds:  %lf bits/sec\n", bcnt*8, delta,
						     (double)(bcnt*8)/delta);

    while ((loop!=0) && (buf_p!=NULL)) {	/*If looping forever */
        edt_start_buffers(edt_p, 1); 		/* Start the DMA read */
        buf_p = edt_wait_for_buffers(edt_p, 1);	/* Wait till DMA complete */
	putchar('.');  fflush(stdout);
    }


/*
 * Alternative way to read, but normal reads take time to allocate
 * DMA resources and hence allow the fifo to overflow after edt_flush_fifo()
 * and before the DMA actually starts.
 *  
 *  if ((buf_p = malloc(bcnt)) == NULL)
 *  { 
 *	edt_perror("malloc") ;
 *	exit(1);
 *  }
 *  edt_read(edt_p, buf_p, bcnt);
 */

    if (outfilename)
    {
	if ((outfile = fopen(outfilename, "wb")) == NULL)
	{
	    edt_perror(outfilename) ;
	    exit(1);
        }
	printf("Writing %d bytes to %s\n", bcnt, outfilename);
	fwrite(buf_p, 1, bcnt, outfile);
	fclose(outfile);
    }

    /* free(buf_p);		/ * Do this only if we did the malloc() */

    return 0;
}


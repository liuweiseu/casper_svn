#include "edtinc.h"
#include <stdlib.h>


#define BCNTMAX  1024*1024		/* 1 Megabytes */
#define BUF_SIZE        256*1024 

void spal(int v) ;

#define SSE_IDLE    0x0101001F		/* register for data to send when idle*/

void
usage()
{
    puts("usage: ssewr [-u unit] filename ");
    puts("-u sets pcd device number - default is 0");
    puts("filename specifies input file for binary data to send");
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p;
    FILE   *infile;
    char   *filename = NULL ;
    u_char  *bufs ;
    u_char  sse_bswap ;
    int     unit = 0;
    int     bcnt = 0;
    int	    swapbits = 0 ;
    int	    swapbytes = 0 ;
    int	    oneshot = 1 ;
    int	    swapshorts = 0 ;
    int	    idle_char = 0xFF;
    int     nbufs = 0;
    int     loop;
    int     i;
    u_char  *wbuf_p ;                                                 
    double  setFreq = 0.0 ;

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'u':
	    ++argv;
	    --argc;
	    unit = atoi(argv[0]);
	    break;

	case 'i':
	    ++argv;
	    --argc;
	    idle_char = atoi(argv[0]);
	    break;

        case 'L':
            oneshot = 0;
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

	case 'f':
	    ++argv;
	    --argc;
	    setFreq = atof(argv[0]);
	    printf("Freq: %f\n", setFreq);
	    break;

	default:
	    usage();
	    exit(0);
	}
	argc--;
	argv++;
    }

    if (argc == 1)
    {
        filename=argv[0] ;
	argc--;
    }
    if ((argc!=0) || (filename == NULL)) {
	usage();
	exit(0);
    }


    if ((infile = fopen(filename, "rb")) == NULL) {
	    edt_perror(filename) ;
	    exit(1);
    }

    bufs = edt_alloc(BCNTMAX);
    if (bufs == NULL) {
	printf("edt_alloc() failed\n");
	exit(1);
    }

    bcnt = fread(bufs, 1, BCNTMAX, infile);
    fclose(infile);
    printf("Writing %d bytes from file %s to GPSSE\n", bcnt, filename);
    if (bcnt&0x03)  {
	printf(" File should be a multiple of 4 bytes, ignoring remainder\n");
    }
    bcnt &= ~3;
    if (bcnt < 128000) {
        printf("File size of 128000 bytes recommended for efficent DMA\n");
    }




    edt_p = edt_open_channel("pcd", unit, 1);
    if (edt_p == NULL)
    {
	fprintf(stderr, "Could not open EDT device for writes");
	exit(1);
    }
    edt_set_autodir(edt_p, 0) ;

/*
    edt_reg_write(edt_p, SSE_IDLE, idle_char) ;
*/

    if (setFreq != 0.0)
	sse_set_out_clk(edt_p, setFreq) ;


    /*
     * Read the SSE_BYTESWAP register.  Set bit, byte, or short swap,
     * but also strobe the RSTB bit.  (Do we also want RSTA set?)
     * Later, after DMA is configured, set RSTB to 0 to
     * start data transfer.
     */

    sse_bswap = edt_intfc_read(edt_p, SSE_BYTESWAP) ;

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

    sse_bswap &= ~(SSE_RSTA | SSE_RSTB);

    edt_intfc_write(edt_p, SSE_BYTESWAP, sse_bswap | SSE_RSTB) ;
    edt_intfc_write(edt_p, SSE_BYTESWAP, sse_bswap ) ;


    /* Disable timeouts */
    edt_set_rtimeout(edt_p, 0);

    edt_flush_fifo(edt_p) ;


puts("Hit return to continue") ;
getchar() ;


    if (oneshot) {		/* Just write the file once */
	edt_write(edt_p, bufs, bcnt) ;
        return 0;
    }
    /* Otherwise, loop forever writing the file using ring buffers */


    if (edt_configure_ring_buffers(edt_p, bcnt, 3,
                                EDT_WRITE, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }
 
    /* initialize the write buffers to data from the file */
    for(loop=0; loop< 3; loop++) {
        wbuf_p = edt_next_writebuf(edt_p) ;
        for(i=0; i< bcnt; i++)  
	    wbuf_p[i] = bufs[i];
    }

    edt_dtime();
    edt_start_buffers(edt_p, 3) ; 	/* Or could just start 1 buffer? */

    /* Loop forever until the user hits control-C */
    while (1) {
        edt_wait_for_buffers(edt_p, 1) ;
        edt_start_buffers(edt_p, 1) ;
        putchar('.') ;
        if (nbufs++ % 16 == 0) {
            if (nbufs > 1) {
                double speed = edt_dtime() / 16.0;
                printf("\nSpeed %12f bytes/sec\n", 1.0 / (speed / (bcnt)));
            } else edt_dtime();
        }
        fflush(stdout) ;
    }
 
    /* Never execute this */
    edt_close(edt_p) ;
    exit(0) ;
    return(0) ;
}

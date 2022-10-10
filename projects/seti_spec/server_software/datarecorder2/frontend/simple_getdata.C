/* #pragma ident "@(#)simple_getdata.c	1.14 04/30/99 EDT" */

/*
 * simple_getdata.c: 
 *
 *  An example program to show use of library for ring buffer mode.
 *  This program performs continuous input from the edt dma device
 *  and optionally writes to a disk file or device.
 *
 *  Just use edt_read() if you just have a simple read to do.
 *
 * To compile:
 *  cc -O -DSYSV -o simple_getdata simple_getdata.c -L. -ledt -lthread
 *
 *  A data source must be provided to the EDT board.
 *
 */

#include "edtinc.h"
#include <stdlib.h>

void
usage()
{
    printf("usage: simple_getdata\n") ;
    printf("    -u <unit>   - specifies edt board unit number\n") ;
    printf("    -v	    - verbose\n") ;
    printf("    -o outfile  - specifies output filename\n") ;
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    FILE   *outfile = NULL ;
    char   *outfile_name = NULL ;
    int     unit = 0, i;
    int     verbose = 0 ;
    u_char *buf ;
	int bufsize = 1024*1024;
	int loops = 300;


    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'u':
            ++argv;
            --argc;
            unit = atoi(argv[1]);
            break ;

        case 'v':
            verbose = 1 ;
            break ;

        case 'o':
            ++argv ;
            --argc ;
            outfile_name = argv[1] ;

	    if ((outfile = fopen(outfile_name, "wb")) == NULL)
	    {
		perror(outfile_name) ;
		exit(1) ;
	    }

            break ;

			case 's':
            ++argv ;
            --argc ;
            bufsize = strtol(argv[1],0,0);
			break;	

			case 'l':
            ++argv ;
            --argc ;
            loops = strtol(argv[1],0,0);
			break;	

        default:
            usage() ;
            exit(1) ;
        }
        --argc ;
        ++argv ;
    }

    if (argc > 1) { usage(); exit(1); }

    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
        perror("edt_open") ;
        exit(1) ;
    }

    if (edt_configure_ring_buffers(edt_p, bufsize, 4, EDT_READ, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

    edt_flush_fifo(edt_p) ;         /* Flush the input fifo */

    // This line added by Aaron 12/3/02
    // Used to set FUNCT3 high
    edt_reg_or(edt_p, PCD_FUNCT, 0x08);
   

    edt_start_buffers(edt_p, 0) ; /* start the transfers in free running mode */

    for (i = 0; (i < loops || loops == 0); i++)
    {
        buf = edt_wait_for_buffers(edt_p, 1) ;

        if (outfile) fwrite(buf, 1, bufsize, outfile) ;

	if (verbose)
	    printf("buffers completed:  application %d  driver %d\n",
                                    i+1, edt_done_count(edt_p)) ;
	else
	{
	    putchar('.') ;
	    fflush(stdout) ;
	}
    }
    printf("buffers completed:  application %d  driver %d\n",
				i, edt_done_count(edt_p)) ;

    edt_close(edt_p) ;

    exit(0) ;
}

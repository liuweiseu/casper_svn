/* #pragma ident "@(#)rd4.c	1.1 05/26/04 EDT" */

#include "edtinc.h"
#include <stdlib.h>

EdtDev *edt_p ;
int unit 	= 0 ;
int channel 	= 0 ;
int loops 	= 10 ;
int numbufs  	= 4 ;
int bufsize  	= 1024 ;

void
usage()
{
    puts("usage: rd4") ;
    puts("    -u <unit>   - specifies edt board unit number (default 0)") ;
    puts("    -c <chan>   - specifies channel number (0-3) (0)") ;
    puts("    -s <size>   - specifies number of bytes per buffer (1024)") ;
    puts("    -l <loops>  - specifies number of buffers to loop over (10)") ;
    puts("    -o <outfile>  - save output to <outfile> (none)") ;
}

int
main(argc, argv)
int argc;
char **argv;
{
    int i, first = 1, errors = 0 ;
    u_char *buf, *testbuf, val ;
    FILE *outfp = NULL ;

    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'u':
            ++argv;
            --argc;
            unit = atoi(argv[1]);
            break ;

	case 'c':
            ++argv;
            --argc;
	    channel = atoi(argv[1]) ;
            break ;

	case 's':
            ++argv;
            --argc;
	    bufsize = atoi(argv[1]) ;
	    if (bufsize % 4)
	    {
		puts("ERROR: bufsize must be a multiple of 4 bytes. Exiting\n");
		exit(1);
	    }
            break ;

	case 'l':
            ++argv;
            --argc;
	    loops = atoi(argv[1]) ;
            break ;

	case 'o':
            ++argv;
            --argc;
	    if ((outfp  = fopen(argv[1], "wb")) == NULL)
	    {
		perror(argv[1]) ;
		exit(1) ;
	    }
            break ;

        default:
            usage() ;
            exit(1) ;
        }
        --argc ;
        ++argv ;
    }

    if ((edt_p = edt_open_channel("pcd", unit, channel)) == NULL)
    {
        perror("edt_open_channel") ;
        exit(1) ;
    }

    if (edt_configure_ring_buffers(edt_p, bufsize, numbufs, EDT_READ, NULL)
								     == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

    edt_flush_fifo(edt_p);
    edt_start_buffers(edt_p, loops) ;

    for (i = 0; (i < loops || loops == 0); i++)
    {
        buf = edt_wait_for_buffers(edt_p, 1) ;

	if (outfp)
	{
	    fwrite(buf, 1, bufsize, outfp);
	    fflush(outfp);
	}

	printf("%d ", i+1);

	fflush(stdout) ;

    }
    puts("") ;

    if (outfp)
	fclose(outfp) ;

    edt_close(edt_p) ;

    exit(errors) ;
}

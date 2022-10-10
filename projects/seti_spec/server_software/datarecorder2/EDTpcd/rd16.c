/* #pragma ident "@(#)rd16.c	1.6 10/21/04 EDT" */

#include "edtinc.h"
#include <stdlib.h>

EdtDev *edt_p ;
int unit 	= 0 ;
int channel 	= 0 ;
int loops 	= 100 ;
int numbufs  	= 4 ;
int verbose	= 0 ;
int bufsize  	= 128*1024 ;

void
usage()
{
    puts("usage: rd16") ;
    puts("    -u <unit>   - specifies edt board unit number") ;
    puts("    -c <chan>   - specifies channel number (0-15)") ;
    puts("    -l <loops>  - specifies number of test loops") ;
    puts("    -o <outfile>  - save output to <outfile>") ;
    puts("    -v	    - verbose") ;
}

int
main(argc, argv)
int argc;
char **argv;
{
    int i;
    u_char *buf ;
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

        case 'v':
            verbose = 1 ;
            break ;

	case 'c':
            ++argv;
            --argc;
	    channel = atoi(argv[1]) ;
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

    if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
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

    edt_start_buffers(edt_p, loops) ;

    for (i = 0; (i < loops || loops == 0); i++)
    {
        buf = edt_wait_for_buffers(edt_p, 1) ;

	printf("r%d:  %3x %3x %3x %3x %3x %3x %3x %3x\n\n",
	    channel,
	    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]) ;

	if (outfp)
	    fwrite(buf, 1, bufsize, outfp) ;

	memset(buf, 0, bufsize) ;
    }
    printf("buffers completed:  application %d  driver %d\n",
				i, edt_done_count(edt_p)) ;

    if (outfp)
	fclose(outfp) ;

    edt_close(edt_p) ;

    exit(0) ;
}

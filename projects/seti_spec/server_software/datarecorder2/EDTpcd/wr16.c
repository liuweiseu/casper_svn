/* #pragma ident "@(#)wr16.c	1.5 10/21/04 EDT" */

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
    puts("    -v	    - verbose") ;
}

int
main(argc, argv)
int argc;
char **argv;
{
    int i;
    u_char *buf ;
    u_char **bufs ;

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

    if (edt_configure_ring_buffers(edt_p, bufsize, numbufs, EDT_WRITE, NULL)
								      == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

    bufs = edt_buffer_addresses(edt_p) ;

    for (i = 0; i < numbufs; i++)
    {
	memset(bufs[i], 0xa5, bufsize) ;
    }

    edt_start_buffers(edt_p, loops) ;

    for (i = 0; (i < loops || loops == 0); i++)
    {
        buf = edt_wait_for_buffers(edt_p, 1) ;

	printf("w%d ", channel) ;
	fflush(stdout) ;
    }
    printf("buffers completed:  application %d  driver %d\n",
				i, edt_done_count(edt_p)) ;
    edt_close(edt_p) ;

    exit(0) ;
}

/* #pragma ident "@(#)wr16.c	1.4 07/17/01 EDT" */

#include "edtinc.h"
#include <stdlib.h>

EdtDev *edt_p ;
int unit 	= 0 ;
int channel 	= 2 ;
int loops 	= 1 ;
int numbufs  	= 1 ;
int verbose	= 0 ;
int bufsize  	= 0 ;

void
usage()
{
    puts("usage: wfile [optargs] <outfilename>") ;
    puts("    -u <unit>   - specifies edt board unit number") ;
    puts("    -c <chan>   - specifies channel number (0-15)") ;
    puts("    -l <loops>  - specifies number of test loops") ;
}

int
main(argc, argv)
int argc;
char **argv;
{
    int i, timeout = 0;
    char datafn[128];
    u_char *buf ;
    u_char **bufs ;
    FILE *outfp = NULL;
    FILE *infp = NULL;
    unsigned int header;

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

    if (argc < 2)
    {
    	usage();
	exit(1);
    }

    if ((outfp = fopen(argv[1], "r")) == NULL)
    {
    	edt_perror(argv[1]);
	exit(1);
    }

    fseek(outfp, 0, SEEK_END);
    bufsize = ftell(outfp);
    fseek(outfp, 0, SEEK_SET);

    if ((edt_p = edt_open_channel("pcd", unit, channel)) == NULL)
    {
        perror("edt_open_channel write") ;
        exit(1) ;
    }
    printf("bufsize %d\n", bufsize);
    if (edt_configure_ring_buffers(edt_p,bufsize,numbufs,EDT_WRITE,NULL) == -1)
    {
        perror("edt_configure_ring_buffers write") ;
        exit(1) ;
    }


    bufs = edt_buffer_addresses(edt_p) ;


    fread(bufs[0], 1, bufsize, outfp) ;
    fclose(outfp);

    printf("** %08x", header);

    printf("** %02x %02x %02x %02x **\n", bufs[0][0],bufs[0][1],bufs[0][2],
		    						bufs[0][3]);
    printf("** %02x %02x %02x %02x **\n", bufs[0][4],bufs[0][5],bufs[0][6],
		    						bufs[0][7]);
    edt_set_firstflush(edt_p, EDT_ACT_NEVER);
    edt_startdma_action(edt_p, EDT_ACT_NEVER);

    edt_reg_write(edt_p, SSD16_LSB, 0x4);
    edt_reg_write(edt_p, PCD_CMD, 0);
    edt_reg_write(edt_p, SSD16_CHEN, 0x4);

    edt_flush_fifo(edt_p);
    edt_start_buffers(edt_p, 1) ;
    edt_reg_write(edt_p, PCD_CMD, 8);

    for (i = 0; timeout == 0 && (i < loops || loops == 0); i++)
    {
        edt_wait_for_buffers(edt_p, 1) ;
	puts("done waiting for output");

	fflush(stdout) ;
    }
    puts("\ndone");
    edt_close(edt_p) ;
}

/* #pragma ident "@(#)wr16.c	1.4 07/17/01 EDT" */

#include "edtinc.h"
#include <stdlib.h>

EdtDev *edt_r ;
int unit 	= 0 ;
int channel 	= 0 ;
int loops 	= 1 ;
int numbufs  	= 1 ;
int verbose	= 0 ;
int bufsize  	= 1024*1080 ;
int test	= 940;

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
    u_char *buf=NULL ;
    u_char **bufs ;
    FILE *outfp = NULL;
    FILE *infp = NULL;
    u_char err[2];

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

    if ((edt_r = edt_open_channel("pcd", unit, channel)) == NULL)
    {
        perror("edt_open_channel read") ;
        exit(1) ;
    }
    printf("bufsize %d\n", bufsize);
    if (edt_configure_ring_buffers(edt_r,bufsize,numbufs,EDT_READ,NULL) == -1)
    {
        perror("edt_configure_ring_buffers read") ;
        exit(1) ;
    }

    edt_set_rtimeout(edt_r, 10000);
    edt_set_firstflush(edt_r, EDT_ACT_NEVER);
    edt_startdma_action(edt_r, EDT_ACT_NEVER);

    if (channel==0) {
    	edt_reg_write(edt_r, SSD16_LSB, 0x1);
    	edt_reg_write(edt_r, SSD16_CHEN , 0x1);
    } else if (channel==1) {
    	edt_reg_write(edt_r, SSD16_LSB, 0x2);
    	edt_reg_write(edt_r, SSD16_CHEN , 0x2);
    }
	    

    edt_reg_write(edt_r, PCD_CMD, 0);


    sprintf(datafn, "%s", argv[1]);
    if ((infp = fopen(datafn, "w")) == NULL)
    {
    	edt_perror(datafn);
	exit(1);
    }

    edt_flush_fifo(edt_r);
    edt_reg_write(edt_r, PCD_CMD, 8);
    edt_start_buffers(edt_r, loops) ;


    for (i = 0; timeout == 0 && (i < loops || loops == 0); i++)
    {
        buf = edt_wait_for_buffers(edt_r, 1) ;
	puts("done waiting for input");

	if ((timeout=edt_timeouts(edt_r)) > 0) {
		bufsize = edt_get_timeout_count(edt_r);
		printf("Timeout: count %d\n", bufsize);
	}
	printf("writing bufsize %d\n", bufsize);
	err[0] = edt_intfc_read(edt_r, 0x50);
        err[1] = edt_intfc_read(edt_r,0x51);
	err[2] = edt_intfc_read(edt_r, 0x52);
	printf("0x50: %02x\n0x51: %02x\n0x52: %02x\n", err[0], err[1], err[2]);
	fwrite(buf, 1, bufsize, infp);
/*	fwrite(&test, 1, 4, infp); */
	perror(datafn);

	putchar((timeout) ? 't' : '.');
	fflush(stdout) ;
    }
    puts("\ndone");
    fclose(infp);
    edt_close(edt_r) ;
}

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

#define ARM 1
#define DISARM 0

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
    int     action = ARM;
    u_char *buf ;
    int bufsize = 1024*1024;
    //int loops = 300;
    int loops = 10;
    unsigned old_reg_val, new_reg_val;


    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'u':
            ++argv;
            --argc;
            unit = atoi(argv[1]);
            break ;

        case 'a':
            action = ARM;
            break ;

        case 'd':
            action = DISARM;
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

    //edt_flush_fifo(edt_p) ;         /* Flush the input fifo */

    // This line added by Aaron 12/3/02
    // Used to set FUNCT3 high
    if (action == ARM) {
        old_reg_val = edt_reg_read(edt_p, PCD_FUNCT);
        new_reg_val = edt_reg_or(edt_p, PCD_FUNCT, 0x08);
        fprintf(stderr, "old val = %08x  new val = %08x\n", old_reg_val, new_reg_val);
    } else if (action == DISARM) {
        old_reg_val = edt_reg_read(edt_p, PCD_FUNCT);
        new_reg_val = edt_reg_and(edt_p, PCD_FUNCT, 0xf7);
        fprintf(stderr, "old val = %08x  new val = %08x\n", old_reg_val, new_reg_val);
    }

    edt_close(edt_p) ;

    exit(0) ;
}

/* #pragma ident "@(#)pdma.c	1.11 10/18/02 EDT" */

/*
 * See /opt/EDTpcd/README.pdma
 */

#include "edtinc.h"
#include "libpdma.h"

static char s[128] ;
static u_char *databuf ;

main(argc, argv)
int argc;
char **argv;
{
    int i, mode, bcount = PDMA_MAX_SIZE ;
    EdtDev *edt_p ;


    if (argc > 1)
	bcount = atoi(argv[1]) ;


    /*
     * Open pcd0
     */
    if ((edt_p = edt_open("pcd", 0)) == NULL)
    {
	perror("edt_open") ;
	exit(1) ;
    }

    databuf = edt_alloc(PDMA_MAX_SIZE) ;


    /*
     * Initialize pdma mode.
     */
    if (pdma_init(edt_p, PDMA_MAX_SIZE, 1, NULL) != 0)
    {
	edt_perror("pdma_init failed") ;
	exit(1) ;
    }

    pdma_dmaint_enable(edt_p, 0) ;



    /*
     * Now ready for Programmed DMA
     */

    for (i = 0; ; ++i)
    {

	pdma_flush_fifo(edt_p) ;


	puts("Hit return to start or 'q' to exit") ;
	fgets(s, 128, stdin) ;
	if (*s == 'q')
	    break ;


	memset(databuf, i, bcount) ;
	pdma_set_size(edt_p, bcount) ;
	pdma_write_databuf(edt_p, 0, databuf, bcount) ;



	puts("Hit return to trigger (enter 'a' to abort)") ;
	fgets(s, 128, stdin) ;

	pdma_start_dma(edt_p) ;
	if (*s == 'a')
	    pdma_abort_dma(edt_p) ;

	sleep(1) ;
    }

    pdma_close(edt_p) ;

}

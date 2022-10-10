
/*
 * set_clock.c: 
 *
 * To compile:
 *  cc -O -DSYSV -o simple_getdata simple_getdata.c -L. -ledt -lthread
 *
 *
 */

#include "edtinc.h"
#include <stdlib.h>

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    edt_pll tx_clock;
    int    unit = 0;
    int verbose = 1;
    double freq, target_freq;

    unit        = atof(argv[1]);
    target_freq = atof(argv[2]);

    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
        perror("edt_open") ;
        exit(1) ;
    }

    freq = edt_find_vco_frequency(edt_p, target_freq, XTAL20, &tx_clock, verbose);

    edt_set_pll_clock(edt_p, XTAL20, &tx_clock, verbose);

    printf("%f\n", freq);

    edt_close(edt_p) ;

    exit(0) ;
}

/*
 * simple_waveform.c: 
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

#ifndef DATAREC_H
#include "datarec.h"
#endif

#define SAMPLES 6400
#define PNTS_PR_SCRN 64

void wave_out(short *data);

void
usage()
{
    printf("usage: simple_waveform\n") ;
    printf("Takes no arguments\n") ;
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p0, *edt_p1 ;
    int     i;
    short *buf ;
	short data0[SAMPLES], data1[SAMPLES];
	int bufsize = 1024*1024;
	int offset = 0;

    if (argc > 1) { usage(); exit(1); }

    if ((edt_p0 = edt_open(EDT_INTERFACE, 0)) == NULL) {
        perror("edt_open") ;
        exit(1) ;
    }

	if ((edt_p1 = edt_open(EDT_INTERFACE, 1)) == NULL) {
		perror("edt_open") ;
		exit(1) ;
	}

    if (edt_configure_ring_buffers(edt_p0, bufsize, 4, EDT_READ, NULL) == -1) {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

    if (edt_configure_ring_buffers(edt_p1, bufsize, 4, EDT_READ, NULL) == -1) {
    	perror("edt_configure_ring_buffers") ;
    	exit(1) ;
    }

	arm_rec(edt_p0); 

    edt_start_buffers(edt_p0, 0) ;
    edt_start_buffers(edt_p1, 0) ;

    buf = (short *) edt_wait_for_buffers(edt_p0, 1) ;
    for (i = 0; i < SAMPLES; i++) {
		data0[i] = buf[i];
    }

    buf = (short *) edt_wait_for_buffers(edt_p1, 1) ;
    for (i = 0; i < SAMPLES; i++) {
        data1[i] = buf[i];
    }
	
	for (i = 0; i < SAMPLES; i += PNTS_PR_SCRN) {
		system("clear");
		wave_out(data0 + i);
		wave_out(data1 + i);
		getchar();
	}

	disarm_rec(edt_p0);
    edt_close(edt_p0) ;
    edt_close(edt_p1) ;
    exit(0) ;
}

void wave_out(short *data) {
	int bit, i;
	
	printf("\n");

	for (bit = 0; bit < 16; bit++) {
		printf("BIT%2d:", bit);
		for (i = 0; i < PNTS_PR_SCRN; i++) {
			if (((data[i] >> bit) & 0x0001) == 0x0001) {
				printf("-");
			} else {
				printf("_");
			}
		}
		printf("\n");
	}
}

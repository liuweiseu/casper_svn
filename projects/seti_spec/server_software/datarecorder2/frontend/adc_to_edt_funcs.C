/*
 *  This is a sample implementation of the functions for
 *  reading the gain control voltage (VGC) for a channel
 *  from the down-converter board.
 *
 * relevant functions:
 *    arm_rec: arms data recorder, must be called after edt_open
 *             and before read_vgc.
 *    read_vgc: returns 16 bit short representing the VGC.
 *    disarm_rec: disarms data recorder, should be called before edt_close.
 *
 *  The data recorder (down-converter board + ADC to EDT board)
 *  must be attached to the EDT card specified by 'unit'.
 */

#ifndef DATAREC_H
#include "edtinc.h"
#include <stdlib.h>
#endif


#define CS_BIT		0x01
#define DIN_BIT		0x02
#define SCLK_BIT	0x04
#define CLK_EN_BIT	0x08
#define DOUT_BIT	0x01
#define SSTRB_BIT	0x02

unsigned short read_vgc(EdtDev *edt_p, int chan);
void set_bit(EdtDev *edt_p, uint bitCode, int val);
void clk_sclk(EdtDev *edt_p);
int is_calc(EdtDev *edt_p);
int next_dout_bit(EdtDev *edt_p);
void arm_rec(EdtDev *edt_p);
void disarm_rec(EdtDev *edt_p);

/*
 * read_vgc:
 *  This function returns an unsigned short representing the gain
 *  control voltage on the specified channel (0-7).  This voltage
 *  is proportional to the incident signal strength (i.e. inversely
 *  proportional to the gain).  Returns 0 on failure.
 */

unsigned short read_vgc(EdtDev *edt_p, int chan) {
	unsigned short data;
	int i;

    // Check channel and EdtDev validity
    if ((edt_p == NULL) || (chan < 0) || (chan > 7)) {
		return 0;
    }
	
	set_bit(edt_p, CS_BIT, 0);
	set_bit(edt_p, DIN_BIT, 1);
	clk_sclk(edt_p);

    // Set Channel Select bit 2
	set_bit(edt_p, DIN_BIT, (chan % 2) ? 1 : 0);
	clk_sclk(edt_p);

    // Set Channel Select bit 1
	set_bit(edt_p, DIN_BIT, (chan >= 4) ? 1 : 0);
	clk_sclk(edt_p);

    // Set Channel Select bit 0
	set_bit(edt_p, DIN_BIT, chan % 4 >= 2);
	clk_sclk(edt_p);

	// Set Mode to...
	set_bit(edt_p, DIN_BIT, 1);		// Unipolar
	clk_sclk(edt_p);
	set_bit(edt_p, DIN_BIT, 1);		// Single Ended
	clk_sclk(edt_p);
	set_bit(edt_p, DIN_BIT, 1);		// Internal Clk bit 1	
	clk_sclk(edt_p);
	set_bit(edt_p, DIN_BIT, 0);		// Internal Clk bit 0
	clk_sclk(edt_p);

	set_bit(edt_p, CS_BIT, 1);

	// Wait while calculating
	while (i = is_calc(edt_p)) {}

	set_bit(edt_p, CS_BIT, 0);
	clk_sclk(edt_p);

	// Get data
	for (i = 0; i < 16; i++) {
		data = 2 * data + next_dout_bit(edt_p);
	}
	
	set_bit(edt_p, CS_BIT, 1);
	return data;
}			

void set_bit(EdtDev *edt_p, uint bitCode, int val) {
	if (val == 1) {
		edt_reg_or(edt_p, PCD_FUNCT, bitCode);
	} else {
		edt_reg_and(edt_p, PCD_FUNCT, bitCode ^ 0x0f);
	}
}

void clk_sclk(EdtDev *edt_p) {
	set_bit(edt_p, SCLK_BIT, 1);
	set_bit(edt_p, SCLK_BIT, 0);
}

int is_calc(EdtDev *edt_p) {
	uint statDat = edt_reg_read(edt_p, PCD_STAT);
	return (statDat ^ SSTRB_BIT);
}

int next_dout_bit(EdtDev *edt_p) {
	uint statDat = edt_reg_read(edt_p, PCD_STAT);
	clk_sclk(edt_p);
	return ((statDat & DOUT_BIT) ? 1 : 0);
}

// Arms the data recorder, edt_p must be a point to unit 0.
void arm_rec(EdtDev *edt_p) {
	if (edt_p != NULL) {
		// To avoid clocking in bad data on a prearmed board, disarm 
		// board, clear fifo, then arm board.
		set_bit(edt_p, CLK_EN_BIT, 0);
		edt_flush_fifo(edt_p);
		set_bit(edt_p, CLK_EN_BIT | CS_BIT, 1);
		set_bit(edt_p, DIN_BIT | SCLK_BIT, 0);
	}
}

// Disarms the data recorder, edt_p must point to unit 0;
void disarm_rec(EdtDev *edt_p) {
	if (edt_p != NULL) {
		set_bit(edt_p, CLK_EN_BIT, 0);
	}
}

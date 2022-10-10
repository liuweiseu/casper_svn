/* 
 *  datarec.h
 * 
 *  This header file includes the functions necessary to talk
 *  to the data recorder.
 */

#ifndef DATAREC_H 

#define DATAREC_H
	
#include "edtinc.h"
#include <stdlib.h>
#include <stdio.h>

extern unsigned short read_vgc(EdtDev *, int);
extern void arm_rec(EdtDev *);
extern void disarm_rec(EdtDev *);

#endif

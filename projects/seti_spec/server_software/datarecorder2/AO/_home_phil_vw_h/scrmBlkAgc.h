#ifndef scrmBAgcInc
#define scrmBAgcInc

#include	<scrmLib.h>
#include	"agcProgState.h"


typedef struct {
	AGC_STATE	st;
#if FALSE 
	int		    statWd;			/* summary bit status*/
	int		    secMLastTick;	/* secs from midnight last tick*/
	int			connectVtxStat; /* 0 no, 1 listen,2 accept,3 connected*/
	int			masterMode;	    /* 1 greg compute ch
                                   2 ch  compute gr,
                                   3 greg user sets ch
                                   4 ch   user sets gr*/


	int		    agcAxisCtrl;    /* 1 az,2 gr,4 ch*/
	int		    timePosReq;		/* millisecs from midnite*/
	int			reqPosTTD[3];	/* 1/10000 of a degree*/
	int			conPosTTD[3];   /* ditto*/
	/* see cblk for actual position */
	double      posErrsReqRd[3]; /*radians. az,gr,ch requested position */
	double      posErrsConRd[3]; /*radians. az,gr,ch constrained position*/

	int		    totVtxErrs;		/* total number from cblk/fblk*/
	VTX_CBLK	cblk;
	VTX_FBLK    fblk;
#endif
 	} SCRM_B_AGC;
#endif

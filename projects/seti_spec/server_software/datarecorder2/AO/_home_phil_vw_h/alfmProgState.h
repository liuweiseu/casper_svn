#ifndef     INCalfmProgStateh
#define     INCalfmProgStateh
#include	"alfaMot.h"

typedef struct {
    unsigned int    free:29; /* status*/
    unsigned int    lastMotStatOk:1;/* last motor status had no error*/
    unsigned int     wdErr:1;/* last i/o canceled via watchdog*/
    unsigned int    ampErr:1;/* last i/o map returned error*/
    } ALFM_PROG_STAT;

typedef struct {
	ALFM_PROG_STAT  prgStat;
	int            secMidL; /* seconds midnight last tick*/

   	float       curPosDeg;
   	int         curPosEnc;

    int         curPosSec;  /* sec midnite last successful query*/
    float       curPosErrDeg;/* current position error in degrees*/

/*
 * requested info
*/
    float       lastReqPosDeg;
    int         lastReqPosEnc;

    float       lastReqVelDs;
    int         lastReqVelRPM;

    int         lastReqSec;
    int         lastMstatSec;/* sec midnite last successful stat*/

    ALFAM_STATUS    mstat;       /* motor status   14 words         */
/*
 * i/o info
*/
	 unsigned int  wdCnt;      /* wd timeouts in task*/
     unsigned int  wdCntWr;    /* wd timeouts write in task*/

     unsigned int  cntWrErr;   /* cum count write errors*/
     int           wrLastErr;  /* errno last write error*/

     unsigned int   wdCntRd;   /* wd timeouts read in task*/
     unsigned int  cntRdErr;   /* cum count write errors*/

     int          rdLastErr;   /* errno last read error*/
     int          curTmOutTicks; /* current timeout to use in ticks*/

	 int		  fill[6];
} ALFM_PROG_STATE;		/* 40 words*/

#endif

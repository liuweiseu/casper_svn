/*****************************************************************************
 *
 * FILE: flip.c
 *
 *   ...
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * jeffh        25 Aug 2004    Original coding
 * lerner        9 Nov 2005    Adapted for move to /home/cima
 * lerner        4 Dec 2006    Added new variables
 * lerner        6 Feb 2007    Added 'wapplib.h' and cleaned up for '-Wall'
 * lerner       19 Nov 2007    Added some new variables to 'flip_pnt' and
 *                             adapted for new 'execshm.h', 'agcProgState.h',
 *                             'pntProgState.h' and 'if2ProgState.h'
 *
 *****************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "mshmLib.h"
#include "execshm.h"
#include "wappshm.h"
#include "alfashm.h"
#include "wapplib.h"


#define FLIP(x)    flip_data(&(x), sizeof(x))


void flip_agc(SCRM_B_AGC *agc);
void flip_tt(SCRM_B_TT *tt);
void flip_pnt(SCRM_B_PNT *pnt);
void flip_tie(SCRM_B_TIE *tie);
void flip_if1(SCRM_B_IF1 *if1);
void flip_if2(SCRM_B_IF2 *if2);
void flip_exec(struct EXECSHM *exec);
void flip_wapp(struct WAPPSHM *wapp);
void flip_alfa(struct ALFASHM *alfa);
void flip_data(void *v, int n);

static void flip_uintbits(unsigned int *u);
static void generic_bitflip(unsigned int *pvalue, char *str);



void flip_agc(SCRM_B_AGC *agc) {

  FLIP(agc->st.secMLastTick);
  FLIP(agc->st.auxReq[0]);
  FLIP(agc->st.auxReq[1]);
  FLIP(agc->st.auxReq[2]);
  FLIP(agc->st.cblkMCur.dat.modeAz);
  FLIP(agc->st.cblkMCur.dat.modeGr);
  FLIP(agc->st.cblkMCur.dat.modeCh);
  FLIP(agc->st.cblkMCur.dat.velAz);
  FLIP(agc->st.cblkMCur.dat.velGr);
  FLIP(agc->st.cblkMCur.dat.velCh);
  FLIP(agc->st.cblkMCur.dat.posAz);
  FLIP(agc->st.cblkMCur.dat.posGr);
  FLIP(agc->st.cblkMCur.dat.posCh);
  FLIP(agc->st.cblkMCur.dat.statAz);
  FLIP(agc->st.cblkMCur.dat.timeMs);
  FLIP(agc->st.fblkMCur.dat.azI.ampStat);
  FLIP(agc->st.fblkMCur.dat.azI.equipStat);
  FLIP(agc->st.fblkMCur.dat.azI.motorStat);
  FLIP(agc->st.fblkMCur.dat.chI.ampStat);
  FLIP(agc->st.fblkMCur.dat.chI.equipStat);
  FLIP(agc->st.fblkMCur.dat.chI.motorStat);
  FLIP(agc->st.fblkMCur.dat.grI.ampStat);
  FLIP(agc->st.fblkMCur.dat.grI.equipStat);
  FLIP(agc->st.fblkMCur.dat.grI.motorStat);
  FLIP(agc->st.mPosDataReq[0]);
  FLIP(agc->st.mPosDataReq[1]);
  FLIP(agc->st.mPosDataReq[2]);
  FLIP(agc->st.masterMode);
  FLIP(agc->st.posErr.aoCurSec.req.posTTD[0]);
  FLIP(agc->st.posErr.aoCurSec.req.posTTD[1]);
  FLIP(agc->st.posErr.aoCurSec.req.posTTD[2]);
  FLIP(agc->st.posErr.aoCurSec.req.timeMs);
  FLIP(agc->st.posErr.aoPrevSec.req.posTTD[0]);
  FLIP(agc->st.posErr.aoPrevSec.req.posTTD[1]);
  FLIP(agc->st.posErr.aoPrevSec.req.posTTD[2]);
  FLIP(agc->st.posErr.conPosDifRd[0]);
  FLIP(agc->st.posErr.conPosDifRd[1]);
  FLIP(agc->st.posErr.conPosDifRd[2]);
  FLIP(agc->st.posErr.reqPosDifRd[0]);
  FLIP(agc->st.posErr.reqPosDifRd[1]);
  FLIP(agc->st.posErr.reqPosDifRd[2]);
}



void flip_tt(SCRM_B_TT *tt) {

  FLIP(tt->st.slv[0].inpMsg.devStat);
  FLIP(tt->st.slv[0].inpMsg.position);
  FLIP(tt->st.slv[1].inpMsg.devStat);
  FLIP(tt->st.slv[1].inpMsg.position);
  FLIP(tt->st.slv[2].inpMsg.devStat);
  FLIP(tt->st.slv[2].inpMsg.position);
  FLIP(tt->st.slv[3].inpMsg.devStat);
  FLIP(tt->st.slv[3].inpMsg.position);

  FLIP(tt->st.slv[0].data.val[TTT_GSIND_TT_DI_UIO3]); /* 15, IlKey */
}



void flip_pnt(SCRM_B_PNT *pnt) {

  FLIP(pnt->st.x.pl.curP.pnt.pos.c1);
  FLIP(pnt->st.x.pl.curP.pnt.pos.c2);
  FLIP(pnt->st.x.pl.curP.pnt.pos.cs);
  FLIP(pnt->st.x.pl.curP.pnt.pos.st);

  FLIP(pnt->st.x.pl.curP.pnt.off.c1);
  FLIP(pnt->st.x.pl.curP.pnt.off.c2);
  FLIP(pnt->st.x.pl.curP.pnt.off.st);
  FLIP(pnt->st.x.pl.curP.pnt.off.cs);

  FLIP(pnt->st.x.pl.curP.pnt.rate.c1);
  FLIP(pnt->st.x.pl.curP.pnt.rate.c2);
  FLIP(pnt->st.x.pl.curP.pnt.rate.st);
  FLIP(pnt->st.x.pl.curP.pnt.rate.cs);
  FLIP(pnt->st.x.pl.curP.pnt.rateStDayNum);
  FLIP(pnt->st.x.pl.curP.pnt.rateDur);

  FLIP(pnt->st.x.pl.curP.raDecAppV[0]);
  FLIP(pnt->st.x.pl.curP.raDecAppV[1]);
  FLIP(pnt->st.x.pl.curP.raDecAppV[2]);
  FLIP(pnt->st.x.pl.curP.raDecTrueV[0]);
  FLIP(pnt->st.x.pl.curP.raDecTrueV[1]);
  FLIP(pnt->st.x.pl.curP.raDecTrueV[2]);

  FLIP(pnt->st.x.pl.curP.haAppRd);
  FLIP(pnt->st.x.pl.curP.azRd);
  FLIP(pnt->st.x.pl.curP.zaRd);
  FLIP(pnt->st.x.pl.curP.corAzRd);
  FLIP(pnt->st.x.pl.curP.corZaRd);
  FLIP(pnt->st.x.pl.curP.modelCorAzRd);
  FLIP(pnt->st.x.pl.curP.modelCorZaRd);

  FLIP(pnt->st.x.pl.curP.raJ);
  FLIP(pnt->st.x.pl.curP.decJ);

  FLIP(pnt->st.x.pl.model.encOffAzRd);
  FLIP(pnt->st.x.pl.model.encOffZaRd[0]);
  FLIP(pnt->st.x.pl.model.encOffZaRd[1]);

  FLIP(pnt->st.x.pl.tm.secMidD);
  FLIP(pnt->st.x.pl.tm.lmstRd);

  FLIP(pnt->st.x.pl.tm.dayNum);
  FLIP(pnt->st.x.pl.tm.mjd);
  FLIP(pnt->st.x.pl.tm.astFrac);
  FLIP(pnt->st.x.pl.tm.ut1Frac);
  FLIP(pnt->st.x.pl.tm.year);
  FLIP(pnt->st.x.pl.req.master);
  FLIP(pnt->st.x.pl.req.trackTol);
  FLIP(pnt->st.x.pl.req.wrapReq);

  FLIP(pnt->st.x.pl.req.reqState);
  FLIP(pnt->st.x.pl.curP.geoVelProj);
  FLIP(pnt->st.x.pl.curP.helioVelProj);
  FLIP(pnt->st.x.pl.curP.parallacticRd);

  flip_uintbits((unsigned int *) &pnt->st.x.statWd); /* all bit fields are :1 */

  FLIP(pnt->st.x.modelCorEncAzZa[0]);
  FLIP(pnt->st.x.modelCorEncAzZa[1]);
}



void flip_tie(SCRM_B_TIE *tie) {

  FLIP(tie->st.slv[0].inpMsg.devStat);
  FLIP(tie->st.slv[0].inpMsg.ldCell1);
  FLIP(tie->st.slv[0].inpMsg.ldCell2);
  FLIP(tie->st.slv[0].inpMsg.position);
  flip_uintbits((unsigned int *) &tie->st.slv[0].statWd);
  FLIP(tie->st.slv[0].data.val[TTT_GSIND_GEN_SAFETY]);
  FLIP(tie->st.slv[0].data.val[TTT_GSIND_GEN_FAULTS]);
  FLIP(tie->st.slv[0].ioFail);
  FLIP(tie->st.slv[0].tickMsg.timeMs);

  FLIP(tie->st.slv[1].inpMsg.devStat);
  FLIP(tie->st.slv[1].inpMsg.ldCell1);
  FLIP(tie->st.slv[1].inpMsg.ldCell2);
  FLIP(tie->st.slv[1].inpMsg.position);
  flip_uintbits((unsigned int *) &tie->st.slv[1].statWd);
  FLIP(tie->st.slv[1].data.val[TTT_GSIND_GEN_SAFETY]);
  FLIP(tie->st.slv[1].data.val[TTT_GSIND_GEN_FAULTS]);
  FLIP(tie->st.slv[1].ioFail);
  FLIP(tie->st.slv[1].tickMsg.timeMs);

  FLIP(tie->st.slv[2].inpMsg.devStat);
  FLIP(tie->st.slv[2].inpMsg.ldCell1);
  FLIP(tie->st.slv[2].inpMsg.ldCell2);
  FLIP(tie->st.slv[2].inpMsg.position);
  flip_uintbits((unsigned int *) &tie->st.slv[2].statWd);
  FLIP(tie->st.slv[2].data.val[TTT_GSIND_GEN_SAFETY]);
  FLIP(tie->st.slv[2].data.val[TTT_GSIND_GEN_FAULTS]);
  FLIP(tie->st.slv[2].ioFail);
  FLIP(tie->st.slv[2].tickMsg.timeMs);
}



void flip_if1(SCRM_B_IF1 *if1) {

  FLIP(if1->st.rfFreq);
  generic_bitflip((unsigned int *) &if1->st.stat1, "5,3,1,1,1,1,1,9,9,1");
  generic_bitflip((unsigned int *) &if1->st.stat2,
		  "4,4,4,4,1,1,1,1,4,1,1,1,1,1,1,1,1");
  FLIP(if1->st.synI.freqHz[0]);
  FLIP(if1->st.synI.freqHz[1]);
  FLIP(if1->st.synI.ampDb[0]);
  FLIP(if1->st.synI.ampDb[1]);

  FLIP(if1->st.pwrmI.pwrDbm[1]);
  FLIP(if1->st.pwrmI.pwrDbm[0]);
  FLIP(if1->st.pwrmI.timeStamp);

  FLIP(if1->st.if1FrqMhz);
  FLIP(if1->st.lbnFbFreq);
}



void flip_if2(SCRM_B_IF2 *if2) {

  FLIP(if2->st.gain[0]);
  FLIP(if2->st.gain[1]);

  FLIP(if2->st.synI.freqHz[0]);
  FLIP(if2->st.synI.freqHz[1]);
  FLIP(if2->st.synI.freqHz[2]);
  FLIP(if2->st.synI.freqHz[3]);
  FLIP(if2->st.synI.freqHz[4]);
  FLIP(if2->st.synI.freqHz[5]);
  FLIP(if2->st.synI.freqHz[6]);
  FLIP(if2->st.synI.freqHz[7]);

  FLIP(if2->st.pwrmI.pwrDbm[0]);
  FLIP(if2->st.pwrmI.pwrDbm[1]);
  FLIP(if2->st.pwrmI.timeStamp);
  generic_bitflip((unsigned int *) &if2->st.stat1,
		  "2,1,1,1,1,1,1,4,1,1,2,1,2,13");

  generic_bitflip((unsigned int *) &if2->st.stat8[0], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[1], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[2], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[3], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[4], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[5], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[6], "2,2,2,7,19");
  generic_bitflip((unsigned int *) &if2->st.stat8[7], "2,2,2,7,19");
}



void flip_exec(struct EXECSHM *exec) {

  int i;

  FLIP(exec->velocity);
  FLIP(exec->ra);
  FLIP(exec->dec);
  FLIP(exec->off1);
  FLIP(exec->off2);

  FLIP(exec->custom_iflo);
  FLIP(exec->dop_mode);
  FLIP(exec->radar_blanker);
  FLIP(exec->winking_cal);

  FLIP(exec->centfreq);
  for ( i = 0 ; i < 8 ; i++ ) {
    FLIP(exec->restfreq[i]);
    FLIP(exec->destfreq[i]);
  }

  FLIP(exec->corr_estart);
  FLIP(exec->corr_dumplen);
  FLIP(exec->corr_icyc);
  FLIP(exec->corr_blank);

  FLIP(exec->wapp_dual);
  FLIP(exec->wapp_alfa);
  for ( i = 0 ; i < 4 ; i++ )
    FLIP(exec->enabled_wapps[i]);

  FLIP(exec->pattern_id);
  FLIP(exec->scan_id);

  FLIP(exec->task_start);
  FLIP(exec->part_start);
  FLIP(exec->part_type);
  FLIP(exec->part_timer);
  FLIP(exec->last_task);
  FLIP(exec->secs_remain);
  FLIP(exec->file_size);

  FLIP(exec->send_count);
  FLIP(exec->timestamp);
}



void flip_wapp(struct WAPPSHM *wapp) {

  FLIP(wapp->bw);
  FLIP(wapp->mode);
  FLIP(wapp->running);
  FLIP(wapp->num_lags);
  FLIP(wapp->attena[0]);
  FLIP(wapp->attenb[0]);
  FLIP(wapp->attena[1]);
  FLIP(wapp->attenb[1]);
  FLIP(wapp->power[0]);
  FLIP(wapp->power[1]);
  FLIP(wapp->power[2]);
  FLIP(wapp->power[3]);
}



void flip_alfa(struct ALFASHM *alfa) {

  int i, n;

  n = sizeof(alfa->bias_voltages) / sizeof(float);

  for ( i = 0 ; i < n ; i++ )
    FLIP(alfa->bias_voltages[i]);

  n = sizeof(alfa->other_voltages) / sizeof(float);

  for ( i = 0 ; i < n ; i++ )
    FLIP(alfa->other_voltages[i]);

  FLIP(alfa->vacuum);
  FLIP(alfa->motor_position);
}



void flip_data(void *v, int n) {

  switch ( n ) {
    case 1:
      break;
    case 2:
      in_swaps(v);
      break;
    case 4:
      in_swapf(v);
      break;
    case 8:
      in_swapd(v);
      break;
    default:
      printf("flip_data:cant flip %d bytes\n", n);
      break;
  }
}



/* handle bit fields when all are :1 */

static void flip_uintbits(unsigned int *u) {

  unsigned int bit;
  unsigned int ret;
  int i;

  in_swapf((float *) u);

  bit = *u;
  ret = 0;

  for ( i = 0 ; i < 32 ; i++ ) {
    ret <<= 1;
    ret |= (bit & 1);
    bit >>= 1;
  }

  *u = ret;
}



/*

There is no -in general- way to bit flip bit fields from Sun to gcc linux.
So I do this for now.

The string defines how many bits and fields from left to right:
So if Phil defines:

typedef struct {
  unsigned int    synDest  :2;
  unsigned int    mixerCfr :2;
  unsigned int    ampInpSrc:2;
  unsigned int    ampExtMsk:7;
  unsigned int    unused   :19;
} IF2_ST_STAT4;


The string is

"2,2,2,7,19"

*/

static void generic_bitflip(unsigned int *pvalue, char *str) {

  char *delim = ",";
  char *tok, *tokst;
  unsigned int new, lmask, v, num, index, value, shift;
  unsigned int all = 0xffffffff;

  value = *pvalue;
  flip_data(&value, sizeof(unsigned int));
  tokst = (char *) malloc(strlen(str)+1);
  memcpy(tokst, str, strlen(str)+1);

  tok = strtok(tokst, delim);
  index = 0;
  new = 0;
  while ( tok ) {
    num = atoi(tok);

    lmask = all >> ( 32 - num );
    lmask = lmask << index;

    if ( index + num > 16 ) {
      shift = 2*(index-16)+num;
      v = value << shift;
    } else {
      shift = 32-index-index-num;
      v = value >> shift;
    }

    v = lmask & v;
    new = v | new;

    index += num;

    tok = strtok(NULL, delim);
  }

  free(tokst);
  *pvalue = new;
}

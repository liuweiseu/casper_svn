#ifndef scrmLibhInc
/* 
 * include file for ao  scramnet routines
*/
#define scrmLibhInc
#ifdef 	VXWORKS
#include	<vxWorks.h>
#else
#include	<vxWorksEm.h>
#endif
#include	<stdio.h>
#ifndef VXWORKS
#endif
#include	"scrm.h"

/*	prototypes 	*/

STATUS scrmAoInit(void);
void   scrmAoAdrDbg (void);
void   scrmAoGetTbl(int *pblkLen,int *pbufLen,int *poffset,int maxEntTbl,
					int *pnumRet);
STATUS scrmRdGBlkI(int blkInd,SCRM_BLK_INFO *pscrmBlkI); 
STATUS scrmWrGBlkI(int blkInd,int bufLenBytes,SCRM_BLK_INFO *pscrmBlkI);
int    scrmNodeId(void);
STATUS scrmRBeg(SCRM_BLK_INFO *pscrmBlkI);
STATUS scrmREnd(SCRM_BLK_INFO *pscrmBlkI,int *pmodCnt);
STATUS scrmWBeg(SCRM_BLK_INFO *pscrmBlkI);
STATUS scrmWEnd(SCRM_BLK_INFO *pscrmBlkI);
#endif

#ifndef INCalfamotH
#define INCalfamotH
#ifdef VXWORKS
#include    <vxWorks.h>
#else
#include    <vxWorksEm.h>
#endif
/*
 * encoder counts
 * 3356500 = 360 degrees.
*/
/*
 * gear ratio  for motor
 * 1. motor to gearbox: 216:1
 * 2. gearbox drive gear to alfa gear 20:3
 * 3. total: 216*20/3. = 1440
 *
 * they measured:
 * 1000 rpms: 100 deg 28 secs
 * 1500 rpms: 100 deg 19 secs
 * cmp/measure: 1.17, 1.19 .. this is ok since there is a ramp up
 *                            and ramp down time.
*/
#define GEAR_RATIO	1440.
/*
 * they told me 3356500 encoder cnts per 360 degrees:
 * or 9323.61 encCnts per deg. 
 * I think this number was measured and did not come from the
 * gear ratios so it is probably not exact..
*/
/* location of zero degrees */
#define INDPOS_ENCCNT    1886135
#define DEG_TO_ENCCNTS_RATIO  9323.61
#define ENCCNTS_TO_DEG_RATIO .000107255

#define DEG_TO_ENCCNTS(a) (iround((a)*DEG_TO_ENCCNTS_RATIO) +INDPOS_ENCCNT)
#define ENCCNTS_TO_DEG(a) (((a)-INDPOS_ENCCNT)*ENCCNTS_TO_DEG_RATIO)

#define DEGSEC_FLR_TO_MOTRPM 240.
#define MOTRPM_TO_DEGSEC    (1./DEGSEC_FLR_TO_MOTRPM)

#define MAXVEL_MOTRPM   1500
#define MAXVEL_DEGSEC   (MAXVEL_MOTRPM/DEGSEC_FLR_TO_MOTRPM)
#define DEFVEL_RPM      960 
#define DEFVEL_DEGSEC    4.
#define MAXPOS_DEG       100.
#define MINPOS_DEG       -100.
/*-----------------------------------------------------------------------
 * combination status words, extended status words.
 * this is the motor stat command
 *
*/
typedef struct {
    unsigned int    free          :27 ;
    unsigned short  hold          :1; /* see stext.hold*/
    unsigned short  spcMode       :1; /* see stext.special*/
    unsigned short  safetyOpn     :1; /* see stext.safety*/
    unsigned short  flt           :1; /* see stext.flt*/
    unsigned short  drvDisa       :1; /* see stext.disa*/
        } ALFAM_STATSUM; 

/*-----------------------------------------------------------------------
 * extended status .. from the motor status command
 * 5 short words... each word then the combination is 
 * listed below
*/  

typedef struct {
	unsigned int  	  free	    :26;
	unsigned short      encNotInit:1;
	unsigned short velLoopDsgnFail:1;
	unsigned short	fltDis	    :1;
	unsigned short	dipSwDis	:1;
	unsigned short	softWDis	:1;
	unsigned short	remoteDis	:1;
		} ALFAM_STWD1_DISABLE; 

typedef struct {
	unsigned int	  free	    :17;
	unsigned short posDev       :1;
	unsigned short overTrvl     :1;
	unsigned short	free1 	    :1;
	unsigned short	foldBack	:1;
	unsigned short	noMotComp   :1;
	unsigned short	eepromChksum:1;
	unsigned short	eeprom      :1;
	unsigned short	overSpeed	:1;
	unsigned short	analogSupply :1;
	unsigned short	motOverTemp :1;
	unsigned short	underVolt	:1;
	unsigned short	feedBackLoss:1;
	unsigned short	overCur  	:1;
	unsigned short	overVolt    :1;
	unsigned short	drvOverTemp :1;
		} ALFAM_STWD2_FLT; 

typedef struct {
    unsigned int    free        :23;
    unsigned short  foldBckMode1:1;
    unsigned short  negOvrTravel:1;
    unsigned short  posOvrTravel:1;
    unsigned short  limDisaCCW  :1;
    unsigned short  limDisaCW   :1;
    unsigned short  foldBack    :1;
    unsigned short  limSwDisa   :1;
    unsigned short  ccwlim      :1;
    unsigned short  cwlim       :1;
        } ALFAM_STWD3_SAFETY; 

typedef struct {
    unsigned int    free        :29;
    unsigned short  drvInZero   :1;
    unsigned short  drvInBurnin :1;
    unsigned short  drvInStep   :1;
        } ALFAM_STWD4_SPEC; 

typedef struct {
    unsigned int    free        :20;
    unsigned short  anaPosHold  :1 ;
    unsigned short  free1       :5 ;
    unsigned short  intHoldReq  :1 ;
    unsigned short  usrInpSw    :1 ;
    unsigned short  limSwTrip   :1 ;
    unsigned short  drvActDisa  :1;
    unsigned short  dipSw7      :1;
    unsigned short  usrReq      :1;
        } ALFAM_STWD5_HOLD; 
/*
 * struct containing all 5 words
*/
typedef struct {
	ALFAM_STWD1_DISABLE	disa;
	ALFAM_STWD2_FLT     flt; 
	ALFAM_STWD3_SAFETY  safety;
	ALFAM_STWD4_SPEC    special;
	ALFAM_STWD5_HOLD    hold;
	int				    free  ;    /* for alignment*/
	} ALFAM_STATUS_EXTD;

/*-----------------------------------------------------------------------
 *
 *  extend status words from  status2 command
*/

typedef struct {
    unsigned int    free        :22 ;
    unsigned short  extFdBkLinBrk:1 ;
    unsigned short  burstOvrFlw  :1 ;
    unsigned short  ABlinOutofRng:1 ;
    unsigned short  linBrkEncCD  :1 ;
    unsigned short  illegalHallSt:1 ;
    unsigned short  linBrkEncInd :1 ;
    unsigned short  linBrkEncAB  :1 ;
    unsigned short  sinEncInitErr:1;
    unsigned short  rslvCnvtErr  :1;
    unsigned short  rslvLineBrk  :1;
        } ALFAM_ST2WD1_ENC; 

typedef struct {
    unsigned int    free        :30 ;
    unsigned short  negAnaSupFlt :1;
    unsigned short  posAnaSupFlt :1;
        } ALFAM_ST2WD2_ANASUP; 

typedef struct {
    unsigned int    free        :28 ;
    unsigned short  negOvrTravel :1 ;
    unsigned short  posOvrTravel :1 ;
    unsigned short  gtMaxPE      :1;
    unsigned short  intPosDev    :1;
        } ALFAM_ST2WD3_POSDEV; 

typedef struct {
    unsigned int    free         :30 ;
    unsigned short  CCWlimSwTrip :1;
    unsigned short  CWlimSwTrip  :1;
        } ALFAM_ST2WD4_LIMSW ; 

typedef struct {
    unsigned int    free          :28 ;
    unsigned short  limSwTrip     :1;
    unsigned short  phBCurMismatch:1;
    unsigned short  phACurMismatch:1;
    unsigned short  loStartFail   :1;
        } ALFAM_ST2WD5_ENCINIT;

typedef struct {
    unsigned int    free          :30 ;
    unsigned short  velFdBkGtVlim :1;
    unsigned short  velFdBkGtVOSPD:1;
        } ALFAM_ST2WD6_OVRSPD ; 
/*
 * the combination of the 6 words
*/
typedef struct {
		ALFAM_ST2WD1_ENC	     enc;  /* stext.flt.feedBackLoss*/
		ALFAM_ST2WD2_ANASUP   anaSup;  /* stext.flt.analogSupply*/
		ALFAM_ST2WD3_POSDEV   posdev;
		ALFAM_ST2WD4_LIMSW     limsw;  /* stext.encint.limSwTrip*/
		ALFAM_ST2WD5_ENCINIT encinit;  /* stextd.disa.encNotInit*/
		ALFAM_ST2WD6_OVRSPD   ovrspd;  /* stextd.flt.overspd */
	} ALFAM_STATUS2_EXTD;
/*-----------------------------------------------------------------------*/

typedef struct {
	ALFAM_STATSUM    	sum;	/* summary status  1 word  */
	int				   fill;	/* to align structs on 8 byte boundardies*/
	ALFAM_STATUS_EXTD   ext;    /* extended status 6 words*/
	ALFAM_STATUS2_EXTD  ext2;   /* extended status 6 words*/
	} ALFAM_STATUS;				/* 14 words total*/
/*
 * prototypes for lib
*/
char *alfmDcd1(char *buf,unsigned int *psh);
void   alfmFmtStat(char *buf,char *plinlab,char **plab,int numperline,int stat);
STATUS alfmExtSt2Dcd(char *buf,ALFAM_STATUS2_EXTD *pst2Ext);
STATUS alfmExtStDcd(char *buf,ALFAM_STATUS_EXTD *pstExt);
void alfmPrStat(FILE *fp,ALFAM_STATUS *pmotStat);
STATUS alfmSumStDcd(char *buf,ALFAM_STATSUM *pstsum);



#endif

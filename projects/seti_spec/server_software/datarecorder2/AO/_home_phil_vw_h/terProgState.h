#ifndef     INCterProgStateh 
#define     INCterProgStateh 
/*
 * terProgState definitions.
 * history:
*/
#if FALSE
#include	"terProg.h"
#endif

/*  requested state of tertiary device */

#define TER_DEVST_ACOFF    0
#define TER_DEVST_STOP     1
#define TER_DEVST_READY    4
#define TER_DEVST_MOVING   5

/* two transitional states that tertiary can move through*/

#define TER_DEVST_STOPPING 2
#define TER_DEVST_STARTING 3


/* 	READY  --> ac on, and all requested drives are enabled
    MOVING --> all requested drives enabled AND at least 1 drive
			  moved by the moveThreshold in the last 5 milliseconds.
 *
 * NOTE-- not in TER_DEVST_MOVING does not mean that all drives are
 *        stopped. If a drive goes offline, you go moving->stopping->stopped.
 *		  it will take the deacceleration time to stop any drives.
*/
/* TER_F_ .. IS TO CONVERT KHZ TO DEG F for temp sensor in drive chassis*/
#define TER_DRV_F_PER_KHZ	.312
#define TER_DRV_F_OFF_KHZ	100000.
#define TER_DRVCNTS_TO_DEGF(c) (( (c)*250. - TER_DRV_F_OFF_KHZ) *TER_DRV_F_PER_KHZ * .001)   
#define TER_MCCCNTS_TO_DEGF(c) ( (c)*200./2047)   

/* following used for ter_index msg cmd.*/
#define TER_INDEX_M     1
#define TER_INDEX_Z     2
/*
 * convert torque to ft-lbs .. see http://www.naic.edu/~phil/devs/tertiary...
 * dc offsets from idc square wave
*/
#define MONPORT_DC_VL	2006.0
#define MONPORT_DC_VR	1996.48
#define MONPORT_DC_HL	1991.45
#define MONPORT_DC_HR	2026.32
#define MONPORT_DC_T	2015.24
/*
 * counts/volt from idc square wave
*/
#define MONPORT_CNTPV_VL 630.3
#define MONPORT_CNTPV_VR 632.7
#define MONPORT_CNTPV_HL 642.7
#define MONPORT_CNTPV_HR 646.5 
#define MONPORT_CNTPV_T  638.4
/*
 * convert counts to foot-lbs.. oz-inches ->ftlbs.. 12*16
*/
#define TOFTLBS (900./2./192.)
#define MONPORT_TO_FTLBS_VL(c) (-TOFTLBS*((c)-MONPORT_DC_VL)/MONPORT_CNTPV_VL)
#define MONPORT_TO_FTLBS_VR(c) (-TOFTLBS*((c)-MONPORT_DC_VR)/MONPORT_CNTPV_VR)
#define MONPORT_TO_FTLBS_HL(c) (-TOFTLBS*((c)-MONPORT_DC_HL)/MONPORT_CNTPV_HL)
#define MONPORT_TO_FTLBS_HR(c) (-TOFTLBS*((c)-MONPORT_DC_HR)/MONPORT_CNTPV_HR)
#define MONPORT_TO_FTLBS_T(c)  ( TOFTLBS*((c)-MONPORT_DC_T )/MONPORT_CNTPV_T)

/* bits to refer to various axis in 5 bit status */

#define TER_AXISB_LV      0x10
#define TER_AXISB_RV         8
#define TER_AXISB_LH         4
#define TER_AXISB_RH         2
#define TER_AXISB_T          1

/*	when enable axis... these are the bits*/

#define TER_CTRL_VER        1 
#define TER_CTRL_HOR        2
#define TER_CTRL_TILT       4

/* when an array of 5 values, this is the axis order*/
#define TER_AXISI_LV		0
#define TER_AXISI_RV		1
#define TER_AXISI_LH		2
#define TER_AXISI_RH		3
#define TER_AXISI_T 		4

/* values for mode..  .. same as tttMsg.h PNT_TTT_MODE_XXX*/
#define TER_MODE_STOP	    1
#define TER_MODE_PT  	    2
#define TER_MODE_SLEW       3
#define TER_MODE_STOW       4
#define TER_MODE_OFF        5
#define TER_MODE_RATE       6
#define TER_MODE_LAST       6

/*
 * tertiary coordinate systems
 * encoder. cast to TER_AXIS_DAT without the filler at end
*/

#define TER_CS_ENCODER		1

typedef struct {
    int     secMid;     /* seconds from midnight*/
    int     intvlCnt;   /* intervals from count*/
    } TER_TM_STAMP;

/*
 * main status word
*/
typedef struct {

	/* the following are */

    unsigned int   curState:4;  /* we are in*/
    unsigned int   reqState:4;  /* we want to move to*/

    unsigned int  terPrgRun:1;  /* true if program is running*/
    unsigned int   ctrlVer:1;  /* we control the verticalaxis*/
    unsigned int   ctrlHor:1;  /* we control the hor axis*/
    unsigned int  ctrlTilt:1;  /* we control the tiltaxis*/

    unsigned int   ptLastOffQSkipped:1;/* last off queue was skipped(tm passed*/
    unsigned int       mode:3;  /* mode we are in.

						   1-stop,2-pt,3-slew,4-stow*/
    unsigned int   posFromPot: 1; /* true if position comes from pot*/
    unsigned int       fill:15; /* we want to move to*/
    } TER_STATUS_MAIN;

/*
 *  bits flag any trouble with data from last encoder read
*/
typedef struct  {
    unsigned int    crcE :5;  /* lv,rv,lH,rH,T*/
    unsigned int    mIndE:5;  /* lv,rv,lH,rH,T*/
    unsigned int    zIndE:5;  /* lv,rv,lH,rH,T*/
    unsigned int    filler:17;
    } TER_ENC_ST;
/*
 * limits/faults
*/
#define 	TER_STATUS_LIM_MASK 0x00003fff
typedef struct {
	unsigned int              fill:8; /* negative pre limit*/
	unsigned int         preLimNeg:5; /* negative pre limit*/
    unsigned int         preLimPos:5; /* positive pre limit*/
	unsigned int         limNeg:5; /* negative limit*/
    unsigned int         limPos:5; /* positive limit*/
    unsigned int       wrackVpos:1; /* positive veritical wracking*/
    unsigned int       wrackVneg:1; /* negative veritical wracking*/
    unsigned int       wrackHpos:1; /* positive hor wracking*/
    unsigned int       wrackHneg:1; /* negative hor wracking*/
    } TER_STATUS_LIMITS;

typedef struct {

    unsigned int            fill:11;/* free*/

    unsigned int   drvSpurFlt:5;/* LV,RV,LH,RH,titlt drive spurious fault*/
    unsigned int       drvFlt:5;/* LV,RV,LH,RH,titlt drive fault*/
    unsigned int     peMccCmd:1;/* parity error on command word*/
    unsigned int mcc15VSupFlt:1;/* mcc 15 volt supply fault*/
	unsigned int       statPe:3;/* 1 -wds 0,1,2=words 2,3,4:words 4,5*/
	unsigned int        vccPe:2;/* 1 tovcc, 2-from vcc*/

	unsigned int vccStatWdNotUpd:1;/* vcc stat word didnot update*/ 
	unsigned int  vccVelFbNotUpd:1;/* vcc velocity feed back didnot complete*/
    unsigned int    emgStpCtrlRm:1;/* control room emergency stop is enabled*/
    unsigned int      emgStpDoor:1;/* door emergency stop is active*/
    } TER_STATUS_FAULT;
/*
 * general device status
 * the Request bits below are set when we send the command to
 * perform the operation. They get reset when the action completes
 * (either failure or success).
*/
typedef struct {
    unsigned int          fill1:13;/* unused*/
    unsigned int ptqSkippedLast:1; /* skipped last queue entry tm in past*/
    unsigned int       ptqFutTm:1; /* ptq has pos for time in future*/
    unsigned int   aDrvIsMoving:1; /* at least 1 drive is still moving*/

    unsigned int        velWrOk:1;  /* 1 if last velocity write was ok*/
    unsigned int        encRdOk:1;  /* 1 if last encoder read ok*/
    unsigned int       statRdOk:1;  /* 1 if last set status words ok*/
    unsigned int          fault:1;  /* 1 if a fault causes us to stop*/

    unsigned int          limit:1;/*1 if an axis is limit or wrack*/
    unsigned int       localMcc:1;  /* 1 if in local mode*/
    unsigned int       localVcc:1;  /* 1 if in local mode*/
    unsigned int     safetyOpen:1;  /* 1 if safety is open*/

    unsigned int        emgStop:1; 	/* 1 if emg stop active*/
    unsigned int        reqAcEna:1   ;/* we requested ac enable.. not yet ena*/
    unsigned int        acEna:1      ;/* ac is enabled*/
    unsigned int        reqDrvsEna:1 ;/* We ve sent drive enable req. not rdy*/

    unsigned int        reqDrvsDis:1 ;/* We ve sent drive dis req..not dis yet*/
    unsigned int        drvsEna:1    ;/* all the drives we want are enabled*/
    unsigned int        zIndEna:1    ;/* all the zInd we want are enabled*/
    unsigned int        mIndEna:1    ;/* all the mInd we want are enabled*/
	}TER_STATUS_GEN;

	/*
 	 * these are some of the values read back from the device.
 	 * they are by drive.
	 * some of the reqxxx may not match what we are asking for if
	 * the command has not yet been seen.
 	*/
typedef struct {
	unsigned int          fill :11; 
	unsigned int        zIndEna:5; /* z index preset is ena,lv,rv,lh,rh,tilt*/
    unsigned int        mIndEna:5; /* m index preset is enabled*/
    unsigned int       reqEnaAc:1 ;/* ac enable requested*/
    unsigned int     reqEnaDrvs:5 ;/* LeftV,RV,LH,RH,tilt*/
    unsigned int        drvsEna:5 ;/* enable drives lv,rv,lh,rh,tilt*/
    } TER_STATUS_DEV;

/* 	hold encoder data */

typedef struct  {
    int     vL;
    int     vR;
    int     hL;
    int     hR;
    int     t;  /* tilt*/
    int   fill;
    } TER_AXIS_DAT;

/* msg info comes back every msg */


typedef struct {
	short           tempMcc;    /* temperature mcc cabinet               0,.5*/
	short           tempDrv;    /* temperature mcc cabinet               0,.5*/
    int          errnoEncRd;    /* last read error we had                1,1 */
    TER_AXIS_DAT       enc;       /* normal encoder updated every 5 ms ..2,6*/
    TER_AXIS_DAT      encM;       /* encoder at last m index             8,6*/
    TER_AXIS_DAT      encZ;       /* encoder at last z index             14,6*/
    TER_AXIS_DAT       pot;       /* potentiometer                       20,6*/
    TER_ENC_ST        encSt;    /* status last enc read. successfulornot 26,1*/
    TER_ENC_ST        encMSt;   /* status last menc read. successfulornot 27,1*/
    TER_ENC_ST        encZSt;   /* status last menc read. successfulornot 28,1*/
    int          errnoStatRd;   /* last read error we had reading stat wds29,1*/
    TER_AXIS_DAT      velSent;  /* vel Sent on this interval daCnts       30,6*/
	float			 velKp[5];	/* velocity from proportional term        36,5*/
	float			 velKi[5];	/* velocity from integral term            41,5*/
	float			 velKf[5];	/* velocity from kF term                  46,5*/
	int				  deadZoneEc;/* posErr=0 if within this distance      51,1*/
    TER_AXIS_DAT      reqPosEc; /* requested position in encoder cnts     52,6*/
	/*
	 * vToF velocity  readback
	*/
	TER_AXIS_DAT    velFb;		/* velocity feedback                     58,6*/
    TER_AXIS_DAT      monPort;  /* monitor port                          64,6*/
	} TER_STAT_IO;			/* tot len: 70*4=280 bytes*/
/*
 * local program track queue
*/
typedef struct {
	int		waitIntvlTmOuts;
	int		waitIntvlMisMatch; /* intvl timer count didnot match ours*/
	int	    ptGetFreeBuf;	   /* getting free buf to send to terOPrg*/
	int	    ptGetMsg;	       /* terOPrg getting messages*/
	int	    ptPutMsg;	       /* terOPrg put msg on local msgQ*/
	int	    ptPutBuf;	       /* terOPrg put back on free list*/
	int	    ptReqSkipped;	   /* terOPrg skipped prgTrk. time passed*/
	int		encRdErr;		   /* error reading encoder*/
	int		statWdRdErr;		   /* error reading encoder*/
	int		drvEnaGlitches;		/* count cumulative glitches*/
	} TER_ERR_CNTS;


/*                                                                   start,len*/
typedef struct {
	TER_TM_STAMP	tm;				    /* sec midnite and interval      0,2*/
	TER_STATUS_MAIN mStat;      /* main (summary) program status         2,1*/
	TER_STATUS_GEN  gStat;		/* general device status                 3,1*/

	TER_STATUS_DEV  dStat;		/* general device status                 4,1*/
#if FALSE
	TER_STATUS_LIM_FAULT limFlts;	/* limits and faults*/
#endif
	TER_STATUS_LIMITS    lim;	/* limits                                5,1*/
	TER_STATUS_FAULT     flt;	/* faults                                6,1*/
	int				fill;		/* for another faults                    7,1*/
	TER_ERR_CNTS    errCnts;    /* cumulative errors                     8,10*/
	TER_STAT_IO		io;			/* status and data                      18,70*/
    } TER_STATE;			/* total len:88*4=352 bytes*/

/*
 * structure used for logging data at 5ms rate
*/
typedef struct {
	int			 encCur[5];	/* vl,vr,hl,hr,t*/
	int			 encReq[5];	/* vl,vr,hl,hr,t*/
	int			    pot[5];	/*potentiometer, vl,vr,hl,hr,t*/
	int			  velCmd[5];/* in dac counts*/
	int			  velFb[5];	/*velocity feedBack, vl,vr,hl,hr,t*/
	int			  monPort[5];/*velocity feedBack, vl,vr,hl,hr,t*/
	int           tmMs;     /* millisecs from midnight*/
	} TER_LOG_POS_DATA;

#endif

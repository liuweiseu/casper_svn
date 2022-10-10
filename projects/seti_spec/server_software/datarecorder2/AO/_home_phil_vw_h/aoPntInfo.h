/*
 *	include file for ao dependant pointing info
*/
#ifndef INCaoPntInfoh
#define INCaoPntInfoh

#include	<stdio.h>
#ifdef VXWORKS
#include	<vxWorks.h>
#else
#include	<vxWorksEm.h>
#endif

/*	defines 	*/

/*
#define	FILES_DIR         "/home/aosun/u4/phil/vw/etc/Pnt"
 now only used for offline code. online code uses initDat ..
*/
#define	FILES_DIR         "/home/online/vw/etc/Pnt"
#define FILE_OBS_POS      FILES_DIR"/obsPosition.dat"
#define FILE_UTC_TO_UT1   FILES_DIR"/utcToUt1.dat"

/*
 *	feeds file info
*/
#define LINES_PER_FEED          26
#define MIN_FEED_NUMBER   	1
#define MAX_FEED_NUMBER   	9
#define MAX_NUM_FEEDS     	9
#define MODEL_PARMS_AZ_OR_ZA    9
/* no longer used*/
#define FILE_FEEDS_DEF  FILES_DIR"/feeds.dat"
/*
 *	encoder file params
*/
#define LINES_PER_ENCODER 	23
#define NUM_ENC_ENTRIES         22
/* nolonger used */
#define FILE_ENCODER_DEF  FILES_DIR"/encoder.dat"

/*
 *	the encoder table is evaluated after the model correctio is 
 *	applied. It has a az/za correction as a function of zenith angle
 *	There are 21 locations that correspond to za 0 to 21 degrees.
*/
typedef  struct	{
	double	zaCorRd;		/* zenith  angle correction Radians */
	double	azCorRd;		/* azimuth angle correction Radians */
	} ENCODER_PNT;
/*
 *	the FEED_INFO structure holds all the information for a given feed
*/
typedef	struct {
	int	     fdNum;		/* feed number 1-9 */
	int	     carHouse;		/* 1 2 */
	double	     RRd;  		/* angle center carHouse to slot */
	double       thetaRd;		/* angle uphill around to slot */
	double       modZaRd[9]; 	/* zenith angle model coeff in rad */
	double       modAzRd[9];	/* azimuth angle model coeff in rad */
 	ENCODER_PNT  encTbl[NUM_ENC_ENTRIES];/* array is za 0 to 21 degrees */
	double       srcAzToCh2AzRd;	/* to go src az to ch2 az */
	} FEED_INFO;
/*
 *	the FEED_INFO_ALL structure holds all the information for all
 *	the feeds.
*/
typedef struct {
	   int numFdsInp;
	   FEED_INFO fdInfo[MAX_NUM_FEEDS];
	}  FEED_INFO_ALL;
/*
 *	function prototypes
*/
#endif  /*INCpntXformLibh */

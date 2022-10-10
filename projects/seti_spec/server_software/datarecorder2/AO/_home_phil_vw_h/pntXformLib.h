/*
 *	include file for pointing transformations library
*/
#ifndef INCpntXformLibh
#define INCpntXformLibh

#ifdef VXWORKS
#include	<vxWorks.h>
#else
#include	<vxWorksEm.h>
#endif
#include	<stdio.h>
#include	"epv.h"
#include	"aoPntInfo.h"
/*	to go from julian date to modified julian date	*/

#define	MJD_OFFSET                2400000.5
#define MJD_OFFSET_RANKIN	    40000 
/*   to go from greg day to julian day 1901 thru 2100. see gregToMjd() */
#define GREG_TO_MJD_OFFSET  -678956

#define VEL_OF_LIGHT_MPS   299792458.
#define VEL_OF_LIGHT_KMPS  299792.458
#define EARTH_RADIUS_EQUATORIAL_M 6378140.
#define TAI_TO_TDT		  32.184
#define JULDATE_J2000		2451545.0
#define JULDAYS_IN_CENTURY	36525
#define AU_SECS		        499.004782
#define AUPERDAY_TO_VOVERC      AU_SECS/86400.
#define SOLAR_TO_SIDEREAL_DAY   1.00273790935
#define SIDEREAL_TO_SOLAR_DAY    .997269566330
#define JUL_CEN_AFTER_J2000(mjd,utFrac) \
	((mjdToJulDate(mjd,utFrac) - JULDATE_J2000)/(double)JULDAYS_IN_CENTURY)
/*
 * pointint coordinate systems
*/
#define PNT_CRD_NOT_IN_USE              0
#define PNT_CRD_GAL                     1
#define PNT_CRD_B1950                   2
#define PNT_CRD_J2000                   3
#define PNT_CRD_BECLIPTIC               4
#define PNT_CRD_JECLIPTIC               5
#define PNT_CRD_CURRENT                 6
#define PNT_CRD_HA_DEC                  7 
#define PNT_CRD_AZZA_SRC                8
#define PNT_CRD_AZZA_FEED               9
#define PNT_CRD_AZZA_NOMOD             10
#define PNT_CRD_AZZA_GC                11 
#define PNT_CRD_MAX_NUM                11

/*
 * velocity coordinate systems
 * topocentric,geocentric,heliocentric,local std reset,galactocentric
*/
#define VEL_CRD_TOPO					0
#define VEL_CRD_GEO 					1
#define VEL_CRD_HEL 					2
#define VEL_CRD_LSR 					3
#define VEL_CRD_GSR 					4

/*	typedefs	*/

/*
 *	hms, dms coordinate structure
*/
typedef struct {
	int	sgn;
	int	hour;
	int	min;
	double	dsec;
	} HMS_COORD;

typedef struct{
	int	sgn;
	int	deg;
	int	min;
	int	dsec;
	} DMS_COORD;

/* 	precession/nutation matrix  and inverses*/

typedef struct {
	double	precN_M[9];	/* precces/nutate  j2000 to date*/
	double	precNI_M[9];/* precces/nutate date to j2000*/
	double  eqEquinox;  /* eq of equinox.meanSidTm--> appSidTm*/
	double	ut1Frac;    /* last computation above */
	double  deltaMjd;	/* for recomputing matrices*/
	int		mjd;		/* last computation above */
	int		fill;
	} PRECNUT_INFO;
/*
 *	observatory position info. loaded by obsPosInp()
*/
typedef	struct	{
	double	latRd;		/* lattiude in radians*/
	double  wLongRd;	/* west longitude*/
	int	hoursToUtc;	/* hours to get to utc*/
	int		fill;
	} OBS_POS_INFO;	
/*
 *      The UTC_INFO struct read in from utcInfoInp tells how to go utc to ut1
*/
typedef struct {
        int     year;                   /* for now the year info 4 digits */
        int     mjdAtOff;               /* mod julian date of offset */
        double  offset;                 /* ms offset at midnite of offset */
        double  rate;                   /* in millisecs/day */
        } UTC_INFO;
/*
 *      structure to hold different times
*/
typedef struct {
		double  locFrac;	/* local fraction of day*/
        double  ut1Frac;        /* ut1 fraction of a day*/
        double  lmstRd;         /* local mean sidereal time*/
        double  lastRd;         /* local apperent sidereal time*/
        int     mjd;            /* modified julian date*/
		int	    fill;
        } PNT_TIME_INFO; 
/*
 * for computing projected velocity of observer in vel coord system
*/

typedef struct  {
    int         mjd;    /* for info utc*/
    int         filler;
    double      utcFrac;/* for info computed*/
    double      ut1Frac;/* for above time*/
    double      srcDirJ_V[3];/* x,y,z J2000 src direction to project onto*/
    double      geoV;   /* geocentric velocity*/
    double      helioV; /* heliocentric velocity*/
    } VEL_PROJ_INFO;
/*
 * source times in radians for rise,set,transit
*/
typedef struct {
    double  riseRd;
    double  transitRd;
    double  setRd;
    }   SRC_TIMES;

/*	function prototypes */
/*	generic time/ position 		*/

void    aoSitePos(double* paoSite);
STATUS  earthPosVelB(int mjd,double frac,double* p,double* v);
STATUS  earthPosVelJ(int mjd,double frac,double* p,double* v);
STATUS epvInput(int  mjd,char *filename, EPV_CHBDAY *pdayI);
STATUS epvCompute(double fractTDB, EPV_CHBDAY *pdayI, double *posvel);
void    lsrPos_A(double *praRdJ,double *pdecRdJ);
void    lsrVelSun_V(double *pvel);
STATUS  obsPosInp(char *filename,OBS_POS_INFO *pobsPosI);
void	riseSet(double raJrd,double decJrd,double zaMaxDeg,int mjd,
				PRECNUT_INFO *pprecI,OBS_POS_INFO *pobsI,SRC_TIMES *plstRd);
double  utcTaiOffset(int mjd,double utcFrac);
double  ut1ToLmst(int mjd,double  ut1Frac,double westLongRd);
double  utcToUt1(int mjd,double utcFrac,UTC_INFO* putcI);
void    timeInfo(int year,int mon,int day,int hour,int min,double dsec,
		   double eqEquinox,UTC_INFO *putcI,OBS_POS_INFO *pobsPos,
		 PNT_TIME_INFO *ptimeI);
void    timeMjdUt1(int year,int mon,int day,int hour,int min,double dsec,
		   UTC_INFO *putcI,int hoursToUtc,int *mjd,double *ut1Frac);
void    velGeo_V(int mjd,double ut1Fract,double *site,PRECNUT_INFO *pprecI,
			   double *pvel_V);
STATUS  velGHProj(VEL_PROJ_INFO *pvelRI,double *psitePos_V,PRECNUT_INFO
				  *pprecI,EPV_INFO *pepvI);
double  velLsrProj(double *pdir_V,double velHelioProj);

/*	julian day routines 	*/

void    gregLocToMjd(double lFrac,int d,int m,int y,int utOff,UTC_INFO* putcI,
		 int* pmjd,double* putFrac); 
int     gregToMjd(int day,int month,int year);
int     gregToMjdDno(int dayNum,int year);
void    mjdToGreg(int mjd,int* pday,int* pmonth,int* pyear);
void    julDateToMjd(double julDate,int* pmjd,double *putFrac);
double  mjdToJulDate(int mjd,double utFrac);
 
/*	matrix-vector operations, 	*/

double  M3D_Det(double* m);
void    M3D_Print(FILE* fp,double* m,int numFrac);
void    MM3D_Copy(double* m,double* mcopy);
void    MM3D_Mult(double* m2,double* m1,double* pmresult);
void    MV3D_Mult(double* m,double* v,double* vresult);
void    MV6D_Mult(double* m,double* v,double* vresult);
void    M3D_Transpose(double* m,double* mT);
void    VV3D_Add(double* v1,double* v2,double* vresult);
void    VV3D_Sub(double* v1,double* v2,double* vresult);
void    VV3D_AddSMult(double* v1,double* v2,double scaler,double* vresult);
double  VV3D_DotProd(double* v1,double* v2);
double  V3D_Mag(double* v1);
void    V3D_Normalize(double* v1,double* vresult);

/* 	routines to move between coordinate systems */
 
void    haToAzEl_V(double* phaDec_V,double latitude,double* pazEl);
void    azElToHa_V(double* pazEl,double latitude,double* phaDec);

void    haToRaDec_V(double* phaDec,double lst,double* praDec);
void    raDecToHa_V(double* praDec,double lst,double* phaDec);

void  	precNut(int mjd,double ut1Frac,int jToCur,PRECNUT_INFO *pI,
         		double *pinpVec,double *poutVec);
void  	precNutInit(double  mjdDelta,PRECNUT_INFO *pI);


/*	Return matrices/angles/vectors */

void    aberAnnual_V(int mjd,double utFrac,double* pv);
void    haAzEl_M(double lattitude,double *pm);
void    haToRaDec_M(double lst,double *pm);
double  meanEqToEcl_A(int mjd,double utFrac);
void    nutation_M(int mjd,double utFrac,double* matNut,double* eqEquinox);
void    nutationI_M(int mjd,double utFrac,double* matNut);
void    precDateToJ2_M(int mjd,double utFrac,double* matPrec);
void    precJ2ToDate_M(int mjd,double utFrac,double* matPrec);
void    raDecToHa_M(double lst,double *pm);
void	rotationX_M(double thetaRd,int rightHanded,double *pm);
void	rotationY_M(double thetaRd,int rightHanded,double *pm);
void	rotationZ_M(double thetaRd,int rightHanded,double *pm);

/*
 *	coordinate system transformation
*/

void	B1950ToJ2000(double* M, double   alphaB,double deltaB,
		     double pmalphaB,double pmdeltaB,
	             double paralaxB, double vradialB,
	             double* pvJ,double* ppmAlphaJ,double* ppmDeltaJ,
		     double* pparallaxJ,double* pvradialJ);
void	BToJMatrix(double* M);
void	BToJMatrixR(double* M);
void	JToBMatrix(double* MInv);
void    galToJ2000(double l,double b,double *pra,double *pdec);
/*
 *	misc
*/

double	parAngle(double azDeg,double decDeg, double sitLatDeg);
int     utcInfoInp(int dayNum,int year,char* utcToUt1File,UTC_INFO*  putcI);

#endif  /*INCpntXformLibh */

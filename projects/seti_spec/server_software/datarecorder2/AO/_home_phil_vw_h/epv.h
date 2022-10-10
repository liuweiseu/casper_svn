#ifndef EPVH
#define EPVH
/* taken from $Log: EPV.h,v $
 * Revision 1.4  1997/05/29 21:21:18  nolan
 * Changed some comments.  Added EpvClose
 *
 * Revision 1.3  1997/05/28 00:45:28  nolan
 * Separated high-level routines into EPV.c.  Low level data formats are
 * hidden in structs
 *
 * Revision 1.2  1997/05/21 00:32:10  nolan
 * const was just a pain
 *
 * Revision 1.1  1997/05/21 00:17:05  nolan
 * Initial revision
 * */
#ifdef VXWORKS
#include	<vxWorks.h>
#else
#include	<vxWorksEm.h>
#endif

/* Constants used in EBPosVel.  KMperAU is from DE403.  */

#define SECperDAY 86400.
#define KMperAU 149597870.691

/* Interval over which the chebychev polynomials are interpolated. */
/* They shouldn't necessarily be evaluated this far out.  Chosen to be */
/* representable in base 2 */

#define T1 (-0.0625)
#define T2 (1.1875)

/* And these are how far the polynomials should be evaluated to */
/* achieve stated interpolation accuracy */

/* 45 minutes before */
#define MINT (-.03125) 
/* 3 hours after */
#define MAXT (1.125) 

/* note .. online pntProg now used initDatLib to get filename */
#define EPV_DEFAULT_CHEBFILE "/share/obs4/dt/dfiles/epvChebs"

#define CHEBORDER 5
#define NUMCOE 6
#define HEADERSIZE sizeof(EPV_CHBHEADER)
#define CHFORMATID 1

typedef struct  {
   long headersize;
   long formatid;
   long mjd1;
   long mjd2;
   long ncoe;
   long order;
   long datasize;
   long dummy1;
}  EPV_CHBHEADER;

typedef struct  {
   long mjd;
   long dummy1;
   double chebvals[NUMCOE][CHEBORDER];
}  EPV_CHBDAY;
typedef struct {
    EPV_CHBDAY  dayI;   /* polynomial coef for above day*/
	double		posVel[6];/* x,y,z vx,vy,vt.. au, au/day .. from epvCompute*/
    char        fileName[132];/* to use, null, 0 --> use default*/
	int			okToUpdate;/* true --> when mjd changes, do update*/
    } EPV_INFO;
#endif

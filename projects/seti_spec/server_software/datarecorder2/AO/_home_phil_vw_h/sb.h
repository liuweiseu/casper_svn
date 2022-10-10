/*
 *	include file for sband transmiter
 * history:
*/
#ifndef INCsbh
#define INCsbh
#include	"aoErrNum.h"

/*	defines		*/

#define	SB_MAX_STAT_IND		46
/*		typedefs	*/
/* nomenclature:
	kl  - klystron
	Fil - filament
	Col - collector
	Mag   magnet
	beam - beam
	V    - volts
	Kv     kiloVolts
	Cur    current
	A      amps
	Ua     micro amps
	Ma     milli amps
	W      watts
	Kw     kilowatts
	Gpm    gallons per minute
	Degc   degrees C
	fwd	   forward
	pwr    pwr  
	refl   reflected
	was    waster
	turn	turnstile
	ant		antenna
	dl      dummy load
	prf		proof 
*/
/*
 * structure to hold the the dump status data
*/
typedef struct {
	int		hour;
	int	    min;
	int	    sec;
	int	    mon;
	int	    day;
	int	    year;
	unsigned char	dat[SB_MAX_STAT_IND+1]; /* 47 bytes*/
	unsigned char   fill1;	/* to be multiple 8 bytes*/
	} SB_STAT_DATA;


/*	format for raw meter data input. we flip bytes in the
	input routine so it is big endian.
*/
typedef struct {
	unsigned short	kl1MagV;	
	unsigned short	kl2MagV;	
	unsigned short	beamKv;
	unsigned short	kl1MagCurA;	
	unsigned short	kl2MagCurA;	
	unsigned short	bodyCurMa;	
	unsigned short	kl1FilV;	
	unsigned short	kl2FilV;	
	unsigned short	kl1FilCurA;	
	unsigned short	kl2FilCurA;	
	unsigned short	kl1ColCurA;	
	unsigned short	kl2ColCurA;	
	unsigned short	kl1VacionCurUa;	
	unsigned short	kl2VacionCurUa;	
	unsigned short	spare1;
	unsigned short	spare2;
	unsigned short	wasterFwdPwrKw;
	unsigned short	kl1FwdPwrKw;
	unsigned short	kl1ReflPwrKw;
	unsigned short	kl2FwdPwrKw;
	unsigned short	kl2ReflPwrKw;
	unsigned short	wasterReflPwrKw;
	unsigned short	turnDlPwrKw;
	unsigned short	antFwdPwrKw;
	unsigned short	antReflPwrKw;
	unsigned short	kl1RfDrvPwrW;
	unsigned short	kl2RfDrvPwrW;
	unsigned short	wasterFlowGpm;
	unsigned short	deltaTempDegc;
	unsigned short	kl2ColFlowGpm;
	unsigned short	exciterInpPrf;
	unsigned short	spare3;
	}	SB_METER_DATA_RAW;
/*
 *	 meter data after conversion from input format to float
 *   it has been scaled to the units ie. implied zeros have been move
 *	 over. 
*/
typedef struct {
	float	kl1MagV;	
	float	kl2MagV;	
	float	beamKv;
	float	kl1MagCurA;	
	float	kl2MagCurA;	
	float	bodyCurMa;	
	float	kl1FilV;	
	float	kl2FilV;	
	float	kl1FilCurA;	
	float	kl2FilCurA;	
	float	kl1ColCurA;	
	float	kl2ColCurA;	
	float	kl1VacionCurUa;	
	float	kl2VacionCurUa;	
	float	spare1;
	float	spare2;
	float	wasterFwdPwrKw;
	float	kl1FwdPwrKw;
	float	kl1ReflPwrKw;
	float	kl2FwdPwrKw;
	float	kl2ReflPwrKw;
	float	wasterReflPwrKw;
	float	turnDlPwrKw;
	float	antFwdPwrKw;
	float	antReflPwrKw;
	float	kl1RfDrvPwrW;
	float	kl2RfDrvPwrW;
	float	wasterFlowGpm;
	float	deltaTempDegc;
	float	kl2ColFlowGpm;
	float	exciterInpPrf;
	float	spare3;
	}	SB_METER_DATA;


#endif	/* INCsbh*/

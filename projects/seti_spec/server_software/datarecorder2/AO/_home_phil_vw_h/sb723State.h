
/*
 *	include file for sband transmiter 723 status.
 *  allocated 1024 bytes. max size can be :(1024-16)/2=504tes=126 ints
*/
#include	"sb.h"
#ifndef INCSb723Stateh
#define INCSb723Stateh

#if FALSE
typedef struct {
 	int		filler[120];
    } SB723_STATE; 
#endif

typedef struct {
/*                                                                len  start */	
	SB_STAT_DATA	sbStat;	/*status data 6*4 + 47+1 =            [72]  [0]*/
	SB_METER_DATA   sbMet ; /* 4*32= bytes                       [128] [72] */

	float 	       secMLastStat;/* secMidnite last stat read req  [4]  [200] */
	float          secMLastMet; /* secMid last met read           [4]  [204] */

	int				statLastRd;  /* 0 ok, 1 error                  [4]  [208] */

	/*  (1024-16)/2 =504. not divisible by 16. use 496 */
	/*  fill= 496-212 = 284 */
	char            fill[284]; /*                                 [284][212] */
 							   /* total len 504  1 buf*/	
	} SB723_STATE; /* len 496*2 +16 = 1008..*/
#endif  /*INCSb723Stateh */

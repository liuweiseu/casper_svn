#include "c2m.h"

/******************************************************************************/
void calc_mjd (int mn, double dy, int yr, double *mjp)
// given a date in months, mn, days, dy, years, yr,
// return the modified Julian date (number of days elapsed since 1900 jan 0.5),
// *mjd.
/******************************************************************************/
{	
	double jul_day;
	double a,b,c;
	
	a = floor(275*mn/9) + dy + 1721028.5;
	b = floor( 3*(floor((yr + (mn - 9)/7)/100)+1)/4 );
	c = 367*yr - floor(7*(yr+ floor((mn+9)/12))/4);
	jul_day = c - b + a - 2400000.5;
	
	*mjp = jul_day;
}

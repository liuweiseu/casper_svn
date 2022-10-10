#include <stdlib.h>
#include <time.h>
extern "C" {
#include "azzaToRaDec.h"
}

void ErrExit(char * Msg);
#define LOCAL_TIME_TO_UTC_HRS 4

//=======================================================
int AzZaToRaDec(double Az, double Za, long timeMs, double &Ra, double &Dec) {
//=======================================================
// This calls Phil's code and does not take model corrections into account.

    AZZA_TO_RADEC_INFO  azzaI; /* info for aza to ra dec*/

    const double d2r =  0.017453292;
    double stDayFrac;
    int dayNum, i_mjd;
    double utcFrac;
    long secs_past_midnight, encoder_read_secs_past_midnight;
    time_t now;
    struct tm * now_tm;

    // craft a tm structure with values corresponding to encoder read time
    time(&now);
    now_tm = localtime(&now);
    secs_past_midnight = now_tm->tm_hour*3600 + now_tm->tm_min*60 + now_tm->tm_sec;
    encoder_read_secs_past_midnight = (long)(timeMs * 0.001);
    if(secs_past_midnight < encoder_read_secs_past_midnight) {
        now -= secs_past_midnight + 86400;      // back up to midnight and then back up a day
    } else {
        now -= secs_past_midnight;              // just back up to midnight
    }                                           // now now is midnight on the day of the encoder reading        
    now += encoder_read_secs_past_midnight;     // now now as the unix time of the encoder reading
    now_tm = localtime(&now);                 

    // arithmetic needed by AO functions
    now_tm->tm_mon += 1;
    now_tm->tm_year += 1900;
    dayNum = dmToDayNo(now_tm->tm_mday,now_tm->tm_mon,now_tm->tm_year);
    i_mjd=gregToMjd(now_tm->tm_mday, now_tm->tm_mon, now_tm->tm_year);
    stDayFrac=(now_tm->tm_hour*3600 + now_tm->tm_min*60 + now_tm->tm_sec)/86400.;
    utcFrac=stDayFrac + LOCAL_TIME_TO_UTC_HRS/24.;
    if (utcFrac >= 1.) {
        i_mjd++;
        utcFrac-=1.;
    }

    // call the AO functions
    if (azzaToRaDecInit(dayNum, now_tm->tm_year, &azzaI) == -1) ErrExit("Bad azzaToRaDecInit\n");
    azzaToRaDec(Az, Za, i_mjd, utcFrac, &azzaI, &Ra, &Dec);

    // Ra to hours and Dec to degrees
    Ra = (Ra / d2r) / 15;
    Dec = Dec / d2r;

    // Take care of wrap situations in RA
    if (Ra < 0.0)       Ra += 24.0;
    else if(Ra >= 24.0) Ra -= 24.0;
}


/* hacked up by AS Jan 2010 to get alfa status without siren includes.  be wary of derived values. */

// #include "dr2.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <fmtmsg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/lwp.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

// ScramNet headers
extern "C"
{
#include "mshmLib.h"
#include "execshm.h"
#include "wappshm.h"
#include "wapplib.h"
#include "alfashm.h"
#include "vtxLib.h"
#include "scram.h"
}


#include "ao_utils.h"

int main(int argc, char ** argv) {
    struct SCRAMNET * scram;
    char    name[256];
    double  az, za, ch;
    double  azerr, zaerr;
    long    t, th, ts, tm;
    double  ra, dec, mjd;
    int     sleep_sec;
    int     iterations = -1;
    int     i;
    bool    need_all = false;
    bool    got_pnt, got_agc, got_if1, got_if2, got_tt, got_alfashm;
    bool    do_socket = false;
    int blocks;
    double Enc2Deg = 1./ ( (4096. * 210. / 5.0) / (360.) );
    time_t clock;
    char * TimeString;
    double Ra, Dec;
    // these are for ra/dec/ conversion
    time_t now;
    double nowjd, nowje;
    double pos1[3]; 
    double pos2[3]; 
    double lst, ddtime;
    long mon, day, year;
    long rahour,ramin,dechour,decmin;
    double rasec,decsec;

    char *usage = "Usage: scram_peek [-i number_of_iterations] [-need_all] [-socket] sleep_sec\n";

    bool useAlfa;

    fprintf(stderr, "scram size : %d   Enc2Deg = %20.10lf  an int is %d bytes long\n", sizeof(scram), Enc2Deg, sizeof(int));

    if (argc < 2) {
      fprintf(stderr,usage);
      exit(1);
      } 
    for (i = 1; i < argc; i++) {
      if (strcmp(argv[i],"-i") == 0) {
        if (sscanf(argv[++i],"%d",&iterations) != 1) {
          fprintf(stderr,usage);
          exit(1);
          } 
        } 
      else if (strcmp(argv[i],"-need_all") == 0) {
        need_all = true;
        }
      else if (strcmp(argv[i],"-socket") == 0) {
        do_socket = true;
        }
      else if (sscanf(argv[i],"%d",&sleep_sec) != 1) {
        fprintf(stderr,usage);
        exit(1);
        } 
      }

    //scram = init_scramread(NULL);

    
    while(iterations != 0) {
        scram = init_scramread(NULL);
        got_pnt = got_agc = got_if1 = got_if2 = got_tt = got_alfashm = false;
    
blocks = 0;
while ((!need_all && blocks < 5) || (need_all && blocks < 6)) {
        if (read_scram(scram) == -1) {
            fprintf(stderr, "GetScramData : bad scram read\n");
            exit(1);
        }

        time(&clock);
        TimeString = ctime(&clock);
        TimeString[strlen(TimeString)-1] = '\0';

        getnameinfo((const struct sockaddr *)&scram->from,
                    sizeof(struct sockaddr_in),
                    name, 256, NULL, 0, 0);

        if (strcmp(scram->in.magic, "PNT") == 0) {
            if (!need_all || !got_pnt) { got_pnt = true; blocks++; }
            ra  = scram->pntData.st.x.pl.curP.raJ;
            dec = scram->pntData.st.x.pl.curP.decJ;
            ra  *= 24.0 / C_2PI;
            dec *= 360.0 / C_2PI;
            mjd  = scram->pntData.st.x.pl.tm.mjd + scram->pntData.st.x.pl.tm.ut1Frac;

            fprintf(stderr,
                   "%s %s from %s, MJD %lf  RA %f hours  Dec %f degress\n", TimeString,
                    scram->in.magic, name, mjd, ra, dec);
        } else if (strcmp(scram->in.magic, "AGC") == 0) {
            if (!need_all || !got_agc) { got_agc = true; blocks++; }
            az = scram->agcData.st.cblkMCur.dat.posAz * 0.0001;
            za = scram->agcData.st.cblkMCur.dat.posGr * 0.0001;
            AzZaToRaDec(az, za, (long)scram->agcData.st.cblkMCur.dat.timeMs, Ra, Dec); 
            azerr = scram->agcData.st.posErr.reqPosDifRd[0];
            zaerr = scram->agcData.st.posErr.reqPosDifRd[1];
            //"%s %s from %s, secMLastTick %d, Az  %ld (%lf deg), ZA %ld (%lf deg), time %ld (%lf secs) Ra %lf hrs  Dec %lf deg\n", TimeString, 
            fprintf(stderr,
                   "%s %s from %s, secMLastTick %d, Az  %d (%lf deg), ZA %d (%lf deg), time %d (%lf secs) Ra %lf hrs  Dec %lf deg - point diff Az %lf deg, ZA %lf deg\n", TimeString, 
                    scram->in.magic, name,  scram->agcData.st.secMLastTick,
                    scram->agcData.st.cblkMCur.dat.posAz,
                    scram->agcData.st.cblkMCur.dat.posAz * 0.0001,
                    scram->agcData.st.cblkMCur.dat.posGr,
                    scram->agcData.st.cblkMCur.dat.posGr * 0.0001,
                    scram->agcData.st.cblkMCur.dat.timeMs,
                    scram->agcData.st.cblkMCur.dat.timeMs * 0.001,
                    Ra, Dec, azerr, zaerr);
        } else if (strcmp(scram->in.magic, "IF1") == 0) {
            if (!need_all || !got_if1) { got_if1 = true; blocks++; }
            // First LO
            fprintf(stderr,
                   "%s %s from %s, synI.freqHz[0] %lf, synI.ampDB[0] %d, rfFreq %lf, if1FrqMhz %lf,  filterbank %d\n", TimeString, 
                    scram->in.magic, name, scram->if1Data.st.synI.freqHz[0], scram->if1Data.st.synI.ampDb[0], 
                    scram->if1Data.st.rfFreq, scram->if1Data.st.if1FrqMhz, scram->if1Data.st.stat2.alfaFb);
        } else if (strcmp(scram->in.magic, "IF2") == 0) {
            if (!need_all || !got_if2) { got_if2 = true; blocks++; }
            // ALFA in use
            if(scram->if2Data.st.stat1.useAlfa) {
                useAlfa = true;
            } else {
                useAlfa = false;
            }
            fprintf(stderr,
                   "%s %s from %s, useAlfa %d\n", TimeString, 
                    scram->in.magic, name, useAlfa);
        } else if (strcmp(scram->in.magic, "TT") == 0) {
            if (!need_all || !got_tt) { got_tt = true; blocks++; }
            // Turret
            fprintf(stderr,
                   "%s %s from %s, turret encoder %d, degrees %lf\n", TimeString, 
                    scram->in.magic, name, scram->ttData.st.slv[0].inpMsg.position,
                    (double)(scram->ttData.st.slv[0].inpMsg.position) * Enc2Deg);
        } else if (strcmp(scram->in.magic, "ALFASHM") == 0) {
            if (!need_all | !got_alfashm) { got_alfashm = true; blocks++; }
            // ALFA shared memory
            fprintf(stderr,
                   "%s %s from %s, first_bias %d (0x%x), second_bias, %d (0x%x), motor_position %f\n", TimeString,
                    scram->in.magic, name, 
                    (int)scram->alfa.first_bias, (int)scram->alfa.first_bias, 
                    (int)scram->alfa.second_bias, (int)scram->alfa.second_bias,
                    scram->alfa.motor_position);
        } else {
            fprintf(stderr, "%s %s from %s\n", TimeString, scram->in.magic, name);
        }
}

        close(fd_scram(scram));

#if 1   
        // the following is using Phil's standard correction model:

        // Get azimuth, zenith angle from scramnet
        // Apply phil's model (currently in receiverconfig.tab #3)
        // convert to ra/dec
        // precess to j2000

        fprintf(stderr,"%s RADEC - az/za: %lf %lf ",TimeString,az,za);

        // correct az/za
        //co_ZenAzCorrection(3,&za,&az);
        fprintf(stderr,"- fixed az/za: %lf %lf ",az,za);

        // convert to ra/dec, but first figure out lst (and get current julian epoch)
        now = time(NULL);
        nowjd = (long(now)/86400.0)+2440587.5;
        //nowje = tm_JulianDateToJulianEpoch(nowjd);
        //tm_FromJul(nowjd,&mon,&day,&year,&ddtime);
        //lst = tm_UtToLst(ddtime,AOLONGITUDE_DEGS,mon,day,year);
        //co_ZenAzToRaDec(za,az,lst,&ra,&dec);
        fprintf(stderr,"- lst: %lf - EOD: %lf - ra/dec: %lf %lf ",lst,nowje,ra,dec);

        // precess to j2000
        //co_EqToXyz(ra,dec,pos1);
        //co_Precess(nowje,pos1,2000.0,pos2);
        //co_XyzToEq(pos2,&ra,&dec);
        fprintf(stderr,"- J2000 ra/dec: %lf %lf",ra,dec);
        rahour = long(ra);
        ramin = int((ra-rahour)*60);
        rasec = (((ra-rahour)*60)-ramin)*60;
        dechour = long(dec);
        decmin = int((dec-dechour)*60);
        decsec = (((dec-dechour)*60)-decmin)*60;
        fprintf(stderr," ( %02ld:%02ld:%04.1lf %02ld:%02ld:%04.1lf )",rahour,ramin,rasec,dechour,decmin,decsec);

        fprintf(stderr,"\n"); 

#endif

#if 0   
        // the following is for RA/Dec conversion based on Mikael's procedure:

        // Take current commanded J2000 coordinates
        // from scram->pntData.st.x.pl.curP.raJ
        // and scram->pntData.st.x.pl.curP.decJ.
        // Precess them to current epoch.
        // Convert them to Az/ZA.         // true Az/Za, not encoder readings
        // Subtract the current pointing errors coming from differences from true Az/Za,
        // scram->agcData.st.posErr.reqPosDifRd[0] // not from encoder readings
        // and               
        // scram->agcData.st.posErr.reqPosDifRd[1].
        // Convert back to Ra/Dec and precess back from current epoch to J2000.
    
        if (need_all) { // only do if we are sure we got everything
          now = time(NULL);
          nowjd = (long(now)/86400.0)+2440587.5;
          nowje = tm_JulianDateToJulianEpoch(nowjd);
          fprintf(stderr,"%s RADEC - EOD: %lf 2000.0 ra/dec: %lf %lf - ",TimeString,nowje,ra,dec);

          // precess ra/dec to current epoch
          co_EqToXyz(ra,dec,pos1);
          co_Precess(2000.0,pos1,nowje,pos2); 
          co_XyzToEq(pos2,&ra,&dec);
          fprintf(stderr,"EOD ra/dec: %lf %lf - ",ra,dec);

          // covert to az/za, but first need lst
          tm_FromJul(nowjd,&mon,&day,&year,&ddtime);
          lst = tm_UtToLst(ddtime,AOLONGITUDE_DEGS,mon,day,year);
          co_EqToHorz(ra,dec,lst,&za,&az,COS_AO_LAT,SIN_AO_LAT);  
          fprintf(stderr,"lst: %lf az/za: %lf %lf - ",lst,az,za);
          
          // subtract current pointing errors
          az -= azerr;
          za -= zaerr;
          fprintf(stderr,"az/za err: %lf %lf - fixed az/za: %lf %lf - ",azerr,zaerr,az,za);

          // convert back to ra/dec 
          co_ZenAzToRaDec(za,az,lst,&ra,&dec);
          fprintf(stderr,"fixed ra/dec: - %lf %lf ",ra,dec);

          // and finally unprecess
          co_EqToXyz(ra,dec,pos1);
          co_Precess(nowje,pos1,2000.0,pos2);
          co_XyzToEq(pos2,&ra,&dec);
          fprintf(stderr,"fixed 2000.0 ra/dec: %lf %lf",ra,dec);
          rahour = long(ra);
          ramin = int((ra-rahour)*60);
          rasec = (((ra-rahour)*60)-ramin)*60;
          dechour = long(dec);
          decmin = int((dec-dechour)*60);
          decsec = (((dec-dechour)*60)-decmin)*60;
          fprintf(stderr," ( %02ld:%02ld:%04.1lf %02ld:%02ld:%04.1lf )",rahour,ramin,rasec,dechour,decmin,decsec);
          fprintf(stderr,"\n"); 
        } // end if need_all

#endif

        sleep(sleep_sec);
        if (iterations > 0) iterations--;
    }
}


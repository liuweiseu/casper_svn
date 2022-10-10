
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <mshmLib.h>
#include <execshm.h>
#include <wappshm.h>
#include <alfashm.h>
#include <scram.h>

#define PNT
#define IF1
#define ALFA
#define AGC
#define EXEC
#define WAPP1
#define OTHER

main()
{
    struct SCRAMNET *scram;
    char    name[256];
    double  az, za, ch;
    long    t, th, ts, tm;
    double  ra, dec;

    scram = init_scramread(NULL);
    while(1) {
        if (read_scram(scram)) {
            if (strcmp(scram->in.magic, "PNT") == 0) {
#ifdef PNT
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s, time=%f\n", 
                    scram->in.magic, name, 
                    scram->pntData.st.x.pl.tm.secMidD);
    
                t =  scram->pntData.st.x.pl.tm.secMidD;
                th = t / 3600;
                t -= th * 3600;
                tm = t / 60;
                t -= tm * 60;
                ts = t;
                printf("   %02d:%02d:%02d\n", th, tm, ts);
                ra = scram->pntData.st.x.pl.curP.raJ;
                dec = scram->pntData.st.x.pl.curP.decJ;
                
                ra *= 24.0 / C_2PI;
                dec *= 360.0 / C_2PI;
                printf("   ra: %f hours, dec: %f degress\n", ra, dec);

                // printf("   geo vel proj: %f\n",
                //     scram->pntData.st.x.pl.curP.geoVelProj);
                // printf("   helio vel proj: %f\n",
                //     scram->pntData.st.x.pl.curP.helioVelProj);
#endif
            } else if (strcmp(scram->in.magic, "IF1") == 0) {
#ifdef IF1
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s\n", scram->in.magic, name);
                printf("    1st LO: %.2f MHz\n", 
                    scram->if1Data.st.synI.freqHz[0] / 1000000.0);
#endif
            } else if (strcmp(scram->in.magic, "ALFASHM") == 0) {
#ifdef ALFA
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s\n", scram->in.magic, name);
                printf("    alfa pos: %f\n", scram->alfa.motor_position);
#endif
            } else if (strcmp(scram->in.magic, "AGC") == 0) {
#ifdef AGC
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                az = scram->agcData.st.cblkMCur.dat.posAz;
                za = scram->agcData.st.cblkMCur.dat.posGr;
                ch = scram->agcData.st.cblkMCur.dat.posCh;

                az /= 10000.0;
                za /= 10000.0;
                ch /= 10000.0;
                printf("Read %s from %s\n", scram->in.magic, name);
                printf("    Az: %.4f, Gr: %.4f  Ch: %.4f  @ %d\n", az, za, ch,
                        scram->agcData.st.cblkMCur.dat.timeMs);
                printf("    velAz: %d, velGr: %d\n",
                        scram->agcData.st.cblkMCur.dat.velAz, 
                        scram->agcData.st.cblkMCur.dat.velGr);
#endif
            } else if (strcmp(scram->in.magic, "EXECSHM") == 0) {
#ifdef EXEC
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s\n", scram->in.magic, name);
                printf("    obs_mode: %s.\n", scram->exec.obs_mode);    
                printf("    obs_type: %s.\n", scram->exec.obs_type);    
                printf("    source:   %s.\n", scram->exec.source);    
                printf("    receiver: %s.\n", scram->exec.receiver);    
                printf("    backends: %s.\n", scram->exec.backends);    
                printf("    ifconfig: %s.\n", scram->exec.ifconfig);    
                printf("    catalog:  %s.\n", scram->exec.catalog);    
                printf("    lastcal:  %s.\n", scram->exec.lastcal);    
                printf("    rcvpower: %s.\n", scram->exec.rcvpower);    
                printf("    project_id: %s.\n", scram->exec.project_id);    
                printf("    observers: %s.\n", scram->exec.observers);    
                printf("    work_directory: %s.\n", scram->exec.work_directory);    
                printf("    line1: %s.\n", scram->exec.line1);    
                printf("    line2: %s.\n", scram->exec.line2);    
                printf("    line3: %s.\n", scram->exec.line3);    
                printf("    line4: %s.\n", scram->exec.line4);    
                printf("    epoch: %s.\n", scram->exec.epoch);    
#endif
            } else if (strcmp(scram->in.magic, "WAPP1") == 0) {
#ifdef WAPP1
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s\n", scram->in.magic, name);
                printf("    config: %s.\n", scram->wapp[0].config);    
#endif
            } else {
#ifdef OTHER
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s\n", scram->in.magic, name);
#endif
            }
        } else
            printf("xxx\n");
    }
}


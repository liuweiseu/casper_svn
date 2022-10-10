
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mshmLib.h"
#include "execshm.h"
#include "wappshm.h"
#include "alfashm.h"
#include "scram.h"

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
    void *  vp;
    char    name[256];
    double  az, za, ch;
    long    t, th, ts, tm;
    double  ra, dec;

    vp = (void *)init_scramread(NULL);
    scram = (struct SCRAMNET *) vp;
    while(1) {
        if (read_scram(scram)) {
            if (strcmp(scram->in.magic, "PNT") == 0) {
                getnameinfo(&scram->from, sizeof(struct sockaddr_in), 
                    name, 256, NULL, 0, 0);
                printf("Read %s from %s, time=%f", 
                    scram->in.magic, name, 
                    scram->pntData.st.x.pl.tm.secMidD);
    
                t =  scram->pntData.st.x.pl.tm.secMidD;
                th = t / 3600;
                t -= th * 3600;
                tm = t / 60;
                t -= tm * 60;
                ts = t;
                printf(" %02d:%02d:%02d", th, tm, ts);
                ra = scram->pntData.st.x.pl.curP.raJ;
                dec = scram->pntData.st.x.pl.curP.decJ;
                
                ra *= 24.0 / C_2PI;
                dec *= 360.0 / C_2PI;
                printf("   ra: %f hours, dec: %f degress\n", ra, dec);
            }
        } 
    }
}


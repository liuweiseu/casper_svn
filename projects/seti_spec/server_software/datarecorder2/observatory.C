#include "dr2.h"
#include "globals.h"

//=======================================================
int ObservatoryInit() {
//=======================================================

    if((Observatory.Message =  (char *)calloc(1, StringSize)) == NULL) {
        ErrExit("\nCannot observatory message string ");
        //return(1);
    }

    if(pthread_mutex_init(&Observatory.Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory mutex ");
    }
    if(pthread_mutex_init(&Observatory.PNT_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory PNT_Mutex mutex ");
    }
    if(pthread_mutex_init(&Observatory.AGC_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory AGC_Mutex mutex ");
    }
    if(pthread_mutex_init(&Observatory.ALFASHM_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory ALFASHM_Mutex mutex ");
    }
    if(pthread_mutex_init(&Observatory.IF1_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory IF1_Mutex mutex ");
    }
    if(pthread_mutex_init(&Observatory.IF2_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory IF2_Mutex mutex ");
    }
    if(pthread_mutex_init(&Observatory.TT_Mutex, NULL)) {
        ErrExit("\nCannot initialize observatory TT_Mutex mutex ");
    }

    if(Observatory.Interface = OBSERVATORY_INTERFACE_AOSCRAM) {
        Observatory.DataPtr = (void *)init_scramread(NULL);
        Observatory.ThreadLive = true;
    }

    Observatory.PNT_SysTime = 0;
    Observatory.AGC_SysTime = 0;
    Observatory.ALFASHM_SysTime = 0;
    Observatory.IF1_SysTime = 0;
    Observatory.IF2_SysTime = 0;
    Observatory.TT_SysTime = 0;

    Observatory.ScramLagTolerance = 10;         // seconds

    Observatory.TT_TurretDegreesAlfa = TT_TurretDegreesAlfa;
    Observatory.TT_TurretDegreesTolerance = TT_TurretDegreesTolerance;

    Observatory.Function   = ObservatoryFunc;
    Observatory.ThreadLive = true;

    return(0);

}

//=======================================================
void * ObservatoryFunc(void *) {
//=======================================================

    if(Observatory.Interface == OBSERVATORY_INTERFACE_AOSCRAM && ObsOnly) {
        GetScramData();
        return(NULL);
    }

    if(Observatory.Interface == OBSERVATORY_INTERFACE_AOSCRAM) {
        // We need no delay between calls to GetScramData()
        // because scram_read() blocks until a new scram block
        // appears on the socket.
        while(Observatory.ThreadLive) GetScramData();
    }

    if(Verbose) {
        sprintf(Observatory.Message, "Observatory Thread: Exiting.\n");
        WriteLog(Observatory.Message, ToUser);
    }

    pthread_exit(NULL);
}

//=======================================================
void GetScramData() {
// Adapted from Jeff Mock code.
//=======================================================
    struct SCRAMNET *scram;
    char    name[256];
    double  az, za, ch;
    long    t, th, ts, tm;
    double  ra, dec, mjd;

    scram = (struct SCRAMNET *)Observatory.DataPtr;

//fprintf(stderr, "scram size : %d\n", sizeof(*scram));

    if (read_scram(scram) == -1) {
        ErrExit("GetScramData : bad scram read\n");
    } else {
        if (Verbose) {
            getnameinfo((const struct sockaddr *)&scram->from,
                            sizeof(struct sockaddr_in),
                            name, 256, NULL, 0, 0);
            sprintf(Observatory.Message, "Read SCRAM block type %s\n", scram->in.magic);
            WriteLog(Observatory.Message, ToUser);
        }

        if (strcmp(scram->in.magic, "PNT") == 0) {
            process_PNT(scram); 
            if(Verbose) {
                print_PNT(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        } else if (strcmp(scram->in.magic, "AGC") == 0) {
            process_AGC(scram);
            if(Verbose) {
                print_AGC(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        } else if (strcmp(scram->in.magic, "ALFASHM") == 0) {
            process_ALFASHM(scram);
            if(Verbose) {
                print_ALFASHM(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        } else if (strcmp(scram->in.magic, "IF1") == 0) {
            process_IF1(scram);
            if(Verbose) {
                print_IF1(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        } else if (strcmp(scram->in.magic, "IF2") == 0) {
            process_IF2(scram);
            if(Verbose) {
                print_IF2(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        } else if (strcmp(scram->in.magic, "TT") == 0) {
            process_TT(scram);
            if(Verbose) {
                print_TT(Observatory.Message);
                WriteLog(Observatory.Message, ToUser);
            }
        }
    }
}

//=======================================================
inline void process_PNT(struct SCRAMNET *scram) {
//=======================================================

    int i;

    if(pthread_mutex_lock(&Observatory.PNT_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.PNT_SysTime                 = time(NULL);
    Observatory.PNT_Ra                      = scram->pntData.st.x.pl.curP.raJ;
    Observatory.PNT_Dec                     = scram->pntData.st.x.pl.curP.decJ;
    //Observatory.ModelCorEncAzZa[0]          = scram->pntData.st.x.modelCorEncAzZa[0];
    Observatory.PNT_MJD                     = scram->pntData.st.x.pl.tm.mjd + scram->pntData.st.x.pl.tm.ut1Frac;
    for(i=0; i<2; i++) Observatory.PNT_ModelCorEncAzZa[i] = scram->pntData.st.x.modelCorEncAzZa[i];

    Observatory.PNT_Ra  *= 24.0 / C_2PI;
    Observatory.PNT_Dec *= 360.0 / C_2PI;

    if(pthread_mutex_unlock(&Observatory.PNT_Mutex)) ErrExit("GetScramData : mutex unlock error \n");
}

//=======================================================
inline void process_AGC(struct SCRAMNET *scram) {
//=======================================================

    if(pthread_mutex_lock(&Observatory.AGC_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.AGC_SysTime                 = time(NULL);
    Observatory.AGC_Az                      = scram->agcData.st.cblkMCur.dat.posAz;
    Observatory.AGC_Za                      = scram->agcData.st.cblkMCur.dat.posGr;
    Observatory.AGC_Time                    = scram->agcData.st.cblkMCur.dat.timeMs;

    Observatory.AGC_Az *= 0.0001;   // to degrees
    Observatory.AGC_Za *= 0.0001;

    if(pthread_mutex_unlock(&Observatory.AGC_Mutex)) ErrExit("GetScramData : mutex unlock error \n");

}

//=======================================================
inline void process_ALFASHM(struct SCRAMNET *scram) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.ALFASHM_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.ALFASHM_SysTime             = time(NULL);
    Observatory.ALFASHM_AlfaFirstBias       = (int)scram->alfa.first_bias;
    Observatory.ALFASHM_AlfaSecondBias      = (int)scram->alfa.second_bias;
    Observatory.ALFASHM_AlfaMotorPosition   = scram->alfa.motor_position;

    if(pthread_mutex_unlock(&Observatory.ALFASHM_Mutex)) ErrExit("GetScramData : mutex unlock error \n");
}

//=======================================================
inline void process_IF1(struct SCRAMNET *scram) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.IF1_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.IF1_SysTime                 = time(NULL);
    Observatory.IF1_synI_freqHz_0           = scram->if1Data.st.synI.freqHz[0];
    Observatory.IF1_synI_ampDB_0            = scram->if1Data.st.synI.ampDb[0];
    Observatory.IF1_rfFreq                  = scram->if1Data.st.rfFreq;
    Observatory.IF1_if1FrqMhz               = scram->if1Data.st.if1FrqMhz;
    Observatory.IF1_alfaFb                  = scram->if1Data.st.stat2.alfaFb;

    if(pthread_mutex_unlock(&Observatory.IF1_Mutex)) ErrExit("GetScramData : mutex unlock error \n");
}

//=======================================================
inline void process_IF2(struct SCRAMNET *scram) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.IF2_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.IF2_SysTime                 = time(NULL);
    Observatory.IF2_useAlfa                 = scram->if2Data.st.stat1.useAlfa;

    if(pthread_mutex_unlock(&Observatory.IF2_Mutex)) ErrExit("GetScramData : mutex unlock error \n");
}

//=======================================================
inline void process_TT(struct SCRAMNET *scram) {
//=======================================================

    static double Enc2Deg = 1./ ( (4096. * 210. / 5.0) / (360.) );

    if(pthread_mutex_lock(&Observatory.TT_Mutex)) ErrExit("GetScramData : mutex lock error \n");

    Observatory.TT_SysTime                  = time(NULL);
    //Observatory.TT_TurretEncoder            = scram->ttData.st.slv[0].tickMsg.position;
    Observatory.TT_TurretEncoder            = scram->ttData.st.slv[0].inpMsg.position;

//fprintf(stderr, ">>>>> %d \n", Observatory.TT_TurretEncoder);

    Observatory.TT_TurretDegrees = Observatory.TT_TurretEncoder * Enc2Deg;

    if(pthread_mutex_unlock(&Observatory.TT_Mutex)) ErrExit("GetScramData : mutex unlock error \n");
}

//=======================================================
inline void print_PNT(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.PNT_Mutex)) ErrExit("print_PNT : mutex lock error \n");

    sprintf(OutString, "SCRAM PNT PNT_SysTime %lf PNT_Ra %lf PNT_Dec %lf PNT_MJD %lf\n", 
            Observatory.PNT_SysTime, Observatory.PNT_Ra, Observatory.PNT_Dec, Observatory.PNT_MJD);

    if(pthread_mutex_unlock(&Observatory.PNT_Mutex)) ErrExit("print_PNT : mutex unlock error \n");
}

//=======================================================
inline void print_AGC(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.AGC_Mutex)) ErrExit("print_AGC : mutex lock error \n");

    sprintf(OutString, "SCRAM AGC AGC_SysTime %ld AGC_Az %lf AGC_Za %lf AGC_Time %ld AGC_LST %lf AGC_Ra %lf AGC_Dec %lf\n", 
            Observatory.AGC_SysTime, Observatory.AGC_Az, Observatory.AGC_Za, Observatory.AGC_Time);

    if(pthread_mutex_unlock(&Observatory.AGC_Mutex)) ErrExit("print_AGC : mutex unlock error \n");
}

//=======================================================
inline void print_IF1(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.IF1_Mutex)) ErrExit("print_IF1 : mutex lock error \n");

    sprintf(OutString, "SCRAM IF1 IF1_SysTime %ld IF1_synI_freqHz_0 %lf IF1_synI_ampDB_0 %d IF1_rfFreq %lf IF1_if1FrqMhz %lf IF1_alfaFb %d\n", 
            Observatory.IF2_SysTime, Observatory.IF1_synI_freqHz_0, Observatory.IF1_synI_ampDB_0, 
            Observatory.IF1_rfFreq, Observatory.IF1_if1FrqMhz, Observatory.IF1_alfaFb);

    if(pthread_mutex_unlock(&Observatory.IF1_Mutex)) ErrExit("print_IF1 : mutex unlock error \n");
}

//=======================================================
inline void print_IF2(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.IF2_Mutex)) ErrExit("print_IF2 : mutex lock error \n");

    sprintf(OutString, "SCRAM IF2 IF2_SysTime %ld IF2_useAlfa %d\n", Observatory.IF2_SysTime, Observatory.IF2_useAlfa);

    if(pthread_mutex_unlock(&Observatory.IF2_Mutex)) ErrExit("print_IF2 : mutex unlock error \n");
}

//=======================================================
inline void print_TT(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.TT_Mutex)) ErrExit("print_TT : mutex lock error \n");

    sprintf(OutString, "SCRAM TT TT_SysTime %ld TT_TurretEncoder %d TT_TurretDegrees %lf\n", 
            Observatory.TT_SysTime, Observatory.TT_TurretEncoder, Observatory.TT_TurretDegrees);

    if(pthread_mutex_unlock(&Observatory.TT_Mutex)) ErrExit("print_TT : mutex unlock error \n");
}

//=======================================================
inline void print_ALFASHM(char * OutString) {
//=======================================================
    if(pthread_mutex_lock(&Observatory.ALFASHM_Mutex)) ErrExit("print_ALFASHM : mutex lock error \n");

    sprintf(OutString, "SCRAM ALFASHM ALFASHM_SysTime %ld ALFASHM_AlfaFirstBias %d ALFASHM_AlfaSecondBias %d ALFASHM_AlfaMotorPosition %f\n", 
            Observatory.IF2_SysTime, Observatory.ALFASHM_AlfaFirstBias, Observatory.ALFASHM_AlfaSecondBias, Observatory.ALFASHM_AlfaMotorPosition);

    if(pthread_mutex_unlock(&Observatory.ALFASHM_Mutex)) ErrExit("print_ALFASHM : mutex unlock error \n");
}


//=======================================================
bool BadAGC() {
//=======================================================
    time_t now;
    bool retval = false;
    bool local_retval = false;

    now = time(NULL);

    if(pthread_mutex_lock(&Observatory.AGC_Mutex)) ErrExit("BadAGC : mutex lock error \n");
    if(now - Observatory.AGC_SysTime > Observatory.ScramLagTolerance) {
        retval = true;
        local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.AGC_Mutex)) ErrExit("BadAGC : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadAGC_time]) {
            IdleCond[BadAGC_time] = true;
            sprintf(IdleWatch.Message,"Bad AGC time added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadAGC_time]) {
            IdleCond[BadAGC_time] = false;
            sprintf(IdleWatch.Message,"Bad AGC time removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    return(retval);
}

//=======================================================
bool BadALFASHM() {
//=======================================================
    time_t now;
    bool retval = false;
    bool local_retval = false;

    now = time(NULL);

    if(pthread_mutex_lock(&Observatory.ALFASHM_Mutex)) ErrExit("BadALFASHM : mutex lock error \n");
    if(now - Observatory.ALFASHM_SysTime > Observatory.ScramLagTolerance) {
        retval = true;
        local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.ALFASHM_Mutex)) ErrExit("BadALFASHM : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadALFASHM_time]) {
            IdleCond[BadALFASHM_time] = true;
            sprintf(IdleWatch.Message,"Bad ALFASHM time added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadALFASHM_time]) {
            IdleCond[BadALFASHM_time] = false;
            sprintf(IdleWatch.Message,"Bad ALFASHM time removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    local_retval = false;
    if(pthread_mutex_lock(&Observatory.ALFASHM_Mutex)) ErrExit("BadALFASHM : mutex lock error \n");
    if(!Observatory.ALFASHM_AlfaFirstBias && !Observatory.ALFASHM_AlfaSecondBias) {
        retval = true;
        local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.ALFASHM_Mutex)) ErrExit("BadALFASHM : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadALFASHM_biases]) {
            IdleCond[BadALFASHM_biases] = true;
            sprintf(IdleWatch.Message,"ALFASHM biases off added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadALFASHM_biases]) {
            IdleCond[BadALFASHM_biases] = false;
            sprintf(IdleWatch.Message,"ALFASHM biases off removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    return(retval);
}

//=======================================================
bool BadIF1() {
//=======================================================
    time_t now;
    bool retval = false;
    bool local_retval = false;

    now = time(NULL);

    if(pthread_mutex_lock(&Observatory.IF1_Mutex)) ErrExit("BadIF1 : mutex lock error \n");
    if(now - Observatory.IF1_SysTime > Observatory.ScramLagTolerance) {
         retval = true;
         local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.IF1_Mutex)) ErrExit("BadIF1: mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadIF1_time]) {
            IdleCond[BadIF1_time] = true;
            sprintf(IdleWatch.Message,"Bad IF1 time added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadIF1_time]) {
            IdleCond[BadIF1_time] = false;
            sprintf(IdleWatch.Message,"Bad IF1 time removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    return(retval);
}

//=======================================================
bool BadIF2() {
//=======================================================
    time_t now;
    bool retval = false;
    bool local_retval = false;

    now = time(NULL);

    if(pthread_mutex_lock(&Observatory.IF2_Mutex)) ErrExit("BadIF2 : mutex lock error \n");
    if(now - Observatory.IF2_SysTime > Observatory.ScramLagTolerance) {
         retval = true;
         local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.IF2_Mutex)) ErrExit("BadIF2 : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadIF2_time]) {
            IdleCond[BadIF2_time] = true;
            sprintf(IdleWatch.Message,"Bad IF2 time added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadIF2_time]) {
            IdleCond[BadIF2_time] = false;
            sprintf(IdleWatch.Message,"Bad IF2 time removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    local_retval = false;
    if(pthread_mutex_lock(&Observatory.IF2_Mutex)) ErrExit("BadIF2 : mutex lock error \n");
    if(!Observatory.IF2_useAlfa) {
         retval = true;
         local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.IF2_Mutex)) ErrExit("BadIF2 : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadIF2_useAlfa]) {
            IdleCond[BadIF2_useAlfa] = true;
            sprintf(IdleWatch.Message,"UseALFA off added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadIF2_useAlfa]) {
            IdleCond[BadIF2_useAlfa] = false;
            sprintf(IdleWatch.Message,"UseALFA off removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    return(retval);
}
//=======================================================
bool BadTT() {
//=======================================================
    time_t now;
    bool retval = false;
    bool local_retval = false;

    now = time(NULL);

    if(pthread_mutex_lock(&Observatory.TT_Mutex)) ErrExit("BadTT : mutex lock error \n");
    if(now - Observatory.TT_SysTime > Observatory.ScramLagTolerance) {
         retval = true;
         local_retval = true;
    }
    if(pthread_mutex_unlock(&Observatory.TT_Mutex)) ErrExit("BadTT : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadTT_time]) {
            IdleCond[BadTT_time] = true;
            sprintf(IdleWatch.Message,"Bad TT time added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadTT_time]) {
            IdleCond[BadTT_time] = false;
            sprintf(IdleWatch.Message,"Bad TT time removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }

    local_retval = false;
    if(pthread_mutex_lock(&Observatory.TT_Mutex)) ErrExit("BadTT : mutex lock error \n");
    if(fabs(Observatory.TT_TurretDegrees - Observatory.TT_TurretDegreesAlfa) > 
        Observatory.TT_TurretDegreesTolerance) retval = true;
    if(pthread_mutex_unlock(&Observatory.TT_Mutex)) ErrExit("BadTT : mutex unlock error \n");
    if(local_retval) {
        if(!IdleCond[BadTT_TurretDegrees]) {
            IdleCond[BadTT_TurretDegrees] = true;
            sprintf(IdleWatch.Message,"Non-ALFA turret position added as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    } else {
        if(IdleCond[BadTT_TurretDegrees]) {
            IdleCond[BadTT_TurretDegrees] = false;
            sprintf(IdleWatch.Message,"Non-ALFA turret position removed as an idle condition\n");
            WriteLog(IdleWatch.Message, ToUser);
        }
    }


    return(retval);
}

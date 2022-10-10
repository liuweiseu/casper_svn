#include "dr2.h"
#include "globals.h"
#include "ao_utils.h"

//=======================================================
int SynthInit() {
//=======================================================

    if((Synth.Message =  (char *)calloc(1, StringSize)) == NULL)
        return(false);
    if((Synth.SerialPortName =  (char *)calloc(1, StringSize)) == NULL)
        return(false);

    Synth.Function        = SynthThread;
    Synth.ThreadLive      = true;
    Synth.Init            = true;
    Synth.Ready           = false;
    Synth.Model           = SynthModel;
    Synth.StepSynth       = SynthStepSynth;
    strcpy(Synth.SerialPortName, SynthSerialPortName);
    Synth.CheckCount      = SynthCheckCount;
    Synth.CurrentSkyFreq  = Synth.SkyFreq[0];
    Synth.Fmin            = Fmin;
    Synth.Fmax            = Fmax;
    Synth.FilteredFmin    = FilteredFmin;
    Synth.FilteredFmax    = FilteredFmax;
    Synth.SynthMin        = SynthMin;
    Synth.SynthMax        = SynthMax;

    Synth.PTS232 = OpenSerialPort(Synth.SerialPortName);

    pthread_mutex_init(&Synth.CheckMutex, NULL);
    pthread_mutex_init(&Synth.DataMutex, NULL);
    if(pthread_cond_init(&Synth.CheckNowCond, NULL)) ErrExit("SynthInit : condition signal init  error \n");

    if(Synth.StepSynth) {
        sprintf(Synth.Message, "Synthesizer will step\n");
        WriteLog(Synth.Message, ToUser);
    } else {
        sprintf(Synth.Message, "Synthesizer will not step\n");
        WriteLog(Synth.Message, ToUser);
    }

    if(NoSynth) {
        Synth.NoSynth = true;
        Synth.ObservableFreq  = true;
        sprintf(Synth.Message, "Configured for NO synthesizer\n");
        WriteLog(Synth.Message, ToUser);
    } else {
        Synth.ObservableFreq  = false;
        sprintf(Synth.Message, "Configured for synthesizer model PTS%d\n", Synth.Model);
        WriteLog(Synth.Message, ToUser);
    }

    //  time tag and program initial frequency
    //if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("SynthInit : mutex lock error \n");
    //ftime(&Synth.CurrentFreqTime);
    //if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("SynthInit : mutex unlock error \n");
    //ProgramSynth(Synth.CurrentFreq);

    return(0);
}


//=======================================================
void * SynthThread(void *) {
//=======================================================

    static double LastRa, LastDec, LastTime;  	// init for first time
    static double FirstRa, FirstDec, FirstTime;
    static double ThisAngle, ObsAngle;
    double CurrentAz, CurrentZa, CurrentRa, CurrentDec, CurrentTime_x;
    long SynthFreq;
    static int Step=0;
    int StartStep;
    static bool Init=true;			// ie, first time in
    long FirstLO;
    long AGC_Time;

    Synth.Ready = true;

#if 0
    if(Synth.StepSynth) {
        sprintf(Synth.Message, "Synthesizer will step\n");
        WriteLog(Synth.Message, ToUser);
#endif
        while(Synth.ThreadLive) {

            if(pthread_mutex_lock(&Synth.CheckMutex)) ErrExit("SynthFunc : mutex lock error \n");
            while(!Synth.CheckNow) {
                // this unlocks Synth.CheckMutex on entry and re-acquires it
                // on exit.
                if(pthread_cond_wait(&Synth.CheckNowCond, &Synth.CheckMutex)) ErrExit("SynthThread : condition wait error \n");
            }
            Synth.CheckNow = false;

            if (!Synth.ThreadLive) break;

            sprintf(Synth.Message, "Checking for frequency step condition\n");
            WriteLog(Synth.Message, ToUser);

            // we re-init on de-idling.  This is set in the idle thread.
            if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("SynthThread : mutex lock error \n");
            Init = Synth.Init;
            if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("SynthThread : mutex unlock error \n");

            // where is the telescope pointing right now?
            if(pthread_mutex_lock(&Observatory.AGC_Mutex)) ErrExit("SynthThread : observatory mutex lock error \n");
            CurrentAz   = Observatory.AGC_Az;
            CurrentZa   = Observatory.AGC_Za;
            AGC_Time    = Observatory.AGC_Time;
            if(pthread_mutex_unlock(&Observatory.AGC_Mutex)) ErrExit("SynthThread : observatory mutex unlock error \n");
            AzZaToRaDec(CurrentAz, CurrentZa, AGC_Time, CurrentRa, CurrentDec); 

            if (Init) {
                sprintf(Synth.Message, "(re)initializing frequency stepping\n");
                WriteLog(Synth.Message, ToUser);
                Init = false;
                if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("SynthThread : mutex lock error \n");
                Synth.Init = Init;
                if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("SynthThread : mutex unlock error \n");
                FirstRa   = LastRa    = CurrentRa;
                FirstDec  = LastDec   = CurrentDec;
                FirstTime = LastTime  = CurrentTime_x;
                Step      = 0;
            } else {
                // ThisAngle : the distance moved since last check
                // ObsAngle  : the distance moved from the position where we first
                //             determined that we were staring, ie the distance
                //             along the radius of the observation,
                ThisAngle = SkyAngle(LastRa, LastDec, CurrentRa, CurrentDec);
                ObsAngle  = SkyAngle(FirstRa, FirstDec, CurrentRa, CurrentDec);
    
                if (AlwaysStepSynth || (Synth.StepSynth && (ThisAngle < 0.1*BeamRes && ObsAngle < 0.5*BeamRes))) {
    
                    // we are staring
                    sprintf(Synth.Message, "Az %lf deg  Za %lf deg  RA %lf hrs  Dec %lf deg  (%lf %lf) *\n",
                        CurrentAz, CurrentZa, CurrentRa, CurrentDec, ThisAngle, ObsAngle);
                    WriteLog(Synth.Message, ToUser);

                    Step = Step >= Synth.NumFreqSteps-1 ? 0 : Step+1;

                    LastRa    = CurrentRa;
                    LastDec   = CurrentDec;

                } else {  

                    // we are not staring so (re)init
                    sprintf(Synth.Message, "Az %lf deg  Za %lf deg  RA %lf hrs  Dec %lf deg  (%lf %lf)\n",
                        CurrentAz, CurrentZa, CurrentRa, CurrentDec, ThisAngle, ObsAngle);
                    WriteLog(Synth.Message, ToUser);

                    FirstRa   = LastRa    = CurrentRa;
                    FirstDec  = LastDec   = CurrentDec;
                    FirstTime = LastTime  = CurrentTime_x;
                    Step      = 0;
                }
            }

#if 1
            // Determine if the currently desired frequency (stepped or not) is observable
            if(pthread_mutex_lock(&Observatory.IF1_Mutex)) ErrExit("SynthThread : observatory mutex lock error \n");
            FirstLO =(long)Observatory.IF1_synI_freqHz_0;
            if(pthread_mutex_unlock(&Observatory.IF1_Mutex)) ErrExit("SynthThread : observatory mutex lock error \n");
            StartStep = Step;
            do {    
                SynthFreq = GetSynthFreq(FirstLO, Synth.SkyFreq[Step]);
                //sprintf(Synth.Message, " Trying synth freq %ld for sky freq %ld ...\n", SynthFreq, Synth.SkyFreq[Step]);
                //WriteLog(Synth.Message, ToUser);

                if(FreqCheck(SynthFreq, Synth.SkyFreq[Step])) {
                    Synth.ObservableFreq = true;
                    break;
                }

                // if we are not stepping, this do loop will execute exactly once 
                if (Synth.StepSynth) {
                    Step = Step >= Synth.NumFreqSteps-1 ? 0 : Step+1;
                }
                if(Step == StartStep) { 
                    Synth.ObservableFreq = false;
                }

            } while(Step != StartStep);
#endif

            if (Synth.ObservableFreq && Synth.CurrentSynthFreq != SynthFreq) {

                if(Synth.CurrentSkyFreq != Synth.SkyFreq[Step]) {
                    sprintf(Synth.Message, "Stepping frequency to sky freq %ld (synth freq %ld)\n", Synth.SkyFreq[Step], SynthFreq);
                } else {
                    sprintf(Synth.Message, "Maintaining sky freq %ld by changing synth freq to %ld)\n", Synth.SkyFreq[Step], SynthFreq);
                }
                WriteLog(Synth.Message, ToUser);

                // assign and time tag new frequency
                if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("SynthThread : mutex lock error \n");
                Synth.CurrentSkyFreq   = Synth.SkyFreq[Step];
                Synth.CurrentSynthFreq = SynthFreq;
                ftime(&Synth.CurrentFreqTime);
                if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("SynthThread : mutex unlock error \n");

                //ProgramSynth(Synth.CurrentSynthFreq);
            }

            if(Synth.ObservableFreq) ProgramSynth(Synth.CurrentSynthFreq);

            // this was acquired on exit from pthread_cond_wait, above.
            if(pthread_mutex_unlock(&Synth.CheckMutex)) ErrExit("SynthThread : mutex unlock error \n");

            QuickLookCheck();

        }
        close(Synth.Fd);
        if(!NoSynth) ProgramSynth(-1);       // back to local control
#if 0
    } else {
        sprintf(Synth.Message, "Synthesizer will not step\n");
        WriteLog(Synth.Message, ToUser);
    }
#endif

    if(Verbose) {
        sprintf(Synth.Message, "Exiting SynthThread\n");
        WriteLog(Synth.Message, ToUser);
    }

    pthread_exit(NULL);

}

//=======================================================
inline long GetSynthFreq(long FirstLO, long SkyFreq) {
//=======================================================
    // High or low side mixing can be determined by:
    // If first LO < RF then mixing is low side
    // If first LO > RF then mixing is high side

    // The following is for low side mixing
    // return(SkyFreq - FirstLO);  

    // We assume high side mixing
    return(FirstLO - SkyFreq);
}

//=======================================================
inline bool FreqCheck(long SynthFreq, long SkyFreq) {
//=======================================================

    bool FilterInPlace;

    if(pthread_mutex_lock(&Observatory.IF1_Mutex)) ErrExit("FreqCheck : mutex lock error \n");
    FilterInPlace = Observatory.IF1_alfaFb;
    if(pthread_mutex_unlock(&Observatory.IF1_Mutex)) ErrExit("FreqCheck : mutex unlock error \n");

    if(FilterInPlace) {
        if(SkyFreq < Synth.FilteredFmin || SkyFreq > Synth.FilteredFmax) return false;
    } else {
        if(SkyFreq < Synth.Fmin || SkyFreq > Synth.Fmax) return false;
    }
        
    if(SynthFreq < Synth.SynthMin || SynthFreq > Synth.SynthMax) return false;

    return true;
}


//=======================================================
void ProgramSynth(long NewFreq) {
//=======================================================

    char SynthString[512];
    char ResponseString[512];
    int i, bytes, retval, alarm_val=1, time_on_alarm;

    if (NewFreq == -1) {
        sprintf(SynthString, "L#");     // reset the synth to local control
    } else {
        switch (Synth.Model) {
        case PTS500 :
            sprintf(SynthString, "F%09ld0#", NewFreq);
            break;
        case PTS3200 :
            sprintf(SynthString, "F%010ld#", NewFreq);
            break;
        default :
            sprintf(Synth.Message, "Invalid synthesizer model : %d\n", Synth.Model);
            WriteLog(Synth.Message, ToUser);
            ErrExit(NULL);
        }
    }

    alarm(alarm_val);       // make sure we do this within N s, this detects
                            // bad serial comm.

    bytes = write(Synth.PTS232, (void *)SynthString, strlen(SynthString));
    if (bytes != strlen(SynthString)) {
        sprintf(Synth.Message, "Synthesizer communication failure!\n");
        WriteLog(Synth.Message, ToUser);
        ErrExit(NULL);
    } else {
        if (NewFreq == -1) {
            sprintf(Synth.Message, "Resetting synthesizer for local control\n");
        } else {
            sprintf(Synth.Message, "Stepping Synthesizer to %d Hz with %s\n",
                    NewFreq, SynthString);
        }
        WriteLog(Synth.Message, ToUser);
    }

    // check response from synth
    for(i=0; i < sizeof(ResponseString)-1; i++) {
        retval = read(Synth.PTS232, (void*)&ResponseString[i], 1);
        if (retval == -1) {
            sprintf(Synth.Message, "Synthesizer communication failure!\n");
            WriteLog(Synth.Message, ToUser);
            ErrExit(NULL);
        } else {
            if(ResponseString[i] == '>') break;  // response delimiter
        }
    }
    time_on_alarm = alarm(0);       // get time remaining on alarm and turn it off
    //sprintf(Synth.Message, "%s in %d seconds\n", ResponseString, alarm_val-time_on_alarm);
    //WriteLog(Synth.Message, ToUser);
}


//=======================================================
double SkyAngle(double Ra1, double Dec1, double Ra2, double Dec2) {
//=======================================================

    double CosSkyAng, SkyAng;
    const double d2r =  0.017453292;

    // everything to radians
    Ra1   *= 15.0;
    Ra2   *= 15.0;
    Ra1   *= d2r;
    Dec1  *= d2r;
    Ra2   *= d2r;
    Dec2  *= d2r;

    // solve spherical triangle
    CosSkyAng = sin(Dec1) * sin(Dec2) +
                cos(Dec1) * cos(Dec2) * cos(Ra1 - Ra2);

    // make sure small rounding errors do not lead
    // to an impossible value for a cosine
    if      (CosSkyAng >  1) CosSkyAng =  1;
    else if (CosSkyAng < -1) CosSkyAng = -1;
    SkyAng = acos(CosSkyAng);

    return (SkyAng /= d2r);
}

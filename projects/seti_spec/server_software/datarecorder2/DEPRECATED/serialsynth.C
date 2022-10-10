#include "dr2.h"
#include "globals.h"

//=======================================================
int SerialSynthInit() {
    //=======================================================

    if((SerialSynth.Message =  (char *)calloc(1, StringSize)) == NULL)
        return(false);
    if((SerialSynth.SerialPortName =  (char *)calloc(1, StringSize)) == NULL)
        return(false);

    SerialSynth.Function          = SerialSynthThread;
    SerialSynth.ThreadLive        = true;
    SerialSynth.StepSynth         = SynthStepSynth;
    strcpy(SerialSynth.SerialPortName, SerialPortName);
    SerialSynth.BaseFreq 		= SynthBaseFreq;
    SerialSynth.CurrentFreq	= SynthBaseFreq;
    SerialSynth.FreqStep 		= SynthFreqStep;
    SerialSynth.CheckCount        = SerialSynthCheckCount;
    SerialSynth.CurrentOffset     = 0;
    SerialSynth.MaxOffset         = SynthMaxOffset;

    SerialSynth.PTS232 = OpenSerialPort(SerialSynth.SerialPortName);

    pthread_mutex_init(&SerialSynth.CheckMutex, NULL);
    pthread_mutex_init(&SerialSynth.CheckCompleteMutex, NULL);
    pthread_cond_init(&SerialSynth.CheckNowCond, NULL);
    return(0);
}


//=======================================================
void * SerialSynthThread(void *) {
    //=======================================================

    static double LastRa=25, LastDec, LastTime;  	// init for first time
    static double FirstRa, FirstDec, FirstTime;
    static double ThisAngle, ObsAngle, BeamRes;
    static long NewFreq;				// Hz
    char SynthString[512];
    char ResponseString[512];
    int i, bytes, retval;
    bool UpdateSynth = false;

    if(SerialSynth.StepSynth) {
        sprintf(SerialSynth.Message, "Synthesizer will step\n");
        WriteLog(SerialSynth.Message, ToUser);
        while(SerialSynth.ThreadLive) {

            pthread_mutex_lock(&SerialSynth.CheckMutex);
            while(!SerialSynth.CheckNow) {
                // this unlocks SerialSynth.CheckMutex on entry and re-acquires it
                // on exit.
                pthread_cond_wait(&SerialSynth.CheckNowCond, &SerialSynth.CheckMutex);
            }
            SerialSynth.CheckNow = false;

            if (LastRa == 25) {
                FirstRa   = LastRa    = Observatory.Ra;
                FirstDec  = LastDec   = Observatory.Dec;
                FirstTime = LastTime  = Observatory.Time;
            } else {
                ThisAngle = SkyAngle(LastRa, LastDec, Observatory.Ra, Observatory.Dec);
                ObsAngle  = SkyAngle(FirstRa, FirstDec, Observatory.Ra, Observatory.Dec);
            }
            //if (ThisAngle < 0.1*BeamRes && ObsAngle < 0.5*BeamRes) {
            if (1) {
                // If we have not gone negative for this offset, do so.
                // Else, if we have not reached a max offset, calc new offset.
                // Else, go back to the base freq (zero offset).
                if (SerialSynth.CurrentOffset > 0 &&
                        SerialSynth.CurrentFreq != SerialSynth.BaseFreq) {
                    SerialSynth.CurrentOffset = 0 - SerialSynth.CurrentOffset;
                } else {
                    SerialSynth.CurrentOffset = 0 - SerialSynth.CurrentOffset +
                                                SerialSynth.FreqStep;
                }
                if (SerialSynth.CurrentOffset <=  SerialSynth.MaxOffset) {
                    NewFreq = SerialSynth.BaseFreq + SerialSynth.CurrentOffset;
                } else {
                    NewFreq = SerialSynth.BaseFreq;
                    SerialSynth.CurrentOffset = 0;
                }

                // assign and time tag new frequency
                // this is mutex protected, right?
                SerialSynth.CurrentFreq = NewFreq;
                ftime(&SerialSynth.CurrentFreqTime);
                UpdateSynth = true;

            } else {  // we are not staring

                // keep current.
                FirstRa   = LastRa    = Observatory.Ra;
                FirstDec  = LastDec   = Observatory.Dec;
                FirstTime = LastTime  = Observatory.Time;

                // go back to base frequency if not already there.
                if (SerialSynth.CurrentFreq != SerialSynth.BaseFreq) {
                    NewFreq = SerialSynth.BaseFreq;
                    SerialSynth.CurrentFreq = NewFreq;
                    ftime(&SerialSynth.CurrentFreqTime);
                    UpdateSynth = true;
                }
            }

            if (UpdateSynth) {
                // send new frequency to synth
                sprintf(SynthString, "F%ld0#", NewFreq);
                bytes = write(SerialSynth.PTS232, (void *)SynthString, strlen(SynthString));
                if (bytes != strlen(SynthString)) {
                    sprintf(SerialSynth.Message, "Synthesizer communication failure!\n");
                    WriteLog(SerialSynth.Message, ToUser);
                    ErrExit(NULL);
                } else {
                    sprintf(SerialSynth.Message, "Stepping Synthesizer to %d Hz with %s\n", NewFreq, SynthString);
                    WriteLog(SerialSynth.Message, ToUser);
                }

                // check response from synth
                for(i=0; i < sizeof(ResponseString)-1; i++) {
                    retval = read(SerialSynth.PTS232, (void*)&ResponseString[i], 1);
                    if (retval == -1) {
                        sprintf(SerialSynth.Message, "Synthesizer communication failure!\n");
                        WriteLog(SerialSynth.Message, ToUser);
                        ErrExit(NULL);
                    } else {
                        if(ResponseString[i] == '>') break;  // response delimiter
                    }
                }
                sprintf(SerialSynth.Message, "%s\n", ResponseString);
                WriteLog(SerialSynth.Message, ToUser);
            }

            pthread_mutex_unlock(&SerialSynth.CheckMutex);	// this was acquired on exit from
            // pthread_cond_wait, above.

        }
        close(SerialSynth.Fd);
    } else {
        sprintf(SerialSynth.Message, "Synthesizer will not step\n");
        WriteLog(SerialSynth.Message, ToUser);
    }

    if(Verbose)
    {
        sprintf(SerialSynth.Message, "Exiting SerialSynthThread\n");
        WriteLog(SerialSynth.Message, ToUser);
    }

    pthread_exit(NULL);

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



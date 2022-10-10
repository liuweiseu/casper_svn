#include "dr2.h"
#include "globals.h"

//=======================================================
void GetData(int dsi) {
//=======================================================

    long r_AtBuf = 0, last_r_AtBuf;

    char edt_msg[256];

    struct timespec nanotime;
    char * p;
    // next buffer structure to contain data pointer and data size
    edt_buffer nextbuffer;

//#if 0
//    sprintf(edt_msg, "%s dsi = %i", "edt_wait", dsi);
//    //fprintf(stderr, "edtp = %p\n", (char *)edt_wait_for_buffers(EdtDevice, 1));
//    p = (char *)edt_wait_for_buffers(MemBuf[dsi].EdtDevice, 1);
//    if(!p) {
//        //if(1) {
//        sprintf(edt_msg, "%s[%d]", "edt_wait_for_buffers", dsi);
//        edt_perror(edt_msg);
//    }
//    fprintf(stderr, "edtp = %p\n", p);
//    return;
//#endif

    MemBuf[dsi].Ready = true;

    while(MemBuf[dsi].ThreadLive) {
        // upon program startup and while in an idle state, we will idle in 
        // edt_wait_for_buffers() until we command the EDT card to begin 
        // data acquisition.
        if(!UseFakeData) {
            edt_wait_for_buffers(MemBuf[dsi].EdtDevice, &nextbuffer, 1);
            MemBuf[dsi].ReadyPtr[r_AtBuf] = (unsigned char *) nextbuffer.data;
            MemBuf[dsi].Size[r_AtBuf] = nextbuffer.buffer_size;
            
        } else {
            nanotime.tv_sec  = 1;           // needs tuning
            nanotime.tv_nsec = 0;
            nanosleep(&nanotime, NULL);
            MemBuf[dsi].ReadyPtr[r_AtBuf] = FakeData.ReadyPtr[r_AtBuf];
        }

        if(!MemBuf[dsi].ThreadLive) break;

        ftime(&MemBuf[dsi].ReadyTime[r_AtBuf]);     // record time and freq immediately
                                                    // not mutex protected - see design doc

        if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("GetData : mutex lock error \n");
        MemBuf[dsi].SynthFreq[r_AtBuf] = Synth.CurrentSynthFreq;
        MemBuf[dsi].SkyFreq[r_AtBuf]   = Synth.CurrentSkyFreq;
        MemBuf[dsi].SynthTime[r_AtBuf] = Synth.CurrentFreqTime;
        if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("GetData : mutex unlock error \n");

        if(MemBuf[dsi].ReadyPtr[r_AtBuf] == NULL) break;        // time to quit?
        if(LimitedRun && LimitedRunNum-- <= 0) SayToExit();

        if(!UseFakeData) {
            MemBuf[dsi].DoneCount = edt_done_count(MemBuf[dsi].EdtDevice);  // update count
        } else {
            MemBuf[dsi].DoneCount++;
        }

        // Go see if the synthesizer should be stepped.  This check gets
        // done every Synth.CheckCount buffers of acquired data.  Only
        // the first of N data streams should make this check (there is
        // only one synthesiser).
        if(dsi == 0 && MemBuf[dsi].DoneCount % Synth.CheckCount == 0 && !Synth.NoSynth) {
            if(pthread_mutex_lock(&Synth.CheckMutex)) ErrExit("GetData : mutex lock error \n");
            Synth.CheckNow = true;
            if(pthread_cond_signal(&Synth.CheckNowCond)) ErrExit("GetData : condition signal error \n");
            if(pthread_mutex_unlock(&Synth.CheckMutex)) ErrExit("GetData : mutex unlock error \n");
        }

        if(VeryVerbose) {
            sprintf(MemBuf[dsi].Message,
                    "MEM[%d] : ringbuf num is %ld ringbuf pointer is %p ringbuf time is %f freq is %ld freqtime is %f\n",
                    dsi, r_AtBuf, MemBuf[dsi].ReadyPtr[r_AtBuf], 
                    MemBuf[dsi].ReadyTime[r_AtBuf].time + MemBuf[dsi].ReadyTime[r_AtBuf].millitm/1000.0,
                    MemBuf[dsi].SynthFreq[r_AtBuf],
                    MemBuf[dsi].SynthTime[r_AtBuf].time + MemBuf[dsi].SynthTime[r_AtBuf].millitm/1000.0);
            WriteLog(MemBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
        }
        // safely move the mutex forward
        last_r_AtBuf = r_AtBuf;
        r_AtBuf = r_AtBuf == MemBuf[dsi].Num - 1 ? 0 : r_AtBuf + 1;
        if(pthread_mutex_lock(&MemBuf[dsi].Mutex[r_AtBuf])) ErrExit("GetData : mutex lock error \n");
        if(pthread_mutex_unlock(&MemBuf[dsi].Mutex[last_r_AtBuf])) ErrExit("GetData : mutex unlock error \n");
    }
}

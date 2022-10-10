#include "dr2.h"
#include "globals.h"

//======================================================
int IdleWatchInit() {
//======================================================

  int dsi, i;

  if((IdleWatch.Message =  (char *)calloc(1, StringSize)) == NULL)
        return(false);

  IdleWatch.Function              = IdleWatchThread;
  IdleWatch.Interval              = IdleWatchInterval;
  IdleWatch.ThreadLive            = true;
  IdleWatch.Ready                 = false;

  for (i=0; i < 10; i++) IdleCond[i] = false;

  if(pthread_mutex_init(&IdleWatch.Mutex, NULL)) {
        ErrExit("\nCannot initialize idle watch mutex ");
  }

  return(0);
}


//=======================================================
void * IdleWatchThread(void *) {
//=======================================================

    int i, dsi, vgc_sum;
    bool do_idle, ObservableFreq=true;
    char local_string[StringSize];
    char * local_string_p;
    time_t now;

    IdleWatch.Ready = true;

    while (IdleWatch.ThreadLive) {
        do_idle = false;
        local_string[0] = '\0';
        local_string_p = &local_string[0];

        // Check VGC values
        for(dsi=0; dsi < NumDatastreams; dsi++) {
            vgc_sum = 0;
            if(pthread_mutex_lock(&Eng[dsi].Mutex)) ErrExit("EngThread : mutex lock error \n");
            for(i=0; i < NumChannels; i++) {
                vgc_sum += Eng[dsi].Vgc[i];
                if(dsi == 0 && i == 0) {        // only check channel 0,0 as per Dan
                    if(Eng[dsi].Vgc[i] < IdleWatch.MinVGC) {
                        do_idle = true;
                        if(!IdleCond[BadVGC]) {
                            IdleCond[BadVGC] = true;
                            sprintf(IdleWatch.Message,
                                "Bad VGC added as an idle condition : VGC[%d][%d] = %d\n", dsi, i, Eng[dsi].Vgc[i]);
                            WriteLog(IdleWatch.Message, ToUser);
                        }
                    } else {
                        if(IdleCond[BadVGC]) {
                            IdleCond[BadVGC] = false;
                            sprintf(IdleWatch.Message,
                                "Bad VGC removed as an idle condition : VGC[%d][%d] = %d\n", dsi, i, Eng[dsi].Vgc[i]);
                            WriteLog(IdleWatch.Message, ToUser);
                        }
                    }
                }
            }
            if(pthread_mutex_unlock(&Eng[dsi].Mutex)) ErrExit("EngThread : mutex unlock error \n");
            //TODO: should this be replaced with another condition?
            //if(vgc_sum == 0) do_idle = true;       // start of program indicator 
        }

        // Check observatory values
        if(!SkipScram) {
            if(BadAGC()) {
                print_AGC(local_string);
                //sprintf(IdleWatch.Message,"%s", local_string);
                //WriteLog(IdleWatch.Message, ToUser);
                do_idle = true;
            }
            fprintf(stderr, "after checking AGC idling is %d\n", do_idle);
            if(BadALFASHM()) {
                print_ALFASHM(local_string);
                //sprintf(IdleWatch.Message,"%s", local_string);
                //WriteLog(IdleWatch.Message, ToUser);
                do_idle = true;
            }
            if(BadIF1()) {
                print_IF1(local_string);
                //sprintf(IdleWatch.Message,"%s", local_string);
                //WriteLog(IdleWatch.Message, ToUser);
                do_idle = true;
            }
            if(BadIF2()) {
                print_IF2(local_string);
                //sprintf(IdleWatch.Message,"%s", local_string);
                //WriteLog(IdleWatch.Message, ToUser);
                do_idle = true;
            }
            if(BadTT()) {
                print_TT(local_string);
                //sprintf(IdleWatch.Message,"%s", local_string);
                //WriteLog(IdleWatch.Message, ToUser);
                do_idle = true;
            }
            fprintf(stderr, "after checking observatory values idling is %d\n", do_idle);
        }

        // Check if we have an observable frequency
        if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("IdleWatchThread : synth mutex lock error \n");
        ObservableFreq = Synth.ObservableFreq;
        if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("IdleWatchThread : synth mutex unlock error \n");
        if(!ObservableFreq) { 
            do_idle = true;
            if(!IdleCond[BadFreq]) {
                IdleCond[BadFreq] = true;
                sprintf(IdleWatch.Message,"No observable frequency added as an idle condition\n");
                WriteLog(IdleWatch.Message, ToUser);
            }
        } else {
            if(IdleCond[BadFreq]) {
                IdleCond[BadFreq] = false;
                sprintf(IdleWatch.Message,"No observable frequency removed as an idle condition\n");
                WriteLog(IdleWatch.Message, ToUser);
            }
        }

        // Change idle state if indicated.
        if(do_idle && !DiskBuf[0].Idle) {        // just DiskBuf[0] ? jeffc

            //(local_string_p-1)[0] = '\0';
            sprintf(IdleWatch.Message,"IDLING DATA ACQUISITION%\n", local_string);
            WriteLog(IdleWatch.Message, ToUser);

            if(pthread_mutex_lock(&IdleWatch.Mutex)) ErrExit("IdleWatchThread : mutex lock error \n");
            for(dsi=0; dsi < NumDatastreams; dsi++) DiskBuf[dsi].Idle = true;
            if(pthread_mutex_unlock(&IdleWatch.Mutex)) ErrExit("IdleWatchThread : mutex unlock error \n");

        } else if(do_idle && DiskBuf[0].Idle) {

            //sprintf(IdleWatch.Message,"STILL IDLING DATA ACQUISITION %s\n", local_string);
            //WriteLog(IdleWatch.Message, ToUser);

        } else if(!do_idle && DiskBuf[0].Idle) {

            sprintf(IdleWatch.Message,"DE-IDLING DATA ACQUISITION (RE-INITIALIZING FREQUENCY STEPPING)\n");
            WriteLog(IdleWatch.Message, ToUser);

            if(pthread_mutex_lock(&Synth.DataMutex)) ErrExit("IdleWatchThread : synth mutex lock error \n");
            Synth.Init = true;
            if(pthread_mutex_unlock(&Synth.DataMutex)) ErrExit("IdleWatchThread : synth mutex unlock error \n");

            if(pthread_mutex_lock(&IdleWatch.Mutex)) ErrExit("IdleWatchThread : mutex lock error \n");
            for(dsi=0; dsi < NumDatastreams; dsi++) DiskBuf[dsi].Idle = false;
            if(pthread_mutex_unlock(&IdleWatch.Mutex)) ErrExit("IdleWatchThread : mutex unlock error \n");
        }

        sleep(IdleWatch.Interval);
    }

    if(Verbose) {
        sprintf(IdleWatch.Message, "IdleWatch : Exiting.\n", dsi);
        WriteLog(IdleWatch.Message, ToUser);
    }

    pthread_exit(NULL);
}

//=======================================================
bool Idling(int dsi) {
//=======================================================

    bool retval;

    if(pthread_mutex_lock(&IdleWatch.Mutex)) ErrExit("Idling : mutex lock error \n");
    retval =  DiskBuf[dsi].Idle;
    if(pthread_mutex_unlock(&IdleWatch.Mutex)) ErrExit("Idling : mutex unlock error \n");

    return(retval);
}

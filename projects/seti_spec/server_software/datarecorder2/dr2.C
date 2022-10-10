//#define DEBUG
#include "dr2.h"
#include "globals.h"

// ---------- Make sure we have the needed system support
#ifdef _POSIX_PRIORITY_SCHEDULING
#ifdef _POSIX_MAPPED_FILES
#ifdef _POSIX_SYNCHRONIZED_IO

//=======================================================
int main(int argc, char ** argv) {
//=======================================================

    int i, dsi, retval = 0;

#ifdef DEBUG
    Debug = true;
#endif

    retval = InitLog();
    if(retval) ErrExit("Could not init logging \n");

    GetOps(argc, argv);
    GetConfig();

    if(VersionOnly) {
        fprintf(stderr, "%s %4.2f\n", argv[0], Version);
        return(retval);
    } else if(ObsOnly) {
        ObservatoryInit();
        ObservatoryFunc(NULL);
        sprintf(Observatory.Message, "In the Observatory struct, RA : %lf  Decl : %lf  Time : %lf\n",
                Observatory.PNT_Ra, Observatory.PNT_Dec, Observatory.PNT_MJD);
        return(retval);
    }

    InitProg();		       // set up buffers, control structure, etc

    MySignals(INIT);      // we catch SIGINT and SIGTERM

    StartThreads();   	  // datastream threads will idle until
                          //  EDT enabled

    MySignals(UNBLOCK);   // only main() thread will catch signals

    while(!AllReady()) sleep(1);
    for(dsi = 0; dsi < NumDatastreams; dsi++) EdtCntl(MemBuf[dsi].EdtDevice, edt_arm);
    EdtEnabled = true;

    Rejoin();         // gather threads upon graceful shutdown

    exit(retval);
}

//======================================================
bool AllReady() {
//======================================================

    bool IsAllReady = true;
    char r_string[StringSize];
    int dsi;

    r_string[0] = '\0';

    for(dsi = 0; dsi < NumDatastreams; dsi++) {
        if(!MemBuf[dsi].Ready)  {IsAllReady = false; sprintf(r_string, "membuf[%d] ", dsi);}
        if(!DiskBuf[dsi].Ready) {IsAllReady = false; sprintf(r_string, "diskbuf[%d] ", dsi);}
        //if(!Eng[dsi].Ready)     {IsAllReady = false; sprintf(r_string, "eng[%d] ", dsi);}
    }
    if(!Trigger.Ready)      {IsAllReady = false; sprintf(r_string, "trigger ");}
    if(!IdleWatch.Ready)    {IsAllReady = false; sprintf(r_string, "idle ");}
    if(!RingWatch.Ready)    {IsAllReady = false; sprintf(r_string, "ringwatch ");}
    if(!Synth.Ready)        {IsAllReady = false; sprintf(r_string, "synth ");}
    if(!IsAllReady) {
        fprintf(stderr, "NOT READY : %s\n", r_string);
    } else {
        fprintf(stderr, "ALL READY\n");
    }

    return(IsAllReady);
}

//======================================================
void InitProg() {
//=======================================================
    int retval;

    if(Headers) {
        retval = InitHeader();
        if(retval) ErrExit("Could not init for headers \n");
    }

    retval = TriggerInit();
    if(retval) ErrExit("Could not init for trigger thread \n");

    retval = RingWatchInit();
    if(retval) ErrExit("Could not init for ring watch thread \n");

    retval = SynthInit();
    if(retval) ErrExit("Could not init for serial synthesizer control thread \n");

    retval = EngInit();
    if(retval) ErrExit("Could not init for engineering control thread \n");

    retval = ObservatoryInit();
    if(retval) ErrExit("Could not init for observatory thread \n");

    retval = MemBufInit();
    if(retval) ErrExit("Could not init for memory buffer threads \n");

    if(!MemBufOnly) {
        retval = DiskBufInit();
        if(retval) ErrExit("Could not init for disk buffer threads \n");
    }

    retval = IdleWatchInit();
    if(retval) ErrExit("Could not init for idle watch thread \n");

    retval = QuickLookInit();
    if(retval) ErrExit("Could not init for quick look \n");

}

//=======================================================
void  StartThreads() {
//=======================================================

    int retval, i;
    int FifoPrio[NUMPRIOLEVELS];

    // get priority levels
    FifoPrio[MAXPRIO] = sched_get_priority_max(SCHED_FIFO);
    FifoPrio[MINPRIO] = sched_get_priority_min(SCHED_FIFO);
    if(Verbose) {
        fprintf(stderr, "System scheduling priorities. Max is %d  Min is %d\n",
                FifoPrio[MAXPRIO], FifoPrio[MINPRIO]);
    }

    // Trigger
    retval = StartThread( &Trigger.ThreadAttr,
                          &Trigger.SchedParam,
                          &Trigger.ThreadId,
                          FifoPrio[MINPRIO],
                          Trigger.Function,
                          NULL);
    // Synthesizer Control
    retval = StartThread( &Synth.ThreadAttr,
                          &Synth.SchedParam,
                          &Synth.ThreadId,
                          FifoPrio[MINPRIO],
                          Synth.Function,
                          NULL);

    // Observatory Interface
    retval = StartThread( &Observatory.ThreadAttr,
                          &Observatory.SchedParam,
                          &Observatory.ThreadId,
                          FifoPrio[MINPRIO],
                          Observatory.Function,
                          NULL);

    // Ring Watch
    retval = StartThread( &RingWatch.ThreadAttr,
                          &RingWatch.SchedParam,
                          &RingWatch.ThreadId,
                          FifoPrio[MINPRIO],
                          RingWatch.Function,
                          NULL);

    // Datastreams
    for(i = 0; i < NumDatastreams; i++) {

        // Engineering for this stream
//        if(Verbose) {
//            sprintf(Eng[i].Message, "Starting engineering thread %ld...\n ", i);
//            WriteLog(Eng[i].Message, ToUser+ToLogFile);
//        }
//        retval = StartThread(&Eng[i].ThreadAttr,
//                             &Eng[i].SchedParam,
//                             &Eng[i].ThreadId,
//                             FifoPrio[MINPRIO],
//                             Eng[i].Function,
//                             (void *)&Eng[i].Index);

        // Memory Buffer for this stream
        if(Verbose) {
            sprintf(MemBuf[i].Message, "Starting memory buffer thread %ld...\n ", i);
            WriteLog(MemBuf[i].Message, ToUser+ToLogFile);
        }
        retval = StartThread(&MemBuf[i].ThreadAttr,
                             &MemBuf[i].SchedParam,
                             &MemBuf[i].ThreadId,
                             FifoPrio[MAXPRIO],
                             MemBuf[i].Function,
                             (void *)&MemBuf[i].Index);

        if(!MemBufOnly) {
            // Disk Buffer for this stream
            if(Verbose) {
                sprintf(DiskBuf[i].Message, "Starting disk buffer thread %ld...\n ", i);
                WriteLog(DiskBuf[i].Message, ToUser+ToLogFile);
            }
            retval = StartThread(&DiskBuf[i].ThreadAttr,
                                 &DiskBuf[i].SchedParam,
                                 &DiskBuf[i].ThreadId,
                                 FifoPrio[MINPRIO],
                                 DiskBuf[i].Function,
                                 (void *)&DiskBuf[i].Index);
        }
    }

    // IdelWatch
    retval = StartThread( &IdleWatch.ThreadAttr,
                          &IdleWatch.SchedParam,
                          &IdleWatch.ThreadId,
                          FifoPrio[MINPRIO],
                          IdleWatch.Function,
                          NULL);

}

//=======================================================
void  Rejoin() {
//=======================================================

    int i;

    for(i = 0; i < NumDatastreams; i++) {
        pthread_join(MemBuf[i].ThreadId, NULL);
        sprintf(MemBuf[i].Message, "MEM[%d] Thread has rejoined\n" ,i);
        WriteLog(MemBuf[i].Message, ToUser+ToLogFile);
    }

    if(!MemBufOnly) {
        for(i = 0; i < NumDatastreams; i++) {
            pthread_join(DiskBuf[i].ThreadId, NULL);
            sprintf(DiskBuf[i].Message,
                    "DSK[%d] Thread has rejoined\n", i);
            WriteLog(DiskBuf[i].Message, ToUser+ToLogFile);
        }
    }

//    for(i = 0; i < NumDatastreams; i++) {
//        pthread_join(Eng[i].ThreadId, NULL);
//        sprintf(Eng[i].Message, "ENG[%d] Thread has rejoined\n" ,i);
//        WriteLog(Eng[i].Message, ToUser+ToLogFile);
//    }

    pthread_join(Synth.ThreadId, NULL);
    sprintf(Synth.Message, "Synth Thread has rejoined\n");
    WriteLog(Synth.Message, ToUser+ToLogFile);

    // the cancel is necessary because the AO scram_read() does
    // a blocking recvfrom().
    pthread_cancel(Observatory.ThreadId);
    pthread_join(Observatory.ThreadId, NULL);
    sprintf(Observatory.Message, "Observatory Thread has rejoined\n");
    WriteLog(Observatory.Message, ToUser+ToLogFile);

    pthread_join(Trigger.ThreadId, NULL);
    sprintf(Trigger.Message, "Trigger Thread has rejoined\n");
    WriteLog(Trigger.Message, ToUser+ToLogFile);

    pthread_join(RingWatch.ThreadId, NULL);
    sprintf(RingWatch.Message, "Ring Watch Thread has rejoined\n");
    WriteLog(RingWatch.Message, ToUser+ToLogFile);

    pthread_join(IdleWatch.ThreadId, NULL);
    sprintf(IdleWatch.Message, "Idle Watch Thread has rejoined\n");
    WriteLog(IdleWatch.Message, ToUser+ToLogFile);

}


//=======================================================
int StartThread(pthread_attr_t          * Attr,
                struct sched_param      * SchedParam,
                pthread_t               * Id,
                int                       Prio,
                void *(*Function)(void *),
                void   			* Parm) {
//=======================================================

    int retval = 0;

    // Init
    retval = pthread_attr_init(Attr);
    if(retval) ErrExit("Could not init thread  \n");

    // set scope
    retval = pthread_attr_setscope(Attr, PTHREAD_SCOPE_SYSTEM);
    if(retval) ErrExit("Could not set bound attribute for string thread  \n");

    if(PrioSched) {
        // set scheduling policy
        retval = pthread_attr_setschedpolicy(Attr, SCHED_FIFO);
        if(retval) ErrExit("Could not set scheduling policy  \n");

        // set scheduling priority
        SchedParam->sched_priority = Prio;
        retval = pthread_attr_setschedparam(Attr, SchedParam);
        if(retval) ErrExit("Could not set scheduling param  \n");
    }

    // create the thread - thread begins execution
    retval = pthread_create(Id, Attr, Function, Parm);
    if(retval) ErrExit("Could not create thread  \n");

    return(0);
}

//=======================================================
void PrintThreadInfo(	char * ThreadName,
                      int Index,
                      pthread_t ThreadId,
                      pthread_attr_t * ThreadAttr) {
//=======================================================

    struct sched_param SchedParam;
    int retval;
    char MsgBuf[StringSize];
    char MsgBuf2[StringSize];

    sprintf(MsgBuf, "%s %ld scheduling info: ", ThreadName, Index);
    retval = pthread_attr_getschedparam(ThreadAttr, &SchedParam);
    if(retval) ErrExit("Could not get scheduling param  \n");
    sprintf(MsgBuf2, "PID is %d   Thread ID is %lu priority is %d\n",
            getpid(), ThreadId, SchedParam.sched_priority);
    strcat(MsgBuf, MsgBuf2);
    WriteLog(MsgBuf, ToUser);
    return;
}


//=======================================================
void SayToExit() {
//=======================================================

    int i, j;

    //exit(0);            // what a kludge!

    EdtEnabled = false;
    for(i = 0; i < NumDatastreams; i++) {
        Eng[i].ThreadLive        = false;
        DiskBuf[i].ThreadLive    = false;
        MemBuf[i].ThreadLive     = false;
    }

    for(i = 0; i < NumDatastreams; i++) {
        for(j = 0; j < MemBuf[i].Num; j++) {
            if(pthread_mutex_unlock(&MemBuf[i].Mutex[j]))  ErrExit("SayToExit : mutex unlock error \n");
        }
    }

    Synth.ThreadLive       = false;
    if(pthread_mutex_lock(&Synth.CheckMutex)) ErrExit("SayToExit : mutex lock error \n");
    Synth.CheckNow = true;
    if(pthread_cond_signal(&Synth.CheckNowCond)) ErrExit("SayToExit : condition signal error \n");
    if(pthread_mutex_unlock(&Synth.CheckMutex)) ErrExit("SayToExit : mutex unlock error \n");

    Trigger.ThreadLive     = false;
    RingWatch.ThreadLive = false;
    Observatory.ThreadLive = false;
    IdleWatch.ThreadLive = false;

    sleep(1);    
    fprintf(stderr, "Waiting for sleeping threads (like engineering) to awake and exit...\n");
    sleep(Eng[0].Interval);    

#if 0
    if(!UseFakeData) {
        for(i = 0; i < NumDatastreams; i++) {
            if(EdtIsOpen(i)) CloseEdt(i);
        }
    }
#endif

}

// ---------- Close off beginning ifdef's

#endif // _POSIX_SYNCHRONIZED_IO
#endif // _POSIX_MAPPED_FILES
#endif // _POSIX_PRIORITY_SCHEDULING

#include "dr2.h"
#include "globals.h"

//=======================================================
int TriggerInit() {
//=======================================================

    if((Trigger.Message =  (char *)calloc(1, StringSize)) == NULL)
        return(false);

    Trigger.Function              = TriggerThread;
    Trigger.ThreadLive            = true;
    Trigger.Ready                 = false;
    Trigger.FileName              = TriggerFileName;
    return(0);
}

//=======================================================
void * TriggerThread(void *) {
//=======================================================

    FILE * Tf;

    struct timespec nanotime;

    Trigger.Ready = true;

    while(Trigger.ThreadLive) {
        if((Tf = fopen(Trigger.FileName, "r")) == NULL) {
            sprintf(Trigger.Message, "Could not open trigger file %s\n", Trigger.FileName);
            WriteLog(Trigger.Message, ToUser);
            SayToExit();
        } else {
            fclose(Tf);
            nanotime.tv_sec = 0;
            nanotime.tv_nsec = 1000000;
            nanosleep(&nanotime, NULL);
        }
    }

    if(Verbose) {
        sprintf(Trigger.Message, "Exiting TriggerThread\n");
        WriteLog(Trigger.Message, ToUser);
    }

    pthread_exit(NULL);
}


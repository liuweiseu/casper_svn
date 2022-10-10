#include "dr2.h"
#include "globals.h"

//=======================================================
int QuickLookInit() {
//=======================================================

    char local_string[StringSize];

    if(pthread_mutex_init(&QuickLook.Mutex, NULL)) {
        ErrExit("\nCannot initialize quick look mutex ");
    }

    if((QuickLook.Message  =  (char *)calloc(1, StringSize)) == NULL) return(1);
    if((QuickLook.BaseName =  (char *)calloc(1, StringSize)) == NULL) return(1);

    sprintf(local_string, "/ramdisk/quicklook.%s.", TapeName);
    strcpy(QuickLook.BaseName, local_string);
    QuickLook.NumBufs = QuickLookBufs;
    QuickLook.Frequency = QuickLookFreq;
    QuickLook.Interval = QuickLookInterval;

    return(0);

}


//=======================================================
void QuickLookCheck() {
//=======================================================

    static time_t then;
    static time_t now;
    int dsi;

    if(then == 0) {
        then = time(NULL);
        return;
    }

    // no mutex protection, no big deal here, skipping
    // a quicklook is OK
    if(Synth.CurrentSkyFreq == QuickLook.Frequency) {
        now = time(NULL);
        if((now - then) >= QuickLook.Interval) {
            sprintf(QuickLook.Message, "QuickLook : turning flag ON\n");
            WriteLog(QuickLook.Message, ToUser);
            if(pthread_mutex_lock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex lock error \n");
            for(dsi = 0; dsi < NumDatastreams; dsi++) DiskBuf[dsi].QuickLook = true;
            if(pthread_mutex_unlock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex unlock error \n");
            then = now;
        }
    }
}


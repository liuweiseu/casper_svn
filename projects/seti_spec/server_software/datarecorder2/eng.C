#include "dr2.h"
#include "globals.h"

//=======================================================
int EngInit() {
//=======================================================

    int i;

    if((Eng = (Eng_t *)calloc(NumDatastreams, sizeof(Eng_t))) == NULL) {
        return(1);
    }

    for (i=0; i < NumDatastreams; i++) {

        Eng[i].Index = i;

        if((Eng[i].Message =  (char *)calloc(1, StringSize)) == NULL) {
            return(false);
        }

        if((Eng[i].String =  (char *)calloc(1, StringSize)) == NULL) {
            return(false);
        }

        if((Eng[i].Vgc = (int *)calloc(NumChannels, sizeof(int))) == NULL) {
            return(1);
        }

        Eng[i].Function        = EngThread;
        Eng[i].Interval        = EngInterval;
        Eng[i].ThreadLive      = true;
        Eng[i].Ready           = false;
        pthread_mutex_init(&Eng[i].Mutex, NULL);

        sprintf(Eng[i].Message, "ENG[%d] : started\n", i);
        WriteLog(Eng[i].Message, ToUser);
        
    }

    return(0);
}

//=======================================================
void * EngThread(void * dsip) {
//=======================================================

    int i, dsi, vgc[NumChannels];
    char local_string[32];
    char eng_string[StringSize];

    dsi = *(int *)dsip;

    Eng[dsi].Ready = true;

    while (Eng[dsi].ThreadLive) {
        eng_string[0] = '\0';
        for(i=0; i < NumChannels; i++) {
            vgc[i] = (int)read_vgc(MemBuf[dsi].EdtDevice, i);     
            if(!vgc[i]) {
                sprintf(Eng[dsi].Message, "ENG[%d] : Error reading channel %d\n", dsi, i);
                WriteLog(Eng[dsi].Message, ToUser);
            }    
            sprintf(local_string, "%d:%d ", i, vgc[i]);
            strcat(eng_string, local_string);
        }

        if(pthread_mutex_lock(&Eng[dsi].Mutex)) ErrExit("EngThread : mutex lock error \n");
        for(i=0; i < NumChannels; i++) Eng[dsi].Vgc[i] = vgc[i];
        strcpy(Eng[dsi].String, eng_string);
        if(pthread_mutex_unlock(&Eng[dsi].Mutex)) ErrExit("EngThread : mutex unlock error \n");

        sprintf(Eng[dsi].Message, "ENG[%d] values : %s\n", dsi, Eng[dsi].String);
        WriteLog(Eng[dsi].Message, ToUser);

        sleep(Eng[dsi].Interval);
    }

    if(Verbose) {
        sprintf(Eng[dsi].Message, "ENG[%d] : Exiting.\n", dsi);
        WriteLog(Eng[dsi].Message, ToUser);
    }

    pthread_exit(NULL);
}

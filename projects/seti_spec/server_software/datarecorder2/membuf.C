#include "dr2.h"
#include "globals.h"

//======================================================
bool MemBufInit() {
//======================================================

    long i, j, k;
    unsigned char data;

    if((MemBuf =
                (MemBuf_t *)calloc(NumDatastreams, sizeof(MemBuf_t))) == NULL)
        return(1);

    for(i = 0; i < NumDatastreams; i++) {

        MemBuf[i].EdtDevice = NULL;
        MemBuf[i].Function  = MemBufFunc;
        MemBuf[i].ThreadLive = true;
        MemBuf[i].Num   = r_NumBufs;
        //MemBuf[i].Size  = r_BufSize;
        MemBuf[i].DoneCount = 0;
        MemBuf[i].Index = i;
        MemBuf[i].AcquireTime = AcquireTime;
        MemBuf[i].AcquireTimeFudge = AcquireTimeFudge;

        if((MemBuf[i].Message =  (char *)calloc(1, StringSize)) == NULL)
            return(1);

        // have to do this after allocating message area!
        if(Verbose) {
            sprintf(MemBuf[i].Message, "Initializing memory buffer %ld... ", i);
            //WriteLog(MemBuf[i].Message, ToUser);
        }

        MemBuf[i].Ready = false;

        // If we want to generate fake data, we do so here.  We only alloacate
        // 1 fake data store (i == 0) to be used by all data streams.  The fake
        // data store consists of a ring of buffers preloaded with data.  This
        // data are simply a series on incrementing integers.
//#if 0
//        if(UseFakeData && i == 0) {
//            if ((FakeData.Data =
//                        (unsigned char *)calloc(MemBuf[i].Num * r_BufSize , sizeof(unsigned char))) == NULL)
//                return(1);
//            if((FakeData.ReadyPtr =
//                        (unsigned char **)calloc(MemBuf[i].Num, sizeof(unsigned char *))) == NULL)
//                return(1);
//            for(j=0; j < MemBuf[i].Num; j++) FakeData.ReadyPtr[j] = FakeData.Data + j*r_BufSize;
//            for(j=0; j < MemBuf[i].Num; j++) {
//                for(k=0, data=0; k < r_BufSize; k++, data = data == 255 ? 0 : data+1) {
//                    FakeData.Data[j*r_BufSize + k] = data;
//                }
//            }
//        }
//#endif


        if((MemBuf[i].ReadyPtr =
                    (unsigned char **)calloc(MemBuf[i].Num, sizeof(unsigned char *))) == NULL)
            return(1);
        if((MemBuf[i].Size =
            (long *)calloc(MemBuf[i].Num, sizeof(long))) == NULL)
            return(1);
        for(j = 0; j < MemBuf[i].Num; j++)
            MemBuf[i].Size[j]=r_BufSize;

        if((MemBuf[i].ReadyTime =
                    (struct timeb *)calloc(MemBuf[i].Num, sizeof(struct timeb))) == NULL)
            return(1);

        if((MemBuf[i].SynthTime =
                    (struct timeb *)calloc(MemBuf[i].Num, sizeof(struct timeb))) == NULL)
            return(1);

        if((MemBuf[i].SynthFreq =
                    (long *)calloc(MemBuf[i].Num, sizeof(long))) == NULL)
            return(1);

        if((MemBuf[i].SkyFreq =
                    (long *)calloc(MemBuf[i].Num, sizeof(long))) == NULL)
            return(1);

        for(j = 0; j < MemBuf[i].Num; j++)
            MemBuf[i].ReadyTime[j].time = 0;

        // get and initialize ring of mutexes
        if((MemBuf[i].Mutex =
                    (pthread_mutex_t *)calloc(MemBuf[i].Num, sizeof(pthread_mutex_t))) == NULL)
            return(1);
        for(j = 0; j < MemBuf[i].Num; j++)
            if(pthread_mutex_init(&MemBuf[i].Mutex[j], NULL))
                ErrExit("\nCannot initialize ring buffer mutexes ");
        // lock the first one straight away
        if(pthread_mutex_lock(&MemBuf[i].Mutex[0])) ErrExit("MemBufInit : mutex lock error \n");

        if((MemBuf[i].MissingBufs =
                    (long *)calloc(MemBuf[i].Num, sizeof(long))) == NULL)
            return(1);

        if(Verbose) {
            strcat(MemBuf[i].Message, "\tOK\n");
            WriteLog(MemBuf[i].Message, ToUser);
        }

        //if(!UseFakeData) {
        //    InitEdt(i);
        //    EdtCntl(MemBuf[i].EdtDevice, edt_disarm);     // arming is done in main thread
        //    edt_flush_fifo(MemBuf[i].EdtDevice);
        //}
    }

    return(0);
}


//======================================================
void * MemBufFunc(void * dsip) {
//======================================================

    int retval;
    int dsi;      // datastream index
    struct sched_param SchedParam;
    EdtDev * EdtDevice;
    char edt_msg[256];

    dsi = *(int *)dsip;

    //fprintf(stderr, "mem dsi = %d\n", dsi);

    if(Debug) PrintThreadInfo("MemBuf", dsi, MemBuf[dsi].ThreadId, &MemBuf[dsi].ThreadAttr);
    fprintf(stderr, "ptr is %x\n", MemBuf[dsi].EdtDevice);

    if(!UseFakeData) {
        InitEdt(dsi);
        EdtCntl(MemBuf[dsi].EdtDevice, edt_disarm);     // arming is done in main thread
        edt_flush_fifo(MemBuf[dsi].EdtDevice);           
    }

    if(1) {
        sprintf(MemBuf[dsi].Message, "MEM[%d] : is starting ring buffers.\n", dsi);
        WriteLog(MemBuf[dsi].Message, ToUser);
    }
    if(!UseFakeData) retval = edt_start_buffers(MemBuf[dsi].EdtDevice, 0) ; // 0 = free running mode
    if(retval) {
        sprintf(edt_msg, "%s[%d]", "edt_start_buffers", dsi);
        edt_perror(edt_msg);
    }

    if(1) {
        sprintf(MemBuf[dsi].Message, "MEM[%d] : Starting data acquisition.\n", dsi);
        WriteLog(MemBuf[dsi].Message, ToUser);
    }
    GetData(dsi);

    sprintf(MemBuf[dsi].Message, "MEM[%d] : Closing down EDT\n", dsi);
    WriteLog(MemBuf[dsi].Message, ToUser);
    //if(!UseFakeData) edt_stop_buffers(MemBuf[dsi].EdtDevice);

    if(!UseFakeData) CloseEdt(dsi);

    if(1) {
        sprintf(MemBuf[dsi].Message, "MEM[%d] : Exiting.\n", dsi);
        WriteLog(MemBuf[dsi].Message, ToUser);
    }

    pthread_exit(NULL);

}  // end RingThread()


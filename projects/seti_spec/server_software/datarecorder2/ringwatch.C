#include "dr2.h"
#include "globals.h"


//======================================================
int RingWatchInit() {
//======================================================

  int i;

  if((RingWatch.DoneCount = (int *)calloc(NumDatastreams, sizeof(int))) 
    == NULL) {
        return(1);
  } 

  if((RingWatch.NoDataElapsed = (int *)calloc(NumDatastreams, sizeof(int))) 
    == NULL) {
        return(1);
  } 
  for (i = 0; i < NumDatastreams; i++) RingWatch.NoDataElapsed[i] = 0;

  if((RingWatch.Message =  (char *)calloc(1, StringSize)) 
    == NULL) {
        return(1);
  }

  RingWatch.Interval = RingWatchInterval;
  RingWatch.Function = RingWatchThread;
  RingWatch.ThreadLive = true;
  RingWatch.Ready    = false;

  return(0);
  }


//=======================================================
void * RingWatchThread(void *) {
//=======================================================
  long i, OldDoneCount, NoDataStreamCount=0;

  struct timespec nanotime;

  if(Verbose) {
    sprintf(RingWatch.Message, "Starting RingWatchThread\n");
    WriteLog(RingWatch.Message, ToUser);
  }

  RingWatch.Ready = true;

  while(RingWatch.ThreadLive)
    {
    nanotime.tv_sec = RingWatch.Interval;
    nanotime.tv_nsec = 0;
    nanosleep(&nanotime, NULL);
    if (!EdtEnabled) continue;       
    for (i = 0; i < NumDatastreams; i++) {
        OldDoneCount = RingWatch.DoneCount[i];
        RingWatch.DoneCount[i] = edt_done_count(MemBuf[i].EdtDevice);
        if(RingWatch.DoneCount[i] == OldDoneCount) {
            RingWatch.NoDataElapsed[i] += RingWatch.Interval;
            sprintf(RingWatch.Message, "No data from EDT %d in last %ld seconds!  DoneCount is %ld\n",
              i, RingWatch.NoDataElapsed[i], RingWatch.DoneCount[i]);
            WriteLog(RingWatch.Message, ToUser);
            NoDataStreamCount++;
        } else {
            RingWatch.NoDataElapsed[i] = 0;
        }
    }
    if (NoDataStreamCount != 0 && NoDataStreamCount < NumDatastreams) {
        sprintf(RingWatch.Message, "A subset of EDT cards has stopped!\n");
        WriteLog(RingWatch.Message, ToUser);
        ErrExit(NULL);
    } else {
        NoDataStreamCount = 0;
    }
  }

  if(Verbose) {
    sprintf(RingWatch.Message, "Exiting RingWatchThread\n");
    WriteLog(RingWatch.Message, ToUser);
    }

  pthread_exit(NULL);
}


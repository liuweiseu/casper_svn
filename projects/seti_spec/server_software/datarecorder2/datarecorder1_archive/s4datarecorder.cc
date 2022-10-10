/*
Source      : s4datarecoder.cc
Programmer  : Jeff Cobb
Project     : Seti@Home
Compiler    : gcc 2.7
OS          : Solaris 2.6

This program enables a Sun workstation to act as a high speed
data recorder, taking digitized time domain input from an EDT
Sbus card and writing it to a tape device.  The EDT card has a 
direct interface to the serendip4 instrument.  The nominal tape
device is a DLT 7000.

Input via the EDT card consists of 1 bit complex time domain samples.
The EDT card DMAs data to a ring of n-byte buffers.  The recorder
SW must grab the data from each buffer before the EDT card  over-
writes it on it's next time around the ring.  From the ring buffers,
the data are written to one of a ring of disk buffers.  Header
information is also written to the disk buffer.  The combination
of 1 header plus the contents of 1 ring buffer are known as a frame 
After m frames are written to a given disk buffer, that disk buffer 
is written to tape.

For verbose documentation on this program, see it's man page.  For
a discussion of the tape (both frame header and frame data, see
man page for s4tape.

Modification History:
jeffc   1.00    10/25/98        first release
jeffc   1.01    10/30/98        include tape name in console messages
jeffc   1.1     11/3/98         no change - placing under RCS control only
jeffc   1.2     1/20/99         Change to missing ring buffer logic.  
                                - MissingBufs state is now kept for each
                                  ring buffer segment, as is was possible
                                  for a global MissingBufs to be overwritten
                                  by CheckRingBuffers() before a positive
                                  miss count could be written to the header 
                                  of the frame containing the "miss".  
                                - Got rid of RingBuf.Overrun, as this was
                                  never used.  
                                - Took all sem_wait and sem_post calls 
                                  out of CheckRingBuffers().
                                - Moved missing ring buffer alert message to
                                  DiskThread().                             
jeffc   1.3     1/25/99         Another change to missing ring buffer logic.
                                - The "MISSED=" header item is now a boolean.
                                - The ring buffer semaphore is now a single
                                  semaphore, rather than an array of semaphores.
                                  This semaphore is used just to keep us from
                                  reading a ring buffer acquire time while it
                                  is being updated.
                                - We now poll for a time change to know when
                                  a new ring buffer segment is ready and use
                                  the magnitude of the time change to ascertain
                                  whether or not we have missed any buffers.
jeffc	1.4	1/3/00		- fixed Y2K bug in call to localtime()
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <fmtmsg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mtio.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>
#include "libedt.h"
#include "scd.h"

// ---------------------------------------------------------
// macros

typedef unsigned char BOOLEAN;
#define TRUE  1
#define FALSE 0

// ---------------------------------------------------------
// global constants
 
const double Version = 1.4;
 
// defaults
const char * StringPort       = "/dev/term/b";
const char * TapeName         = "/dev/rmt/1hn"; // for DLT, l = 20GB; h = 35GB
const char * DBaseName        = "/mydisks/a/users/serendip/files/diskbuf";
const char * LogFileName      = "/mydisks/a/users/serendip/files/Log";
const char * TriggerFileName  = "/mydisks/a/users/serendip/files/do_s4datarecorder";
const long MessageInterval    = 100;
const long RingBufCheckInterval = 1000;      // 1 millisecond
const char str_delimit  = '\n';
const char str_delay    = 1;                 // 1 second
const long r_NumBufs    = 4;
const long d_NumBufs    = 2;
const long num_r_in_d   = 8;
const long r_BufSize    = 1024*1024;
const long HeaderSize   = 1024;
const long d_BufSize    = (r_BufSize + HeaderSize) * num_r_in_d;
 
// Message routing
const unsigned char ToConsole = 0x01;
const unsigned char ToLogFile = 0x02;
const unsigned char ToUser    = 0x04;
 
const long LogLen    = 256;
const long StringLen = 256;
 
// EDT bits
long Bits     = 2;
BOOLEAN DoMsb = FALSE;

// ---------------------------------------------------------
// types


typedef struct
  {
  long               Num;
  long               Size;
  sem_t              Sem;
  char            ** ReadyPtr;
  struct timeb     * ReadyTime;
  long             * MissingBufs;         
  unsigned long      DoneCount;         
  double             AcquireTime;
  double             AcquireTimeFudge;
  } RingBuf_t;
  
typedef struct
  {
  long         Num;
  long         Size;
  int        * Fd;
  sem_t      * Sem;
  char       * BaseName;
  id_t         PId;
  id_t         LwpId;
  pthread_t    ThreadId;
  pthread_attr_t ThreadAttr;
  int          ThreadStatus;
  int        * ThreadStatusPtr;
  char         ThreadClass[3];
  int          ThreadPri; 
  id_t         ClassID;
  pcinfo_t     PcInfo;
  BOOLEAN      ThreadLive;
  char             Message[LogLen];
  } DiskBuf_t;

typedef struct
  {
  char       * Name;
  int          Fd;
  long         DoneCount;
  id_t         PId;
  id_t         LwpId;
  pthread_t    ThreadId;
  pthread_attr_t ThreadAttr;
  int          ThreadStatus;
  int        * ThreadStatusPtr;
  char         ThreadClass[3];
  int          ThreadPri;
  id_t         ClassID;
  pcinfo_t     PcInfo;
  BOOLEAN      ThreadLive;
  char         Message[LogLen];
  long         MessageInterval;
  } Tape_t;

const long ScopeStringSize = 128;
typedef struct
  { 
  char       * Name;
  int          Fd; 
  sem_t        Sem;
  sem_t        FirstStringSem;
  char         GlobalStr[ScopeStringSize];
  char         Delimiter;
  long         Delay;
  id_t         PId;
  id_t         LwpId;
  pthread_t    ThreadId;
  pthread_attr_t ThreadAttr;
  int          ThreadStatus;
  int        * ThreadStatusPtr;
  char         ThreadClass[3]; 
  int          ThreadPri; 
  id_t         ClassID;
  pcinfo_t     PcInfo;
  BOOLEAN      ThreadLive; 
  char         Message[LogLen];
  } TelStr_t;

typedef struct
  {
  char       * Name;
  int          Fd;
  id_t         PId;
  id_t         LwpId;
  pthread_t    ThreadId;
  pthread_attr_t ThreadAttr;
  int          ThreadStatus;
  int        * ThreadStatusPtr;
  char         ThreadClass[3];
  int          ThreadPri;
  id_t         ClassID;
  pcinfo_t     PcInfo;
  BOOLEAN      ThreadLive;
  char         Message[LogLen];
  } Trigger_t;

typedef struct
  {
  int          Interval;
  EdtDev     * EdtDevice;
  id_t         PId;
  id_t         LwpId;
  pthread_t    ThreadId;
  pthread_attr_t ThreadAttr;
  int          ThreadStatus;
  int        * ThreadStatusPtr;
  char         ThreadClass[3];
  int          ThreadPri;
  id_t         ClassID;
  pcinfo_t     PcInfo;
  BOOLEAN      ThreadLive;
  char         Message[LogLen];
  } RingWatch_t;

typedef struct
  {
  id_t             PId;
  id_t             LwpId;
  pthread_t        ThreadId;
  pthread_attr_t   ThreadAttr;
  int              ThreadStatus;
  int            * ThreadStatusPtr;
  char             ThreadClass[3];
  int              ThreadPri;
  id_t             ClassID;
  pcinfo_t         PcInfo;
  BOOLEAN          ProcLive; 
  char             Message[LogLen];
  } MainProc_t;

typedef struct
  {
  sem_t            Sem;
  char           * Name;
  long             RecordType;
  char           * Receiver;
  unsigned long    SequenceNum;
  double           CenterFreq;
  double           SampleRate;
  } FrameHeader_t;

typedef struct
  {
  sem_t            Sem;
  char           * LogFileName; 
  FILE           * ConsoleFp;
  FILE           * LogFileFp;
  FILE           * UserFileFp;
  }  Log_t;

// ---------------------------------------------------------
// global variables

// Control blocks
MainProc_t     MainProc;
DiskBuf_t      DiskBuf;
RingBuf_t      RingBuf;
Tape_t         Tape;
TelStr_t       TelStr;
Trigger_t      Trigger;
FrameHeader_t  FrameHeader;
Log_t          Log;
RingWatch_t    RingWatch;

char * TapeFileName   = "20nov98aa";
char * Receiver       = "ao1420";
long RecordType       = 1;
double CenterFreq     = 1420.0;
double SampleRate     = 2.5;
long RingWatchInterval = 5;

BOOLEAN Verbose = FALSE;
BOOLEAN NoWait  = FALSE;

// items for fake data generator
char    * fake_buffers; 
char    * fake_buffer_ptr[r_NumBufs];
long      fake_buffer_index;
double    MyTime, TimeMark;
struct    timeb mytimeb;

// ---------------------------------------------------------
// prototypes

BOOLEAN IsTrigger();
long    CheckRingBuffers(EdtDev * EdtDevice, long MissingBufs);
void    SayToExit(long i);
BOOLEAN RingBufInit();
void    RingBufShutdown();
BOOLEAN RingWatchInit();
BOOLEAN DiskBufInit();
void    DiskBufShutdown();
BOOLEAN TapeInit();
void    TapeShutdown();
BOOLEAN TelStrInit();
void    TelStrShutdown();
BOOLEAN FrameHeaderInit();
BOOLEAN LogInit();
void    LogShutdown();
BOOLEAN TriggerInit();
EdtDev * ConfigureEdtDevice(long Bits, BOOLEAN DoMsb);
void PrintSchedInfo(char    * Name, 
                    id_t      MyLwpId,
                    id_t      MyPid,
                    id_t      MyThreadId,
                    id_t      MyClassID,
                    pcinfo_t  PcInfo,
                    char    * Message);
void SemSwitch(char * Caller, 
               long * AtSem, 
               sem_t Sem[], 
               long NumSems, 
               long SemI,
               char * Message);
void fake_configure_ring_buffers(long r_BufSize, long r_NumBufs);
char * fake_wait_for_buffer(long NumFakes, double DataRate); 
void GetOps(int       argc,
            char   ** argv,
            BOOLEAN * Verbose,
            BOOLEAN * FakeIt,
            double  * DataRate,
            char   ** TapeName,
            char   ** DiskName,
            char   ** SportName);
void ErrExit(char * Msg);
void * StringThread(void *);
void * DiskThread(void *);
void * TapeThread(void *);
void * TriggerThread(void *);
void * RingWatchThread(void *);
void WriteHeader(int fd, struct timeb milliclock, long MissingBufs, long DataSeq);
void LwpToRt(id_t        MyLwpId,
             long        MyPriority,
             pcparms_t   PcParms,
             rtparms_t * RtParms,
             short       MaxRtPri,
             char      * Message);
int GetClassID(long IdType, id_t Pid);
id_t GetRtInfo(short * MaxPri);
void WriteLog(char * Msg, unsigned char To);

// Turn off name mangling for the EDT lib calls
extern "C" {
   int edt_configure_ring_buffers(EdtDev *edt_p, 
                                  int bufsize, 
                                  int numbufs, 
                                  int write_flag, 
                                  void **bufarray);
   int edt_start_buffers(EdtDev *edt_p, int count);
   int edt_stop_buffers(EdtDev *edt_p);
   void * edt_wait_for_buffers(EdtDev *edt_p, int count);
   EdtDev * edt_open(char *device_name, int unit);
   BOOLEAN edt_ring_buffer_overrun(EdtDev *edt_p);
   int edt_done_count(EdtDev *edt_p);
   int edt_abort_current_dma(EdtDev *edt_p);
   int edt_close(EdtDev *edt_p);
           }


//=======================================================
int main(int argc, char ** argv)
//=======================================================
  {
  long r_AtBuf               = 0;
  long i, j;
  BOOLEAN FakeIt             = FALSE;
  BOOLEAN LimitedRun         = FALSE;
  long LimitedRunNum         = 40;
  long NumFakes              = 40;
  //double DataRate          = 1.6;    // seconds to gen 1MB of data
  double DataRate            = 1.0;    // seconds to gen 1MB of data

  pcparms_t   PcParms;
  rtparms_t * RtParms; 
  short       MaxRtPri;

  long SemI;

  char Message[LogLen];

  struct EdtDev * EdtDevice;


  // -----------------------------------------------------
  // Initialize

  if(!LogInit())
    ErrExit("Could not init log\n");

  if(!RingBufInit())
    ErrExit("Could not init disk buffers\n");

  if(!DiskBufInit())
    ErrExit("Could not init disk buffers\n");

  if(!TapeInit())
    ErrExit("Could not init tape\n");

  if(!TelStrInit())
    ErrExit("Could not init telescope string\n");

  if(!FrameHeaderInit())
    ErrExit("Could not init frame header\n");

  if(!TriggerInit())
    ErrExit("Could not init trigger check\n");

  if(!RingWatchInit())
    ErrExit("Could not init ring watch\n");

  GetOps(argc, argv, 
         &Verbose, 
         &FakeIt, 
         &DataRate, 
         &Tape.Name, 
         &DiskBuf.BaseName, 
         &TelStr.Name);

  if(!IsTrigger())                 // exit now if trigger file not present
    {
    sprintf(MainProc.Message, "No trigger file... exiting.\n");
    WriteLog(MainProc.Message, ToLogFile+ToConsole);
    exit(1);
    }

  sprintf(MainProc.Message, "s4datarecorder v%5.2lf starting on %s\n", 
          Version, FrameHeader.Name); 
  WriteLog(MainProc.Message, ToLogFile+ToConsole);


  if(FakeIt)
    {
    DataRate = 1.0 / (DataRate / 8.0);    // Mb to MB and then to secs/MB
    sprintf(MainProc.Message, "Fake data rate : 1 MB in  %lf seconds.\n", 
            DataRate);
    WriteLog(MainProc.Message, ToUser);
    }

  // ----------------------------------------------------- 
  // Spawn threads.
  // Note: saying PTHREAD_SCOPE_SYSTEM makes these *bound* threads.

  TelStr.ThreadLive = TRUE;
  pthread_attr_init(&TelStr.ThreadAttr);
  if(pthread_attr_setscope(&TelStr.ThreadAttr, PTHREAD_SCOPE_SYSTEM) != 0)
    ErrExit("Could not set bound attribute for string thread.\n");
  if((pthread_create(&TelStr.ThreadId, 
                     &TelStr.ThreadAttr, StringThread, NULL)) != 0)
    ErrExit("Could not create telescope string thread\n");
  else
    if(Verbose)
      {
      sprintf(MainProc.Message, "String thread successfully created...\n\n");
      WriteLog(MainProc.Message, ToUser);
      }

  DiskBuf.ThreadLive = TRUE;
  pthread_attr_init(&DiskBuf.ThreadAttr);
  if(pthread_attr_setscope(&DiskBuf.ThreadAttr, PTHREAD_SCOPE_SYSTEM) != 0)
    ErrExit("Could not set bound attribute for string thread.\n");
  if((pthread_create(&DiskBuf.ThreadId, 
                     &DiskBuf.ThreadAttr, DiskThread, NULL)) != 0)
    ErrExit("Could not create disk thread\n");
  else
    if(Verbose)
      {
      sprintf(MainProc.Message, "DiskBuf thread successfully created...\n\n");
      WriteLog(MainProc.Message, ToUser);
      }

  Tape.ThreadLive = TRUE;
  pthread_attr_init(&Tape.ThreadAttr);
  if(pthread_attr_setscope(&Tape.ThreadAttr, PTHREAD_SCOPE_SYSTEM) != 0)
    ErrExit("Could not set bound attribute for string thread.\n");
  if((pthread_create(&Tape.ThreadId, 
                     &Tape.ThreadAttr, TapeThread, NULL)) != 0)
    ErrExit("Could not create tape thread\n");
  else
    if(Verbose)
      {
      sprintf(MainProc.Message, "Tape thread successfully created...\n\n");
      WriteLog(MainProc.Message, ToUser);
      }

  Trigger.ThreadLive = TRUE;
  pthread_attr_init(&Trigger.ThreadAttr);
  if(pthread_attr_setscope(&Trigger.ThreadAttr, PTHREAD_SCOPE_SYSTEM) != 0)
    ErrExit("Could not set bound attribute for trigger thread.\n");
  if((pthread_create(&Trigger.ThreadId,
                     &Trigger.ThreadAttr, TriggerThread, NULL)) != 0)
    ErrExit("Could not create trigger thread\n");
  else   
    if(Verbose)  
      {  
      sprintf(MainProc.Message, "Trigger thread successfully created...\n\n");
      WriteLog(MainProc.Message, ToUser);
      }

  // ----------------------------------------------------- 
  // Set up MainProc
  // This need to be done after thread spawning, otherwise
  // spawned threads will inherit MainProc's scheduling
  // class.

  MainProc.PId      =  getpid();
  MainProc.LwpId    = _lwp_self();
  MainProc.ThreadId =  thr_self();
  LwpToRt(MainProc.LwpId, 1, PcParms, RtParms, MaxRtPri, MainProc.Message);
  MainProc.ProcLive = TRUE; 
/*
  if(Verbose)
    {
    PrintSchedInfo("MainProc",
                   MainProc.LwpId,
                   MainProc.PId,
                   MainProc.ThreadId,
                   MainProc.ClassID,
                   MainProc.PcInfo,
                   MainProc.Message);
    PrintSchedInfo("DiskThread",
                   DiskBuf.LwpId,
                   DiskBuf.PId,
                   DiskBuf.ThreadId,
                   DiskBuf.ClassID,
                   DiskBuf.PcInfo,
                   MainProc.Message);
    PrintSchedInfo("TapeThread",
                   Tape.LwpId,
                   Tape.PId,
                   Tape.ThreadId,
                   Tape.ClassID,
                   Tape.PcInfo,
                   MainProc.Message);
    PrintSchedInfo("StringThread",
                   TelStr.LwpId,
                   TelStr.PId,
                   TelStr.ThreadId,
                   TelStr.ClassID,
                   TelStr.PcInfo,
                   MainProc.Message);
    PrintSchedInfo("TriggerThread",
                   Trigger.LwpId,
                   Trigger.PId,
                   Trigger.ThreadId,
                   Trigger.ClassID,
                   Trigger.PcInfo,
                   MainProc.Message);
    PrintSchedInfo("RingWatchThread",
                   RingWatch.LwpId,
                   RingWatch.PId,
                   RingWatch.ThreadId,
                   RingWatch.ClassID,
                   RingWatch.PcInfo,
                   MainProc.Message);
    }
*/

  // -----------------------------------------------------
  // Wait for arrival of first scope string unless told not to
     
  if(NoWait)
    {
    sprintf(MainProc.Message, "Not waiting for first telescope string!\n",
            FrameHeader.Name);
    WriteLog(MainProc.Message, ToLogFile+ToConsole);
    }
  else
    {
    sprintf(MainProc.Message, "Waiting for first telescope string...\n",
            FrameHeader.Name);
    WriteLog(MainProc.Message, ToLogFile+ToConsole);
    sem_wait(&TelStr.FirstStringSem);
    }

  // ----------------------------------------------------- 
  // Configure data source - either EDT or fake
  
  if(FakeIt)
    {
    fake_configure_ring_buffers(RingBuf.Size, RingBuf.Num);
    MyTime = mytimeb.time;
    MyTime += mytimeb.millitm / 1000.0;
    }
  else
    {
    EdtDevice = ConfigureEdtDevice(Bits, DoMsb);
    if (edt_configure_ring_buffers(EdtDevice, 
                                   RingBuf.Size, 
                                   RingBuf.Num, 
                                   EDT_READ, 
                                   NULL) == -1)
      ErrExit("Could not configure EDT ring buffers");
      edt_start_buffers(EdtDevice, 0) ; /* start the xfers in free running mode */

    RingWatch.EdtDevice = EdtDevice;
    RingWatch.ThreadLive = TRUE;
    pthread_attr_init(&RingWatch.ThreadAttr);
    if(pthread_attr_setscope(&RingWatch.ThreadAttr, PTHREAD_SCOPE_SYSTEM) != 0)
    ErrExit("Could not set bound attribute for ring watch thread.\n");
    if((pthread_create(&RingWatch.ThreadId,
                       &RingWatch.ThreadAttr, RingWatchThread, NULL)) != 0)
      ErrExit("Could not create ring watch thread\n");
    else
      if(Verbose)
        {  
        sprintf(MainProc.Message, 
                "Ring watch thread successfully created...\n\n");
        WriteLog(MainProc.Message, ToUser);
        }  
    }

  WriteLog("data source configured.......\n", ToLogFile+ToConsole);

  // ----------------------------------------------------- 
  // Main loop.  Wait for ring buffer segments.  When we get 
  // one, post it's semaphore.  Disk and tape threads will
  // handle the rest.

  WriteLog("taking data.......\n", ToLogFile+ToConsole);
  
  if(FakeIt)
    {
    while((RingBuf.ReadyPtr[r_AtBuf] = 
           fake_wait_for_buffer(NumFakes, DataRate)) != NULL)
      {
      sem_wait(&RingBuf.Sem);
      ftime(&RingBuf.ReadyTime[r_AtBuf]); 
      sem_post(&RingBuf.Sem);
      }
    }
  else
    {
    while(((RingBuf.ReadyPtr[r_AtBuf] = 
           (char *)edt_wait_for_buffers(EdtDevice, 1)) != NULL)  &&
          (!LimitedRun || LimitedRunNum-- > 0)                   &&
          (MainProc.ProcLive == TRUE))
      {
      // Get time we were notified of this ring segment being ready.
      sem_wait(&RingBuf.Sem);
      ftime(&RingBuf.ReadyTime[r_AtBuf]); 
      sem_post(&RingBuf.Sem);
      RingBuf.DoneCount = edt_done_count(EdtDevice);
      r_AtBuf = r_AtBuf == RingBuf.Num - 1 ? 0 : r_AtBuf + 1;
      }
    }
      
  // ----------------------------------------------------- 
  // We are done... bring threads to a close and clean up

  edt_close(EdtDevice);
  RingBufShutdown();

  DiskBuf.ThreadLive = FALSE;
  pthread_join(DiskBuf.ThreadId, (void**)&DiskBuf.ThreadStatusPtr);
  DiskBufShutdown();

  Tape.ThreadLive = FALSE;
  pthread_join(Tape.ThreadId, (void**)&Tape.ThreadStatusPtr);
  TapeShutdown();

  TelStr.ThreadLive = FALSE;
  pthread_join(TelStr.ThreadId, (void**)&TelStr.ThreadStatusPtr);
  TelStrShutdown();

  RingWatch.ThreadLive = FALSE;
  pthread_join(RingWatch.ThreadId, (void**)&RingWatch.ThreadStatusPtr);
  TelStrShutdown();

  sprintf(MainProc.Message, "Completed %s\n\n", FrameHeader.Name); 
  WriteLog(MainProc.Message, ToLogFile+ToConsole);

  LogShutdown();

  exit(0);
  }

//======================================================
BOOLEAN RingBufInit()
//======================================================
  {
  long i;

  RingBuf.Num   = r_NumBufs;
  RingBuf.Size  = r_BufSize;
 
  if((RingBuf.ReadyPtr = (char **)calloc(RingBuf.Num, sizeof(char *))) == NULL)
    return(FALSE);
 
  if((RingBuf.ReadyTime = 
      (struct timeb *)calloc(RingBuf.Num, sizeof(struct timeb))) == NULL)
    return(FALSE);

  for(i = 0; i < RingBuf.Num; i++)
    RingBuf.ReadyTime[i].time = 0;
 
  if((RingBuf.MissingBufs = 
      (long *)calloc(RingBuf.Num, sizeof(long))) == NULL)
    return(FALSE);
 
  if(sem_init(&RingBuf.Sem, 0, 1) != 0)
    ErrExit("Cannot initialize ring buffer semaphore\n");

  return(TRUE);
  }

//======================================================
void RingBufShutdown()
//======================================================
  {
  }

//======================================================
BOOLEAN RingWatchInit()
//======================================================
  {
  RingWatch.Interval = RingWatchInterval;

  return(TRUE);
  }

//======================================================
BOOLEAN DiskBufInit()
//======================================================
  {
  long i;
  char NameExt[32];
  char DiskBufName1[256];

  DiskBuf.Num       = d_NumBufs;
  DiskBuf.Size      = d_BufSize; 
  DiskBuf.BaseName  = DBaseName;
  DiskBuf.ThreadStatusPtr = &DiskBuf.ThreadStatus;

  if((DiskBuf.Fd  = (int *)calloc(DiskBuf.Num, sizeof(int))) == NULL)
    return(FALSE);

  if((DiskBuf.Sem = (sem_t *)calloc(DiskBuf.Num, sizeof(sem_t))) == NULL)
    return(FALSE);

  for(i = 0; i < DiskBuf.Num; i++)
    if(sem_init(&DiskBuf.Sem[i], 0, i == 0 ? 0 : 1) != 0)
      ErrExit("Cannot initialize disk buffer semaphores\n");

  for(i = 0; i < DiskBuf.Num; i++)
    {
    strcpy(DiskBufName1, DiskBuf.BaseName);
    sprintf(NameExt, "_%d", i);
    strcat(DiskBufName1, NameExt);
    if((DiskBuf.Fd[i] = open(DiskBufName1, O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
      ErrExit("DiskThread : Cannot open disk buffers\n");
    }

  return(TRUE);
  }
  
//======================================================
void DiskBufShutdown()
//======================================================
  {
  long i;

  for(i = 0; i < DiskBuf.Num; i++)
    close(DiskBuf.Fd[i]);
  }

//======================================================
BOOLEAN TapeInit()
//======================================================
  {
  Tape.Name = TapeName;

  if((Tape.Fd = open(Tape.Name, O_RDWR|O_CREAT|O_TRUNC, 0666)) == -1)
    ErrExit("Cannot open tape device\n");

  Tape.MessageInterval - MessageInterval;

  Tape.ThreadStatusPtr = &Tape.ThreadStatus;

  Tape.DoneCount = 0;

  return(TRUE);
  }

//======================================================
void TapeShutdown()
//======================================================
  {
  close(Tape.Fd);
  }

//======================================================
BOOLEAN TelStrInit()
//======================================================
  {
  TelStr.Name = StringPort;

  if(sem_init(&TelStr.Sem, 0, 1) != 0)
      ErrExit("Cannot initialize scope string semaphore\n");

  if(sem_init(&TelStr.FirstStringSem, 0, 0) != 0)
      ErrExit("Cannot initialize first-string semaphore\n");

  if((TelStr.Fd = open(TelStr.Name, O_RDWR | O_NOCTTY)) < 0)
    ErrExit("Open failed for telescope string port");

  TelStr.ThreadStatusPtr = &TelStr.ThreadStatus;

  TelStr.Delimiter = str_delimit;
  TelStr.Delay     = str_delay;

  return(TRUE);
  }
  
//======================================================
void TelStrShutdown()
//======================================================
  {
  close(TelStr.Fd);
  }

//======================================================
BOOLEAN FrameHeaderInit()
//======================================================
  {
  if(sem_init(&FrameHeader.Sem, 0, 1) != 0)
      ErrExit("Cannot initialize frame header semaphore\n");

  FrameHeader.Name        = NULL;
  FrameHeader.RecordType  = RecordType;
  FrameHeader.Receiver    = NULL;
  FrameHeader.CenterFreq  = 0;
  FrameHeader.SampleRate  = 0;

  return(TRUE);
  }

//======================================================
BOOLEAN LogInit()
//======================================================
  {
  if(sem_init(&Log.Sem, 0, 1) != 0)
      ErrExit("Cannot initialize log semaphore\n");

  Log.LogFileName = LogFileName;

  if((Log.LogFileFp = fopen(LogFileName, "a")) == NULL)
    ErrExit("Open failed for log file");

  setbuf(Log.LogFileFp, NULL);   // unbuffered log output

  Log.UserFileFp = stderr;

  return(TRUE);
  }

//======================================================
BOOLEAN TriggerInit()
//======================================================
  {
  Trigger.Name = TriggerFileName; 
  }

//======================================================
void LogShutdown()
//======================================================
  {
  }


//======================================================
EdtDev * ConfigureEdtDevice(long Bits, BOOLEAN DoMsb)
//======================================================
  {
  EdtDev * EdtDevice; 
  unsigned char funct;
  unsigned short iodir;
  unsigned char SsdMsb = 0x4;
  int unit = 0;

  if ((EdtDevice = edt_open("scd", unit)) == NULL)
    ErrExit("Could not open EDT device") ;

  ioctl(EdtDevice->fd, SCDG_FUNCT, &funct);
  funct &= 0xf8;
  if (DoMsb)
    funct |=  SsdMsb;
  else
    funct &= ~SsdMsb;
  switch (Bits)
    {   
    case 2:
      funct |= 0x1;
      break;
    case 4:
      funct |= 0x3;
      break;
    default:
      break;
    }
  ioctl(EdtDevice->fd, SCDS_FUNCT, &funct);

  iodir = 0x6f0f;
  ioctl(EdtDevice->fd, SCDS_DIR_CTRL, &iodir);
  ioctl(EdtDevice->fd, SCDS_DEF_READ_DIR, &iodir);

  return(EdtDevice);
  }

//======================================================
void fake_configure_ring_buffers(long r_BufSize, long r_NumBufs)
//======================================================
  {
  long i;
  fake_buffers = (char *)calloc(r_NumBufs, r_BufSize);
  for(i = 0; i  < r_NumBufs; i++)
    {
    fake_buffer_ptr[i] = fake_buffers + r_BufSize * i;
    memset(fake_buffer_ptr[i], (int)i, r_BufSize);   // make easy to look at
    }
  fake_buffer_index = 0;
  }

//======================================================
char * fake_wait_for_buffer(long NumFakes, double DataRate)
//======================================================
  {
  char * rtn; 
  double ThisTime = 0.0;
  static long FakeCnt = 0;
  struct timespec nanotime;

  while(ThisTime < MyTime + DataRate)
    {
    nanotime.tv_sec  = 0;
    nanotime.tv_nsec = 1000000;
    nanosleep(&nanotime, NULL);
    ftime(&mytimeb);
    ThisTime = mytimeb.time;
    ThisTime += mytimeb.millitm / 1000.0;
    }
  sprintf(MainProc.Message, "%lf\n", ThisTime - MyTime);
  WriteLog(MainProc.Message, ToUser);
  MyTime = ThisTime;
  rtn = fake_buffer_ptr[fake_buffer_index];
  fake_buffer_index = fake_buffer_index == r_NumBufs-1 ? 
                      0 : fake_buffer_index + 1;
  return(++FakeCnt == NumFakes ? NULL : rtn);
  }
 
//======================================================
void SemSwitch(char * Caller, 
               long * AtSem, 
               sem_t Sem[], 
               long NumSems,
               long i,
               char * Message)
//======================================================
  {
  if(!MainProc.ProcLive)
    return;

  if(Verbose)
    {
    sprintf(Message, "==== semaphores (%20s) prepost  ===> ", Caller);
    WriteLog(Message, ToUser);
    for(i = 0; i < NumSems; i++)
      {
      sprintf(Message,"%d  ", Sem[i].sem_count);
      WriteLog(Message, ToUser);
      }
    sprintf(Message, " : on %ld\n", *AtSem);
    WriteLog(Message, ToUser);
    }
 
  sem_post(&Sem[*AtSem]);
 
  *AtSem = *AtSem == NumSems - 1 ? 0 : *AtSem + 1;
 
  sem_wait(&Sem[*AtSem]);

  if(Verbose)
    {
    sprintf(Message, "==== semaphores (%20s) postwait ===> ", Caller);
    WriteLog(Message, ToUser);
    for(i = 0; i < NumSems; i++)
      {
      sprintf(Message,"%d  ", Sem[i].sem_count);
      WriteLog(Message, ToUser);
      }
    sprintf(Message, " : on %ld\n", *AtSem);
    WriteLog(Message, ToUser);
    }
  }

//======================================================
void * DiskThread(void *)
//======================================================
  {
  long m_in_d     = 0;
  long r_AtBuf    = 0;
  long d_AtBuf    = 0;

  struct timespec nanotime;

  timeb ThisReadyTime, LastReadyTime, ThisReadyTimePostWrite;
  double ThisReadySec, LastReadySec;
  off_t OffsetSave;

  BOOLEAN MissingBufs = FALSE;

  long SemI;

  long i;

  long DataSeq = 0;

  if(Verbose)
    {
    sprintf(DiskBuf.Message, "Entering DiskThread : %ld\n", DiskBuf.Fd[0]);
    WriteLog(DiskBuf.Message, ToUser);
    }

  DiskBuf.PId   =  getpid();
  DiskBuf.LwpId = _lwp_self();

  if(Verbose)
    PrintSchedInfo("DiskThread", 
                   DiskBuf.LwpId,
                   DiskBuf.PId, 
                   DiskBuf.ThreadId,
                   DiskBuf.ClassID, 
                   DiskBuf.PcInfo,
                   DiskBuf.Message);

  ThisReadyTime.time = 0;
  LastReadyTime.time = 0;


  // main disk thread loop
  while(DiskBuf.ThreadLive)
    {
    // poll for new data
    for(sem_wait(&RingBuf.Sem), ThisReadyTime = RingBuf.ReadyTime[r_AtBuf];
        (ThisReadyTime.time < LastReadyTime.time || 
         ThisReadyTime.time == 0)                && 
        DiskBuf.ThreadLive;
        sem_wait(&RingBuf.Sem), ThisReadyTime = RingBuf.ReadyTime[r_AtBuf])
      {
      sem_post(&RingBuf.Sem);
      nanotime.tv_sec  = 0;
      nanotime.tv_nsec = 1000000;
      nanosleep(&nanotime, NULL);
      }
    sem_post(&RingBuf.Sem);
    if(!DiskBuf.ThreadLive)
      continue;
    DataSeq = RingBuf.DoneCount - 1;

    // check for ring buf overrun - we ask if the current ready time
    // indicates that we have gone all the way around the ring since
    // the last ready time.
    ThisReadySec = ThisReadyTime.time + double(ThisReadyTime.millitm/1000.0);
    if(ThisReadySec > LastReadySec                            + 
                      RingBuf.AcquireTime * (RingBuf.Num + 1) - 
                      RingBuf.AcquireTimeFudge                &&
       LastReadyTime.time != 0)
      {  
      sprintf(DiskBuf.Message, "RING BUFFER OVERRUN !!\n");
      WriteLog(DiskBuf.Message, ToConsole+ToLogFile+ToUser);
      MissingBufs = TRUE; 
      }
 
    if(Verbose)
      {
      sprintf(DiskBuf.Message, 
              "In DiskThread : r_AtBuf is %ld  RingBuf.ReadyPtr is %p\n", 
              r_AtBuf, RingBuf.ReadyPtr[r_AtBuf]);
      WriteLog(DiskBuf.Message, ToUser);
    }

    // write the header and copy data to the disk buffer
    // ... but first save our place in case the copy is bad
    OffsetSave = lseek(DiskBuf.Fd[d_AtBuf], 0L, SEEK_CUR);
    WriteHeader(DiskBuf.Fd[d_AtBuf], 
                RingBuf.ReadyTime[r_AtBuf],
                MissingBufs,
                DataSeq);
    write(DiskBuf.Fd[d_AtBuf], RingBuf.ReadyPtr[r_AtBuf], RingBuf.Size);

    // see if we overwrote *while* copying to disk buffer
    // no need to protect with semaphore here!
    ThisReadyTimePostWrite = RingBuf.ReadyTime[r_AtBuf];
    if(ThisReadyTimePostWrite.time != ThisReadyTime.time)
      {
      lseek(DiskBuf.Fd[d_AtBuf], OffsetSave, SEEK_SET);
      m_in_d--;
      sprintf(DiskBuf.Message, "RING BUFFER OVERRUN DURING BUFFER COPY !!\n");
      WriteLog(DiskBuf.Message, ToConsole+ToLogFile+ToUser);
      MissingBufs = TRUE;     // will show up in next header
      }
    else
      MissingBufs = FALSE;    // successful write - re-init   

    // prep for next ring buf
    LastReadyTime.time    = ThisReadyTimePostWrite.time;
    LastReadyTime.millitm = ThisReadyTimePostWrite.millitm;
    LastReadySec  = LastReadyTime.time + double(LastReadyTime.millitm/1000.0);
    r_AtBuf = r_AtBuf == RingBuf.Num - 1 ? 0 : r_AtBuf + 1;

    //fsync(DiskBuf[d_AtBuf]);

    // Once we have filled this disk buffer, flip to other.
    if(++m_in_d == num_r_in_d) 
      {
      m_in_d = 0;
      SemSwitch("DiskThread", 
                &d_AtBuf, DiskBuf.Sem, DiskBuf.Num, SemI, DiskBuf.Message);
      lseek(DiskBuf.Fd[d_AtBuf], 0, SEEK_SET);   // init to disk buf start
      }

    }  

  if(Verbose)
    {
    sprintf(DiskBuf.Message, "Exiting DiskThread\n");
    WriteLog(DiskBuf.Message, ToUser);
    }

  pthread_exit(NULL);
  }

//======================================================
void * TapeThread(void *)
//======================================================
  {
  long i;
  long NumWritten;
  long d_AtBuf = 0;
  caddr_t DiskBufBase[d_NumBufs];

  long SemI, stack_i;

  if(Verbose)
    {
    sprintf(Tape.Message, "Entering TapeThread\n");
    WriteLog(Tape.Message, ToUser);
    }

  Tape.PId   =  getpid();
  Tape.LwpId = _lwp_self();

  if(Verbose)
    PrintSchedInfo("TapeThread", 
                   Tape.LwpId,
                   Tape.PId, 
                   Tape.ThreadId,
                   Tape.ClassID, 
                   Tape.PcInfo,
                   Tape.Message);

  // memory map the disk buffers
  for(i = 0; i < DiskBuf.Num; i++)
    {
    if((DiskBufBase[i] = mmap(0, DiskBuf.Size, PROT_READ, MAP_SHARED, 
                          DiskBuf.Fd[i], 0)) == MAP_FAILED)
      ErrExit("Memory mapping failed for disk buffers\n");
    if(Verbose)
      {
      sprintf(Tape.Message, 
              "Mmaped disk buf address  %ld : %p  size : %ld FD : %d\n", 
              i, DiskBufBase[i], DiskBuf.Size, DiskBuf.Fd[i]);
      WriteLog(Tape.Message, ToUser);
      }
    }

  sem_wait(&DiskBuf.Sem[d_AtBuf]);    // initial wait for 1st disk buffer
  while(Tape.ThreadLive)  
    {  
    if(Verbose)
      {
      sprintf(Tape.Message, "In TapeThread\n");
      WriteLog(Tape.Message, ToUser);
      }

    if((NumWritten = write(Tape.Fd, DiskBufBase[d_AtBuf], DiskBuf.Size)) == -1)
      ErrExit("Tape write failed\n");   
    else
      {
      Tape.DoneCount++;

      sprintf(Tape.Message, 
              "%ld records written to tape %s.  Current frame seq # is %ld\n", 
              Tape.DoneCount, FrameHeader.Name, FrameHeader.SequenceNum);
      WriteLog(Tape.Message, ToConsole);
      if(Tape.DoneCount % Tape.MessageInterval == 0)
        WriteLog(Tape.Message, ToLogFile);

      if(Verbose)
        {
        sprintf(Tape.Message, "Bytes written to tape : %ld\n", NumWritten);
        WriteLog(Tape.Message, ToUser);
        }

      if(Verbose)
        {
        sprintf(Tape.Message, "+++++++++++++++++++++++++++ %ld\n", 
                FrameHeader.SequenceNum);
        WriteLog(Tape.Message, ToUser);
        }

      if(NumWritten < DiskBuf.Size)     // EOT?
        SayToExit(stack_i);
      }
    
    SemSwitch("TapeThread", 
              &d_AtBuf, DiskBuf.Sem, DiskBuf.Num, SemI, DiskBuf.Message);
    }    

  // un-memory map and close the disk buffers
  for(i = 0; i < DiskBuf.Num; i++)
    munmap(DiskBufBase[i], DiskBuf.Size);

  if(Verbose)
    {
    sprintf(Tape.Message, "Exiting TapeThread\n");
    WriteLog(Tape.Message, ToUser); 
    }

  pthread_exit(NULL);
  }  

//=======================================================
void WriteHeader(int fd, struct timeb milliclock, long MissingBufs, long DataSeq)
//=======================================================
  {
  char HeaderBlock[HeaderSize];

  long i = 0;
  long j;
  long item;
  time_t clock;
  tm * Ast;

  struct timespec nanotime;

  memset(HeaderBlock, '\0', HeaderSize);

  for(item = 0; item < 13; item++, i += strlen(&HeaderBlock[i]) + 1)
    switch(item)
      {  

      // Tape name
      case 0 : sprintf(&HeaderBlock[i], "%s %s", "NAME= ", FrameHeader.Name);
               break;

      // Record Type
      case 1 : sprintf(&HeaderBlock[i], "%s %ld", "RCDTYPE= ", 
                       FrameHeader.RecordType);
               break;

      // Frame Sequence Number
      case 2 : sprintf(&HeaderBlock[i], "%s %lu", 
                       "FRAMESEQ= ", FrameHeader.SequenceNum++);
               break;

      // Real Data Sequence Number
      case 3 : sem_wait(&FrameHeader.Sem);
               sprintf(&HeaderBlock[i], "%s %lu", 
                       "DATASEQ= ", DataSeq);
               sem_post(&FrameHeader.Sem);
               break;

      // Missed Buffers
      case 4 : sem_wait(&FrameHeader.Sem);
               sprintf(&HeaderBlock[i], "%s %lu", 
                       "MISSED= ", MissingBufs);
               sem_post(&FrameHeader.Sem);
               break;

      // Unix time
      case 5 : Ast   = localtime(&milliclock.time);
               sprintf(&HeaderBlock[i], "%s %02d%03d%02d%02d%02d%02d", "AST= ",  
                       Ast->tm_year > 99 ? Ast->tm_year - 100 : Ast->tm_year,  
                       Ast->tm_yday+1, 
                       Ast->tm_hour, 
                       Ast->tm_min, 
                       Ast->tm_sec, 
                       (int)((double)milliclock.millitm/10));
               //HeaderBlock[i + strlen(&HeaderBlock[i]) - 1] = '\0';
               break;

      // Telescope String
      case 6 : for(j = 0; j < 2; j++)
                 if(sem_trywait(&TelStr.Sem) == 0)
                   break;
                 else
                   {
                   nanotime.tv_sec  = 0;
                   nanotime.tv_nsec = 1000000;
                   nanosleep(&nanotime, NULL);
                   }
               sprintf(&HeaderBlock[i], "%s %s", "TELSTR= ", TelStr.GlobalStr);
               if(j < 2)
                 sem_post(&TelStr.Sem);
               break;

      // Receiver ID
      case 7 : sprintf(&HeaderBlock[i], "%s %s", "RECEIVER= ", 
                       FrameHeader.Receiver);
               break;

      // Center Frequency
      case 8 : sprintf(&HeaderBlock[i], "%s %lf", "CENTERFREQ= ", 
                       FrameHeader.CenterFreq);
               break;

      // Sample Rate
      case 9 : sprintf(&HeaderBlock[i], "%s %lf", "SAMPLERATE= ", 
                       FrameHeader.SampleRate);
               break;

      // Data recorder (this program) version number
      case 10 : sprintf(&HeaderBlock[i], "%s %5.2lf", "VER= ", Version);
               break;

      // Number of ring buffers configured
      case 11 : sprintf(&HeaderBlock[i], "%s %ld", "NUMRINGBUFS= ", RingBuf.Num);
               break;

      // Number of disk buffers configured
      case 12 : sprintf(&HeaderBlock[i], "%s %ld", "NUMDISKBUFS= ", DiskBuf.Num);
               break;
      }  

  sprintf(&HeaderBlock[i], "%s", "EOH=");

  write(fd, HeaderBlock, sizeof(HeaderBlock));
  }

//======================================================
void PrintSchedInfo(char    * Name,
                    id_t      MyLwpId,
                    id_t      MyPid,
                    id_t      MyThreadId,
                    id_t      MyClassID,
                    pcinfo_t  PcInfo,
                    char    * Message)
//======================================================
  {
  sprintf(Message, "%s.  P_ID : %d  LWP_ID : %d  THREAD_ID : %d\n",
          Name, MyPid, MyLwpId, MyThreadId);
  WriteLog(Message, ToUser);

  MyClassID = GetClassID(P_LWPID, MyLwpId);
  PcInfo.pc_cid = MyClassID;
  if(priocntl(P_LWPID, MyLwpId, PC_GETCLINFO, (char *)&PcInfo) == -1L)
    ErrExit("PC_GETCLINFO failed\n");
  sprintf(Message, "LWP ID : %d  Class : %s\n\n", MyLwpId, PcInfo.pc_clname);
  WriteLog(Message, ToUser);
  }

//=======================================================
void LwpToRt(id_t        MyLwpId, 
             long        MyPriority, 
             pcparms_t   PcParms, 
             rtparms_t * RtParms, 
             short       MaxRtPri,
             char      * Message)
//=======================================================
  {
  if((PcParms.pc_cid = GetRtInfo(&MaxRtPri)) == -1)
    ErrExit("GetRtInfo failed\n");
  RtParms = (struct rtparms_t *)PcParms.pc_clparms;  // get ptr to RT cntl block
  RtParms->rt_pri = MaxRtPri - MyPriority;
  RtParms->rt_tqnsecs = RT_TQDEF;
  if(priocntl(P_LWPID, MyLwpId, PC_SETPARMS, (char *)&PcParms) == -1)
    ErrExit("Bump to Real Time class failed\n");
  else
    if(Verbose)
      {
      sprintf(Message, "LWP %d has been elavated to RealTime.\n", MyLwpId);
      WriteLog(Message, ToUser);
      }
  }

//=======================================================
int GetClassID(long IdType, id_t Id)
//=======================================================
  {
  pcparms_t PcParms;

  PcParms.pc_cid = PC_CLNULL;

  if(priocntl(IdType, Id, PC_GETPARMS, (char *)&PcParms) == -1)
    return(-1);

  return(PcParms.pc_cid);
  }

//=======================================================
id_t GetRtInfo(short * MaxPri)
//=======================================================
  {
  pcinfo_t   PcInfo;
  rtinfo_t * RtInfo;

  strcpy(PcInfo.pc_clname, "RT");

  if(priocntl(0L, 0L, PC_GETCID, (char *)&PcInfo) == -1L)
    return(-1);

  RtInfo = (struct rtinfo *) PcInfo.pc_clinfo;
  *MaxPri = RtInfo->rt_maxpri;

  return(PcInfo.pc_cid);
  }

//=======================================================
void * StringThread(void *)
//=======================================================
  {
  int i;
  struct termios options;
  aio_result_t result;
  timeval StringTimer;

  struct timespec nanotime;

  char LocalString[ScopeStringSize];

  if(Verbose)
    {
    sprintf(TelStr.Message, "Entering StringThread\n");
    WriteLog(TelStr.Message, ToUser);
    }
 
  TelStr.PId   =  getpid();
  TelStr.LwpId = _lwp_self();

  if(Verbose)
    PrintSchedInfo("StringThread", 
                   TelStr.LwpId,
                   TelStr.PId, 
                   TelStr.ThreadId,
                   TelStr.ClassID, 
                   TelStr.PcInfo,
                   TelStr.Message);

  if((tcgetattr(TelStr.Fd, &options)) != 0)
    perror(NULL);
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);
  options.c_iflag |= (ICANON | ECHO | ISIG);
  options.c_iflag |= (IXON | IXOFF | IXANY);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_cflag |= CREAD;
  if((tcsetattr(TelStr.Fd, TCSAFLUSH, &options)) != 0)
    perror(NULL);

  sem_init(&TelStr.Sem, 0, 1);
  while(TelStr.ThreadLive)
    {
    if(Verbose)
      {
      sprintf(TelStr.Message, "Waiting for scope string...\n");
      WriteLog(TelStr.Message, ToUser);
      }

    for(i = 0; i < ScopeStringSize && 
                   LocalString[i-1] != TelStr.Delimiter; i++)
      read(TelStr.Fd, &LocalString[i], 1);

    sem_wait(&TelStr.Sem); 
    memset(TelStr.GlobalStr, '\0', ScopeStringSize);
    strcpy(TelStr.GlobalStr, LocalString);
    sem_post(&TelStr.Sem); 
    if(sem_trywait(&TelStr.FirstStringSem) != 0)
      sem_post(&TelStr.FirstStringSem);
    if(Verbose)
      {
      sprintf(TelStr.Message, "%s", TelStr.GlobalStr);
      WriteLog(TelStr.Message, ToUser);
      }
    memset(LocalString, '\0', ScopeStringSize);
    nanotime.tv_sec = TelStr.Delay;
    nanotime.tv_nsec = 0;
    nanosleep(&nanotime, NULL);
    }

  if(Verbose)
    {
    sprintf(Tape.Message, "Exiting ScopeThread\n");
    WriteLog(Tape.Message, ToUser);
    }

  pthread_exit(NULL);
  }

//=======================================================
void * TriggerThread(void *)
//=======================================================
  {
  FILE * Tf;
  long stack_i;

  struct timespec nanotime;

  while(Trigger.ThreadLive)
    {
    if((Tf = fopen(Trigger.Name, "r")) == NULL)
      SayToExit(stack_i);
    else
      {
      fclose(Tf);
      nanotime.tv_sec = 0; 
      nanotime.tv_nsec = 1000000;  
      nanosleep(&nanotime, NULL); 
      }
    }

  if(Verbose)
    {
    sprintf(Trigger.Message, "Exiting TriggerThread\n");
    WriteLog(Trigger.Message, ToUser);
    }

  pthread_exit(NULL);
  }

//======================================================
BOOLEAN IsTrigger()
//======================================================
  {
   FILE * Tf;

   if((Tf = fopen(Trigger.Name, "r")) == NULL)
     return(FALSE);
   else
     {
     fclose(Tf);
     return(TRUE);
     }
  }
   
//======================================================
void GetOps(int       argc,
            char   ** argv,
            BOOLEAN * Verbose,
            BOOLEAN * FakeIt,
            double  * DataRate,
            char   ** TapeName,
            char   ** DiskName,
            char   ** SportName)
//=======================================================
  {
  char Usage[] = 
     "usage : s4datarecorder -tname TapeName -rname ReceiverName -freq CenterFreq -srate SampleRate \n [-v] [-dr datarate] [-disk DiskBufName] [-sport StringPort [-tape TapeDevice]\n [-mi MessageInterval] [-nowait]";

  long i;

  for(i = 1; i < argc; i++)
    {
    if(strcmp(argv[i], "-v") == 0)
      {
      *Verbose = TRUE;
      continue;
      }
    if(strcmp(argv[i], "-dr") == 0)
      {
      *DataRate = atof(argv[++i]);
      *FakeIt = TRUE;
      continue;
      }
    if(strcmp(argv[i], "-disk") == 0)
      {
      *DiskName = argv[++i];
      continue;
      }
    if(strcmp(argv[i], "-sport") == 0)
      {
      *SportName =  argv[++i];
      continue;
      }
    if(strcmp(argv[i], "-tname") == 0)
      {
      FrameHeader.Name = argv[++i];
      continue;
      }
    if(strcmp(argv[i], "-rname") == 0)
      {
      FrameHeader.Receiver = argv[++i];
      continue;
      }
    if(strcmp(argv[i], "-freq") == 0)
      {
      FrameHeader.CenterFreq = atof(argv[++i]);
      continue;
      }
    if(strcmp(argv[i], "-srate") == 0)
      {
      FrameHeader.SampleRate = atof(argv[++i]);
      // Acquire time = ring buffer size in Mbits / sample rate in Mbits per second
      // Fudge for floating point roundoff to be 1/100 of acquire time
      RingBuf.AcquireTime    = (r_BufSize * 8) / (FrameHeader.SampleRate * 2 * 1e6);
      RingBuf.AcquireTimeFudge = RingBuf.AcquireTime/100.0;
      continue;
      }
    if(strcmp(argv[i], "-nowait") == 0)
      {
      NoWait = TRUE;
      continue;
      }
    if(strcmp(argv[i], "-mi") == 0)
      {
      Tape.MessageInterval = atol(argv[++i]);
      continue;
      }
    ErrExit(Usage);
    }

  if(FrameHeader.Name        == NULL  ||
     FrameHeader.Receiver    == NULL  ||
     FrameHeader.CenterFreq  == 0     ||
     FrameHeader.SampleRate  == 0)
    ErrExit(Usage);
  }
  
//=======================================================
void WriteLog(char * Msg, unsigned char To)
//=======================================================
  {
  time_t clock;
  char * TimeString;
  char MsgOut[LogLen];

  sem_wait(&Log.Sem);
  time(&clock);
  TimeString = ctime(&clock);
  TimeString[strlen(TimeString)-1] = '\0';
  sprintf(MsgOut, "%s : %s", TimeString, Msg);
  if(To & ToConsole)
    fmtmsg(MM_CONSOLE, "s4datarecorder", MM_INFO, MsgOut, NULL, NULL);
  if(To & ToLogFile)
    fprintf(Log.LogFileFp, "%s : %s", TimeString, Msg);
  if(To & ToUser)
    fprintf(Log.UserFileFp, "%s", Msg);
  sem_post(&Log.Sem);
  }

//=======================================================
void ErrExit(char * Msg)
//=======================================================
  {
  perror(Msg);
  exit(1);
  }

//=======================================================
void SayToExit(long i)
//=======================================================
  {

  MainProc.ProcLive      = FALSE;
  DiskBuf.ThreadLive     = FALSE;
  Tape.ThreadLive        = FALSE;
  Trigger.ThreadLive     = FALSE;
  TelStr.ThreadLive      = FALSE;
  RingWatch.ThreadLive   = FALSE;

  for(i = 0; i < DiskBuf.Num; i++)
    if(sem_post(&DiskBuf.Sem[i]) != 0)
      ErrExit("SayToExit : Cannot post all disk buffer semaphores\n"); 

  if(sem_post(&RingBuf.Sem) != 0)
    ErrExit("SayToExit : Cannot post all ring buffer semaphores\n"); 

  if(sem_post(&TelStr.Sem) != 0)
    ErrExit("SayToExit : Cannot post string semaphore\n");

  if(sem_post(&FrameHeader.Sem) != 0)
    ErrExit("SayToExit : Cannot post frame header semaphore\n");

  // do not post log sem!
  }

//=======================================================
long CheckRingBuffers(EdtDev * EdtDevice, long MissingBufs)
//=======================================================
// This routine is called by main() each time we are notified
// of EDT data being ready.  It is possible that EDT will
// have gone completely around the ring buffer between "waits"
// on such notification.  If so, we may have missed some 
// data - that is what we check for here. 
  {
  long RealDiff;
  static long KnownBehind = 0;

  RingBuf.DoneCount = edt_done_count(EdtDevice);

  RealDiff = RingBuf.DoneCount - (FrameHeader.SequenceNum + 1);

  if(Verbose)
    {
    sprintf(MainProc.Message, "sssssssssssss> %ld\n", FrameHeader.SequenceNum);
    WriteLog(MainProc.Message, ToUser);
    sprintf(MainProc.Message, "rrrrrrrrrrrrr> %ld\n", RingBuf.DoneCount);
    WriteLog(MainProc.Message, ToUser);
    sprintf(MainProc.Message, "ndndndndndndn> %ld\n", RealDiff - KnownBehind);
    WriteLog(MainProc.Message, ToUser);
    }

  if(RealDiff - KnownBehind >= RingBuf.Num)
    {
    // We *may* have gone around twice!  This will let us know,
    // by adding on to the last miss.
    MissingBufs += (RealDiff - KnownBehind) - (RingBuf.Num - 1); 
    KnownBehind  = long(RealDiff/RingBuf.Num) * RingBuf.Num;
    } 
  else
    MissingBufs = 0; 

  return(MissingBufs);
  }

//=======================================================
void * RingWatchThread(void *)
//=======================================================
  {
  long DoneCount, OldDoneCount;

  long NoDataElapsed = 0;

  struct timespec nanotime;

  while(RingWatch.ThreadLive)
    {
    nanotime.tv_sec = RingWatch.Interval; 
    nanotime.tv_nsec = 0;  
    nanosleep(&nanotime, NULL); 
    DoneCount = edt_done_count(RingWatch.EdtDevice);
    if(DoneCount == OldDoneCount)
      {
      NoDataElapsed += RingWatch.Interval;
      sprintf(RingWatch.Message, "No data from EDT in last %ld seconds!\n", 
              NoDataElapsed);
      WriteLog(RingWatch.Message, ToConsole+ToLogFile+ToUser);
      }
    else
      NoDataElapsed = 0;
    OldDoneCount = DoneCount;
    }

  if(Verbose)
    {
    sprintf(RingWatch.Message, "Exiting RingWatchThread\n");
    WriteLog(RingWatch.Message, ToUser);
    }

  pthread_exit(NULL);
  }

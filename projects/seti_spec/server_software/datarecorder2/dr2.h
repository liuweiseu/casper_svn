//#define DEBUG

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <fmtmsg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/lwp.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>

// App headers
#include "serial.h"

// EDT headers
#include "edtinc.h"

// ScramNet headers
extern "C" 
{
#include "mshmLib.h"
#include "execshm.h"
#include "wappshm.h"
#include "wapplib.h"
#include "alfashm.h"
#include "vtxLib.h"
#include "scram.h"
}

// front end stuff
#include "datarec.h"


typedef struct {
  EdtDev * 			    EdtDevice;
  long                  Num;			// def**, no conf
  long *              	Size;			// def**, no conf
  pthread_mutex_t * 	Mutex;         	// ring of "fast" mutexes
  unsigned char ** 		ReadyPtr;		// ring
  struct timeb * 		ReadyTime;		// ring
  struct timeb * 		SynthTime;		// ring
  long *			    SynthFreq;		// ring (Hz)
  long *			    SkyFreq;		// ring (Hz)
  long * 			    MissingBufs;		
  unsigned long      	DoneCount;
  double             	AcquireTime;
  double             	AcquireTimeFudge;
  pthread_t          	ThreadId;
  pthread_attr_t     	ThreadAttr;
  bool          		ThreadLive;
  bool          		Ready;
  char * 			    Message;
  struct sched_param 	SchedParam;
  void *			    (*Function)(void*);
  int				    Index;
  } MemBuf_t;

typedef struct {
  unsigned char * 		Data;
  unsigned char **      ReadyPtr;
  } FakeData_t;

typedef struct {
  char *                Name;
  char *			    DoneLink;
  char *			    ThisLink;
  long                  Num;
  long                  Size;
  long                  MmapSize;
  long				    Offset;
  int                   Fd;        
  int				    AtBuf;
  caddr_t               MmapBase; 
  caddr_t               MmapCurrent; 
  char *       			BaseName;  	// for file name generation	// def**, conf*
  pthread_t             ThreadId;
  pthread_attr_t        ThreadAttr;
  int                   ThreadPri;
  bool          		Idle;
  long          		IdleCount;
  bool                  QuickLook;
  int                   QLFile;
  int                   QLFileSkipCounter;
  int                   QLFileBufCounter;
  bool    	            ThreadLive;
  bool          		Ready;
  char *       			Message;
  struct sched_param    SchedParam;
  void *       			(*Function)(void*);
  int				    Index;
  } DiskBuf_t;

typedef struct {
  pthread_t             ThreadId;
  pthread_attr_t        ThreadAttr;
  int                   ThreadPri;
  bool                  ThreadLive;
  bool          		Ready;
  struct sched_param    SchedParam;
  void *                (*Function)(void*);
  pthread_mutex_t       Mutex;
  int                   Interval;
  //char *                SerialPortName;         // def**, conf*
  //int                   SerialPort;
  int *                 Vgc;
  char *                String;
  char *                Message;
  int				    Index;
  } Eng_t;

typedef struct {
  char *       			FileName;			// def**, conf*
  int          			Fd;
  pthread_t    			ThreadId;
  pthread_attr_t 		ThreadAttr;
  int          			ThreadPri;
  bool    			ThreadLive;
  bool          		Ready;
  char *        		Message;
  struct sched_param            SchedParam;
  void *       			(*Function)(void*);
  } Trigger_t;

typedef struct {
  pthread_mutex_t  		Mutex;
  char * 			LogFileName;			// def**, conf*
  FILE * 			ConsoleFp;
  FILE * 			LogFileFp;
  FILE * 			UserFileFp;
  }  Log_t;
 
#define OBSERVATORY_INTERFACE_AOSCRAM 1
typedef struct {
  pthread_mutex_t  		Mutex;        
  pthread_t    			ThreadId;
  pthread_attr_t 		ThreadAttr;
  int          			ThreadPri;
  bool    			ThreadLive;
  bool          		Ready;
  struct sched_param            SchedParam;
  void *       			(*Function)(void*);
  int				Interface;
  void *			DataPtr;
  char *        		Message;
  int               ScramLagTolerance;      // seconds

  pthread_mutex_t  	PNT_Mutex;        
  double			PNT_Ra;
  double			PNT_Dec;
  double            PNT_ModelCorEncAzZa[2];
  double			PNT_MJD;
  time_t            PNT_SysTime;

  // this is where the accurate pointing is
  pthread_mutex_t  	AGC_Mutex;        
  double            AGC_Az;
  double            AGC_Za;
  long              AGC_Time;
  time_t            AGC_SysTime;

  pthread_mutex_t  	ALFASHM_Mutex;        
  int               ALFASHM_AlfaFirstBias;      // bitmap - 4 beam x 2 channel alpha bias bools (idle point)
  int               ALFASHM_AlfaSecondBias;     // bitmap - 3 beam x 2 channel alpha bias bools (idle point)
  float             ALFASHM_AlfaMotorPosition;  // rotation angle
  time_t            ALFASHM_SysTime;

  pthread_mutex_t  	IF1_Mutex;        
  double            IF1_synI_freqHz_0;          // First LO
  int               IF1_synI_ampDB_0;
  double            IF1_rfFreq;
  double            IF1_if1FrqMhz;
  int               IF1_alfaFb;
  time_t            IF1_SysTime;

  pthread_mutex_t  	IF2_Mutex;        
  bool              IF2_useAlfa;                // alfa bool (ilde point)
  time_t            IF2_SysTime;

  pthread_mutex_t  	TT_Mutex;        
  int               TT_TurretEncoder;           
  double            TT_TurretDegrees;           // idle point
  double            TT_TurretDegreesAlfa;
  double            TT_TurretDegreesTolerance;
  time_t            TT_SysTime;

  } Observatory_t;


typedef struct {
  pthread_t    			ThreadId;
  pthread_attr_t 		ThreadAttr;
  int          			ThreadPri;
  bool    			ThreadLive;
  bool          		Ready;
  struct sched_param            SchedParam;
  void *       			(*Function)(void*);
  int				Fd;
  int               Model;
  long              NumFreqSteps;      
  struct timeb  	CurrentFreqTime;
  long *            SkyFreq;            // sky
  long				CurrentSkyFreq;     // sky
  long              Fmin;               // sky
  long              Fmax;               // sky
  long              FilteredFmin;       // sky
  long              FilteredFmax;       // sky
  long              CurrentSynthFreq;   // synth
  long              SynthMin;           // synth
  long              SynthMax;           // synth
  bool				NoSynth;	
  bool				ObservableFreq;	
  bool				Init;	
  bool				StepSynth;			// def**, conf*
  bool				StepFreq;
  bool				CheckNow;
  int				CheckCount;
  pthread_mutex_t	DataMutex;
  pthread_mutex_t	CheckMutex;
  pthread_cond_t	CheckNowCond;
  char *        	SerialPortName;			// def**, conf*
  int				PTS232;				// the serial port descriptor for PTS232 comm
  char *        	Message;
  } Synth_t;

typedef struct {
    pthread_mutex_t   Mutex;
    time_t            Interval;
    int               NumBufs;
    long              Frequency;
    char *            BaseName;
    char *            Message;
} QuickLook_t;

typedef struct {
  pthread_t             ThreadId;
  pthread_attr_t        ThreadAttr;
  int                   ThreadPri;
  bool                  ThreadLive;
  bool          		Ready;
  struct sched_param    SchedParam;
  void *                (*Function)(void*);
  int                   Interval;
  int  *                DoneCount;
  int  *                NoDataElapsed;
  char *                Message;
  } RingWatch_t;

typedef struct {
  pthread_t             ThreadId;
  pthread_attr_t        ThreadAttr;
  pthread_mutex_t  		Mutex;        
  int                   ThreadPri;
  bool                  ThreadLive;
  bool          		Ready;
  struct sched_param    SchedParam;
  void *                (*Function)(void*);
  int                   Interval;
  int                   MinVGC;
  char *                Message;
  } IdleWatch_t;

typedef struct {
  pthread_mutex_t    		Mutex;
  char * 			Name;
  long             		RecordType;
  char * 			Receiver;			// conf only*
  unsigned long    		SequenceNum;
  double           		CenterFreq;			// conf only - what about synth stepping?
  double           		SampleRate;			// conf only*
  char *	   		Block;
  } FrameHeader_t;

// ---------- Function Protoypes

int StartThread(	pthread_attr_t          * Attr,
                	struct sched_param      * SchedParam,
                	pthread_t               * Id,
                	int                       Prio,
                	void *(*Function)(void *),
			void 			* Parm);

bool MemBufInit();
void * MemBufFunc(void *);
void GetData(int dsi);
bool AllReady();
bool DiskBufInit();
void * DiskBufFunc(void *);
void AdvanceDiskBuffer(long m_in_d, int dsi);
bool RingOverRunCheck(timeb ThisReadyTime, timeb LastReadyTime, int dsi);
bool RingSegCorruptCheck(timeb& ThisReadyTime, int r_AtBuf, int dsi);
int InitHeader();
void FormatHeader(int fd, 
                  long DataSize, 
                  struct timeb DataTime, 
                  struct timeb SynthTime,
                  long SynthFreq, 
                  long SkyFreq, 
                  bool MissingBufs,
                  long DataSeq, 
                  int dsi);
bool InitLog();
void WriteLog(char * Msg, unsigned char To);
void ErrExit(char * Msg);
void EdtErrExit(char * Msg);
//void PrintThreadInfo(char * MsgBuf, char * ThreadName);
//void PrintThreadInfo(char * ThreadName, int Index, struct sched_param SchedParam,  char * MsgBuf);
void PrintThreadInfo(	char * ThreadName, 
			int Index, 
			pthread_t ThreadId, 
			pthread_attr_t * ThreadAttr);

int IdleWatchInit();
void * IdleWatchThread(void *);
bool Idling(int dsi);
int TriggerInit();
void * TriggerThread(void *);
int EngInit();
void * EngThread(void * dsip);
int SynthInit();
void * SynthThread(void *);
long GetSynthFreq(long FirstLO, long SkyFreq);
bool FreqCheck(long SynthFreq, long SkyFreq);
void ProgramSynth(long NewFreq);
void SayToExit();
void GetOps(int argc, char ** argv);
void GetConfig();
int ParseVarLongs(char * s, long ** l);
void InitProg();
void InitEdt(int dsi);
void MySignals(int action);
#define INIT 1
#define UNBLOCK 2
void  StartThreads();
void  EdtCntl(EdtDev * edt_p, int action);
void  Rejoin();
bool  EdtIsOpen(int dsi);
void  CloseEdt(int dsi);
int   ObservatoryInit();
void * ObservatoryFunc(void *);
void process_PNT(struct SCRAMNET *scram);
void process_AGC(struct SCRAMNET *scram);
void process_ALFASHM(struct SCRAMNET *scram);
void process_IF1(struct SCRAMNET *scram);
void process_IF2(struct SCRAMNET *scram);
void process_TT(struct SCRAMNET *scram);
void process_TT(struct SCRAMNET *scram);
void print_PNT(char * OutString);
void print_AGC(char * OutString);
void print_ALFASHM(char * OutString);
void print_IF1(char * OutString);
void print_IF2(char * OutString);
void print_TT(char * OutString);
bool BadAGC();
extern bool BadALFASHM();
extern bool BadIF1();
extern bool BadIF2();
extern bool BadTT();
int   RingWatchInit();
void * RingWatchThread(void *);
int QuickLookInit();
void QuickLookCheck();
void WriteQuickLookData(int dsi, int r_AtBuf);
void GetScramData();
double SkyAngle(double Ra1, double Dec1, double Ra2, double Dec2); 
void SigHandler(int signal);


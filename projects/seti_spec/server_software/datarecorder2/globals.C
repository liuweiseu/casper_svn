#include "dr2.h"
#include "globals.h"

bool IdleCond[10];

long            NumDatastreams;
DiskBuf_t *     DiskBuf;        // array of disk buffers
MemBuf_t *      MemBuf;         // array of memory buffers
FakeData_t      FakeData;
FrameHeader_t * FrameHeader;
Eng_t *         Eng;
//FrameHeader_t  FrameHeader;
Log_t           Log;
IdleWatch_t     IdleWatch;
Trigger_t       Trigger;
Synth_t   	    Synth;
Observatory_t   Observatory;
RingWatch_t     RingWatch;
QuickLook_t     QuickLook;

//TODO: put into config file
char DBaseName[StringSize]     	  = "diskbuf/diskbuf";
char LogFileName[StringSize]   	  = "/tmp/dr2_log";
char ConfigFileName[StringSize]	  = "dr2_config";
char TriggerFileName[StringSize]  = "/tmp/dr2_run";
char SynthSerialPortName[StringSize]  = "/dev/ttyS0";
char EngSerialPortName[2][StringSize]   = {"/dev/ttyS2", "/dev/ttyS3"};
char Receiver[StringSize];
char TapeName[StringSize]           = "NoName";

bool VersionOnly        = false;
bool EdtEnabled         = false;
bool ObsOnly            = false;
bool NoSynth            = false;
bool LimitedRun         = false;
bool UseFakeData        = false;
long LimitedRunNum      = 20;

// sleep intervals
long RingWatchInterval  = 5;
long EngInterval        = 5;
long IdleWatchInterval  = 2;

pthread_mutex_t            SignalMutex;
sigset_t SigBlockMask, SigOriginalMask;
struct sigaction SigAction;

char ConfigBlock[ConfigBlockSize];            	// copy of config file contents
double SampleRate          = 0.0;
double AcquireTime         = 0.0;
double AcquireTimeFudge    = 0.0;
int    NumChannels         = 8;
//int SynthCheckCount	   = (BufLen/LongestFftLen) x (NumInputBeams/NumEdtCards);
//int SynthCheckCount = 256;        // make dynamic?  1048576/131072 x 16 x 2 ?
int SynthCheckCount = 64;        // make dynamic?  1048576/131072 x 16 x 2 ?

// the following defaults mean we have 2 disk buffers,
// each to contain 16 ring buffers and we mmap the disk
// buffers 8 ring buffers at a time.
long d_NumBufs            = 2;
long num_m_in_d           = 16;
long num_r_in_m           = 8;

bool SynthStepSynth = false;
int  SynthModel = 0;

long SynthMin       =   50000000;
long SynthMax       =  450000000;
long Fmin           = 1200000000;
long Fmax           = 1530000000;
long FilteredFmin   = 1380000000;
long FilteredFmax   = 1500000000;

double BeamRes      = 0.05;         // degrees

long QuickLookFreq;                 // defaults? jeffc
int QuickLookInterval;
int QuickLookBufs;

double TT_TurretDegreesAlfa = 26.64;
double TT_TurretDegreesTolerance = 1.0;

bool SkipScram = false;
int ScramRateTolerance = 5;         // seconds

bool Debug           = false;
bool Verbose         = false;
bool VeryVerbose     = false;
bool AlwaysStepSynth = false;
bool IdleData        = false;
bool Headers         = true;
bool MemBufOnly      = false;
bool PrioSched       = false;

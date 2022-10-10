
const double Version     = 2.00;
const long NUMPRIOLEVELS = 2;
const long MINPRIO =  0;
const long MAXPRIO =  1;
//const long MemBufCheckInterval = 1000;  // 1 millisecond
const char str_delimit  = '\n';
const char str_delay    = 1;            // 1 second
const long r_NumBufs    = 4;
const long r_BufSize    = 1024*1024;
const long HeaderSize   = 1024*4;        
const long StringSize   = 512;
//const long ConfigBlockSize = 1024;
const long ConfigBlockSize = 2048;
const long RcdType      = 1;            // Berkeley SETI
const unsigned char ToConsole = 0x01;   // items for message routing
const unsigned char ToLogFile = 0x02;
const unsigned char ToUser    = 0x04;
const int edt_disarm            = 0;
const int edt_arm               = 1;
const int edt_abort_current_buf = 2;
const int PTS500        = 500;      // PTS synthesizer model number
const int PTS3200       = 3200;     // PTS synthesizer model number

enum IdleCond_enum {BadAGC_time, BadALFASHM_time, BadALFASHM_biases,
                    BadIF1_time, BadIF2_time, BadIF2_useAlfa, BadTT_time, 
                    BadTT_TurretDegrees, BadVGC, BadFreq};
extern bool IdleCond[];

extern bool SynthStepSynth;
extern int  SynthModel;


extern long            	NumDatastreams;
extern DiskBuf_t *     	DiskBuf;        // array of disk buffers                
extern MemBuf_t *      	MemBuf;         // array of memory buffers
extern FakeData_t      	FakeData;
extern FrameHeader_t * 	FrameHeader;
extern Eng_t *          Eng;
extern Log_t           	Log;
extern IdleWatch_t      IdleWatch;
extern Trigger_t       	Trigger;
extern Synth_t   	    Synth;
extern Observatory_t   	Observatory;
extern RingWatch_t   	RingWatch;
extern QuickLook_t      QuickLook;

extern char DBaseName[];
extern char LogFileName[];
extern char ConfigFileName[];
extern char TriggerFileName[];
extern char SynthSerialPortName[];
extern char EngSerialPortName[][StringSize];
extern char Receiver[];
extern char TapeName[];

extern int PTS232;

extern bool VersionOnly;       
extern bool ObsOnly;          
extern bool EdtEnabled;
//extern bool IdleData;      
extern bool NoSynth;      
extern bool LimitedRun;      
extern bool UseFakeData;    
extern long LimitedRunNum; 

extern long RingWatchInterval; 
extern long EngInterval; 
extern long IdleWatchInterval; 

extern pthread_mutex_t            SignalMutex;
extern sigset_t SigBlockMask, SigOriginalMask;
extern struct sigaction SigAction;

extern char ConfigBlock[ConfigBlockSize];        // copy of config file contents
extern double SampleRate;    
extern double AcquireTime;    
extern double AcquireTimeFudge;  
extern int    NumChannels;
extern int SynthCheckCount;

extern long SynthMin;
extern long SynthMax;
extern long Fmin;
extern long Fmax;
extern long FilteredFmin;
extern long FilteredFmax;

extern double BeamRes;

extern long QuickLookFreq;            
extern int QuickLookInterval;
extern int QuickLookBufs;

extern double TT_TurretDegreesAlfa;
extern double TT_TurretDegreesTolerance;

extern bool SkipScram;
extern int ScramRateTolerance;

extern long d_NumBufs;          
extern long num_m_in_d;        
extern long num_r_in_m;       

extern bool Debug;          
extern bool Verbose;       
extern bool VeryVerbose;  
extern bool AlwaysStepSynth;
extern bool Headers;     
extern bool MemBufOnly; 
extern bool PrioSched; 

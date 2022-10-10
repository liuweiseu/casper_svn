#include "dr2.h"
#include "globals.h"

//======================================================
void GetOps(int  argc, char  ** argv)
//=======================================================
{
    int i;
    char LocalString[StringSize];

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-version")) {
            VersionOnly = true;
        } else if (!strcmp(argv[i], "-obs")) {
            ObsOnly = true;
        } else if (!strcmp(argv[i], "-name")) {
            strcpy(TapeName, argv[++i]);
            sprintf(LocalString, "Tape Name is %s\n", TapeName);
            WriteLog(LocalString, ToUser);
        } else if (!strcmp(argv[i], "-config")) {
            strcpy(ConfigFileName, argv[++i]);
        } else {
            fprintf(stderr, "Unrecognized arg: %s\n", argv[i]);
            exit(1);
        }
    }

}

//======================================================
void GetConfig()
//=======================================================
{
    FILE * f;
    int i, j, k;
    char Buf[512];
    char Keyword[512];
    char Value[512];

    long * FreqSteps;

    double CenterFreq; //place holder

    f = fopen(ConfigFileName, "r");
    if(!f) ErrExit("Could not open config file");

    WriteLog("Start Configuration\n", ToUser);
    i = 0;
    while (i < sizeof(ConfigBlock)  && fgets(&ConfigBlock[i], sizeof(ConfigBlock)/2, f)) {

        //fprintf(stderr, "cb size : %d\n", i);

        if(ConfigBlock[i] != '#') {

            if((sscanf(&ConfigBlock[i], "%s %s", Keyword, Value)) < 1) {
                 ErrExit("Bad line in config file");
            }

            if(strcasecmp(Keyword, "freq_steps") == 0) {
                int n;
                Synth.NumFreqSteps = ParseVarLongs(&ConfigBlock[i], &Synth.SkyFreq);
            }
            if(strcasecmp(Keyword, "num_data_streams") == 0) {
                NumDatastreams = (long)atoi(Value);
            }
            if(strcasecmp(Keyword, "center_freq") == 0) {
                CenterFreq = atof(Value);
            }
            else if (strcasecmp(Keyword, "sample_rate") == 0) {
                SampleRate = atof(Value);
                // Acquire time = ring buffer size in Mbits / sample rate in million bits per second
                // Fudge for floating point roundoff to be 1/100 of acquire time
                // 8 = #bytes to #bits
                // 2 = #bits per sample
                // 1e6 = convert sample rate from million samples per second to samples per second
                AcquireTime      = (r_BufSize * 8) / (SampleRate * 2 * 8 * 1e6);
                AcquireTimeFudge = AcquireTime/100.0;
            }
            else if (strcasecmp(Keyword, "limited_run") == 0) {
                LimitedRunNum = (long)atoi(Value);
                if (LimitedRunNum) LimitedRun = true;
            }
            else if (strcasecmp(Keyword, "use_fake_data") == 0) {
                UseFakeData = true;
            }
            else if (strcasecmp(Keyword, "verbose") == 0) {
                Verbose = true;
            }
            else if (strcasecmp(Keyword, "very_verbose") == 0) {
                VeryVerbose = true;
            }
            if(strcasecmp(Keyword, "min_vgc") == 0) {
                IdleWatch.MinVGC = (int)atol(Value);
            }
            if(strcasecmp(Keyword, "synth_serial_port") == 0) {
                strcpy(SynthSerialPortName, Value);
            }
            if(strcasecmp(Keyword, "synth_step_synth") == 0) {
                SynthStepSynth = true;
            }
            if(strcasecmp(Keyword, "synth_model") == 0) {
                if(strcasecmp(Value, "PTS500") == 0) {
                    SynthModel = PTS500;
                }
                if(strcasecmp(Value, "PTS3200") == 0) {
                    SynthModel = PTS3200;
                }
            }
            if(strcasecmp(Keyword, "skip_scram") == 0) {
                SkipScram = true;
            }
            if(strcasecmp(Keyword, "scram_rate_tolerance") == 0) {
                ScramRateTolerance = atol(Value);
            }
            if(strcasecmp(Keyword, "diskbuf_base_name") == 0) {
                strcpy(DBaseName, Value);
            }
            if(strcasecmp(Keyword, "trigger_file_name") == 0) {
                strcpy(TriggerFileName, Value);
            }
            if(strcasecmp(Keyword, "log_file_name") == 0) {
                strcpy(LogFileName, Value);
            }
            if(strcasecmp(Keyword, "receiver") == 0) {
                strcpy(Receiver, Value);
            }
            if(strcasecmp(Keyword, "num_m_in_d") == 0) {
                num_m_in_d = atol(Value);
            }
            if(strcasecmp(Keyword, "num_diskbufs") == 0) {
                d_NumBufs = atol(Value);
            }
            if(strcasecmp(Keyword, "quicklook_freq") == 0) {
                QuickLookFreq = atol(Value);
            }
            if(strcasecmp(Keyword, "quicklook_interval") == 0) {
                QuickLookInterval = atoi(Value);
            }
            if(strcasecmp(Keyword, "quicklook_bufs") == 0) {
                QuickLookBufs = atoi(Value);
            }
            if(strcasecmp(Keyword, "turret_degrees_alfa") == 0) {
                TT_TurretDegreesAlfa = atof(Value);
            }
            if(strcasecmp(Keyword, "turret_degrees_tolerance") == 0) {
                TT_TurretDegreesTolerance = atof(Value);
            }
            if(strcasecmp(Keyword, "min_rec_freq") == 0) {
                Fmin = atol(Value);
            }
            if(strcasecmp(Keyword, "max_rec_freq") == 0) {
                Fmax = atol(Value);
            }
            if(strcasecmp(Keyword, "filtered_min_rec_freq") == 0) {
                FilteredFmin = atol(Value);
            }
            if(strcasecmp(Keyword, "filtered_max_rec_freq") == 0) {
                FilteredFmax = atol(Value);
            }
            if(strcasecmp(Keyword, "min_synth_freq") == 0) {
                SynthMin = atol(Value);
            }
            if(strcasecmp(Keyword, "max_synth_freq") == 0) {
                SynthMax = atol(Value);
            }
            if(strcasecmp(Keyword, "beam_res") == 0) {
                BeamRes = atof(Value);
            }
            if(strcasecmp(Keyword, "no_synth") == 0) {
                NoSynth = true;
            }

            sprintf(Buf, "%s", &ConfigBlock[i]);
            WriteLog(Buf, ToUser);

        }
        i += strlen(&ConfigBlock[i]) + 1;

        //sprintf(Buf, "%-32s %s\n", Keyword, Value);
        Value[0]   = '\0';        // some keywords have no values
    }

    WriteLog("End Configuration\n", ToUser);

    sprintf(Buf, "Ring Buffer Segment Acquire Time is %lf seconds\n", AcquireTime);
    WriteLog(Buf, ToUser);

    // derived values
    if(VeryVerbose) Verbose = true; 

    // need to check for required config items   -- jeffc
    if(strlen(Receiver) == 0) {
        ErrExit("No receiver is configured \n");
    }


    fclose(f);
}

//======================================================
bool InitLog()
//======================================================
{
    if(pthread_mutex_init(&Log.Mutex, NULL))
        ErrExit("Cannot initialize log mutex ");

    Log.LogFileName = (char *)LogFileName;

    if((Log.LogFileFp = fopen(LogFileName, "a")) == NULL)
        ErrExit("Open failed for log file ");

    setbuf(Log.LogFileFp, NULL);   // unbuffered log output

    Log.UserFileFp = stderr;

    return(0);
}

//=======================================================
void WriteLog(char * Msg, unsigned char To)
//=======================================================
{
    time_t clock;
    char * TimeString;
    char MsgOut[StringSize];

    if(pthread_mutex_lock(&Log.Mutex)) ErrExit("WriteLog : mutex lock error \n");
    time(&clock);
    TimeString = ctime(&clock);
    TimeString[strlen(TimeString)-1] = '\0';
    sprintf(MsgOut, "%s : %s", TimeString, Msg);
    if(To & ToConsole)
        fmtmsg(MM_CONSOLE, "s4datarecorder", MM_INFO, MsgOut, NULL, NULL);
    //if(To & ToLogFile)
    if(To & ToUser)
        fprintf(Log.LogFileFp, "%s : %s", TimeString, Msg);
    if(To & ToUser)
        fprintf(Log.UserFileFp, "%s : %s", TimeString, Msg);
    //Msg[0] = '\0';                      // clear message area
    if(pthread_mutex_unlock(&Log.Mutex)) ErrExit("WriteLog : mutex lock error \n");
}

//=======================================================
int ParseVarLongs(char * s, long ** l) {
//=======================================================
// Parses a string containing a variable number of integers
// into an array of longs.  Imbedded noninteger "words" are
// skipped. Returns the length of the resulting array.

    char substr[256];
    int i=0,j=0;

    while(*s != '\0') {
        while(isspace(*s) || isalpha(*s) || ispunct(*s)) s++;
        if(*s == '\0') break;
        i = 0;
        while(isdigit(*s)) {
            substr[i] = (*s);
            s++;
            i++;
        }
        if(i != 0) {
            substr[i] = '\0';
            *l = (long *)realloc((void *)*l, sizeof(long) * (j+1));
            if(*l == NULL) {
                perror("bad realloc in ParseVarLongs");
                exit(1);
            }
            (*l)[j] = atoi(substr);
            j++;
        }
    }

    return(j);
}


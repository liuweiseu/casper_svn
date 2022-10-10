#include "dr2.h"
#include "globals.h"

//=======================================================
int InitHeader() {
//=======================================================

    int dsi;

    if((FrameHeader = (FrameHeader_t *)calloc(NumDatastreams, sizeof(FrameHeader_t))) == NULL) {
        return(1);
    }

    for (dsi = 0; dsi < NumDatastreams; dsi++) {
        FrameHeader[dsi].Name              = TapeName;
        FrameHeader[dsi].RecordType        = RcdType;
        if((FrameHeader[dsi].Block             = (char *)calloc(1, HeaderSize)) == NULL) {
            return(1);
        }
        FrameHeader[dsi].SequenceNum       = 1;
        FrameHeader[dsi].SampleRate        = SampleRate;
        if((FrameHeader[dsi].Receiver =  (char *)calloc(1, StringSize)) == NULL) {
            return(1);
        }
        strcpy(FrameHeader[dsi].Receiver, Receiver);
    }

    return(0);
}

//=======================================================
void FormatHeader(int fd,
                  long DataSize, 
                  struct timeb DataTime,
                  struct timeb SynthTime,
                  long SynthFreq,
                  long SkyFreq,
                  bool MissingBufs,
                  long DataSeq,
                  int dsi) {
//=======================================================
    char HeaderBlock[HeaderSize];

    long i = 0;
    long j;
    long item;
    time_t clock;
    tm * ThisTime;
    char * HeaderCurrent;

    struct timespec nanotime;

    HeaderCurrent = FrameHeader[dsi].Block;
    memset(HeaderCurrent, '\0', HeaderSize);    // (re)init

    sprintf(HeaderCurrent, "HEADER\n");
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Header Size
    sprintf(HeaderCurrent, "HEADER_SIZE %d\n", HeaderSize);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Data Size
    sprintf(HeaderCurrent, "DATA_SIZE %d\n", DataSize);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Tape name
    sprintf(HeaderCurrent, "NAME %s\n", FrameHeader[dsi].Name);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

#if 0
    // Record Type
    sprintf(HeaderCurrent, "RCDTYPE %ld\n", FrameHeader[dsi].RecordType);
    HeaderCurrent += strlen(HeaderCurrent) + 1;
#endif

    // Data Stream Index
    sprintf(HeaderCurrent, "DSI %d\n", dsi);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Frame Sequence Number
    sprintf(HeaderCurrent, "FRAMESEQ %lu\n", FrameHeader[dsi].SequenceNum);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Real Data Sequence Number
    sprintf(HeaderCurrent, "DATASEQ %lu\n", DataSeq);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Real Data Sequence Number
    sprintf(HeaderCurrent, "IDLECOUNT %lu\n", DiskBuf[dsi].IdleCount);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Missed Buffers
    sprintf(HeaderCurrent, "MISSED %d\n", MissingBufs);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

if(MissingBufs) {
sprintf(DiskBuf[dsi].Message, "DSI %d MISSED %d\n", dsi, MissingBufs);
WriteLog(DiskBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
}

    // Data acquisition time
    ThisTime   = localtime(&DataTime.time);
    sprintf(HeaderCurrent, "AST %02d%03d%02d%02d%02d%02d\n",
            ThisTime->tm_year > 99 ? ThisTime->tm_year - 100 : ThisTime->tm_year,
            ThisTime->tm_yday+1,
            ThisTime->tm_hour,
            ThisTime->tm_min,
            ThisTime->tm_sec,
            (int)((double)DataTime.millitm/10));
    //HeaderBlock[i + strlen(&HeaderBlock[i]) - 1] = '\0';
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Synthesizer time and freq
    ThisTime   = localtime(&SynthTime.time);
    sprintf(HeaderCurrent, "SYNTH %02d%03d%02d%02d%02d%02d %ld %ld\n",
            ThisTime->tm_year > 99 ? ThisTime->tm_year - 100 : ThisTime->tm_year,
            ThisTime->tm_yday+1,
            ThisTime->tm_hour,
            ThisTime->tm_min,
            ThisTime->tm_sec,
            (int)((double)SynthTime.millitm/10),
            SynthFreq,
            SkyFreq);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Engineering
    if(pthread_mutex_lock(&Eng[dsi].Mutex)) ErrExit("FormatHeader : mutex lock error \n");
    sprintf(HeaderCurrent, "ENG %s\n", Eng[dsi].String);
    if(pthread_mutex_unlock(&Eng[dsi].Mutex)) ErrExit("FormatHeader : mutex unlock error \n");
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Receiver ID
    sprintf(HeaderCurrent, "RECEIVER %s\n",
            FrameHeader[dsi].Receiver);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Sample Rate - need this?  it's in config block  jeffc
    sprintf(HeaderCurrent, "SAMPLERATE %lf\n", 
            FrameHeader[dsi].SampleRate);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Data recorder (this program) version number
    sprintf(HeaderCurrent, "VER %5.2lf\n", Version);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // SCRAM data
    //print_PNT(HeaderCurrent);
    //HeaderCurrent += strlen(HeaderCurrent) + 1;
    print_AGC(HeaderCurrent);
    HeaderCurrent += strlen(HeaderCurrent) + 1;
    print_ALFASHM(HeaderCurrent);
    HeaderCurrent += strlen(HeaderCurrent) + 1;
    print_IF1(HeaderCurrent);
    HeaderCurrent += strlen(HeaderCurrent) + 1;
    print_IF2(HeaderCurrent);
    HeaderCurrent += strlen(HeaderCurrent) + 1;
    print_TT(HeaderCurrent);
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    // Config Block
    sprintf(HeaderCurrent, "CONFIG_BLOCK\n");
    HeaderCurrent += strlen(HeaderCurrent) + 1;
    memcpy(HeaderCurrent, ConfigBlock, ConfigBlockSize);
    HeaderCurrent += ConfigBlockSize + 1;
    sprintf(HeaderCurrent, "END_OF_CONFIG_BLOCK\n");
    HeaderCurrent += strlen(HeaderCurrent) + 1;

    sprintf(HeaderCurrent, "END_OF_HEADER\n");
    HeaderCurrent += strlen(HeaderCurrent) + 1;
}

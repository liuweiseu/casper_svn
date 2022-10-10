#include "dr2.h"
#include "globals.h"

//======================================================
bool DiskBufInit() {
//======================================================

    int i;
    char NameExt[32];
    char DiskBufName1[256];

    if((DiskBuf =
                (DiskBuf_t *)calloc(NumDatastreams, sizeof(DiskBuf_t))) == NULL)
        return(false);

    for(i = 0; i < NumDatastreams; i++) {

        if((DiskBuf[i].Message =  (char *)calloc(1, StringSize)) == NULL)
            return(false);

        // have to do this after allocating message area!
        if(Verbose) {
            sprintf(DiskBuf[i].Message, "Initializing disk buffer %ld... ", i);
        }

        DiskBuf[i].Ready = false;

        if((DiskBuf[i].Name =  (char *)calloc(1, StringSize)) == NULL)
            return(false);

        if((DiskBuf[i].DoneLink =  (char *)calloc(1, StringSize)) == NULL)
            return(false);

        if((DiskBuf[i].ThisLink =  (char *)calloc(1, StringSize)) == NULL)
            return(false);

        DiskBuf[i].Function             = DiskBufFunc;
        DiskBuf[i].ThreadLive           = true;
        DiskBuf[i].Num                  = d_NumBufs;
        DiskBuf[i].AtBuf                = 0;
        DiskBuf[i].BaseName             = (char *)DBaseName;
        DiskBuf[i].Index                = i;
        DiskBuf[i].Idle                 = true;
        DiskBuf[i].QuickLook            = false; 
        DiskBuf[i].QLFile               = 0;
        DiskBuf[i].QLFileSkipCounter    = 0;
        DiskBuf[i].QLFileBufCounter     = 0;
        DiskBuf[i].IdleCount            = 0;
        if(Headers) {
            DiskBuf[i].Size = (r_BufSize + HeaderSize) * num_m_in_d;
            DiskBuf[i].MmapSize = (r_BufSize + HeaderSize) * num_r_in_m;
        } else {
            DiskBuf[i].Size = r_BufSize * num_m_in_d;
            DiskBuf[i].MmapSize     = r_BufSize * num_r_in_m;
        }

        if(Verbose) {
            strcat(DiskBuf[i].Message, "\tOK\n");
            WriteLog(DiskBuf[i].Message, ToUser);
        }
    }

    return(0);
}
//======================================================
void * DiskBufFunc(void * dsip) {
//======================================================

    int retval;
    int dsi;      // datastream index
    struct sched_param SchedParam;

    //long m_in_d     = 0;
    long r_AtBuf    = 0;
    long d_AtBuf    = 0;
            
    long overrun_count = 0;
    long overrun_max   = 10;

    struct timespec nanotime;
    dsi = *(int *)dsip;

    //fprintf(stderr, "disk dsi = %d\n", dsi);
    PrintThreadInfo("DiskBuf", dsi, DiskBuf[dsi].ThreadId, &DiskBuf[dsi].ThreadAttr);
    //pthread_exit(NULL);

    timeb ThisReadyTime, LastReadyTime, ThisSynthTime;
    //double ThisReadySec, LastReadySec;
    long ThisSynthFreq, ThisSkyFreq;
    off_t OffsetSave;

    bool MissingBufs = false;
    bool Idle        = false;
    bool WasIdle     = false;
    caddr_t MmapSave;
    long i;
    long DataSeq = 0;
    double x, y;
    long thismmapsize = 0;
    long totalsize = 0;
    long spaceavailable;
    long spaceneeded; 
    long ThisBufferSize;

    retval = pthread_attr_getschedparam(&DiskBuf[dsi].ThreadAttr, &SchedParam);
    if(retval) ErrExit("DiskBufFunc: Could not get scheduling param \n");
    //if(Verbose) PrintThreadInfo(DiskBuf[dsi].Message, "DiskBufFunc");

    ThisReadyTime.time = 0;
    LastReadyTime.time = 0;

    AdvanceDiskBuffer(0, dsi);                    // get first disk buffer

    DiskBuf[dsi].Ready = true;

    // main disk thread loop
    while(DiskBuf[dsi].ThreadLive) {
        // wait on next ring buffer timestamp segment
        if(pthread_mutex_lock(&MemBuf[dsi].Mutex[r_AtBuf])) ErrExit("DiskBufFunc : mutex lock error \n");
        if(!DiskBuf[dsi].ThreadLive) break;
        ThisReadyTime = MemBuf[dsi].ReadyTime[r_AtBuf];
        ThisSynthTime = MemBuf[dsi].SynthTime[r_AtBuf];
        ThisSynthFreq = MemBuf[dsi].SynthFreq[r_AtBuf];
        ThisSkyFreq   = MemBuf[dsi].SkyFreq[r_AtBuf];
        if(pthread_mutex_unlock(&MemBuf[dsi].Mutex[r_AtBuf])) ErrExit("DiskBufFunc : mutex unlock error \n");

        if(VeryVerbose) {
            sprintf(DiskBuf[dsi].Message,
                    "DSK[%d] : ringbuf num is %ld ringbuf pointer is %p ringbuf time is %lf\n",
                    dsi, r_AtBuf, MemBuf[dsi].ReadyPtr[r_AtBuf], 
                    ThisReadyTime.time + ThisReadyTime.millitm/1000.0);
            WriteLog(DiskBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
        }

        DataSeq = MemBuf[dsi].DoneCount; 

        MissingBufs = RingOverRunCheck(ThisReadyTime, LastReadyTime, dsi);
        if (MissingBufs) {
            overrun_count++;
            sprintf(DiskBuf[dsi].Message,
                    "DSK[%d] : RING BUFFER OVERRUN !!  %lf  %lf %lf\n",
                    dsi, 
                    ThisReadyTime.time + ThisReadyTime.millitm/1000.0,
                    LastReadyTime.time + LastReadyTime.millitm/1000.0,
                    MemBuf[dsi].AcquireTime);
            WriteLog(DiskBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
        }

#if 0
        fprintf(stderr, "%d : %lf %lf %lf %lf %d\n", 
                dsi,  
                MemBuf[dsi].ReadyTime[0].time+(MemBuf[dsi].ReadyTime[0].millitm/1000.0),
                MemBuf[dsi].ReadyTime[1].time+(MemBuf[dsi].ReadyTime[1].millitm/1000.0),
                MemBuf[dsi].ReadyTime[2].time+(MemBuf[dsi].ReadyTime[2].millitm/1000.0),
                MemBuf[dsi].ReadyTime[3].time+(MemBuf[dsi].ReadyTime[3].millitm/1000.0), 
                r_AtBuf);
#endif

        if (RingSegCorruptCheck(ThisReadyTime, r_AtBuf, dsi)) {
            overrun_count++;
            MissingBufs = true;
        } else {
            if(pthread_mutex_lock(&IdleWatch.Mutex)) ErrExit("DiskBufFunc : idle watch mutex lock error \n");
            Idle = DiskBuf[dsi].Idle;
            if(pthread_mutex_unlock(&IdleWatch.Mutex)) ErrExit("DiskBufFunc : idle watch mutex unlock error \n");
            if(Idle) {
                DiskBuf[dsi].IdleCount++;
                WasIdle = true;
            } else {

                // Good to go...
            
                // write the header and copy data to the disk buffer
                // ... but first save our place in case the copy is bad
                //OffsetSave = lseek(DiskBuf.Fd, 0L, SEEK_CUR);
                
                
                MmapSave = DiskBuf[dsi].MmapCurrent;
                // save the size we are writing in case it gets overwritten at some point
                ThisBufferSize = MemBuf[dsi].Size[r_AtBuf];
                if(Verbose || DataSeq % 10 == 0) {
                sprintf(DiskBuf[dsi].Message, "DSK[%d] : FrameSeq is %ld   DataSeq is %ld   IdleCount is %d   Corrected Diff is %d\n",
                        dsi, FrameHeader[dsi].SequenceNum, DataSeq, DiskBuf[dsi].IdleCount, 
                        DataSeq-FrameHeader[dsi].SequenceNum-DiskBuf[dsi].IdleCount);
                WriteLog(DiskBuf[dsi].Message, ToUser+ToLogFile);
                }
            
                if(Headers) {
                    if(WasIdle) MissingBufs = true;

                    FormatHeader(DiskBuf[dsi].Fd,
                                ThisBufferSize, 
                                ThisReadyTime,
                                ThisSynthTime,
                                ThisSynthFreq,
                                ThisSkyFreq,
                                MissingBufs,
                                DataSeq,
                                dsi);
                    
                    //bounds checking on the mmap
                    if(DiskBuf[dsi].ThreadLive && thismmapsize+HeaderSize>DiskBuf[dsi].MmapSize)
                    {
                        spaceneeded = thismmapsize+HeaderSize-DiskBuf[dsi].MmapSize;
                        spaceavailable = DiskBuf[dsi].MmapSize-thismmapsize;
                        
                        //copy as much as we can
                        memcpy(DiskBuf[dsi].MmapCurrent,
                               FrameHeader[dsi].Block,
                               spaceavailable);
                        thismmapsize+=spaceavailable;
                        DiskBuf[dsi].MmapCurrent += spaceavailable;
                        
                        //advance the buffer
                        totalsize += thismmapsize;
                        AdvanceDiskBuffer(totalsize, dsi);
                        if(totalsize >= DiskBuf[dsi].Size) totalsize = 0;
                        thismmapsize=0;
                        
                        //make sure the thread wasn't killed while we attempted to advance the disk buffer
                        if(DiskBuf[dsi].ThreadLive)
                        {
                            //copy the rest
                            memcpy(DiskBuf[dsi].MmapCurrent,
                                   FrameHeader[dsi].Block+spaceavailable,       //Block is a char *, need to advance the pointer to where we left off
                                   spaceneeded);
                            thismmapsize+=spaceneeded;
                            DiskBuf[dsi].MmapCurrent += spaceneeded;
                        }
                        
                    }
                    else if(DiskBuf[dsi].ThreadLive)
                    {
                        memcpy(DiskBuf[dsi].MmapCurrent,
                            FrameHeader[dsi].Block,
                            HeaderSize);
                        thismmapsize+= HeaderSize;
                        DiskBuf[dsi].MmapCurrent += HeaderSize;
                    }
                    

                    WasIdle = false;
                }

                FrameHeader[dsi].SequenceNum++; 
                
                //bounds checking on the mmap
                //this may cause partial data to be written
                if(DiskBuf[dsi].ThreadLive && thismmapsize+ThisBufferSize>DiskBuf[dsi].MmapSize)
                {
                    spaceneeded = thismmapsize+ThisBufferSize-DiskBuf[dsi].MmapSize;
                    spaceavailable = DiskBuf[dsi].MmapSize-thismmapsize;
                    
                    //copy as much as we can
                    memcpy(DiskBuf[dsi].MmapCurrent, MemBuf[dsi].ReadyPtr[r_AtBuf], spaceavailable);
                    thismmapsize+=spaceavailable;
                    DiskBuf[dsi].MmapCurrent += spaceavailable;
                    
                    //advance the buffer
                    totalsize += thismmapsize;
                    AdvanceDiskBuffer(totalsize, dsi);
                    if(totalsize >= DiskBuf[dsi].Size) totalsize = 0;
                    thismmapsize=0;
                    
                    //make sure the thread wasn't killed while we attempted to advance the disk buffer
                    if(DiskBuf[dsi].ThreadLive)
                    {
                        //copy the rest
                        memcpy(DiskBuf[dsi].MmapCurrent, MemBuf[dsi].ReadyPtr[r_AtBuf]+spaceavailable, spaceneeded);
                        thismmapsize+=spaceneeded;
                        DiskBuf[dsi].MmapCurrent += spaceneeded;
                    }
                }
                else if(DiskBuf[dsi].ThreadLive)
                {
                    memcpy(DiskBuf[dsi].MmapCurrent, MemBuf[dsi].ReadyPtr[r_AtBuf], ThisBufferSize);
                    DiskBuf[dsi].MmapCurrent += ThisBufferSize;          // this may extend beyond
                                                                                    // the disk buffer but
                                                                                    // will get fixed in 
                                                                                    // AdvanceDiskBuffer()
                    thismmapsize += ThisBufferSize;
                }
                fprintf(stderr, "writing %d bytes to buffer\n", ThisBufferSize);
                fprintf(stderr, "current size is %d total size is %d\n", thismmapsize, totalsize);

                WriteQuickLookData(dsi, r_AtBuf);

                // see if we overwrote *while* copying to disk buffer.
                // this may rarely return a false positive in the case
                // where EDT just started into the just read buffer.
                if (RingSegCorruptCheck(ThisReadyTime, r_AtBuf, dsi)) {
                    //DiskBuf[dsi].MmapCurrent = MmapSave;
                    //thismmapsize -= (MemBuf[dsi].Size[r_AtBuf]+HeaderSize);
                    sprintf(DiskBuf[dsi].Message, "DSK[%d] : RING BUFFER OVERRUN DURING BUFFER COPY !!\n", dsi);
                    WriteLog(DiskBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
                    overrun_count++;
                    MissingBufs = true;     // will show up in next header
                } else {
                    MissingBufs = false;    // successful write - re-init
                }

                // full mmap ?  go to next
//                if(thismmapsize>=DiskBuf[dsi].MmapSize) {
//                    totalsize += thismmapsize;
//                    AdvanceDiskBuffer(totalsize, dsi);
//                    if(totalsize >= DiskBuf[dsi].Size) totalsize = 0;
//                    thismmapsize=0;
//                }
            }
        }

        // prep for next ring buf
        LastReadyTime = ThisReadyTime; 
        r_AtBuf = r_AtBuf == MemBuf[dsi].Num - 1 ? 0 : r_AtBuf + 1;

        // full mmap ?  go to next
        //if(m_in_d % num_r_in_m == 0 && m_in_d != 0) {     // data idling could result in  m_in_d == 0
        //    AdvanceDiskBuffer(m_in_d, dsi);
        //    if(m_in_d >= num_m_in_d) m_in_d = 0;
        //}

        if(overrun_count > overrun_max) SayToExit();

    }  // end main diskthread loop

    if(Verbose) {
        sprintf(DiskBuf[dsi].Message, "DSK[%d] : Exiting\n", dsi);
        WriteLog(DiskBuf[dsi].Message, ToUser);
    }

    pthread_exit(NULL);
}

//======================================================
void WriteQuickLookData(int dsi, int r_AtBuf) {
//======================================================

    bool QuickLookFlag;
    char QLFileName[StringSize];

    if(pthread_mutex_lock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex lock error \n");
    QuickLookFlag = DiskBuf[dsi].QuickLook;
    if(pthread_mutex_unlock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex unlock error \n");

    if(QuickLookFlag || DiskBuf[dsi].QLFile) {
        if(!DiskBuf[dsi].QLFile) {
            sprintf(QLFileName, "%s%d", QuickLook.BaseName, dsi);
            DiskBuf[dsi].QLFile = open(QLFileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
            if(DiskBuf[dsi].QLFile == -1) ErrExit("QuickLook : unable to open file \n");
            DiskBuf[dsi].QLFileSkipCounter = 0;
            DiskBuf[dsi].QLFileBufCounter  = 0;
        }
        if(DiskBuf[dsi].QLFileSkipCounter < r_NumBufs) {
            DiskBuf[dsi].QLFileSkipCounter++;
        } else {
            if(DiskBuf[dsi].QLFileBufCounter >= QuickLook.NumBufs) {
                sprintf(QuickLook.Message, "DSK[%d] : turning quicklook flag OFF\n", dsi);
                WriteLog(QuickLook.Message, ToUser);
                close(DiskBuf[dsi].QLFile);
                DiskBuf[dsi].QLFile = 0;
                if(pthread_mutex_lock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex lock error \n");
                DiskBuf[dsi].QuickLook = false;
                if(pthread_mutex_unlock(&QuickLook.Mutex)) ErrExit("QuickLook : mutex unlock error \n");
            } else {
                write(DiskBuf[dsi].QLFile, FrameHeader[dsi].Block, HeaderSize);
                write(DiskBuf[dsi].QLFile, MemBuf[dsi].ReadyPtr[r_AtBuf], MemBuf[dsi].Size[r_AtBuf]);
                DiskBuf[dsi].QLFileBufCounter++;
            }
        }
    }
}


//======================================================
inline bool RingOverRunCheck(timeb ThisReadyTime, timeb LastReadyTime, int dsi) {
//======================================================
// check for ring buf overrun - we ask if the current ready time
// indicates that we have gone all the way around the ring since
// the last ready time.

    double ThisReadySec, LastReadySec;
    bool OverRun = false;

    ThisReadySec = ThisReadyTime.time + double(ThisReadyTime.millitm/1000.0);
    LastReadySec = LastReadyTime.time + double(LastReadyTime.millitm/1000.0);
    if(Verbose) {
        sprintf(DiskBuf[dsi].Message, "DSK[%d] : ThisReadySec is %lf  LastReadySec is %lf  Diff is %lf\n",
                dsi, ThisReadySec, LastReadySec, ThisReadySec - LastReadySec);
        WriteLog(DiskBuf[dsi].Message, ToUser);
    }
    if(ThisReadySec > LastReadySec                          +
            MemBuf[dsi].AcquireTime * (MemBuf[dsi].Num + 1) -
            MemBuf[dsi].AcquireTimeFudge                    &&
            LastReadyTime.time != 0) {
        OverRun = true;
        //sprintf(DiskBuf[dsi].Message,
        //        "DSK[%d] : RING BUFFER OVERRUN !!  %lf  %lf %lf\n",
        //        dsi, ThisReadySec, LastReadySec, MemBuf[dsi].AcquireTime);
        //WriteLog(DiskBuf[dsi].Message, ToConsole+ToLogFile+ToUser);
    }

    return (OverRun);
}


//======================================================
inline bool RingSegCorruptCheck(timeb& ThisReadyTime, int r_AtBuf, int dsi) {
//======================================================

    timeb BufReadyTime;
    double ThisReadySec, BufReadySec;
    bool Corrupt = false;

    if(pthread_mutex_lock(&MemBuf[dsi].Mutex[r_AtBuf])) ErrExit("DiskBufFunc : mutex lock error \n");
    BufReadyTime = MemBuf[dsi].ReadyTime[r_AtBuf];
    if(pthread_mutex_unlock(&MemBuf[dsi].Mutex[r_AtBuf])) ErrExit("DiskBufFunc : mutex unlock error \n");
    
    ThisReadySec = ThisReadyTime.time + double(ThisReadyTime.millitm/1000.0);
    BufReadySec  = BufReadyTime.time + double(BufReadyTime.millitm/1000.0);

    // if the time has changed, EDT has written *through*
    // the current buf and so we are corrupt.
    if (ThisReadySec != BufReadySec) {
        Corrupt = true;
        ThisReadyTime = BufReadyTime;   // correct the ready time
    } else {
        // else, if EDT is currently at our current buf,
        // then it is currently writing *into* it and we
        // are corrupt
        if ((edt_done_count(MemBuf[dsi].EdtDevice) % MemBuf[dsi].Num) == r_AtBuf) {
            Corrupt = true;
        }
    }

    return (Corrupt);
}


//=======================================================
void AdvanceDiskBuffer(long totalsize, int dsi) {
//=======================================================
// This function advances the memory mapped portion of
// the disk file associated with the calling disk buffer
// thread.  Also, once the mmap has reached the end of
// the configured disk file that file is closed and it's
// name  appended with the string "done".  A new disk file
// is then opened and the mmap positioned at the beginning.
// m_in_d is the number of memory buffers that are in
// current disk buffer so  far.

    //static int d_AtBuf = 0;
    int fd;
    //char NameExt[16];
    //static char ThisName[512];   // this is thread safe, right?

    // if we have an active (full) mmap (ie, not the first time in).
    if(totalsize) {
        if(VeryVerbose) {
            sprintf(DiskBuf[dsi].Message, "DSK[%d] : Advancing mmap...\n", dsi);
            WriteLog(DiskBuf[dsi].Message, ToUser);
        }

        // sync, unlock, and unmap the full mmap
        // NEED TO CHECK RETURN VALUES HERE
        if(msync(DiskBuf[dsi].MmapBase, DiskBuf[dsi].MmapSize, MS_SYNC)) {
            ErrExit("AdvanceDiskBuffer : Msync failed \n");
        }
        if(munlock((void *)DiskBuf[dsi].MmapBase, DiskBuf[dsi].MmapSize)) {
            ErrExit("AdvanceDiskBuffer : Memory unlock failed for disk file \n");
        }
        if(munmap(DiskBuf[dsi].MmapBase, DiskBuf[dsi].MmapSize)) {
            ErrExit("AdvanceDiskBuffer : Memory unmap failed for disk file \n");
        }

        // if we still have room in this disk file, simply advance the mmap
        if(totalsize < DiskBuf[dsi].Size) {
            DiskBuf[dsi].Offset += DiskBuf[dsi].MmapSize;
            // ... otherwise close this disk file and set the next mmap to start at offset 0
        } else {
            if(VeryVerbose) {
                sprintf(DiskBuf[dsi].Message, "DSK[%d] : Advancing disk...\n", dsi);
                WriteLog(DiskBuf[dsi].Message, ToUser);
            }
            close(DiskBuf[dsi].Fd);                                 // close it
            link(DiskBuf[dsi].ThisLink, DiskBuf[dsi].DoneLink);     // mark it done
            unlink(DiskBuf[dsi].ThisLink);                          // unlink it
            DiskBuf[dsi].AtBuf =
                DiskBuf[dsi].AtBuf == DiskBuf[dsi].Num - 1 ? 0 : DiskBuf[dsi].AtBuf + 1; // next
            DiskBuf[dsi].Offset = 0;
        }
    }

    // if we need a new disk file...
    if(DiskBuf[dsi].Offset == 0) {

        sprintf(DiskBuf[dsi].ThisLink, "%s_ds%d_%d",  DBaseName, dsi, DiskBuf[dsi].AtBuf);   // name next buffer
        sprintf(DiskBuf[dsi].DoneLink, "%s.done", DiskBuf[dsi].ThisLink);

        // Check to see if this disk file name already exists and if it does,
        // spin until some other process deletes it.
        while(DiskBuf[dsi].ThreadLive) {                                // check if it's active
            if((fd = open(DiskBuf[dsi].DoneLink, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1) {
                if(errno == EEXIST) {                           // active...
                    sprintf(DiskBuf[dsi].Message,
                            "DSK[%d] : Disk buffer %s is marked done but still exists\n",
                            dsi, DiskBuf[dsi].DoneLink);
                    WriteLog(DiskBuf[dsi].Message, ToUser);

                    sleep(1);                               //  sleep and retry
                    continue;			

                } else {
                    ErrExit("AdvanceDiskBuffer : DiskBufFunc : Cannot check done link \n");
                }
            } else {                                                // not active,
                close(fd);                                      //  close and carry on
                unlink(DiskBuf[dsi].DoneLink);
                break;
            }
        }
        if(!DiskBuf[dsi].ThreadLive) return;

        // open and size the new disk file
        DiskBuf[dsi].Fd = open(DiskBuf[dsi].ThisLink, O_RDWR | O_CREAT | O_TRUNC, 0666); // open it
        if(DiskBuf[dsi].Fd == -1) {
            sprintf(DiskBuf[dsi].Message, "AdvanceDiskBuffer : Cannot open disk file %s \n", DiskBuf[dsi].ThisLink);
            ErrExit(DiskBuf[dsi].Message);
        } else {
            sprintf(DiskBuf[dsi].Message, "DSK[%d] : New disk buffer %s is now open\n", dsi, DiskBuf[dsi].ThisLink);
            WriteLog(DiskBuf[dsi].Message, ToUser);
        }
        if(ftruncate(DiskBuf[dsi].Fd, DiskBuf[dsi].Size)) {
            ErrExit("AdvanceDiskBuffer : Cannot size disk file \n");
        }

    }  // end if we need a new disk file

    // mmap first/next area
    DiskBuf[dsi].MmapBase = (caddr_t)mmap(        0,
                            DiskBuf[dsi].MmapSize,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            DiskBuf[dsi].Fd,
                            DiskBuf[dsi].Offset);
    if(DiskBuf[dsi].MmapBase == MAP_FAILED) {
        ErrExit("AdvanceDiskBuffer : Memory mapping failed for disk file \n");
    }
    if(mlock((void *)DiskBuf[dsi].MmapBase, DiskBuf[dsi].MmapSize)) {
        ErrExit("AdvanceDiskBuffer : Memory lock failed for disk file \n");
    }
    DiskBuf[dsi].MmapCurrent = DiskBuf[dsi].MmapBase;

    //fprintf(stderr, ">>> advanced disk %p : ...\n", DiskBuf.MmapCurrent);

}


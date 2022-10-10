/*
TODO
    - include DSI as a parameter to seti_GetDr2Data()
    - parse the headers for DSI and header/data lengths
*/

#include <vector>

const int DataSize    = 1024*1024;
const int HeaderSize  = 4096;
const int               BufSize = HeaderSize + DataSize;
const int               BufWords = BufSize/2;
const int               HeaderWords = HeaderSize/2;

template<class T>
int GetDr2Data(int fd, int channel, long numsamples, std::vector<T> &c_samples) {
//  fd          device that contains tape data 
//  channel     the channel (0-15) from which you wish to extract
//  numsamples  the number of samples you wish to extract
//  c_samples   a std vector of type T onto which numsamples will be pushed.
//  return val  0 == success, 1 == failure

    static unsigned short   buf[BufSize/2];       // BufSize is in bytes
    static int              buf_i=0;              // will index an array of 16 bit words (short ints)
    size_t                  readbytes;
    unsigned short          dataword;
    long                    samplecount = 0, i = 0;

    // Keep reading BufSize bytes worth of data until we have
    // numsamples 2 bit samples.  Note that numsamples might be,
    // but does not have to be, satisfied with one read.
    while (samplecount < numsamples) {
   
        // read the next buffer if need be
        if (buf_i == 0) {
            if ((readbytes = read(fd, buf, BufSize)) != BufSize) break;
            buf_i = HeaderWords;         // first word past the header
        }

        for (;
             buf_i < BufWords && samplecount < numsamples;
             buf_i += 1, samplecount += 1) {

            // isolate the desired channel
            dataword = buf[buf_i] >> channel*2;
            dataword &= (unsigned short)0x0003;

            // push the complex sample onto the vector
            c_samples.push_back(dataword & (unsigned short)0x0001 ? 1 : -1);
            c_samples.push_back(dataword & (unsigned short)0x0002 ? 1 : -1);
        }

        if (buf_i >= BufWords) {
            buf_i = 0;    // we'll need to read the next buffer next time 'round
        }
    }

    if(samplecount < numsamples) {
        return(1);
    } else {
        return(0);
    }
}


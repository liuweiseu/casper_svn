#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>


#define FILENAME "largefile0"
#define BUFFER_SIZE 1024
#define HEADER_STRING "HEADER\n"
#define HEADER_SIZE_STRING "HEADER_SIZE "
#define DATA_SIZE_STRING "DATA_SIZE "
#define CORRUPT_STRING "MISSED "
#define DR_HEAER_SIZE 1024*4
#define MAX_DATA_SIZE 1024*1024
#define HEADER_SIZE 8
#define ERROR_BUF_SIZE 1024

int find_first_header(int fd);
int read_header(char * header);
void read_data(char * data, int datasize);

typedef struct data_vals{
    unsigned int raw_data;
    unsigned int overflow_cntr;
}data_vals;

int main()
{
    int fd;
    ssize_t numbytes;
    int compare = 1;
    off_t offset = 0;
    char error_buf[ERROR_BUF_SIZE];
   
    int headersize=DR_HEAER_SIZE;
    int next_buffer_size=1;
    int datasize=next_buffer_size;

    char *header = (char *) malloc(DR_HEAER_SIZE);
    char *data = (char *) malloc(MAX_DATA_SIZE);
    
    fd = open(FILENAME, O_RDONLY);
    
    //check for error opening file
    if(fd==-1)
    {
        snprintf(error_buf, ERROR_BUF_SIZE, "Error opening file %s", FILENAME);
        perror(error_buf);
        return 0;
    }
    
    //if we can't find a header, nothing to be done
    if(find_first_header(fd)==-1) return 0;
  
    //make sure we are reading as much data as expected, otherwise might be at eof
    while(headersize==DR_HEAER_SIZE && datasize==next_buffer_size)
    {
        headersize = read(fd, (void *) header, DR_HEAER_SIZE);
        next_buffer_size = read_header(header);
        //in case we are at EOF
        datasize = read(fd, (void *) data, next_buffer_size);
        //doesn't do any bounds checking yet...
        if(headersize==DR_HEAER_SIZE && datasize==next_buffer_size)
        {
            read_data(data, datasize);
        }
        else if(headersize==DR_HEAER_SIZE)
        {
            fprintf(stderr, "EOF found in data can't print full spectra\n");
        }
        else
        {
            fprintf(stderr, "EOF found in header\n");
        }
        offset = lseek(fd, 0, SEEK_CUR);
        fprintf(stderr, "Now at %d\n", offset);
    }
    
    close(fd);
    
    return 0;
}


//moves the fd to the beginning of the first header in the file
int find_first_header(int fd)
{
    int i;
    int header_not_found=1;
    int header_location=0;
    //we should be able to find the first header within DR_HEAER_SIZE+MAX_DATA_SIZE+strlen(HEADER_STRING)
    char *buffer = (char *) malloc(MAX_DATA_SIZE+DR_HEAER_SIZE+strlen(HEADER_STRING));
    
    read(fd, (void *) buffer, MAX_DATA_SIZE+DR_HEAER_SIZE+strlen(HEADER_STRING));
          
    for(i=0; header_not_found && i<DR_HEAER_SIZE+MAX_DATA_SIZE; i++)
    {
        header_not_found=strncmp(buffer+i, HEADER_STRING, strlen(HEADER_STRING));
        if(!header_not_found)
        {
            // need to match HEADER_SIZE also then are other instances where HEADER will occur in the header
            header_not_found = strncmp(buffer+i+strlen(HEADER_STRING)+1, HEADER_SIZE_STRING, strlen(HEADER_SIZE_STRING));
            fprintf(stderr, buffer+i);
            fprintf(stderr, buffer+i+strlen(HEADER_STRING)+1);
        }
        header_location=i;
    }
    if(header_not_found)
    {
        fprintf(stderr, "Error finding header in %s\n", FILENAME);
        return -1;
    }
    else
    {
        fprintf(stderr, "header found at %d\n", header_location);
        lseek(fd, header_location, SEEK_SET);
        return 0;
    }
}

int read_header(char * header)
{
    //fprintf(stderr, header);
    int i;
    int datasize;
    for(i=0; i<DR_HEAER_SIZE; )
    {
        //fprintf(stderr, header+i);
        if(strncmp(header+i, DATA_SIZE_STRING, strlen(DATA_SIZE_STRING))==0)
        {
            sscanf(header+i, "DATA_SIZE %d\n", &datasize);
            fprintf(stderr, "found data size %d at %d\n", datasize, i);
        }
        //look for corrupt data
        if(strncmp(header+i, CORRUPT_STRING, strlen(CORRUPT_STRING))==0)
        {
            fprintf(stderr, header+i);
        }
        i+=strlen(header+i)+1;
    }
    
    return datasize;
}

static unsigned int slice(unsigned int value,unsigned int width,unsigned int offset){
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}

void read_data(char * data, int datasize)
{
    long record_count = ((long *) data)[0];
    long beam_no = ((long *) data)[1];
    int i;
    
    data_vals *records;
    unsigned int fields;
    
    unsigned int pfb_bin = 0;
    unsigned int fft_bin = 0;
    unsigned int over_thresh = 0;
    unsigned int blank = 0;
    unsigned int event = 0;
    unsigned int pfb_fft_power = 0;
    
    fprintf(stderr, "record ct %ld beam no %ld\n", record_count, beam_no);
    
    fprintf(stderr, &data[16]);
    
    fprintf(stderr, "\n\n");
    records = (data_vals *) (data+8+8+512);
    
    for(i=0;i<record_count;i++)
    {
        fields = ntohl((records+i)->raw_data);
        fft_bin = slice(fields,15,0);
        pfb_bin = slice(fields,12,15);
        over_thresh = slice(fields,1,27);
        blank = slice(fields,3,28);
        event = slice(fields,1,31);
        pfb_fft_power = ntohl((records+i)->overflow_cntr); 
//        if(pfb_fft_power>50)
//        {
//            fprintf(stderr, "Record %u pfb bin %u fft bin %u power %u\n", i, pfb_bin, fft_bin, pfb_fft_power);
//        }
    }

}

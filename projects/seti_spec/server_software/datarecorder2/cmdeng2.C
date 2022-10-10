#include <stdio.h>   
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h>
#include <fcntl.h>   
#include <sys/ioctl.h>   
#include <errno.h>   
#include <termios.h> 

#include "serial.h"

int main(int argc, char ** argv) {

    int i, j, fd, CommandLength, bytesread, retval, bytes;
    char LocalString[256];
    char CommandString[256];
    char num[2] = {'9', '\0'};

    fd = OpenSerialPort("/dev/ttyS2");

exit(0);

    strcpy(CommandString, "!0RA");

    CommandLength = strlen(CommandString);
    CommandString[CommandLength++] = 1;

    fprintf(stderr, "Sending %d bytes to engineering module\n", CommandLength);
    bytes = write(fd, (void *)CommandString, CommandLength);
    if (bytes != CommandLength) {
        fprintf(stderr, "Write of command string (%d bytes) failed\n", bytes);
    } else {
        fprintf(stderr, "Wrote %d bytes.  Waiting on response from engineering module...\n", bytes);
    }
    
    fprintf(stderr, "eng 0 : ");
    for(i=0; i < 256; i++) {
        retval = read(fd, (void*)&LocalString[i], 1);
        if (retval == -1) {
            fprintf(stderr, "errno = %d\n", errno);
            perror(NULL);
            return(1);
        } else {
	    j = LocalString[i];
            fprintf(stderr, "%x ", j);
        }
    }

    putchar('\n');

    if(close(fd)) fprintf(stderr, "Bad serial port close\n");
}

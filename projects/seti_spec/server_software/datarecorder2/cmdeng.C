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

    int i, j, fd, CommandLength=0, bytesread, retval, bytes;
    char LocalString[256];
    char CommandString[256];
    char char_in, high_channel;

    high_channel = atoi(argv[1]);

    fd = OpenSerialPort("/dev/ttyS2");

/*
    CommandString[0] = '!';    
    bytes = write(fd, (void *)&CommandString[0], 1);
    CommandLength++;

    CommandString[1] = '0';     
    bytes = write(fd, (void *)&CommandString[1], 1);
    CommandLength++;

    CommandString[2] = 'R';     
    bytes = write(fd, (void *)&CommandString[2], 1);
    CommandLength++;

    CommandString[3] = 'A';      
    bytes = write(fd, (void *)&CommandString[3], 1);
    CommandLength++;

    CommandString[4] = high_channel;     
    bytes = write(fd, (void *)&CommandString[4], 1);
    CommandLength++;
*/

    strcpy(CommandString, "!0RA");
    CommandLength = strlen(CommandString);
    CommandString[CommandLength] = high_channel;
    CommandLength++;
    bytes = write(fd, (void *)CommandString, CommandLength);

    fprintf(stderr, "Sending %d bytes to engineering module : %x %x %x %x %x\n", 
        CommandLength, CommandString[0], CommandString[1], CommandString[2], CommandString[3], CommandString[4]);
    
    fprintf(stderr, "eng : ");
    alarm(5);
    for(i=0; i < ((high_channel+1)*2); i++) {
        retval = read(fd, (void*)&char_in, 1);
        if (retval == -1) {
            fprintf(stderr, "errno = %d\n", errno);
            perror(NULL);
            return(1);
        } else {
            fprintf(stderr, "%d (0x%x) ", char_in, char_in);
        }
    }
    alarm(0);

    putchar('\n');

    if(close(fd)) fprintf(stderr, "Bad serial port close\n");
}

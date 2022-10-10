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

    int i, j, fd, CommandLength, bytesread, retval, bytes, Loops=1;
    char LocalString[256];
    char CommandString[256];
    bool Step = false;

    long Steps[5] = {15830000, 16840000, 17820000, 18850000, 19810000};

    fd = OpenSerialPort("/dev/ttyS0");

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-q")) {
            strcpy(CommandString, "Q#");
        } else if (!strcmp(argv[i], "-f")) {
            sprintf(CommandString, "%s0%s%s", "F", argv[++i], "#");
        } else if (!strcmp(argv[i], "-fk")) {
            sprintf(CommandString, "%s%s0000%s", "F", argv[++i], "#");
        } else if (!strcmp(argv[i], "-fm")) {
            sprintf(CommandString, "%s%s0000000%s", "F", argv[++i], "#");
        } else if (!strcmp(argv[i], "-step")) {
            Step = true;
            Loops = 5;
        } else if (!strcmp(argv[i], "-c")) {
            strcpy(CommandString, argv[++i]);
        } else {
            fprintf(stderr, "unrecognized arg: %s\n", argv[i]);
            exit(1);
        }
    }

    for(j=0; j < Loops; j++) {

        if(Step) sprintf(CommandString, "F0%ld0#", Steps[j]);

        CommandLength = strlen(CommandString);

        fprintf(stderr, "Sending %s to PTS232\n", CommandString);
        bytes = write(fd, (void *)CommandString, CommandLength);
        if (bytes != CommandLength) {
            fprintf(stderr, "Write of command string (%d bytes) failed\n", bytes);
        } else {
            fprintf(stderr, "Wrote %d bytes.  Waiting on response from PTS232...\n");
        }

        for(i=0; i < sizeof(LocalString)-1; i++) {
            //while(1) {
            //	ioctl(fd, FIONREAD, &bytes);
            //	fprintf(stderr, "bytes ready: %d\n", bytes);
            //	if(bytes != 0) break;
            //}
            retval = read(fd, (void*)&LocalString[i], 1);
            if (retval == -1) {
                fprintf(stderr, "errno = %d\n", errno);
                perror(NULL);
                return(1);
            } else {
                putchar(LocalString[i]);
                if(LocalString[i] == '>') break;
            }
        }
        putchar('\n');

        if(Step) sleep(3);
    }

    if(close(fd)) fprintf(stderr, "Bad serial port close\n");

}

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

//======================================================
int OpenSerialPort(char * Device) {
//======================================================
    int fd; /* File descriptor for the port */
    char perror_str[512];
    struct termios options;


    fd = open(Device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        sprintf(perror_str, "OpenSerialPort: Unable to open %s ", Device);
        perror(perror_str);
    } else {
        fcntl(fd, F_SETFL, 0);
        tcflush(fd, TCIOFLUSH);
    }


    if((tcgetattr(fd, &options)) != 0)
        perror(NULL);

    // set speed to 9600
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // set to 8N1
    options.c_cflag &= ~CSIZE;      // clear all bits currently specifying char size
    options.c_cflag |= CS8;         // 8 char bits
    options.c_cflag &= ~PARENB;     // no parity
    options.c_cflag &= ~CSTOPB;     // 2 stop bits is OFF, ie there will be 1 stop bit

    // flow control is OFF
    options.c_iflag |= (IXON | IXOFF | IXANY);

    // canonical mode, echo, echo erase, signal gen are OFF
    // to support raw (not line oriented) input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // receiver is ON so that we can read data from the port
    options.c_cflag |= CREAD;

    if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
        perror(NULL);


    return (fd);
}


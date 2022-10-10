#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include "serial.h"

int main() {

    int i, fd, retval;
    struct termios options;
    char LocalString[256];

    LocalString[0] = '\0';

    fd = OpenSerialPort("/dev/ttyS0");

    /*
      tcgetattr(fd, &options);
      cfsetispeed(&options, B9600);
      cfsetospeed(&options, B9600);
      options.c_cflag |= (CLOCAL | CREAD);
      options.c_cflag &= ~PARENB;
      options.c_cflag &= ~CSTOPB;
      options.c_cflag &= ~CSIZE;
      options.c_cflag |= CS8;
      tcsetattr(fd, TCSANOW, &options);
    */

    if((tcgetattr(fd, &options)) != 0)
        perror(NULL);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_iflag |= (ICANON | ECHO | ISIG);
    options.c_iflag |= (IXON | IXOFF | IXANY);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= CREAD;
    if((tcsetattr(fd, TCSAFLUSH, &options)) != 0)
        perror(NULL);


    printf("%d\n", fd);

    retval = write(fd, "Q#", 2);
    if (retval < 0) fputs("write() of 3 bytes failed!\n", stderr);
    printf("%d\n", retval);

    fcntl(fd, F_SETFL, FNDELAY);

    sleep(3);

    retval = read(fd, LocalString, 200);

    if (retval == -1) {
        fprintf(stderr, "errno = %d\n", errno);
        perror(NULL);
    } else {
        fprintf(stderr, "%d bytes read : %s\n", retval, LocalString);
    }

    close(fd);

}

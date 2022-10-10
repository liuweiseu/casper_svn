/* PocketSpec WalletChain v. 0.01                   */
/* One ET in the Pocket is worth two in the sky.    */
/* Daniel Chapman, Henry Chen, Christina DeJesus,   */ 
/* Pierre Yves-Droz, Aaron Parsons, Andrew Siemion  */
/* SSL, BWRC, CASPER Collaboration  -- October 2005 */
/* dependencies: gnuplot 4.0 (www.gnuplot.org)      */
/*               strong intestinal fortitude        */

#include <stdio.h>     /* Standard input/output definitions */
#include <string.h>    /* String function definitions */
#include <unistd.h>    /* UNIX standard function definitions */
#include <fcntl.h>     /* File control definitions */
#include <errno.h>     /* Error number definitions */
#include <termios.h>   /* POSIX terminal control definitions */
#include <signal.h>    /*Signal functions*/
#include <sys/time.h>  /* Time... is on my side... */
#include <time.h>      /* Yes it is */

char specport[100]="/dev/ttyS0";
#define BUFFER_SIZE 100000

int main(int argc, const char * argv[])
{
  char buffer[BUFFER_SIZE];
  int fd;      
  int result;
  int bufferCounter;

  if(argc != 2){
    printf("Wrong number of arguments.\nPlease supply the accumulation value.\n");
    return 0;
  }

  system("stty -F /dev/ttyS0 ispeed 115200 ospeed 115200 rows 0 columns 0 line 0 intr ^C quit '^\' kill ^U eof ^D start ^Q stop ^S susp ^Z rprnt ^R werase ^W lnext ^V flush ^O min 1 time 5 -parenb -parodd cs8 hupcl -cstopb cread clocal -crtscts ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke");

  fd = open(specport, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
  if (fd == -1){
    printf("open_port: Unable to open port: %s\n", specport);
    return 0;
  }
  else{
    printf("Serial port up...\n");
  }
  
  write(fd, "\n", 1);
  printf("Searching for percent ... \n");
  do{
    result = read(fd, buffer, 1);
    if (result == 1) {
      //      printf("%c", buffer[0]);
    }
  }while(buffer[0] != '%');
  printf("Found percent.\n");

  sprintf(buffer, "setacc %s\n", argv[1]);
  write(fd, buffer, strlen(buffer));
  strcpy(buffer, "");

  bufferCounter = 0;
  do{
    result = read(fd, &buffer[bufferCounter], 1);
    if (result == 1) {
      //printf("%c", buffer[bufferCounter]);
      bufferCounter++;
    }
  }while(buffer[bufferCounter-1] != '%');

  printf("IBOB%%%s\n", buffer);

  close(fd);
  exit(0);
}

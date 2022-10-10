#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sysexits.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>

#define YMODEM_TX_BIN "/usr/local/bin/lsb.exe"
#define YMODEM_BIN_NAME "lsb"

#define TERM_SPEED B115200

int setup_serial(char *filename)
/* returns -1 on fail, serial FD otherwise */
{
  int stty_fd;
  struct termios term_prev,term_curr;

  if ((stty_fd=open(filename, O_RDWR)) < 0){
    fprintf(stderr,"open error: %s\n",strerror(errno));
    return -1;
  }

  if (!isatty(stty_fd)) {
    fprintf(stderr,"error: %s is not a tty\n", filename);
    close(stty_fd);
    return -1;
  }

  term_curr.c_cc[VMIN]=1; /*1 char at a time*/
  term_curr.c_cc[VTIME]=0;
  term_curr.c_ispeed=TERM_SPEED;
  term_curr.c_ospeed=TERM_SPEED;
  term_curr.c_cflag = CREAD|CLOCAL|CS8|TERM_SPEED;
  term_curr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP |INLCR|IGNCR|ICRNL|IXON);
  term_curr.c_oflag &= ~OPOST;
  term_curr.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  term_curr.c_cflag &= ~(CSIZE|PARENB);
  term_curr.c_cflag |= CS8;

  if (tcsetattr(stty_fd,TCSANOW,&term_curr)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return -1;
  }

  return stty_fd;
}

int exec_xfer_app(int stty_fd, char *app, char **argv)
{
  pid_t pid;
  pid = fork();
  switch (pid) {
  case -1:
    close(stty_fd);
    return EX_OSERR;
  case 0:

    if(dup2(stty_fd, STDOUT_FILENO) != STDOUT_FILENO){
      exit(EX_OSERR);
    }

    if(dup2(stty_fd, STDIN_FILENO) != STDIN_FILENO){
      exit(EX_OSERR);
    }

    execvp(app, argv);
    exit(EX_UNAVAILABLE);
    return -1;
  default :

    waitpid(pid, NULL, 0); 
    close(stty_fd);
 
    return EX_OK;
  }
}

void print_usage(void)
{
  printf("usage: xyzmodem_tx serial_tty txfile executable [executable_arguments ...]\n");
}

char* get_exec_name(char *filename)
{
  char *ret;
  if (!(ret = strrchr(filename, '/'))){
    /* if no '/' is found return the whole filename */
    return filename;
  }
  /* otherwise return the pointer from the last occurance + 1*/
  return (ret + 1);
}

int main(int argc, char **argv){
  int stty_fd;
  char* xfer_app_argv[1 + (argc < 5 ? 0 : argc - 4) + 1]; 
  int i;
  /* the xfer app argv contains: the name of the executable, the executable arguments and a NULL */

  if (argc < 4) {
    print_usage();
    return -1;
  }

  if ((stty_fd = setup_serial(argv[1])) < 0){
    return -1;
  }
  /* assemble argv for transfer application */
  xfer_app_argv[0] = get_exec_name(argv[3]);
  for (i=0; i < argc - 4; i++){
    xfer_app_argv[1 + i] = argv[4 + i];
  }
  xfer_app_argv[1 + argc - 4] = argv[2];
  xfer_app_argv[1 + argc - 4 + 1] = NULL;

  return exec_xfer_app(stty_fd, argv[3], xfer_app_argv);
}

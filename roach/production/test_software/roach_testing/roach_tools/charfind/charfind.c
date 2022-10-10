#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define BUFFER 2000

int main(int argc, char **argv){
  int i;
  unsigned char val;
  unsigned char data[BUFFER];
  int ret;
  struct termios term_prev,term_curr;
  int stdin_fd = STDIN_FILENO;

  if (argc != 2) {
    printf("usage: charfind char\n");
    return -1;
  }
  if (argv[1][0] == '\0') {
    printf("error: got empty argument\n");
    return -1;
  }
  val = 0;
  if (strlen(argv[1]) != 1) {
    if (strlen(argv[1]) == 4 && argv[1][0] == '\\'){
      for (i=0; i < 3; i++){
        if (argv[1][i+1] < 48 || argv[1][i+1] > 55){
          printf("error: invalid format ?\n");
          return -1;
        } else {
          val += (argv[1][i+1] - 48) * (1 << (3*(2-i)));
        }
      }
    } else {
      printf("error: invalid format\n");
      return -1;
    }
  } else {
    val = argv[1][0];
  }
    if (tcgetattr(stdin_fd, &term_prev)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    return -1;
  }

  memcpy(&term_curr, &term_prev, sizeof(struct termios));

  term_curr.c_cc[VMIN]=1; /*1 char at a time*/
  term_curr.c_cc[VTIME]=0;
  term_curr.c_cflag = CREAD|CLOCAL|CS8;
  term_curr.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP |INLCR|IGNCR|ICRNL|IXON);
  term_curr.c_oflag &= ~OPOST;
  term_curr.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  term_curr.c_cflag &= ~(CSIZE|PARENB);
  term_curr.c_cflag |= CS8;


  if (tcsetattr(stdin_fd,TCSANOW,&term_curr)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    return -1;
  }

  while (1) {
    ret = read(stdin_fd, data, BUFFER);

    if (ret < 0) {
      fprintf(stderr, "file read error\n");
      tcsetattr(stdin_fd,TCSANOW,&term_prev);
      return -1;
    }
    
    if (ret == 0){
      fprintf(stderr, "error: end of file\n");
      tcsetattr(stdin_fd,TCSANOW,&term_prev);
      return -1;
    }
    for (i=0; i < ret; i++) {
      if (data[i] == val) {
        tcsetattr(stdin_fd,TCSANOW,&term_prev);
        return 0;
      }
    }
  }
}

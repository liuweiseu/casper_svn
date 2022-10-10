#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <string.h>
#include <unistd.h>
#include <sysexits.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <ctype.h>
#include <pty.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#define TERM_SPEED B115200

#define BUFFER 1200

int setup_serial(char *filename)
/* returns -1 on fail, serial FD otherwise */
{
  int stty_fd, i;
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

  if (tcgetattr(stty_fd, &term_prev)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return -1;
  }

  memset(&term_curr, '\0', sizeof(struct termios));
  /* a wtf cygwin hack */
  if (tcsetattr(stty_fd,TCSANOW,&term_curr)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return -1;
  }

  term_curr.c_cflag |= (CS8 | CLOCAL);
  //term_curr.c_cflag |= (CS8 | TERM_SPEED);

  term_curr.c_cc[VMIN]=0; /*1 char at a time*/
  term_curr.c_cc[VTIME]=0;
  term_curr.c_ispeed=TERM_SPEED;
  term_curr.c_ospeed=TERM_SPEED;
  cfsetispeed(&term_curr, TERM_SPEED);
  cfsetospeed(&term_curr, TERM_SPEED);

  if (tcsetattr(stty_fd,TCSANOW,&term_curr)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return -1;
  }
  return stty_fd;

  /* OOPS - I was going back right away
  if (tcsetattr(stty_fd,TCSANOW,&term_prev)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return EX_OSERR;
  }
  */
}

int create_symlink(char* target, char* destination)
{
  int ret;
  ret = symlink(target, destination);
  if (ret < 0){
    switch (errno){
      case EEXIST: 
    #ifdef DEBUG
        fprintf(stderr, "syslink exists, attempting to delete it\n");
    #endif
        if (unlink(destination)){
          fprintf(stderr, "warning: failed to delete existing symlink - %s\n", strerror(errno));
          break;
        }
        if ((ret = symlink(target, destination))){
          fprintf(stderr, "warning: could not create symlink again - %s\n", strerror(errno));
        }
        break;
      default:
        fprintf(stderr, "warning: could not create symlink to %s - %s\n", destination, strerror(errno));
        break;
    }
  }
  if (!ret)
    printf("xportd: symbolic link created to pty at %s\n", target);

  return ret;
}

int setup_ptys(int* tx_fd, int* rx_fd, char* tx_filename, char *rx_filename)
{
  int tx_sfd, rx_sfd;
  char tbuf[BUFFER];

  struct termios term_pts;

  term_pts.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  term_pts.c_oflag &= ~OPOST;
  term_pts.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  term_pts.c_cflag &= ~(CSIZE | PARENB);
  term_pts.c_cflag |= CS8;
  term_pts.c_cc[VTIME]=0;
  term_pts.c_cc[VMIN]=0;

  if(openpty(tx_fd, &tx_sfd, tbuf, &term_pts, NULL) < 0){
    fprintf(stderr, "error: unable to open tx pty: %s\n", strerror(errno));
    return -1;
  }
  printf("created tx pty at %s\n", tbuf);
  create_symlink(tbuf, tx_filename);

  term_pts.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  term_pts.c_oflag &= ~OPOST;
  term_pts.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  term_pts.c_cflag &= ~(CSIZE | PARENB);
  term_pts.c_cflag |= CS8;
  term_pts.c_cc[VTIME]=0;
  term_pts.c_cc[VMIN]=1;

  if(openpty(rx_fd, &rx_sfd, tbuf, &term_pts, NULL) < 0){
    fprintf(stderr, "error: unable to open rx pty: %s\n", strerror(errno));
    close(*(tx_fd));
    close(tx_sfd);
    return -1;
  }
  printf("created rx pty at %s\n", tbuf);
  create_symlink(tbuf, rx_filename);

  return 0;
}

void loop(int serial_fd, int in_fd, int out_fd)
{
  char tbuf[BUFFER];
  struct timeval tv;
  int tr, max;
  fd_set fsr, fsw;
  int ret;

  FD_ZERO(&fsr);
  FD_ZERO(&fsw);

  max = (serial_fd > in_fd) ? serial_fd : in_fd;
  max++;

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  FD_SET(serial_fd, &fsr);
  FD_SET(in_fd, &fsr);

  /* could add to write set in case things start to block */

  while( (ret=select(max, &fsr, &fsw, NULL, &tv)) >= 0){

    if(FD_ISSET(in_fd, &fsr)){
      tr = read(in_fd, tbuf, BUFFER);
#ifdef DEBUG
      fprintf(stderr, "got %d pts bytes\n", tr);
#endif
      if(tr > 0){
        write(serial_fd, tbuf, tr);
#ifdef DEBUG
        fprintf(stderr, "wrote %d serial bytes\n", tr);
#endif
      }
    }

    if(FD_ISSET(serial_fd, &fsr)){
      tr = read(serial_fd, tbuf, BUFFER);
#ifdef DEBUG
      fprintf(stderr, "got %d serial bytes\n", tr);
#endif
      if(tr > 0){
        write(out_fd, tbuf, tr);
#ifdef DEBUG
        fprintf(stderr, "wrote %d pts bytes\n", tr);
#endif
      }
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_SET(serial_fd, &fsr);
    FD_SET(in_fd, &fsr);
    
  }
  close(serial_fd);
  close(in_fd);
  close(out_fd);
  
}

void print_usage(void)
{
  printf("usage: serial_ptys serial_tty serial_tty_in serial_tty_out\n");
}

int main(int argc, char **argv){
  int stty_fd;
  int in_fd, out_fd;

  if (argc != 4) {
    print_usage();
    return -1;
  }

  if ((stty_fd = setup_serial(argv[1])) < 0){
    return -1;
  }

  if (setup_ptys(&in_fd, &out_fd, argv[2], argv[3]) < 0){
    return -1;
  }
  loop(stty_fd, in_fd, out_fd);

  return 0;
}

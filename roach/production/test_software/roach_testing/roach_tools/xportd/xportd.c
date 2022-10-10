#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER 1200
//#define DEBUG 
#define TTY_FILE "/tmp/tty_rmon"

static int ufd, tfd, xfd;

void cleanup(int i)
{
  close(ufd);
  close(tfd);
  close(xfd);
}

int loop(char *host, int port)
{
  fd_set fsr, fsw;
  int reuse;
  int ret, max, ur, tr, us;
  char ubuf[BUFFER], tbuf[BUFFER];
  struct timeval tv;
  struct sockaddr_in sa;

  struct termios term_pts;

  term_pts.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  term_pts.c_oflag &= ~OPOST;
  term_pts.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  term_pts.c_cflag &= ~(CSIZE | PARENB);
  term_pts.c_cflag |= CS8;
  term_pts.c_cc[VMIN]=1;

  /* could set "speed", etc in here */
  if(openpty(&tfd, &xfd, tbuf, &term_pts, NULL) < 0){
    fprintf(stderr, "xportd: unable to open pty: %s\n", strerror(errno));
    return -1;
  }

  printf("xportd: created pty %s\n", tbuf);

  ret = symlink(tbuf, TTY_FILE);
  if (ret < 0){
    switch (errno){
      case EEXIST: 
    #ifdef DEBUG
        fprintf(stderr, "syslink exists, attempting to delete it\n");
    #endif
        if (unlink(TTY_FILE)){
          fprintf(stderr, "warning: failed to delete existing symlink - %s\n", strerror(errno));
          break;
        }
        if ((ret = symlink(tbuf, TTY_FILE))){
          fprintf(stderr, "warning: could not create symlink again - %s\n", strerror(errno));
        }
        break;
      default:
        fprintf(stderr, "warning: could not create symlink to %s - %s\n", TTY_FILE, strerror(errno));
        break;
    }
  }
  if (!ret)
    printf("xportd: symbolic link created to pty at %s\n", TTY_FILE);


  ufd = socket(AF_INET, SOCK_STREAM, 0);
  if(ufd < 0){
    fprintf(stderr, "xportd: unable to create socket: %s\n", strerror(errno));
    return -1;
  }

  reuse = 1;
  if (setsockopt(ufd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))){
    printf("warning: setsockopt failed - %d %s\n", ret, strerror(errno));
  }

  reuse = 0;
  struct linger linger;
  linger.l_onoff = 1; 
  linger.l_linger = 0; 
  if (setsockopt (ufd, SOL_SOCKET, SO_LINGER, &linger, sizeof (linger))){
    printf("warning: setsockopt failed - %d %s\n", ret, strerror(errno));
  }

  /* WARNING: assumes the same port on both sides */

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(ufd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
    fprintf(stderr, "xportd: unable to bind port %d: %s\n", port, strerror(errno));
    return -1;
  }

#ifdef DEBUG
  fprintf(stderr, "xportd: bound udp port %d\n", port);
#endif

  /* populate the server IP */

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  if(inet_aton(host, &(sa.sin_addr)) == 0){
    fprintf(stderr, "xportd: unable to connect: <%s> does not parse as IP\n", host);
    return -1;
  }

  if(connect(ufd, (struct sockaddr *)&sa, sizeof(sa))){
    fprintf(stderr, "xportd: unable to connect to %s:%d: %s\n", host, port, strerror(errno));
    return -1;
  }

  FD_ZERO(&fsr);
  FD_ZERO(&fsw);

  max = (ufd > tfd) ? ufd : tfd;
  max++;

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  FD_SET(ufd, &fsr);
  FD_SET(tfd, &fsr);

  us = 0;

  /* could add to write set in case things start to block */

  while(select(max, &fsr, &fsw, NULL, &tv) >= 0){
    if(FD_ISSET(ufd, &fsr)){
      ur = recv(ufd, ubuf, BUFFER, 0);
#ifdef DEBUG
      fprintf(stderr, "got %d udp bytes\n", ur);
#endif
      if(ur > 0){
        write(tfd, ubuf, ur);
#ifdef DEBUG
        fprintf(stderr, "wrote %d tty bytes\n", ur);
#endif
      } else {
        fprintf(stderr, "error: got 0 sized packet\n");
        break;
      }
    }

    if(FD_ISSET(tfd, &fsr)){
      tr = read(tfd, tbuf, BUFFER);
#ifdef DEBUG
      fprintf(stderr, "got %d tty bytes\n", tr);
#endif
      if(tr > 0){
        write(ufd, tbuf, tr);
#ifdef DEBUG
        fprintf(stderr, "wrote %d udp bytes\n", tr);
#endif
      }
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_SET(ufd, &fsr);
    FD_SET(tfd, &fsr);
  }
  close(ufd);
  close(tfd);
  close(xfd);

  return -1;
}

int main(int argc, char **argv)
{
   int port;

   if(argc <= 2){
     fprintf(stderr, "xportd: usage %s remote_address remote_port\n", argv[0]);
     return 1;
   }

   port = atoi(argv[2]);
   if(port <= 0){
     fprintf(stderr, "xportd: port <%s> should be a nonzero integer\n", argv[2]);
     return 1;
   }
   signal(SIGTERM, &cleanup);

   return (loop(argv[1], port) < 0) ? 1 : 0;
}

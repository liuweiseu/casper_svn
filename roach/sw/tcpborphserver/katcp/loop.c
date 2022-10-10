
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "katpriv.h"
#include "katcp.h"
#include "netc.h"

int run_katcp(struct katcp_dispatch *d, int server, char *host, int port)
{
  if(server == 0){
    return run_client_katcp(d, host, port);
  } else{
    return run_server_katcp(d, host, port);
  }
}

int run_client_katcp(struct katcp_dispatch *d, char *host, int port)
{
  int fd, result;
  fd_set fsr, fsw;

#if 0
  d->d_exit = KATCP_EXIT_ABORT;
#endif

  fd = net_connect(host, port, 0);
  if(fd < 0){
    return terminate_katcp(d, KATCP_EXIT_ABORT);
  }

  exchange_katcp(d, fd);

  while((result = exited_katcp(d)) == KATCP_EXIT_NOTYET){

    FD_ZERO(&fsr);
    FD_ZERO(&fsw);

    FD_SET(fd, &fsr);
    if(flushing_katcp(d)){ /* only write data if we have some */
#ifdef DEBUG
      fprintf(stderr, "client: want to flush data\n");
#endif
      FD_SET(fd, &fsw);
    }

    result = select(fd + 1, &fsr, &fsw, NULL, NULL);
    switch(result){
      case -1 :
        switch(errno){
          case EAGAIN :
          case EINTR  :
            continue; /* WARNING */
          default  :
            return terminate_katcp(d, KATCP_EXIT_ABORT);
        }
        break;
#if 0
      case  0 :
        return 0;
#endif
    }

    if(FD_ISSET(fd, &fsr)){
      result = read_katcp(d);
      if(result){
        terminate_katcp(d, KATCP_EXIT_HALT);
        if(result < 0){
          return KATCP_EXIT_HALT;
        }
      }
    }

    result = dispatch_katcp(d);
    if(result < 0){
      return terminate_katcp(d, KATCP_EXIT_ABORT);
    }

    if(FD_ISSET(fd, &fsw)){
      result = write_katcp(d);
      if(result < 0){
        return terminate_katcp(d, KATCP_EXIT_ABORT);
      }
    }
  }

  return result;
}

int run_server_katcp(struct katcp_dispatch *d, char *host, int port)
{
  int lfd, fd, nfd, max, result;
  unsigned int len;
  struct sockaddr_in sa;
  fd_set fsr, fsw;

#if 0
  d->d_exit = KATCP_EXIT_ABORT;
#endif

  lfd = net_listen(host, port, 0);
  if(lfd < 0){
    return terminate_katcp(d, KATCP_EXIT_ABORT);
  }

  nfd = (-1);
  fd = (-1);

  /* WARNING: may also need to test for a quit */
  while((result = exited_katcp(d)) == KATCP_EXIT_NOTYET){

    FD_ZERO(&fsr);
    FD_ZERO(&fsw);

    max = (-1);

#if 0
    /* rather track things explicitly */
    fd = fileno_katcl(d->d_line);
#endif

    if(fd >= 0){
      if(flushing_katcp(d)){ /* if flushing, don't exchange data */
#ifdef DEBUG
        fprintf(stderr, "client: want to flush data\n");
#endif
        FD_SET(fd, &fsw);
      } else {
        if(nfd >= 0){ /* if new connection and everything flushed, then exchange */
          exchange_katcp(d, nfd);
          fd = nfd;
          nfd = (-1);
          on_connect_katcp(d);
        }
      }
      FD_SET(fd, &fsr); /* always be prepared to read data */
      if(fd > max){
        max = fd;
      }
    }

    if(nfd < 0){ /* if no new connection pending, try to accept */
      FD_SET(lfd, &fsr);
      if(lfd > max){
        max = lfd;
      }
    }

    result = select(max + 1, &fsr, &fsw, NULL, NULL);
    switch(result){
      case -1 :
        switch(errno){
          case EAGAIN :
          case EINTR  :
            continue; /* WARNING */
          default  :
            return terminate_katcp(d, KATCP_EXIT_ABORT);
        }
        break;
#if 0
      case  0 :
        return 0;
#endif
    }
 
    if(fd >= 0){
      result = 0;
      if(FD_ISSET(fd, &fsr)){
        if(read_katcp(d)){
          result = (-1);
        }
      }
      if(nfd < 0){ /* stop processing if new connection pending */
        if(dispatch_katcp(d) < 0){
          result = (-1);
        }
      }
      if(result < 0){
        exchange_katcp(d, -1);
        fd = (-1);
#if 0
      } else {
        if(problem_katcl(d->d_line)){

        }
#endif
      }
    }

    if(FD_ISSET(lfd, &fsr)){
#ifdef DEBUG
      if(nfd >= 0){
        fprintf(stderr, "server: major logic failure, accepting with already pending file descriptor\n");
      }
#endif
      len = sizeof(struct sockaddr_in);
      nfd = accept(lfd, (struct sockaddr *) &sa, &len);
      if(nfd >= 0){
        if(fd >= 0){
          on_disconnect_katcp(d, "displaced by %s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
        } else {
          /* exchange immediately if no old connection */
          exchange_katcp(d, nfd);
          fd = nfd;
          nfd = (-1);
          on_connect_katcp(d);
        }
      }
    }

    if(fd >= 0){
      if(FD_ISSET(fd, &fsw)){
        if(write_katcp(d) < 0){
          exchange_katcp(d, -1);
          fd = (-1);
        }
      }
    }
  }

  if(fd >= 0){
#ifdef DEBUG
    fprintf(stderr, "server: final io flush\n");
#endif

    on_disconnect_katcp(d, NULL); /* for null paramater decode exit code */

    flush_katcp(d);
  } 

  return result;
}

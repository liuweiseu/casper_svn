#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define XPORT_GPIO_PORT 0x77F0
#define COMMAND_SIZE    0x9
#define RESPONSE_SIZE   0x5


int xport_conf(char *host, char* conf_data)
{
  fd_set fsr, fsw;
  int ret;
  int max, ufd, ur, us;
  struct timeval tv;
  struct sockaddr_in sa;
  int port = XPORT_GPIO_PORT;
  char resp_data[RESPONSE_SIZE];
  int i;

  ufd = socket(AF_INET, SOCK_DGRAM, 0);
  if(ufd < 0){
    fprintf(stderr, "xport_gpio: unable to create socket: %s\n", strerror(errno));
    return -1;
  }

  /* WARNING: assumes the same port on both sides */

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(ufd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
    fprintf(stderr, "xport_gpio: unable to bind port %d: %s\n", port, strerror(errno));
    return -1;
  }

#ifdef DEBUG
  fprintf(stderr, "xport_gpio: bound udp port %d\n", port);
#endif

  /* populate the server IP */

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  if(inet_aton(host, &(sa.sin_addr)) == 0){
    fprintf(stderr, "xport_gpio: unable to connect: <%s> does not parse as IP\n", host);
    return -1;
  }

  if(connect(ufd, (struct sockaddr *)&sa, sizeof(sa))){
    fprintf(stderr, "xport_gpio: unable to connect to %s:%d: %s\n", host, port, strerror(errno));
    return -1;
  }

  if (write(ufd, conf_data, 9) < 0){
    fprintf(stderr, "xport_gpio: tx failed");
    close(ufd);
    return -1;
  }

  FD_ZERO(&fsr);
  FD_ZERO(&fsw);

  max = ufd;
  max++;

  tv.tv_sec = 1;
  tv.tv_usec = 0;

  FD_SET(ufd, &fsr);

  us = 0;

  /* could add to write set in case things start to block */

  if((ret = select(max, &fsr, &fsw, NULL, &tv)) > 0){
    ur = recv(ufd, resp_data, RESPONSE_SIZE, 0);
    if (ur != 5){
      fprintf(stderr, "error: expected 5 bytes back, got %d\n", ur);
      close(ufd);
      return -1;
    }
    for (i=0; i < RESPONSE_SIZE; i++){
      printf("%2x",resp_data[i]);
      if (i < RESPONSE_SIZE - 1)
        printf(" ");
    }
    printf("\n");
    close(ufd);
    return 0;
  } else if (ret){
    fprintf(stderr, "error: select failed\n");
    close(ufd);
    return -1;
  } else {
    fprintf(stderr, "error: no response from xport\n");
    close(ufd);
    return -1;
  }
}

int get_conf(char* conf_file, char* buffer)
{
  int fd, ret;
  if ((fd = open(conf_file, O_RDONLY)) < 0) {
    fprintf(stderr, "error: open failed - %s\n", strerror(errno));
    return -1;
  } else {
    if ((ret = read(fd, buffer, COMMAND_SIZE)) < 0) {
      fprintf(stderr, "error: read failed - %s\n", strerror(errno));
      close(fd);
      return -1;
    } else if (ret < COMMAND_SIZE) {
      fprintf(stderr, "error: could not read 9 config bytes\n");
      close(fd);
      return -1;
    }
  }
  close(fd);
  return 0;
}

int main(int argc, char **argv)
{
   char config_data [COMMAND_SIZE];

   if(argc != 3){
     fprintf(stderr, "usage: xport_gpio ip conf_file\n");
     return 1;
   }

   if (get_conf(argv[2], config_data)) {
     return 1;
   }

   if (xport_conf(argv[1], config_data)) {
     return 1;
   }

   return 0;
}

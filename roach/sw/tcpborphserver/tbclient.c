/* A commandline utility allowing one to send commands as arguments, 
 * where replies return "ok" strings, these are converted to exit
 * codes, in an attempt to make scripting a bit easier
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/select.h>
#include <sys/time.h>

#include "netc.h"
#include "katcp.h"
#include "katcl.h"

void usage(char *app)
{
  printf("usage: %s [options] command [args]\n", app);
  printf("-h          this help\n");
  printf("-v          increase verbosity\n");
  printf("-q          run quietly\n");
  printf("-s          specify server:port\n");
  printf("-t seconds  set timeout\n");
  printf("-r          toggle printing of reply messages\n");
  printf("-i          toggle printing of inform messages\n");

  printf("return codes:\n");
  printf("0     command completed successfully\n");
  printf("1     command failed\n");
  printf("2     other errors\n");

  printf("environment variables:\n");
  printf("KATCP_SERVER default server (overridden by -s option)\n");
}

int main(int argc, char **argv)
{
  char *app, *server, *match, *ptr;
  int i, j, c;
  int verbose, result, status, fd, base, run, info, reply, display, max, prefix, timeout;
  struct katcl_line *l;
  fd_set fsr, fsw;
  struct timeval tv;
  
  server = getenv("KATCP_SERVER");
  if(server == NULL){
    server = "localhost";
  }
  
  info = 1;
  reply = 1;
  verbose = 1;
  i = j = 1;
  app = argv[0];
  base = (-1);
  timeout = 5;

  while (i < argc) {
    if (argv[i][0] == '-') {
      c = argv[i][j];
      switch (c) {

        case 'h' :
          usage(app);
          return 0;

        case 'v' : 
          verbose++;
          j++;
          break;
        case 'q' : 
          verbose = 0;
          info = 0;
          reply = 0;
          j++;
          break;
        case 'i' : 
          info = 1 - info;
          j++;
          break;
        case 'r' : 
          reply = 1 - reply;
          j++;
          break;

        case 's' :
        case 't' :

          j++;
          if (argv[i][j] == '\0') {
            j = 0;
            i++;
          }
          if (i >= argc) {
            fprintf(stderr, "%s: argument needs a parameter\n", app);
            return 2;
          }

          switch(c){
            case 's' :
              server = argv[i] + j;
              break;
            case 't' :
              timeout = atoi(argv[i] + j);
              break;
          }

          i++;
          j = 1;
          break;

        case '-' :
          j++;
          break;
        case '\0':
          j = 1;
          i++;
          break;
        default:
          fprintf(stderr, "%s: unknown option -%c\n", app, argv[i][j]);
          return 2;
      }
    } else {
      base = i;
      i = argc;
    }
  }

  if(base < 0){
    fprintf(stderr, "%s: need a command to send (use -h for help)\n", app);
    return 2;
  }

  status = 1;

  fd = net_connect(server, 0, verbose);
  if(fd < 0){
    return 2;
  }

  l = create_katcl(fd);
  if(l == NULL){
    fprintf(stderr, "%s: unable to allocate state\n", app);
    return 2;
  }

  i = base;
  match = NULL;
  switch(argv[i][0]){
    case KATCP_REQUEST :
      match = argv[i] + 1;
      /* FALL */
    case KATCP_INFORM :
    case KATCP_REPLY  :
      print_katcl(l, 0, "%s", argv[i]);
    break;
    default :
      match = argv[i];
      print_katcl(l, 0, "%c%s", KATCP_REQUEST, argv[i]);
    break;
  }
  i++;
  while(i < argc){
    print_katcl(l, 0, " %s", argv[i]);
    i++;
  }
  print_katcl(l, 0, "\n");

  if(match){
    for(prefix = 0; (match[prefix] != '\0') && (match[prefix] != ' '); prefix++);
#ifdef DEBUG
    fprintf(stderr, "debug: checking prefix %d of %s\n", prefix, match);
#endif
  }

  /* WARNING: logic a bit intricate */

  for(run = 1; run;){

    FD_ZERO(&fsr);
    FD_ZERO(&fsw);

    if(match){ /* only look for data if we need it */
      FD_SET(fd, &fsr);
    }

    if(flushing_katcl(l)){ /* only write data if we have some */
      FD_SET(fd, &fsw);
    }

    tv.tv_sec  = timeout;
    tv.tv_usec = 0;

    result = select(fd + 1, &fsr, &fsw, NULL, &tv);
    switch(result){
      case -1 :
        switch(errno){
          case EAGAIN :
          case EINTR  :
            continue; /* WARNING */
          default  :
            return 2;
        }
        break;
      case  0 :
        if(verbose){
          fprintf(stderr, "%s: no io activity within %d seconds\n", app, timeout);
        }
        /* could terminate cleanly here, but ... */
        return 2;
    }

    if(FD_ISSET(fd, &fsw)){
      result = write_katcl(l);
      if(result < 0){
      	fprintf(stderr, "%s: write failed: %s\n", app, strerror(error_katcl(l)));
      	return 2;
      }
      if((result > 0) && (match == NULL)){ /* if we finished writing and don't expect a match then quit */
      	run = 0;
      }
    }

    if(FD_ISSET(fd, &fsr)){
      if(read_katcl(l) < 0){
      	fprintf(stderr, "%s: read failed: %s\n", app, strerror(error_katcl(l)));
      	return 2;
      }
    }

    while(have_katcl(l) > 0){
      ptr = arg_string_katcl(l, 0);
      if(ptr){
        display = 0;
      	switch(ptr[0]){
          case KATCP_INFORM : 
            display = info;
            break;
          case KATCP_REPLY : 
            display = reply;
            if(match){
              if(strncmp(match, ptr + 1, prefix) || ((ptr[prefix + 1] != '\0') && (ptr[prefix + 1] != ' '))){
      	        fprintf(stderr, "%s: warning, encountered nonmatching reply <%s>\n", app, ptr);
              } else {
              	ptr = arg_string_katcl(l, 1);
              	if(ptr && !strcmp(ptr, KATCP_OK)){
              	  status = 0;
                }
              	run = 0;
              }
            }
            break;
          case KATCP_REQUEST : 
      	    fprintf(stderr, "%s: warning, encountered an unanswerable request <%s>\n", app, ptr);
            break;
          default :
            fprintf(stderr, "%s: read malformed message <%s>\n", app, ptr);
            break;
        }
        if(display){
          max = arg_count_katcl(l);
          for(i = 0; i < max; i++){
            ptr = arg_string_katcl(l, i);
            if(ptr == NULL){
              fprintf(stderr, "%s: argument %u of %u unavailable\n", app, i, max);
              return 2; /* should not happen */
            }
            fputs(ptr, stdout);
            fputc(((i + 1) == max) ? '\n' : ' ' , stdout);
          }
        }
      }
    }
  }

  destroy_katcl(l, 1);

  return status;
}

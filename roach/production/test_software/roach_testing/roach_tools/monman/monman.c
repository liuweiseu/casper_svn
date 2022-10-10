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
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "monman.h"

#define STR_MAX 512

/* Global Configuration Bits */

static int display_comments;
static int response_byte;

/***** Supporting Functions ******/

char hexnibble_to_chr(char nib)
{
  if (nib < 10) {
    return 48 + nib;
  } else if (nib < 16) {
    return 97 + nib - 10; //lower case ascii
  } else {
    return 'f';
  }
}
int str_n_prepend_chr(char *str, int N, char chr)
{
  if (strlen(str) < N - 2){ 
    memmove(&str[1], &str[0],strlen(str) + 1);
    str[0]=chr;
  }
  return 0;
}
int str_n_append_chr(char *str, int N, char chr)
{
  if (strlen(str) < N - 2){ 
    str[strlen(str)+1] = '\0';
    str[strlen(str)] = chr;
  }
  return 0;
}
/* convert hex string to  a byte array */
int hexstr_bytes(const char *str, unsigned char* data, int num_bytes){
  int cnt=0;
  char localstr[3];
  memset((void*)data, 0, num_bytes);
  while (cnt < num_bytes && cnt*2 < strlen(str)){
    if (strlen(str) - cnt*2 == 1) { //special case last byte
      localstr[0]=str[0];
      localstr[1]='\0';
      data[cnt]=(unsigned char) strtol(localstr,NULL,16);
    } else {
      memcpy(localstr,&(str[strlen(str) - cnt*2 - 2]),2);
      localstr[2]='\0';
      data[cnt]=(unsigned char) strtol(localstr,NULL,16);
    }
    cnt++;
  }
  return 0;
}

/* convert a byte array to a hex string */
int bytes_hexstr(unsigned char* data, int num_bytes,char *str, int N){
  int i;
  int got_a_val=0;
  unsigned char val;
  str[0]='\0';
  for (i=0; i < num_bytes; i++){
    val=data[num_bytes - i - 1];
    str_n_append_chr(str, N, hexnibble_to_chr((val & 0xf0) >> 4));
    str_n_append_chr(str, N, hexnibble_to_chr(val & 0x0f));
    /*if (got_a_val || val != 0){
      
      if (got_a_val || val > 0xf) { //dont print first nibble if it's 0 and haven't written anything yet
        str_n_append_chr(str, N, hexnibble_to_chr((val & 0xf0) >> 8));
      }
      str_n_append_chr(str, N, hexnibble_to_chr(val & 0x0f));
    } */
    if (val != 0) {
      got_a_val = 1;
    }
  }
  return 0;
}

int bytes_int(unsigned char* data, int num_bytes){
  int i;
  int ret=0;
  for (i=0; i < num_bytes; i++){
    ret=ret +( data[i] << (8*i));
  }
  return ret;
}

int get_hex_word(int input_fd, int num_bytes, unsigned char* data)
{
  int i=0;
  int ret;
  char c;
  char buff[STR_MAX];
  buff[0]='\0';
  do{
    if ((ret=read(input_fd,&c,1)) < 0){
      fprintf(stderr,"stdin read error: %s\n",strerror(errno));
      return -1;
    } else if (ret < 1) {
      fprintf(stderr,"term eof?\n");
      return -1;
    } else {
      if (strchr("0123456789abcdefABCDEF",(int)c)){
        str_n_append_chr((char*)buff,STR_MAX,c);
        printf("%c",c);
        fflush(stdout);
        i++;
      } else if (strchr("\n\r ",(int)c)){
        break;
      } else {
        fprintf(stderr,"warning: invalid character ignored\n");
      }
    }
  }while(i < num_bytes*2);
  printf("\n");
  hexstr_bytes(buff,data,num_bytes);
  return 0;
}


int safe_write(int fd, const void *buf, int count) {
  int ret;
  int new_count=count;
#ifdef DEBUG
  printf("writing %d bytes\n", count);
#endif
  while (1) {
    ret=write(fd,buf + count - new_count,new_count);
    if (ret==new_count){
      usleep(1);//why are bytes getting lost???
      return 0;
    }
    if (ret < 0){
      if (errno != EAGAIN){
        fprintf(stderr,"write error: %s\n",strerror(errno));
        fprintf(stderr,"             errno = %d, count = %d, new_count = %d\n", errno, count, new_count);
        return -1;
      } //else try again
    } else {
      new_count=new_count-ret;
    }
  }
}

int safe_read(int fd, char* buf, int count)
{
  int i;
  int ret;
  fd_set rfds;
  struct timeval tv;
  FD_ZERO(&rfds);
  FD_SET(fd,&rfds);
  tv.tv_sec=1;
  tv.tv_usec=0;
  //usleep(1);
  //TODO: This surely isn't efficient
#ifdef DEBUG
  printf("reading %d bytes\n", count);
#endif
  for (i=0; i < count; i++){
    ret=select(fd+1,&rfds,NULL,NULL,&tv);
    if (ret == -1){
      fprintf(stderr,"select error: %s \n",strerror(errno));
      return -1;
    } else if (!ret) {
      return 0;
    }

    if ((ret=read(fd,buf+i,1)) < 0){
      fprintf(stderr,"tty error: %s\n",strerror(errno));
      return -1;
    } else if (ret == 0) {
      fprintf(stderr,"tty error: nothing read\n");
      return -1;
    } 
  }
  return count;
}

/*** Transmit/Receive Command State Machine ***/

int process_comm(struct monman_state* ms, struct serial_command *s_comm,int stty_fd)
{
  int read_bytes;
  int ret;
  char *buffer;
  tcflush(stty_fd, TCIFLUSH);

  buffer = (char*) malloc(ms->addr_bytes + ms->data_bytes + 2);
  if (!buffer) {
    fprintf(stderr, "error: buffer malloc failed\n");
    return -1;
  }
  buffer[0] = s_comm->command;
  memcpy(buffer + 1, s_comm->address, ms->addr_bytes);
  memcpy(buffer + 1 + ms->addr_bytes, s_comm->data, ms->data_bytes);

  //transmit stuff
  switch (s_comm->command){
    case COMMAND_NOP:
      if ((ret=safe_write(stty_fd,buffer, 1)) < 0){
        fprintf(stderr, "error: NOP tx failed");
        free(buffer);
        return -1;
      }
      return 0;
    case COMMAND_WRITE:
      if ((ret=safe_write(stty_fd,buffer, 1 + ms->addr_bytes + ms->data_bytes)) < 0){
        fprintf(stderr, "error: READ tx failed");
        free(buffer);
        return -1;
      }
      if (!response_byte){
        free(buffer);
        return -1;
      }
      read_bytes = 1;
      break;
    case COMMAND_READ:
      if ((ret=safe_write(stty_fd,buffer, 1 + ms->addr_bytes)) < 0){
        fprintf(stderr, "error: READ tx failed");
        free(buffer);
        return -1;
      }
      read_bytes = ms->data_bytes + (response_byte ? 1 : 0);
      break;
    default: 
      fprintf(stderr,"error: unrecognized command");
      free(buffer);
      return -1;
  }
  if ((ret = safe_read(stty_fd, buffer, read_bytes)) < 0){
    fprintf(stderr, "error: RX failed");
    free(buffer);
    return -1;
  } else if (ret == 0) {
    free(buffer);
    return 1;
  }
  memcpy(s_comm->data, buffer + 1, ms->addr_bytes); 
  free(buffer);
  return 0;
}

/*** Decode and transmit command from file source ***/
#define ISTATE_START   0
#define ISTATE_ADDR    1
#define ISTATE_RDATA   2
#define ISTATE_WDATA   3
#define ISTATE_EXIT    4
#define ISTATE_ERROR   5
#define ISTATE_COMMAND 6
#define ISTATE_WAIT    7
#define ISTATE_DONE    8
#define ISTATE_SPECIAL 9

int run_file(struct monman_state* ms, int stty_fd, const char* filename){
  struct serial_command s_comm;
  int special_val;
  int state=ISTATE_START;
  int file_fd,ret;
  unsigned char buffer[STR_MAX];
  unsigned char cbuff;
  int fresh_command;
  if ((file_fd=open(filename,O_RDONLY))<0){
    fprintf(stderr,"file open error: %s\n",strerror(errno));
    return -1;
  }
  buffer[0]='\0'; //init string
  fresh_command = 0;
  while (state != ISTATE_EXIT && state != ISTATE_ERROR){
    if ((ret=read(file_fd,&cbuff,1)) < 0){
      fprintf(stderr,"file read error: %s\n",strerror(errno));
      state=ISTATE_ERROR;
    } else if (ret == 0) {
      state=ISTATE_DONE;
#ifdef LAME_DEBUG
      printf("read eof \n");
#endif
    } else if (cbuff=='\r' || cbuff=='\n') {
      state=ISTATE_COMMAND;
#ifdef LAME_DEBUG
      printf("read newline \n");
#endif
    } 
    switch (state) {
      case ISTATE_START:
        if (isspace(cbuff)){
          break;
        } 
        switch (cbuff) {
          case 'r':
            s_comm.command=COMMAND_READ;
            state=ISTATE_ADDR;
            break;
          case 'w':
            s_comm.command=COMMAND_WRITE;
            state=ISTATE_ADDR;
            break;
          case 'n':
            s_comm.command=COMMAND_NOP;
            state=ISTATE_COMMAND;
            break;
          case 's':
            s_comm.command=COMMAND_SLEEP;
            state=ISTATE_SPECIAL;
            break;
          case '#':
          default:
            s_comm.command=COMMAND_COMMENT;
            state=ISTATE_SPECIAL;
            break;
        }
        break;
      case ISTATE_SPECIAL:
        if (isspace(cbuff) && strlen((char*)buffer) == 0){
          break;
        } 
        str_n_append_chr((char*)buffer,STR_MAX,cbuff);
        fresh_command = 1;
        break;
      case ISTATE_ADDR:
        if (isspace(cbuff) && strlen((char*)buffer) == 0){
          break;
        } 
        if (!isspace(cbuff)){
          if (strlen((char*)buffer) < ms->addr_bytes*2){
            str_n_append_chr((char*)buffer,STR_MAX,cbuff);
            hexstr_bytes((char*)buffer, s_comm.address, ms->addr_bytes*2);

            if (s_comm.command == COMMAND_READ) //when got address cmnd ok
              fresh_command = 1;
            
          }
          break;
        }
        /*short address in read case followed by space & standard write case*/ 
        if (s_comm.command == COMMAND_READ){
          state=ISTATE_WAIT;
        } else {
          state=ISTATE_WDATA;
          buffer[0]='\0';
        }
        break;
      case ISTATE_WDATA:
        if (isspace(cbuff) && strlen((char*)buffer) == 0){
          break;
        } 
        if (!isspace(cbuff)){
          if (strlen((char*)buffer) < ms->data_bytes*2){
            str_n_append_chr((char*)buffer,STR_MAX,cbuff);
            hexstr_bytes((char*)buffer, s_comm.data, ms->data_bytes*2);
            if (s_comm.command == COMMAND_WRITE) //when got data cmnd ok
              fresh_command = 1;
          }
          break;
        }
        /*short data*/
        state=ISTATE_WAIT;
        break;
      case ISTATE_DONE: //run command after done
        state=ISTATE_EXIT;
        if (buffer[0] == '\0')
          break;
      case ISTATE_COMMAND:
        if (!fresh_command) {
          if (state!=ISTATE_EXIT)
            state=ISTATE_START;
          buffer[0]='\0';
          break;
        }
        switch (s_comm.command){
          case COMMAND_READ:
          case COMMAND_WRITE:
          case COMMAND_NOP:
#ifdef DEBUG
            bytes_hexstr(s_comm.address, ms->addr_bytes, (char*)buffer, STR_MAX);
            printf("got command=%s addr=%s ", s_comm.command == COMMAND_READ ? "READ" :  s_comm.command == COMMAND_WRITE ? "WRITE" : "NOP" , buffer);
            bytes_hexstr(s_comm.data, ms->data_bytes, (char*)buffer, STR_MAX);
            printf("data=%s\n", buffer);
#endif
            if (process_comm(ms,&s_comm,stty_fd) < 0){
              printf("TO\n");
            } else if (s_comm.command ==  COMMAND_READ){
              bytes_hexstr(s_comm.address, ms->addr_bytes, (char*)buffer, STR_MAX);
              //printf("x%s ", buffer);
              bytes_hexstr(s_comm.data, ms->data_bytes, (char*)buffer, STR_MAX);
              printf("%s\n", buffer);
              //printf("%s\n",buffer);
            }
            fresh_command = 0;
            break;
          case COMMAND_SLEEP:
            special_val=(unsigned int)strtol((char*)buffer,NULL,10);
            usleep(special_val);
            fresh_command = 0;
            break;
          case COMMAND_COMMENT:
            if (display_comments){
              printf("%s\n",buffer);
            }
            fresh_command = 0;
            break;
          default: //do nothing
            break;
        }
        if (state!=ISTATE_EXIT)
          state=ISTATE_START;
        buffer[0]='\0';
        break;
      case ISTATE_WAIT: 
        /* Wait for newline */
      default:
        break;
    }
  }
  close(file_fd);
  return (state==ISTATE_ERROR);
}

/*** Decode and transmit command from stdin source ***/
void run_interactively(struct monman_state* ms, int stty_fd){
  struct serial_command s_comm;
 
  int inputfd=0;
  unsigned char buff[MAX_BYTES];
  struct termios prev,current;

  int ret;
  int state=ISTATE_START;

  tcgetattr(inputfd, &current);
  memcpy(&prev,&current,sizeof(struct termios));
  current.c_cc[VTIME] = 0;
  current.c_cc[VMIN]  = 1;
  current.c_lflag = current.c_lflag & (~(ECHO|ICANON));
  tcsetattr(inputfd, TCSANOW, &current);
  
  while (state != ISTATE_EXIT){
#ifdef DEBUG
    printf("state = %d\n",state);
#endif
    switch (state){
      case ISTATE_START:
        printf("enter command: [r]ead [w]rite [n]op e[x]it\n");
        if ((ret=read(inputfd,&buff,1)) < 0){
          fprintf(stderr,"stdin read error: %s\n",strerror(errno));
          state=ISTATE_EXIT;
        } else if (ret==0) {
          fprintf(stderr,"term eof?\n");
          state=ISTATE_ERROR;
        } else {
        switch (buff[0]){
            case 'q':
            case 'Q':
            case 'x':
            case 'X':
              state=ISTATE_EXIT;
              break;
            case 'r':
              state=ISTATE_ADDR;
              s_comm.command=COMMAND_READ;
              break;
            case 'w':
              state=ISTATE_ADDR;
              s_comm.command=COMMAND_WRITE;
              break;
            case 'n':
              state=ISTATE_COMMAND;
              s_comm.command=COMMAND_NOP;
              break;
            default:
              break;
          }
        }
        break;
      case ISTATE_ADDR:
        printf("enter address: [%dbit hex(%d chars)]\n",ms->addr_bytes*8,ms->addr_bytes);
	if (!get_hex_word(inputfd,ms->addr_bytes,buff)) {
          memcpy(s_comm.address, buff, ms->addr_bytes);
          if (s_comm.command == COMMAND_WRITE){
            state=ISTATE_WDATA;
          } else if (s_comm.command == COMMAND_READ){
            state=ISTATE_COMMAND;
          } else {
	    fprintf(stderr,"special commands not implemented\n");
            state=ISTATE_COMMAND;
	  }
        } else {
          state=ISTATE_ERROR;
        }
        break;
      case ISTATE_WDATA:
        printf("enter data word: [%dbit hex(%d chars)]\n",ms->data_bytes*8,ms->data_bytes);
	if (!get_hex_word(inputfd,ms->data_bytes,buff)) {
          memcpy(s_comm.data, buff, ms->data_bytes);
          state=ISTATE_COMMAND;
        } else {
          state=ISTATE_ERROR;
        }
        break;
      case ISTATE_COMMAND:
        if (process_comm(ms,&s_comm,stty_fd)){
          state=ISTATE_ERROR;
        } else if (s_comm.command ==  COMMAND_READ){
          bytes_hexstr(s_comm.data, ms->data_bytes, (char*)buff, STR_MAX);
          printf("read result: hex %s \n", buff); 
          state=ISTATE_START;
        } else {
          state=ISTATE_START;
        }
        break;
      case ISTATE_ERROR:
        printf("An error has occurred\n");
        state=ISTATE_START;
        break;
      default:
        printf("This should not happen\n");
        state=ISTATE_EXIT;
        break;
    }
  }
  tcsetattr(inputfd, TCSANOW, &prev);
  return;
}

void usage(){
  fprintf(stderr,"usage: monman [-r] [-c] [-d x] [-a y] [-b z] [-f command_file] tty_file\n");
  fprintf(stderr,"       -r,   response bytes enabled \n");
  fprintf(stderr,"       -c,   display comments in command_file \n");
  fprintf(stderr,"       -d x, data word width x bits\n");
  fprintf(stderr,"       -a y, address width y bits\n");
  fprintf(stderr,"       -b z, bitrate = z baud\n");
  return; 
};

int main(int argc, char **argv){
  char *command_file=NULL;
  char *tty_file=NULL;
  int stty_fd;
  struct termios term_prev,term_curr;
  int j,i;
  int flag_incomplete = 0;
  int term_speed = DEFAULT_TERM_SPEED;

  struct monman_state ms;
  display_comments=0;
  ms.data_bytes=DEFAULT_DATA_BYTES;
  ms.addr_bytes=DEFAULT_ADDR_BYTES;

  if (argc <= 1 || !strcmp(argv[1],"-h") || !strcmp(argv[1],"--help")){
    usage();
    return EX_USAGE;
  }

  for (i=1; i < argc ; i=i+1) {
    if (argv[i] == NULL) {
      fprintf(stderr,"error: null case in args");
      return EX_SOFTWARE;
    }
    if (i == argc - 1){
      tty_file=argv[i];
    } else if (flag_incomplete) {
      switch (flag_incomplete){
        case 1:
          ms.data_bytes=atoi(argv[i])/8;
          if (atoi(argv[i]) % 8 != 0){
            fprintf(stderr,"error: data width must be multiple of 8\n");
            return EX_USAGE;
          }
          if (ms.data_bytes < 1 || ms.data_bytes > MAX_BYTES) {
            fprintf(stderr,"error: max data width is %d, provided was %d\n",MAX_BYTES*8,ms.data_bytes*8);
            return EX_USAGE;
          }
          break;
        case 2:
          if (atoi(argv[i]) % 8 != 0){
            fprintf(stderr,"error: address width must be multiple of 8\n");
            return EX_USAGE;
          }
          ms.addr_bytes=atoi(argv[i])/8;
          if (ms.addr_bytes < 1 || ms.addr_bytes > MAX_BYTES) {
            fprintf(stderr,"error: max address width is %d, provided was %d\n",MAX_BYTES*8,ms.addr_bytes*8);
            return EX_USAGE;
          }
          break;
        case 3:
          j=0;
          while (1){
            if (term_speeds[j] == NULL){
              fprintf(stderr,"warning: invalid serial terminal bitrate\n");
              fprintf(stderr,"         using default: %s\n",term_speeds[term_speed]);
              break;
            }

            if (strstr(term_speeds[j],argv[i])){
              term_speed=j;
              break;
            } 
            j++;
          }
          break;
	case 4:
	  command_file=argv[i];
	  break;
        default:
          fprintf(stderr,"error: this shouldn't happen args\n");
          return EX_USAGE;
          break;
      }
      flag_incomplete=0;
    } else if (strstr(argv[i],"-r")){
      response_byte = 1;
      flag_incomplete = 0;
    } else if (strstr(argv[i],"-c")){
      flag_incomplete=0;
      display_comments=1;
    } else if (strstr(argv[i],"-d")){
      flag_incomplete=1;
    } else if (strstr(argv[i],"-a")){
      flag_incomplete=2;
    } else if (strstr(argv[i],"-b")){
      flag_incomplete=3;
    } else if (strstr(argv[i],"-h")){
      usage();
      return EX_OK;
    } else if (strstr(argv[i],"-f")) {
      flag_incomplete=4;
    } else {
      usage();
      return EX_USAGE;
    }
  }
  if (flag_incomplete) {
    usage();
    return EX_USAGE;
  }

  if (tty_file == NULL){
    fprintf(stderr,"error: NULL tty_file\n");
    return EX_SOFTWARE;
  }
#ifdef DEBUG
  printf("tty=%s file?=%x data_bytes=%d addr_bytes=%d bitrate=%s\n", tty_file, (unsigned int) command_file, ms.data_bytes, ms.addr_bytes, term_speeds[term_speed]);
#endif
  fflush(stdin);

  if ((stty_fd=open(tty_file, O_RDWR)) < 0){
    fprintf(stderr,"open error: %s\n",strerror(errno));
    return EX_IOERR;
  }

  if (!isatty(stty_fd)) {
    fprintf(stderr,"error: tty_file is not a tty\n");
    close(stty_fd);
    return EX_USAGE;
  }

  if (tcgetattr(stty_fd, &term_prev)){
    fprintf(stderr,"tcgetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return EX_OSERR;
  }
  memcpy(&term_curr,&term_prev,sizeof(struct termios));

  term_curr.c_cc[VMIN]=1; /*1 char at a time*/
  term_curr.c_cc[VTIME]=0;
  term_curr.c_ispeed=term_speeds_n[term_speed];
  term_curr.c_ospeed=term_speeds_n[term_speed];
  term_curr.c_cflag = CREAD|CLOCAL|CS8|term_speeds_n[term_speed];
#ifndef CYGWIN
  cfsetspeed(&term_curr,term_speeds_n[term_speed]);
  cfmakeraw(&term_curr);
#endif

  if (tcsetattr(stty_fd,TCSANOW,&term_curr)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return EX_OSERR;
  }

  if (command_file==NULL){
    run_interactively(&ms,stty_fd);
  } else {
    run_file(&ms,stty_fd,command_file);
  }
  

  if (tcsetattr(stty_fd,TCSANOW,&term_prev)){
    fprintf(stderr,"tcsetattr error: %s\n",strerror(errno));
    close(stty_fd);
    return EX_OSERR;
  }
  
  close(stty_fd);
  return EX_OK;
}

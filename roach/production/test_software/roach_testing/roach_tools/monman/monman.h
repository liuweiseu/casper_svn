#ifndef MONMAN_H
#define MONMAN_H

/* remote command types: interpretted by remote bridge*/
#define COMMAND_NOP    0
#define COMMAND_READ   1
#define COMMAND_WRITE  2
/* Special commands: interpretted by local state machines */
#define COMMAND_COMMENT 254
#define COMMAND_SLEEP   255

#define DEFAULT_TERM_SPEED 1
#define DEFAULT_DATA_BYTES 4
#define DEFAULT_ADDR_BYTES 2

#define MAX_BYTES 8

const char* term_speeds  [9] = {"B230400","B115200","B57600","B38400","B19200","B9600","B4800","B2400",NULL};
const int   term_speeds_n[9] = { B230400,   B115200,  B57600,  B38400,  B19200,  B9600,  B4800,  B2400, B0};

struct serial_command{
  unsigned char command;
  unsigned char address[MAX_BYTES];
  unsigned char data[MAX_BYTES];  
};

struct monman_state{
  int data_bytes;
  int addr_bytes;
};
#endif

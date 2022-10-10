#ifndef _KATCP_H_
#define _KATCP_H_

struct katcp_sensor;
struct katcp_dispatch;
struct katcp_cmd;

#include <stdarg.h>

#define KATCP_REQUEST '?' 
#define KATCP_REPLY   '!' 
#define KATCP_INFORM  '#' 

#define KATCP_OK      "ok"
#define KATCP_FAIL    "fail"
#define KATCP_INVALID "invalid"

#define KATCP_LEVEL_TRACE    0
#define KATCP_LEVEL_DEBUG    1
#define KATCP_LEVEL_INFO     2
#define KATCP_LEVEL_WARN     3
#define KATCP_LEVEL_ERROR    4
#define KATCP_LEVEL_FATAL    5
#define KATCP_LEVEL_OFF      6
#define KATCP_MAX_LEVELS     7

#define KATCP_FLAG_FIRST   0x1
#define KATCP_FLAG_LAST    0x2
#define KATCP_FLAG_MORE    0x4

#define KATCP_FLAG_STRING 0x10
#define KATCP_FLAG_ULONG  0x20
#define KATCP_FLAG_XLONG  0x40
#define KATCP_FLAG_BUFFER 0x80

#define KATCP_TYPE_FLAGS  0xf0
#define KATCP_ORDER_FLAGS 0x0f

#define KATCP_EXIT_ABORT (-1)
#define KATCP_EXIT_NOTYET  0
#define KATCP_EXIT_QUIT    1
#define KATCP_EXIT_HALT    2
#define KATCP_EXIT_RESTART 3

#define KATCP_RESULT_RESUME    2
#define KATCP_RESULT_OWN       1
#define KATCP_RESULT_OK        0
#define KATCP_RESULT_FAIL    (-1)
#define KATCP_RESULT_INVALID (-2)

/******************* core api ********************/

/* create a dispatch handler */
struct katcp_dispatch *startup_katcp(void);
/* create a dispatch handler on file descriptor */
struct katcp_dispatch *setup_katcp(int fd); 

#if 0
struct katcp_dispatch *setup_version_katcp(int fd, char *subsystem, int major, int minor);
struct katcp_dispatch *startup_version_katcp(char *subsystem, int major, int minor);
#endif

/* make a copy of of an instance */
struct katcp_dispatch *clone_katcp(struct katcp_dispatch *cd);

/* destroy handler */
void shutdown_katcp(struct katcp_dispatch *d);

/* set build and version fields, needed for welcome banner */
int version_katcp(struct katcp_dispatch *d, char *subsystem, int major, int minor);
int build_katcp(struct katcp_dispatch *d, char *build);

/* make life easier for users, let them store their state here */
void *get_state_katcp(struct katcp_dispatch *d);
void set_state_katcp(struct katcp_dispatch *d, void *p);
void set_clear_state_katcp(struct katcp_dispatch *d, void *p, void (*clear)(struct katcp_dispatch *d, void *p));

void *get_multi_katcp(struct katcp_dispatch *d);
void set_multi_katcp(struct katcp_dispatch *d, void *p);

/* prints version information, to be called on incomming connection */
void on_connect_katcp(struct katcp_dispatch *d);
/* print explanation on ending connection, if null decodes exit code */
void on_disconnect_katcp(struct katcp_dispatch *d, char *fmt, ...);

/* add a callback to handler (match includes type), help the help message */
int register_katcp(struct katcp_dispatch *d, char *match, char *help, int (*call)(struct katcp_dispatch *d, int argc));

/* change the callback in mid-flight */
int continue_katcp(struct katcp_dispatch *d, int when, int (*call)(struct katcp_dispatch *d, int argc));

/* see if there is a command, then parse it */
int lookup_katcp(struct katcp_dispatch *d);
/* invoke command for a quantum */
int call_katcp(struct katcp_dispatch *d);
/* combines lookup and call while there is stuff to do */
int dispatch_katcp(struct katcp_dispatch *d);

/* run the dispatch handler until error or shutdown */
int run_katcp(struct katcp_dispatch *d, int server, char *host, int port);
int run_client_katcp(struct katcp_dispatch *d, char *host, int port);
int run_server_katcp(struct katcp_dispatch *d, char *host, int port);

/******************* io functions ****************/

int fileno_katcp(struct katcp_dispatch *d);
struct katcl_line *line_katcp(struct katcp_dispatch *d);
int read_katcp(struct katcp_dispatch *d);
int have_katcp(struct katcp_dispatch *d);
int flushing_katcp(struct katcp_dispatch *d);
int flush_katcp(struct katcp_dispatch *d);
int write_katcp(struct katcp_dispatch *d);
void exchange_katcp(struct katcp_dispatch *d, int fd);

/******************* read arguments **************/

int arg_request_katcp(struct katcp_dispatch *d);
int arg_reply_katcp(struct katcp_dispatch *d);
int arg_inform_katcp(struct katcp_dispatch *d);

unsigned int arg_count_katcp(struct katcp_dispatch *d);
int arg_null_katcp(struct katcp_dispatch *d, unsigned int index);

char *arg_string_katcp(struct katcp_dispatch *d, unsigned int index);
char *arg_copy_string_katcp(struct katcp_dispatch *d, unsigned int index);
unsigned long arg_unsigned_long_katcp(struct katcp_dispatch *d, unsigned int index);
unsigned int arg_buffer_katcp(struct katcp_dispatch *d, unsigned int index, void *buffer, unsigned int size);

/******************* write arguments *************/

int append_string_katcp(struct katcp_dispatch *d, int flags, char *buffer);
int append_unsigned_long_katcp(struct katcp_dispatch *d, int flags, unsigned long v);
int append_hex_long_katcp(struct katcp_dispatch *d, int flags, unsigned long v);
int append_buffer_katcp(struct katcp_dispatch *d, int flags, void *buffer, int len);
int append_vargs_katcp(struct katcp_dispatch *d, int flags, char *fmt, va_list args);
int append_args_katcp(struct katcp_dispatch *d, int flags, char *fmt, ...);

/******************* write list of arguments *****/

int vsend_katcp(struct katcp_dispatch *d, va_list ap);
int send_katcp(struct katcp_dispatch *d, ...);

/******************* utility functions ***********/

int log_message_katcp(struct katcp_dispatch *d, int level, char *name, char *fmt, ...);

int error_katcp(struct katcp_dispatch *d);
int exited_katcp(struct katcp_dispatch *d); /* has this client requested a quit */
int terminate_katcp(struct katcp_dispatch *d, int code); /* mark this task as quitting */

/******************* sensor work *****************/

#define KATCP_STRATEGY_OFF  0

int register_integer_sensor_katcp(struct katcp_dispatch *d, char *name, char *description, char *units, int min, int max, int (*get)(void *local), void *local);

int append_type_sensor_katcp(struct katcp_sensor *s, struct katcp_dispatch *d);


#endif

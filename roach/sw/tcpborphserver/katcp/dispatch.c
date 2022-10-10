/* Slightly higher level dispatch routines, handles callback registration
 * and dispatch. Wraps a fair amount of line handling stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <sys/time.h>

#include "katpriv.h"
#include "katcl.h"
#include "katcp.h"

#define KATCP_READY_MAGIC 0x7d68352c

int help_cmd_katcp(struct katcp_dispatch *d, int argc);
int halt_cmd_katcp(struct katcp_dispatch *d, int argc);
int restart_cmd_katcp(struct katcp_dispatch *d, int argc);
int log_level_cmd_katcp(struct katcp_dispatch *d, int argc);
int watchdog_cmd_katcp(struct katcp_dispatch *d, int argc);
int sensor_list_cmd_katcp(struct katcp_dispatch *d, int argc);
int sensor_sampling_cmd_katcp(struct katcp_dispatch *d, int argc);

/************ paranoia checks ***********************************/

#ifdef DEBUG
static void sane_katcp(struct katcp_dispatch *d)
{
  if(d == NULL){
    fprintf(stderr, "sane: invalid handle\n");
    abort();
  }
  if(d->d_line == NULL){
    fprintf(stderr, "sane: invalid line handle\n");
    abort();
  }
  if(d->d_ready != KATCP_READY_MAGIC){
    fprintf(stderr, "sane: not called from within dispatch\n");
    abort();
  }
}
#else
#define sane_katcp(d)
#endif

/******************************************************************/

static struct katcp_dispatch *setup_internal_katcp(int fd)
{
  struct katcp_dispatch *d;

  d = malloc(sizeof(struct katcp_dispatch));
  if(d == NULL){
    return NULL;
  }

  d->d_line = create_katcl(fd);
  d->d_commands = NULL;
  d->d_current = NULL;
  d->d_ready = 0;

  d->d_exit = KATCP_EXIT_ABORT;
  d->d_run = 1;

  d->d_level = KATCP_LEVEL_INFO;

  d->d_state = NULL;
  d->d_clear = NULL;
  d->d_multi = NULL;
  d->d_sensors = NULL;

  d->d_version_subsystem = NULL;
  d->d_version_major = 0;
  d->d_version_minor = 0;

  d->d_build_state = NULL;

#ifdef DEBUG
  fprintf(stderr, "setup: have version %d.%d\n", d->d_version_major, d->d_version_minor);
#endif

  if(d->d_line == NULL){
    shutdown_katcp(d);
    return NULL;
  }

  return d;
}

/******************************************************************/

struct katcp_dispatch *setup_katcp(int fd)
{
  struct katcp_dispatch *d;

  d = setup_internal_katcp(fd);
  if(d == NULL){
    return NULL;
  }

  register_katcp(d, "?halt", "shuts the system down", &halt_cmd_katcp);
  register_katcp(d, "?restart", "restarts the system", &restart_cmd_katcp);
  register_katcp(d, "?help", "displays this help", &help_cmd_katcp);
  register_katcp(d, "?log-level", "sets the minimum reported log priority", &log_level_cmd_katcp);
  register_katcp(d, "?watchdog", "pings the system", &watchdog_cmd_katcp);
  register_katcp(d, "?sensor-list", "lists available sensors", &sensor_list_cmd_katcp);
  register_katcp(d, "?sensor-sampling", "sets the sensor reporting mode", &sensor_sampling_cmd_katcp);

  return d;
}

struct katcp_dispatch *setup_version_katcp(int fd, char *subsystem, int major, int minor)
{
  struct katcp_dispatch *d;

  d = setup_katcp(fd);
  if(d == NULL){
    return NULL;
  }

  if(version_katcp(d, subsystem, major, minor) < 0){
    shutdown_katcp(d);
    return NULL;
  }

  return d;
}

struct katcp_dispatch *startup_katcp()
{
  return setup_katcp(-1);
}

struct katcp_dispatch *startup_version_katcp(char *subsystem, int major, int minor)
{
  return setup_version_katcp(-1, subsystem, major, minor);
}

/******************/

struct katcp_dispatch *clone_katcp(struct katcp_dispatch *cd)
{
  /* if this ever gets used often, the katcp_cmd list should have a refcount instead of being copied deeply */
  struct katcp_dispatch *d;
  struct katcp_cmd *c;
  struct katcp_sensor *s;

  d = setup_internal_katcp(-1);
  if(d == NULL){
    return NULL;
  }

  d->d_level = cd->d_level;
  /* d_ready */
  /* d_run */
  /* d_exit */
  /* d_line */

  for(c = cd->d_commands; c; c = c->c_next){
    if(register_katcp(d, c->c_name, c->c_help, c->c_call) < 0){
      shutdown_katcp(d);
      return NULL;
    }
  }

  for(s = cd->d_sensors; s; s = s->s_next){
    /* TODO */
  }

  /* d_clear */
  if(cd->d_state || cd->d_multi){
#ifdef DEBUG
    fprintf(stderr, "warning: cloning function with state set\n");
#endif
    shutdown_katcp(d);
    return NULL;
  }

  d->d_version_major = cd->d_version_major;
  d->d_version_minor = cd->d_version_minor;

  if(cd->d_version_subsystem){
    d->d_version_subsystem = strdup(cd->d_version_subsystem);
    if(d->d_version_subsystem == NULL){
      shutdown_katcp(d);
      return NULL;
    }
  }

  if(cd->d_build_state){
    d->d_build_state = strdup(cd->d_build_state);
    if(d->d_build_state == NULL){
      shutdown_katcp(d);
      return NULL;
    }
  }

  return d;
}

/***********************************************************************/

static void shutdown_cmd_katcp(struct katcp_cmd *c)
{
  if(c){
    if(c->c_name){
      free(c->c_name);
      c->c_name = NULL;
    }

    if(c->c_help){
      free(c->c_help);
      c->c_help = NULL;
    }

    free(c);
  }
}

void shutdown_katcp(struct katcp_dispatch *d)
{
  struct katcp_cmd *c;

  if(d == NULL){
    return;
  }

  if(d->d_line){
    destroy_katcl(d->d_line, 1);
    d->d_line = NULL;
  }

  d->d_current = NULL;

  while(d->d_commands != NULL){
    c = d->d_commands;
    d->d_commands = c->c_next;

    shutdown_cmd_katcp(c);
  }

  if(d->d_version_subsystem){
    free(d->d_version_subsystem);
    d->d_version_subsystem = NULL;
  }

  if(d->d_build_state){
    free(d->d_build_state);
    d->d_build_state = NULL;
  }

  d->d_state = NULL;

  free(d);
}

/********************/

int version_katcp(struct katcp_dispatch *d, char *subsystem, int major, int minor)
{
  if(d == NULL){
    return -1;
  }

  if(d->d_version_subsystem){
    free(d->d_version_subsystem);
    d->d_version_subsystem = NULL;
  }

  d->d_version_major = major;
  d->d_version_minor = minor;

  if(subsystem){
    d->d_version_subsystem = strdup(subsystem);
    if(d->d_version_subsystem == NULL){
      return -1;
    }
  }

  return 0;
}

int build_katcp(struct katcp_dispatch *d, char *build)
{
  if(d == NULL){
    return -1;
  }

  if(d->d_build_state){
    free(d->d_build_state);
    d->d_build_state = NULL;
  }

  if(build){
    d->d_build_state = strdup(build);
    if(d->d_build_state == NULL){
      return -1;
    }
  }

  return 0;
}

/********************/

void set_state_katcp(struct katcp_dispatch *d, void *p)
{
  set_clear_state_katcp(d, p, NULL);
}

void set_clear_state_katcp(struct katcp_dispatch *d, void *p, void (*clear)(struct katcp_dispatch *d, void *p))
{
  if(d == NULL){
    return;
  }

  if(d->d_state && d->d_clear){
    (*(d->d_clear))(d, p);
  }

  d->d_state = p;
  d->d_clear = clear;
}

void *get_state_katcp(struct katcp_dispatch *d)
{
  return d ? d->d_state : NULL;
}

void set_multi_katcp(struct katcp_dispatch *d, void *p)
{
  if(d){
    d->d_multi = p;
  }
}

void *get_multi_katcp(struct katcp_dispatch *d)
{
  return d ? d->d_multi : NULL;
}

/********************/

void on_disconnect_katcp(struct katcp_dispatch *d, char *fmt, ...)
{
  va_list args;

  d->d_ready = KATCP_READY_MAGIC;

  append_string_katcp(d, KATCP_FLAG_FIRST, "#disconnect");

#ifdef DEBUG
  fprintf(stderr, "disconnect: ending d=%p fd=%d\n", d, fileno_katcp(d));
#endif

  if(fmt){
    va_start(args, fmt);
    append_vargs_katcp(d, KATCP_FLAG_LAST, fmt, args);
    va_end(args);
  } else {
    switch(d->d_exit){
      case KATCP_EXIT_HALT :
        append_string_katcp(d, KATCP_FLAG_LAST, "system halting");
        break;
      case KATCP_EXIT_RESTART :
        append_string_katcp(d, KATCP_FLAG_LAST, "system restarting");
        break;
      /* case KATCP_EXIT_ABORT : */
      default :
        append_string_katcp(d, KATCP_FLAG_LAST, "system aborting");
        break;
    }
  }

  d->d_ready = 0;
}

void on_connect_katcp(struct katcp_dispatch *d)
{
  /* prints initial inform messages, could possibly be made dynamic */

  if(d == NULL){
    return;
  }

  d->d_ready = KATCP_READY_MAGIC;

  if(d->d_version_subsystem){
    append_string_katcp(d, KATCP_FLAG_FIRST, "#version");
    append_args_katcp(d, KATCP_FLAG_LAST, "%s-%d.%d", d->d_version_subsystem, d->d_version_major, d->d_version_minor);
  }

  if(d->d_build_state){
    append_string_katcp(d, KATCP_FLAG_FIRST, "#build-state");
    append_args_katcp(d, KATCP_FLAG_LAST, "%s", d->d_build_state);
  }

#if 0
  print_katcl(d->d_line, 1, "#build-state\n");
#endif

  d->d_ready = 0;
}

/* Need a call back for a sensor - unclear how to do this, as
 * sensors can happen over intervals, when values change, etc 
 */

int register_katcp(struct katcp_dispatch *d, char *match, char *help, int (*call)(struct katcp_dispatch *d, int argc))
{
  struct katcp_cmd *c;

  c = malloc(sizeof(struct katcp_cmd));
  if(c == NULL){
    return -1;
  }

  c->c_name = strdup(match);
  c->c_help = strdup(help);

  if((c->c_name == NULL) || (c->c_help == NULL)){
    shutdown_cmd_katcp(c);
    return -1;
  }

  c->c_next = d->d_commands;
  c->c_call = call;

  d->d_commands = c;

  return 0;
}

int continue_katcp(struct katcp_dispatch *d, int when, int (*call)(struct katcp_dispatch *d, int argc))
{
  /* calling this function outside a registered callback is likely to cause grief */
  sane_katcp(d);

  d->d_current = call;

  return 0;
}

int basic_response_katcp(struct katcp_dispatch *d, int code)
{
  char *message, *result;

  if(code > KATCP_RESULT_OK){
    return 0;
  }

  message = arg_string_katcl(d->d_line, 0);
  if((message == NULL) || (message[0] != KATCP_REQUEST)){
    return -1;
  }

  switch(code){
    case KATCP_RESULT_OK :
      result = KATCP_OK;
      break;
    case KATCP_RESULT_FAIL :
      result = KATCP_FAIL;
      break;
      /* case KATCP_RESULT_INVALID : */
    default :
      result = KATCP_INVALID;
      break;
  }

  send_katcp(d, 
    KATCP_FLAG_FIRST | KATCP_FLAG_MORE | KATCP_FLAG_STRING, "!", 
                                         KATCP_FLAG_STRING, message + 1, 
    KATCP_FLAG_LAST | KATCP_FLAG_STRING, result);

  return 0;
}

int lookup_katcp(struct katcp_dispatch *d)
{
  char *s;
  int r;
  struct katcp_cmd *search;

#ifdef DEBUG
  if(d == NULL){
    return -1;
  }
#endif

  if(d->d_current){
    return 1; /* still busy from last time */
  }

  r = have_katcl(d->d_line);
  if(r < 0){
    return -1;
  }
  
  if(r == 0){
    return 0; /* nothing to be looked up */
  }

  /* now (r  > 0) */

  d->d_ready = KATCP_READY_MAGIC;
  s = arg_string_katcl(d->d_line, 0);
  d->d_ready = 0;

  search = d->d_commands;
  for(search = d->d_commands; search; search = search->c_next){
    if(!strcmp(search->c_name, s)){
#ifdef DEBUG
      fprintf(stderr, "dispatch: found match for <%s>\n", s);
#endif
      d->d_current = search->c_call;
      return 1; /* found */
    }
  }
  
  return 1; /* not found, d->d_current == NULL */
}

int call_katcp(struct katcp_dispatch *d)
{
  int r, n;
  char *s;

#ifdef DEBUG
  if(d == NULL){
    return -1;
  }
#endif

  d->d_ready = KATCP_READY_MAGIC;

  s = arg_string_katcl(d->d_line, 0);
  n = arg_count_katcl(d->d_line);
  r = KATCP_RESULT_FAIL;

  if(d->d_current){
    r = (*(d->d_current))(d, n);

    if(r <= 0){
      basic_response_katcp(d, r);
    }
    if(r != KATCP_RESULT_RESUME){
      d->d_current = NULL;
    }
  } else { /* force a fail for unknown commands */
    if((s[0] == KATCP_REQUEST) && (s[1] != '\0')){
      send_katcp(d, 
          KATCP_FLAG_FIRST | KATCP_FLAG_MORE | KATCP_FLAG_STRING, "!", 
          KATCP_FLAG_STRING, s + 1, 
          KATCP_FLAG_STRING, KATCP_FAIL, 
          KATCP_FLAG_LAST | KATCP_FLAG_STRING, "unknown command");
    }
  }

  d->d_ready = 0;

  return r;
}

int dispatch_katcp(struct katcp_dispatch *d)
{
  int r;

  while((r = lookup_katcp(d)) > 0){
    call_katcp(d);
  }

  return r;
}

#if 0
int dispatch_katcp(struct katcp_dispatch *d)
{
  char *s;
  int n, r;

  if(d == NULL){
    return -1;
  }

  d->d_ready = KATCP_READY_MAGIC;

  while((r = have_katcl(d->d_line)) > 0){
    s = arg_string_katcl(d->d_line, 0);
    n = arg_count_katcl(d->d_line);
    if(s){
      d->d_current = d->d_commands;
      while(d->d_current && (strcmp(d->d_current->c_name, s))){
        d->d_current = d->d_current->c_next;
      }

      if(d->d_current){
#ifdef DEBUG
        fprintf(stderr, "dispatch: found match for <%s>, args=%d\n", s, n);
#endif
        r = (*(d->d_current->c_call))(d, n);

        if(r <= 0){
          basic_response_katcp(d, r);
        }
        if(r != KATCP_RESULT_RESUME){
          d->d_current = NULL;
        }
      } else { /* force a fail for unknown commands */
        if((s[0] == KATCP_REQUEST) && (s[1] != '\0')){
#if 0
          print_katcl(d->d_line, 1, "!%s %s unknown command\n", s + 1, KATCP_FAIL);
#endif
          send_katcp(d, 
            KATCP_FLAG_FIRST | KATCP_FLAG_MORE | KATCP_FLAG_STRING, "!", 
                                                 KATCP_FLAG_STRING, s + 1, 
                                                 KATCP_FLAG_STRING, KATCP_FAIL, 
                               KATCP_FLAG_LAST | KATCP_FLAG_STRING, "unknown command");
        }
      }
    }
  }

  d->d_ready = 0;

  return r;
}
#endif

/************ line wrappers, could be made macros ***************/

int fileno_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return fileno_katcl(d->d_line);
  }

  return -1;
}

struct katcl_line *line_katcp(struct katcp_dispatch *d)
{
  if(d == NULL){
    return NULL;
  }

  return d->d_line;
}

int flushing_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return flushing_katcl(d->d_line);
  }

  return 0;
}

int flush_katcp(struct katcp_dispatch *d)
{
  fd_set fsw;
  struct timeval tv;
  int result, fd;

  /* WARNING: not quite sure if unexpected flushes break some assumptions in main loop */

  if((d == NULL) || (d->d_line == NULL)){
#ifdef DEBUG
    fprintf(stderr, "flush: broken state\n");
#endif
    return -1;
  }

  if(!flushing_katcl(d->d_line)){
#ifdef DEBUG
    fprintf(stderr, "flush: nothing to do\n");
#endif
    return 1;
  }

  tv.tv_sec = 0;
  tv.tv_usec = 1;

  fd = fileno_katcl(d->d_line);

  FD_ZERO(&fsw);
  FD_SET(fd, &fsw);

  result = select(fd + 1, NULL, &fsw, NULL, &tv);
  if(result <= 0){
    return result;
  }

  /* returns zero if still more to flush */
  
  return write_katcl(d->d_line);
}

int write_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return write_katcl(d->d_line);
  }
  
  return -1;
}

int read_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return read_katcl(d->d_line);
  }

  return -1;
}

int have_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return have_katcl(d->d_line);
  }
  
  return 0;
}

int error_katcp(struct katcp_dispatch *d)
{
  if(d && d->d_line){
    return error_katcl(d->d_line);
  }
  
  return EINVAL;
}

int exited_katcp(struct katcp_dispatch *d)
{
  if(d){
    if(d->d_run){
      return KATCP_EXIT_NOTYET;
    }
    return d->d_exit;
  }
  
  return KATCP_EXIT_ABORT;
}

int terminate_katcp(struct katcp_dispatch *d, int code)
{
  if(d){
    d->d_run = 0;
    d->d_exit = code;
    return code;
  }
  
  return KATCP_EXIT_ABORT;
}

#if 0
int running_katcp(struct katcp_dispatch *d)
{
  if(d){
    return d->d_run;
  }
  
  return 0;
}
#endif

void exchange_katcp(struct katcp_dispatch *d, int fd)
{
  if(d && d->d_line){
    d->d_run = 1;
    d->d_exit = KATCP_EXIT_ABORT;

    return exchange_katcl(d->d_line, fd);
  }

  return;
}

/**************************************************************/

int help_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  struct katcp_cmd *c;
  unsigned int count;
  char *s;

  count = 0;
  s = (argc <= 1) ? NULL : arg_string_katcp(d, 1);

  for(c = d->d_commands; c; c = c->c_next){
    if((s == NULL) || (!strcmp(s, c->c_name + 1))){
      send_katcp(d, KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "#help", KATCP_FLAG_STRING, c->c_name + 1, KATCP_FLAG_LAST | KATCP_FLAG_STRING, c->c_help);
      count++;
    }
  }

  if(count > 0){
    send_katcp(d, 
      KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!help", 
                         KATCP_FLAG_STRING, KATCP_OK,
      KATCP_FLAG_LAST  | KATCP_FLAG_ULONG, (unsigned long) count);
  } else {
    send_katcp(d, 
      KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!help", 
                         KATCP_FLAG_STRING, KATCP_INVALID,
      KATCP_FLAG_LAST  | KATCP_FLAG_STRING, "unknown request");
  }
  
  return KATCP_RESULT_OWN;
}

int halt_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  terminate_katcp(d, KATCP_EXIT_HALT);

  return KATCP_RESULT_OK;
}

int restart_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  terminate_katcp(d, KATCP_EXIT_RESTART);

  return KATCP_RESULT_OK;
}

int watchdog_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  return KATCP_RESULT_OK;
}

int sensor_list_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  struct katcp_sensor *s;
  unsigned int count;
  char *name;

  if(argc <= 1){
    count = 0;
    for(s = d->d_sensors; s; s = s->s_next){
      if(append_type_sensor_katcp(s, d) < 0){
        return KATCP_RESULT_FAIL;
      }
      count++;
    }
  } else {
    name = arg_string_katcp(d, 1);
    if(name == NULL){
      return KATCP_RESULT_FAIL;
    }
    for(s = d->d_sensors; s && strcmp(name, s->s_name); s = s->s_next);
    if(s == NULL){
      log_message_katcp(d, KATCP_LEVEL_ERROR, d->d_version_subsystem ? d->d_version_subsystem : "unknown" , "unknown sensor %s", name);
      return KATCP_RESULT_INVALID;
    }
    if(append_type_sensor_katcp(s, d) < 0){
      return KATCP_RESULT_FAIL;
    }
    count = 1;
  }

  send_katcp(d, 
      KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!sensor-list", 
      KATCP_FLAG_STRING, KATCP_OK,
      KATCP_FLAG_LAST  | KATCP_FLAG_ULONG, (unsigned long) count);

  return KATCP_RESULT_OWN;
}

int sensor_sampling_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  struct katcp_sensor *s;
  char *name;

  if(argc <= 1){
    return KATCP_RESULT_INVALID;
  } 

  name = arg_string_katcp(d, 1);
  if(name == NULL){
    return KATCP_RESULT_FAIL;
  }

  for(s = d->d_sensors; s && strcmp(name, s->s_name); s = s->s_next);
  if(s == NULL){
    log_message_katcp(d, KATCP_LEVEL_ERROR, d->d_version_subsystem ? d->d_version_subsystem : "unknown" , "unknown sensor %s", name);
    return KATCP_RESULT_INVALID;
  }

  if(append_strategy_sensor_katcp(s, d) < 0){
    return KATCP_RESULT_FAIL;
  }

  return KATCP_RESULT_OWN;
}

int log_level_cmd_katcp(struct katcp_dispatch *d, int argc)
{
  int ok, code;
  char *s;

  s = NULL;

  if(argc > 1){
    ok = 0;
    s = arg_string_katcp(d, 1);

    if(s){
      if(!strcmp(s, "all")){
        ok = 1;
        d->d_level = KATCP_LEVEL_TRACE;
      }
      code = log_to_code(s);
      if(code >= 0){
        ok = 1;
        d->d_level = code;
      }
    }
  } else {
    ok = 1;
  }

  s = log_to_string(d->d_level);
  if(s == NULL){
    ok = 0;
  }

  if(ok){
    send_katcp(d, 
      KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!log-level", 
                         KATCP_FLAG_STRING, KATCP_OK,
      KATCP_FLAG_LAST  | KATCP_FLAG_STRING, s);
  } else {
    send_katcp(d, 
      KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!log-level", 
                         KATCP_FLAG_STRING, KATCP_INVALID,
      KATCP_FLAG_LAST  | KATCP_FLAG_STRING, "malformed log level");

    if(s){
      log_message_katcp(d, KATCP_LEVEL_WARN, d->d_version_subsystem ? d->d_version_subsystem : "unknown" , "unknown log level %s", s);
    }
  }

  return KATCP_RESULT_OK;
}

/**************************************************************/

int log_message_katcp(struct katcp_dispatch *d, int level, char *name, char *fmt, ...)
{
  va_list args;
  int result;

  sane_katcp(d);

  if(level < d->d_level){
    return 0;
  }

  va_start(args, fmt);
  result = vlog_message_katcl(d->d_line, level, name ? name : d->d_version_subsystem, fmt, args);
  va_end(args);
 
  return result;
}

int arg_request_katcp(struct katcp_dispatch *d)
{
  sane_katcp(d);

  return arg_request_katcl(d->d_line);
}

int arg_reply_katcp(struct katcp_dispatch *d)
{
  sane_katcp(d);

  return arg_reply_katcl(d->d_line);
}

int arg_inform_katcp(struct katcp_dispatch *d)
{
  sane_katcp(d);

  return arg_inform_katcl(d->d_line);
}

int arg_null_katcp(struct katcp_dispatch *d, unsigned int index)
{
  sane_katcp(d);

  return arg_null_katcl(d->d_line, index);
}

unsigned int arg_count_katcp(struct katcp_dispatch *d)
{
  sane_katcp(d);

  return arg_count_katcl(d->d_line);
}

char *arg_string_katcp(struct katcp_dispatch *d, unsigned int index)
{
  sane_katcp(d);

  return arg_string_katcl(d->d_line, index);
}

char *arg_copy_string_katcp(struct katcp_dispatch *d, unsigned int index)
{
  sane_katcp(d);

  return arg_copy_string_katcl(d->d_line, index);
}

unsigned long arg_unsigned_long_katcp(struct katcp_dispatch *d, unsigned int index)
{
  sane_katcp(d);

  return arg_unsigned_long_katcl(d->d_line, index);
}

unsigned int arg_buffer_katcp(struct katcp_dispatch *d, unsigned int index, void *buffer, unsigned int size)
{
  sane_katcp(d);

  /* returns the number of bytes it wanted to copy, more than size indicates a failure */
  return arg_buffer_katcl(d->d_line, index, buffer, size);
}

/**************************************************************/

int append_string_katcp(struct katcp_dispatch *d, int flags, char *buffer)
{
  sane_katcp(d);

  return append_string_katcl(d->d_line, flags, buffer);
}

int append_unsigned_long_katcp(struct katcp_dispatch *d, int flags, unsigned long v)
{
  sane_katcp(d);

  return append_unsigned_long_katcl(d->d_line, flags, v);
}

int append_hex_long_katcp(struct katcp_dispatch *d, int flags, unsigned long v)
{
  sane_katcp(d);

  return append_hex_long_katcl(d->d_line, flags, v);
}

int append_buffer_katcp(struct katcp_dispatch *d, int flags, void *buffer, int len)
{
  sane_katcp(d);

  return append_buffer_katcl(d->d_line, flags, buffer, len);
}

int append_args_katcp(struct katcp_dispatch *d, int flags, char *fmt, ...)
{
  int result;
  va_list args;

  sane_katcp(d);

  va_start(args, fmt);

  result = append_vargs_katcl(d->d_line, flags, fmt, args);

  va_end(args);

  return result;
}

int append_vargs_katcp(struct katcp_dispatch *d, int flags, char *fmt, va_list args)
{
  int result;
  va_list copy;

  sane_katcp(d);

  /* NOTE: not sure if this level of paranoia is warranted */
  /* not copying vargs at works on my platform, but not on */
  /* all of them, so not sure how much copying is really needed */

  va_copy(copy, args);
  result = append_vargs_katcl(d->d_line, flags, fmt, copy);
  va_end(copy);

  return result;
}

/**************************************************************/

int vsend_katcp(struct katcp_dispatch *d, va_list ap)
{
  int flags, result, check;
  char *string;
  void *buffer;
  unsigned long value;
  int len;

  check = KATCP_FLAG_FIRST;
  
  do{
    flags = va_arg(ap, int);
    if((check & flags) != check){
      /* WARNING: tests first arg for FLAG_FIRST */
      return -1;
    }
    check = 0;

    switch(flags & KATCP_TYPE_FLAGS){
      case KATCP_FLAG_STRING :
        string = va_arg(ap, char *);
        result = append_string_katcp(d, flags & KATCP_ORDER_FLAGS, string);
        break;
      case KATCP_FLAG_ULONG :
        value = va_arg(ap, unsigned long);
        result = append_unsigned_long_katcp(d, flags & KATCP_ORDER_FLAGS, value);
        break;
      case KATCP_FLAG_XLONG :
        value = va_arg(ap, unsigned long);
        result = append_hex_long_katcp(d, flags & KATCP_ORDER_FLAGS, value);
        break;
      case KATCP_FLAG_BUFFER :
        buffer = va_arg(ap, void *);
        len = va_arg(ap, int);
        result = append_buffer_katcp(d, flags & KATCP_ORDER_FLAGS, buffer, len);
        break;
      default :
        result = (-1);
    }
#ifdef DEBUG
    fprintf(stderr, "vsend: appended: flags=0x%02x, result=%d\n", flags, result);
#endif
    if(result <= 0){
      return -1;
    }
  } while(!(flags & KATCP_FLAG_LAST));

  return 0;
}

/* This function is *NOT* a printf style function, instead
 * a format string, each value to be output is preceeded by
 * a flag field describing its type, as well as its position.
 * The first parameter needs to include the KATCP_FLAG_FIRST flag,
 * the last one has to contain the KATCP_FLAG_LAST. In the case
 * of binary output, it is necessary to include a further parameter
 * containing the length of the value. Consider this function to 
 * be a convenience which allows one perform multiple append_*_katcp 
 * calls in one go
 */

int send_katcp(struct katcp_dispatch *d, ...)
{
  int result;
  va_list ap;

  va_start(ap, d);

  result = vsend_katcp(d, ap);

  va_end(ap);

  return result;
}

/**************************************************************/

#ifdef UNIT_TEST_DISPATCH

#include "netc.h"

int main(int argc, char **argv)
{
  struct katcp_dispatch *d;
  int fd;

  fprintf(stderr, "dispatch.c test\n");

  if(argc > 1){
    fd = net_listen(argv[1], 0, 1);
    if(fd < 0){
      return 1;
    }
  } else {
    fd = STDIN_FILENO;
  }

  d = setup_katcp(fd);
  if(d == NULL){
    fprintf(stderr, "dispatch: start failed\n");
    return 1;
  }

  shutdown_katcp(d);

  return 0;
}
#endif

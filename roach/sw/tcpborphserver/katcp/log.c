#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include "katpriv.h"
#include "katcp.h"
#include "katcl.h"

static char *log_levels_vector[KATCP_MAX_LEVELS] = {
  "trace", 
  "debug", 
  "info", 
  "warn", 
  "error", 
  "fatal",
  "off"
};

int log_to_code(char *name)
{
  int code;

  if(name == NULL){
    return -1;
  }

  for(code = 0; code < KATCP_MAX_LEVELS; code++){
    if(!strncmp(name, log_levels_vector[code], 4)){
      return code;
    }
  }

  return -1;
}

char *log_to_string(int code)
{
  if((code < 0) || (code >= KATCP_MAX_LEVELS)){
    return NULL;
  }

  return log_levels_vector[code];
}

int log_message_katcl(struct katcl_line *cl, int level, char *name, char *fmt, ...)
{
  va_list args;
  int result;

  va_start(args, fmt);
  result = vlog_message_katcl(cl, level, name, fmt, args);
  va_end(args);

  return result;
}

int vlog_message_katcl(struct katcl_line *cl, int level, char *name, char *fmt, va_list args)
{
  int result[5], sum, i;
  struct timeval now;
  unsigned int milli;
  char *subsystem, *logstring;

  logstring = log_to_string(level);
  if(logstring == NULL){
    return -1;
  }

  subsystem = name ? name : "unknown" ;

  gettimeofday(&now, NULL);
  milli = now.tv_usec / 1000;

  result[0] = append_string_katcl(cl, KATCP_FLAG_FIRST, "#log");
  result[1] = append_string_katcl(cl, 0, logstring);
  result[2] = append_args_katcl(cl, 0, "%lu%03d", now.tv_sec, milli);
  result[3] = append_string_katcl(cl, 0, subsystem);

#ifdef DEBUG
  fprintf(stderr, "log: my fmt string is <%s>\n", fmt);
#endif

  result[4] = append_vargs_katcl(cl, KATCP_FLAG_LAST, fmt, args);

  /* do the right thing, collect error codes */
  sum = 0;
  for(i = 0; i < 5; i++){
#ifdef DEBUG
    fprintf(stderr, "log: [%d]=%d\n", i, result[i]);
#endif
    if(result[i] < 0){
      return -1;
    }
    sum += result[i];
  }

  return sum;
}


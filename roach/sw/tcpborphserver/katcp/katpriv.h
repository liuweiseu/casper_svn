#ifndef _KATPRIV_H_
#define _KATPRIV_H_

#include <sys/time.h>

struct katcl_larg{
  unsigned int a_begin;
  unsigned int a_end;
};

struct katcl_line{
  int l_fd;

  char *l_input;
  unsigned int l_isize;
  unsigned int l_ihave;
  unsigned int l_iused;

  struct katcl_larg *l_args;
  unsigned int l_asize;
  unsigned int l_ahave;

  char *l_output;
  unsigned int l_osize;
  unsigned int l_owant;
  unsigned int l_odone;

  int l_error;
  int l_problem;
};

struct katcp_dispatch;

struct katcp_cmd{
  char *c_name;
  char *c_help;
  int (*c_call)(struct katcp_dispatch *d, int argc);
  struct katcp_cmd *c_next;
};

#define KATCP_SENSOR_INTEGER 0 
#define KATCP_SENSORS_COUNT  1

struct katcp_sensor{
  int s_type;
  char *s_name;
  char *s_description;
  char *s_units;
  int s_strategy;

  void *s_more;
  struct timeval s_now;
  struct timeval s_before;
  struct katcp_sensor *s_next;
};

struct katcp_sensor_integer{
  int si_min;
  int si_max;
  int si_current;
  int si_previous;
  int (*si_get)(void *local);
  void *si_local;
};

struct katcp_dispatch{
  int d_level;
  int d_ready;
  int d_run;
  int d_exit;
  struct katcl_line *d_line;
  struct katcp_cmd *d_commands;

  int (*d_current)(struct katcp_dispatch *d, int argc);

  void *d_state;
  void (*d_clear)(struct katcp_dispatch *d, void *p);
  void *d_multi;

  struct katcp_sensor *d_sensors;

  int d_version_major;
  int d_version_minor;
  char *d_version_subsystem; /* not ideally named */

  char *d_build_state;
};

#define KATCP_BUFFER_INC 512
#define KATCP_ARGS_INC     8

void exchange_katcl(struct katcl_line *l, int fd);

void destroy_sensor_katcp(struct katcp_sensor *s);
int append_type_sensor_katcp(struct katcp_sensor *s, struct katcp_dispatch *d);
int append_strategy_sensor_katcp(struct katcp_sensor *s, struct katcp_dispatch *d);



#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "katpriv.h"
#include "katcp.h"

#ifdef DEBUG
static void sane_sensor(struct katcp_sensor *s)
{
  if(s == NULL){
    fprintf(stderr, "sane: null sensor\n");
    abort();
  }
  if((s->s_type < 0) || (s->s_type >= KATCP_SENSORS_COUNT)){
    fprintf(stderr, "sane: null sensor\n");
    abort();
  }
}
#else
#define sane_sensor(s)
#endif

static char *sensor_strategy_table[] = { "off", NULL };

static int strategy_code_sensor_katcp(char *name)
{
  int i;

  for(i = 0; sensor_strategy_table[i] && strcmp(name, sensor_strategy_table[i]); i++);

  return sensor_strategy_table[i] ? i : -1;
}

static char *strategy_name_sensor_katcp(int code)
{
  int i;

  if(code < 0){
    return NULL;
  }

  for(i = 0; sensor_strategy_table[i] && (i < code); i++);

  return sensor_strategy_table[i];
}

static struct katcp_sensor *create_sensor_katcp(char *name, char *description, char *units, int type)
{
  struct katcp_sensor *s;

  s = malloc(sizeof(struct katcp_sensor));
  if(s == NULL){
    return NULL;
  }

  s->s_type = type;
  s->s_name = strdup(name);
  s->s_description = strdup(description);
  s->s_units = strdup(units);
  s->s_more = NULL;

  s->s_strategy = KATCP_STRATEGY_OFF;

  /* could be something saner */
  s->s_now.tv_sec = 0;
  s->s_before.tv_sec = 0;

  s->s_next = NULL;

  if((s->s_name == NULL) || 
     (s->s_description == NULL) || 
     (s->s_units == NULL)){
    destroy_sensor_katcp(s);
    return NULL;
  }

  return s;
}

/* integer specific stuff */

int register_integer_sensor_katcp(struct katcp_dispatch *d, char *name, char *description, char *units, int min, int max, int (*get)(void *local), void *local)
{
  struct katcp_sensor *s;
  struct katcp_sensor_integer *si;

  s = create_sensor_katcp(name, description, units, KATCP_SENSOR_INTEGER);
  if(s == NULL){
    return -1;
  }

  si = malloc(sizeof(struct katcp_sensor_integer));
  if(si == NULL){
    destroy_sensor_katcp(s);
    return -1;
  }

  si->si_min = min;
  si->si_max = max;

  si->si_current = 0;
  si->si_previous = 0;

  si->si_get = get;
  si->si_local = local;

  s->s_more = si;

  s->s_next = d->d_sensors;
  d->d_sensors = s;

  return 0;
}

static int get_integer_sensor(struct katcp_sensor *s)
{
  struct katcp_sensor_integer *si;
  int value;

  si = s->s_more;
 
  value = (*(si->si_get))(si->si_local);

  if((value < si->si_min) || (value > si->si_max)){
    return -1;
  }

  si->si_previous = si->si_current;
  si->si_current = value;

  return 0;
}

static void destroy_integer_sensor(struct katcp_sensor *s)
{
  if(s == NULL){
    return;
  }

  if(s->s_more){
    free(s->s_more);
    s->s_more = NULL;
  }
}

static int append_type_integer_sensor(struct katcp_sensor *s, struct katcp_dispatch *d)
{
  struct katcp_sensor_integer *si;

  if(s == NULL){
    return -1;
  }

  si = s->s_more;

  if(append_unsigned_long_katcp(d, KATCP_FLAG_ULONG, (unsigned long)(si->si_min)) < 0){
    return -1;
  }

  return append_unsigned_long_katcp(d, KATCP_FLAG_ULONG | KATCP_FLAG_LAST, (unsigned long)(si->si_max));
}

/* lookup table */

struct katcp_sensor_lookup{
  char *sd_name;
  int (*sd_get)(struct katcp_sensor *s);
  void (*sd_destroy)(struct katcp_sensor *s);
  int (*sd_append_type)(struct katcp_sensor *s, struct katcp_dispatch *d);
};

static struct katcp_sensor_lookup sensor_type_table[] = {
  [KATCP_SENSOR_INTEGER] = { "integer", &get_integer_sensor, &destroy_integer_sensor, &append_type_integer_sensor }
};

int get_sensor_katcp(struct katcp_sensor *s)
{
  sane_sensor(s);

  return (*(sensor_type_table[s->s_type].sd_get))(s);
}

void destroy_sensor_katcp(struct katcp_sensor *s)
{
  if(s == NULL){
    return;
  }

  if(s->s_more){
    if((s->s_type >= 0) && (s->s_type < KATCP_SENSORS_COUNT)){
      (*(sensor_type_table[s->s_type].sd_destroy))(s);
    }
    s->s_more = NULL;
  }

  if(s->s_name){
    free(s->s_name);
    s->s_name = NULL;
  }
  if(s->s_description){
    free(s->s_description);
    s->s_description = NULL;
  }
  if(s->s_units){
    free(s->s_units);
    s->s_units = NULL;
  }

  s->s_next = NULL;
  s->s_type = (-1);

  free(s);
}

int append_type_sensor_katcp(struct katcp_sensor *s, struct katcp_dispatch *d)
{
  sane_sensor(s);

  if(append_string_katcp(d, KATCP_FLAG_STRING | KATCP_FLAG_FIRST, "#sensor-list") < 0){
    return -1;
  }

  if(append_string_katcp(d, KATCP_FLAG_STRING, s->s_name) < 0){
    return -1;
  }

  if(append_string_katcp(d, KATCP_FLAG_STRING, s->s_description) < 0){
    return -1;
  }

  if(append_string_katcp(d, KATCP_FLAG_STRING, s->s_units) < 0){
    return -1;
  }

  if(append_string_katcp(d, KATCP_FLAG_STRING, sensor_type_table[s->s_type].sd_name) < 0){
    return -1;
  }

  return (*(sensor_type_table[s->s_type].sd_append_type))(s, d);
}

int append_strategy_sensor_katcp(struct katcp_sensor *s, struct katcp_dispatch *d)
{
  char *strategy;

  sane_sensor(s);

  strategy = strategy_name_sensor_katcp(s->s_strategy);

  return send_katcp(d,
    KATCP_FLAG_FIRST | KATCP_FLAG_STRING, "!sensor-sampling",
    KATCP_FLAG_STRING, KATCP_OK,
    KATCP_FLAG_STRING, (strategy ? strategy : "unknown"), 
    KATCP_FLAG_LAST  | KATCP_FLAG_STRING, s->s_name);
}


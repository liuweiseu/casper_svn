/* lower level parsing routes, arbitrarily having the katcl suffix */
/* these routines do their own IO buffering over file descriptors */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "katpriv.h"
#include "katcl.h"
#include "katcp.h"

int problem_katcl(struct katcl_line *l)
{
  int result;

  if(l == NULL){
    return 1;
  }

  result = l->l_problem;

  l->l_problem = 0;

  return result;
}

void destroy_katcl(struct katcl_line *l, int mode)
{
  if(l == NULL){
    return;
  }

  if(l->l_input){
    free(l->l_input);
    l->l_input = NULL;
  }
  l->l_isize = 0;

  if(l->l_args){
    free(l->l_args);
    l->l_args = NULL;
  }
  l->l_asize = 0;

  if(l->l_output){
    free(l->l_output);
    l->l_output = NULL;
  }
  l->l_osize = 0;

  if(mode){
    if(l->l_fd >= 0){
      close(l->l_fd);
      l->l_fd = (-1);
    }
  }

  free(l);
}

struct katcl_line *create_katcl(int fd)
{
  struct katcl_line *l;

  l = malloc(sizeof(struct katcl_line));
  if(l == NULL){
    return NULL;
  }

  l->l_fd = fd;

  l->l_input = NULL;
  l->l_isize = 0;
  l->l_ihave = 0;
  l->l_iused = 0;

  l->l_args = NULL;
  l->l_asize = 0;
  l->l_ahave = 0;

  l->l_output = NULL;
  l->l_osize = 0;
  l->l_owant = 0;
  l->l_odone = 0;

  l->l_error = 0;
  l->l_problem = 0;

  return l;
}

int fileno_katcl(struct katcl_line *l)
{
  return l ? l->l_fd : (-1);
}

void exchange_katcl(struct katcl_line *l, int fd)
{
  if(l->l_fd >= 0){
    close(l->l_fd);
  }
  l->l_fd = fd;

  /* WARNING: exchanging fds forces discarding of pending io */
  l->l_ihave = 0;
  l->l_iused = 0;

  l->l_odone = 0;
  l->l_owant = 0;
}

int read_katcl(struct katcl_line *l)
{
  int rr;
  char *ptr;

  if(l->l_isize <= l->l_ihave){
    ptr = realloc(l->l_input, l->l_isize + KATCP_BUFFER_INC);
    if(ptr == NULL){
#ifdef DEBUG
      fprintf(stderr, "read: realloc to %d failed\n", l->l_isize + KATCP_BUFFER_INC);
#endif
      l->l_error = ENOMEM;
      return -1;
    }

    l->l_input = ptr;
    l->l_isize += KATCP_BUFFER_INC;
  }

  rr = read(l->l_fd, l->l_input + l->l_ihave, l->l_isize - l->l_ihave);
  if(rr < 0){
    switch(errno){
      case EAGAIN :
      case EINTR  :
        return 0; /* more to be read */
      default : /* serious error */
        l->l_error = errno;
#ifdef DEBUG
        fprintf(stderr, "read: read error: %s\n", strerror(errno));
#endif
        return -1;
    }
  }

  if(rr == 0){ /* EOF */
#ifdef DEBUG
    fprintf(stderr, "read: end of file\n");
#endif
    return 1;
  }

  l->l_ihave += rr;
  return 0;
}

#define STATE_PRETOKEN  0 
#define STATE_INTOKEN   1
#define STATE_POSTOKEN  2

static int extract_katcl(struct katcl_line *l)
{
  int escape, state;
  unsigned int i, j;
  struct katcl_larg *ta;

  if(l->l_asize <= (l->l_ahave + 1)){
    ta = realloc(l->l_args, sizeof(struct katcl_larg) * (l->l_asize + KATCP_ARGS_INC));
    if(ta == NULL){
      l->l_error = ENOMEM;
      return -1;
    }
    l->l_args = ta;
    l->l_asize += KATCP_ARGS_INC;
  }

  ta = &(l->l_args[l->l_ahave]);

  j = i = l->l_iused;
  escape = 0;

#ifdef DEBUG
  fprintf(stderr, "extract: scaning from %d to %d\n", j, l->l_ihave);
#endif

  state = STATE_PRETOKEN;

  ta->a_begin = j;
  while(i < l->l_ihave){
    if(escape){
      escape = 0;
      switch(l->l_input[i]){
        case 'e' :
          l->l_input[j] = 27;
          j++;
          break;
        case 'n' :
          l->l_input[j] = '\n';
          j++;
          break;
        case 'r' :
          l->l_input[j] = '\r';
          j++;
          break;
        case 't' :
          l->l_input[j] = '\t';
          j++;
          break;
        case '0' :
          l->l_input[j] = '\0';
          j++;
          break;
        case '_' :
          l->l_input[j] = ' ';
          j++;
          break;
        case '@' :
          break;
        /* case ' ' : */
        /* case '\\' : */
        default :
          l->l_input[j] = l->l_input[i];
          j++;
          break;
      }
    } else {
      switch(state){
        case STATE_PRETOKEN :
          switch(l->l_input[i]){
            case ' '  :
            case '\t' :
            case '\n' :
            case '\r' :
              break;
            case '\\' :
              state = STATE_INTOKEN;
              escape = 1;
              break;
            default   :
              l->l_input[j] = l->l_input[i];
              j++;
              state = STATE_INTOKEN;
            break;
          }
        break;

        case STATE_INTOKEN  :
          switch(l->l_input[i]){
            case ' '  :
            case '\t' :
              ta->a_end = j;
              l->l_input[j] = '\0';
              l->l_ahave++;
              state = STATE_POSTOKEN;
              break;
            case '\n' :
            case '\r' :
              ta->a_end = j;
              l->l_input[j] = '\0';
              l->l_ahave++;
              l->l_iused = i + 1;
              return 1;
            case '\\' :
              escape = 1;
              break;
            default   :
              l->l_input[j] = l->l_input[i];
              j++;
            break;
          }
        break;

        case STATE_POSTOKEN :
          switch(l->l_input[i]){
            case ' '  :
            case '\t' :
              break;
            case '\n' :
            case '\r' :
              l->l_iused = i + 1;
              return 1;
            default :
              l->l_iused = i;
              return 0;
          }
        break;
      }
    }
    i++; 
  }

  /* logic problem */
  l->l_error = EINVAL;
  return -1;
}

int have_katcl(struct katcl_line *l)
{
  unsigned int i, start, stop;
  int result, newline;

  if(l->l_iused >= l->l_ihave){
    l->l_ihave = 0;
    l->l_iused = 0;
    return 0;
  }

  if (l->l_iused > (l->l_isize / 2)){
    memmove(l->l_input, l->l_input + l->l_iused, l->l_ihave - l->l_iused);
    l->l_ihave -= l->l_iused;
    l->l_iused = 0;
  }

  start = l->l_ihave;
  stop = l->l_ihave;
  i = l->l_iused;
  newline = 1;

#ifdef DEBUG
  fprintf(stderr, "have: checking from <%u-%u> (%c...%c)\n", i, stop, l->l_input[i], l->l_input[stop - 1]);
#endif

  while(i < stop){
    switch(l->l_input[i]){
      case '\n' :
      case '\r' :
        newline = 1;
        if(start < l->l_ihave){
          stop = i;
          l->l_iused = start;
#ifdef DEBUG
    fprintf(stderr, "have: line <%u-%u> (%c...%c)\n", start, stop, l->l_input[start], l->l_input[stop]);
#endif
        }
        break;
      case KATCP_INFORM  :
      case KATCP_REPLY   :
      case KATCP_REQUEST :
        if(newline){
          start = i;
        }
        newline = 0;
        break;
      case ' '  : 
      case '\t' : 
        break;
      default :
        if(newline){
          l->l_problem++;
        }
        newline = 0;
        break;
    }
    i++;
  }
  
  if(stop >= l->l_ihave){
    return 0;
  }

  l->l_ahave = 0;

  while(!(result = extract_katcl(l)));

  return result;
}

/******************************************************************/

unsigned int arg_count_katcl(struct katcl_line *l)
{
  return l->l_ahave;
}

static int arg_type_katcl(struct katcl_line *l, char mode)
{
  if(l->l_ahave <= 0){
    return 0;
  }

  if(l->l_input[l->l_args[0].a_begin] == mode){
    return 1;
  }

  return 0;
}

int arg_request_katcl(struct katcl_line *l)
{
  return arg_type_katcl(l, KATCP_REQUEST);
}

int arg_reply_katcl(struct katcl_line *l)
{
  return arg_type_katcl(l, KATCP_REPLY);
}

int arg_inform_katcl(struct katcl_line *l)
{
  return arg_type_katcl(l, KATCP_INFORM);
}

int arg_null_katcl(struct katcl_line *l, unsigned int index)
{
  if(index >= l->l_ahave){
    return 1;
  } 

  if(l->l_args[index].a_begin >= l->l_args[index].a_end){
    return 1;
  }

  return 0;
}

char *arg_string_katcl(struct katcl_line *l, unsigned int index)
{
  if(index >= l->l_ahave){
    return NULL;
  }

  if(l->l_args[index].a_begin >= l->l_args[index].a_end){
    return NULL;
  }

  return l->l_input + l->l_args[index].a_begin;
}

char *arg_copy_string_katcl(struct katcl_line *l, unsigned int index)
{
  char *ptr;

  ptr = arg_string_katcl(l, index);
  if(ptr){
    return strdup(ptr);
  } else {
    return NULL;
  }
}

unsigned long arg_unsigned_long_katcl(struct katcl_line *l, unsigned int index)
{
  unsigned long value;

  if(index >= l->l_ahave){
    return 0;
  } 

  value = strtoul(l->l_input + l->l_args[index].a_begin, NULL, 0);

  return value;
}

unsigned int arg_buffer_katcl(struct katcl_line *l, unsigned int index, void *buffer, unsigned int size)
{
  unsigned int want, done;

  if(index >= l->l_ahave){
    return 0;
  } 

  want = l->l_args[index].a_end - l->l_args[index].a_begin;
  done = (want > size) ? size : want;

  if(buffer && (want > 0)){
    memcpy(buffer, l->l_input + l->l_args[index].a_begin, done);
  }

  return want;
}

/******************************************************************/

int append_string_katcl(struct katcl_line *l, int flags, void *buffer)
{
  return append_buffer_katcl(l, flags, buffer, strlen(buffer));
}

int append_unsigned_long_katcl(struct katcl_line *l, int flags, unsigned long v)
{
#define TMP_BUFFER 32
  char buffer[TMP_BUFFER];
  int result;

  result = snprintf(buffer, TMP_BUFFER, "%lu", v);
  if((result <= 0) || (result >= TMP_BUFFER)){
    return -1;
  }

  return append_buffer_katcl(l, flags, buffer, result);
#undef TMP_BUFFER
}

int append_hex_long_katcl(struct katcl_line *l, int flags, unsigned long v)
{
#define TMP_BUFFER 32
  char buffer[TMP_BUFFER];
  int result;

  result = snprintf(buffer, TMP_BUFFER, "0x%lx", v);
  if((result <= 0) || (result >= TMP_BUFFER)){
    return -1;
  }

  return append_buffer_katcl(l, flags, buffer, result);
#undef TMP_BUFFER
}

int append_buffer_katcl(struct katcl_line *l, int flags, void *buffer, int len)
{
  /* returns greater than zero on success */

  unsigned int newsize, had, want, result, i, problem;
  char *s, *tmp, v;

  problem = KATCP_FLAG_MORE | KATCP_FLAG_LAST;
  if((flags & problem) == problem){
#ifdef DEBUG
    fprintf(stderr, "append: usage problem: can not have last and more together\n");
#endif
    return -1;
  }

  if(len < 0){
    return -1;
  }

  s = buffer;
  want = s ? len : 0;

  /* extra checks */
  if(flags & KATCP_FLAG_FIRST){
    if(s == NULL){
      return -1;
    }
    switch(s[0]){
      case KATCP_INFORM  :
      case KATCP_REPLY   :
      case KATCP_REQUEST :
        break;
      default :
        return -1;
    }
  }

  had = l->l_owant;

  for(i = 0; i < want; i++){
    if((l->l_owant + 8) >= l->l_osize){
      newsize = l->l_osize + KATCP_BUFFER_INC;
      tmp = realloc(l->l_output, newsize);
      if(tmp == NULL){
#ifdef DEBUG
        fprintf(stderr, "append: unable to resize output to <%u> bytes\n", newsize);
#endif
        return -1;
      } 

      l->l_output = tmp;
      l->l_osize = newsize;
    }
    switch(s[i]){
      case  27  : v = 'e';  break;
      case '\n' : v = 'n';  break;
      case '\r' : v = 'r';  break;
      case '\0' : v = '0';  break;
      case '\\' : v = '\\'; break;
      case ' '  : v = '_';  break; 
#if 0
      case ' '  : v = ' ';  break; 
#endif
      case '\t' : v = 't';  break;
      default   : 
        v = 0; 
        l->l_output[l->l_owant++] = s[i];
        break;
    }
    if(v){
      l->l_output[l->l_owant++] = '\\';
      l->l_output[l->l_owant++] = v;
    }
  }

  /* check if we are dealing with null args, if so put in null token */
  if(!(flags & KATCP_FLAG_MORE) || (flags & KATCP_FLAG_LAST)){
    if((l->l_owant > 0) && (l->l_output[l->l_owant - 1] == ' ')){
#ifdef DEBUG
      fprintf(stderr, "append: warning: empty argument considered bad form\n");
#endif
      l->l_output[l->l_owant++] = '\\';
      l->l_output[l->l_owant++] = '@';
    }
  }

  if(flags & KATCP_FLAG_LAST){
    l->l_output[l->l_owant++] = '\n';
  } else if(!(flags & KATCP_FLAG_MORE)){
    l->l_output[l->l_owant++] = ' ';
  }

  result = l->l_owant - had;

  return result;
}

int append_vargs_katcl(struct katcl_line *l, int flags, char *fmt, va_list args)
{
  char *tmp, *buffer;
  va_list copy;
  int want, got, x, result;

  got = strlen(fmt) + 16;
#ifdef DEBUG
  fprintf(stderr, "vargs: my fmt string is <%s>\n", fmt);
#endif

  /* in an ideal world this would be an insitu copy to save the malloc */
  buffer = NULL;

  for(x = 1; x < 8; x++){ /* paranoid nutter check */
    tmp = realloc(buffer, sizeof(char) * got);
    if(tmp == NULL){
#ifdef DEBUG
      fprintf(stderr, "append: unable to allocate %d tmp bytes\n", got);
#endif
      free(buffer);
      return -1;
    }

    buffer = tmp;

    va_copy(copy, args);
    want = vsnprintf(buffer, got, fmt, copy);
    va_end(copy);
#ifdef DEBUG
    fprintf(stderr, "vargs: printed <%s> (iteration=%d, want=%d, got=%d)\n", buffer, x, want, got);
#endif

    if((want >= 0) && ( want < got)){
      result = append_buffer_katcl(l, flags, buffer, want);
      free(buffer);
      return result;
    }

    if(want >= got){
      got = want + 1;
    } else {
      /* old style return codes, with x termination check */
      got *= 2;
    }

  }

#ifdef DEBUG
  fprintf(stderr, "append: sanity failure with %d bytes\n", got);
  abort();
#endif

  return -1;
}

int append_args_katcl(struct katcl_line *l, int flags, char *fmt, ...)
{
  va_list args;
  int result;

  va_start(args, fmt);

  result = append_vargs_katcl(l, flags, fmt, args);

  va_end(args);

  return result;
}

/* need a vappend function here to append a single escaped parameter from vprint */

int vprint_katcl(struct katcl_line *l, int full, char *fmt, va_list args)
{
  unsigned int newsize, delta;
  int actual;
  char *tmp;
  va_list copy;

  /* Warning: no string escaping happens in here, would need to 
   * provide other routines to escape data */

  for(;;){

#ifdef DEBUG
    if(l->l_osize < l->l_owant){
      fprintf(stderr, "vprint: logic problem: want(%u) to clobber maximum(%u)\n", l->l_owant, l->l_osize);
      abort();
    }
#endif

    va_copy(copy, args);

    delta = l->l_osize - l->l_owant; 
    actual = vsnprintf(l->l_output + l->l_owant, delta, fmt, copy);

    va_end(copy);

    if((actual >= 0) && (actual < delta)){
      if(full){
        switch(l->l_output[l->l_owant]){
          case KATCP_INFORM  :
          case KATCP_REPLY   :
          case KATCP_REQUEST :
            break;
          default :
            return -1;
        }
      }
      l->l_owant += actual;
      if(full){
        if(l->l_output[l->l_owant - 1] != '\n'){
          l->l_output[l->l_owant++] = '\n';
        }
      }
#ifdef DEBUG
      fprintf(stderr, "vprint: new buffer <%s>\n", l->l_output);
#endif
      return 0;
    } 

#if 0
    newsize = (actual >= 0) ? (l->l_owant + actual + 2) : (l->l_osize + KATCP_BUFFER_INC);
#endif

    newsize = l->l_osize + KATCP_BUFFER_INC;
    tmp = realloc(l->l_output, newsize);
    if(tmp == NULL){
#ifdef DEBUG
      fprintf(stderr, "vprint: unable to resize output to <%u> bytes\n", newsize);
#endif
      return -1;
    } 

    l->l_output = tmp;
    l->l_osize = newsize;
  }
}

int print_katcl(struct katcl_line *l, int full, char *fmt, ...)
{
  va_list args;
  int result;

  va_start(args, fmt);

  result = vprint_katcl(l, full, fmt, args);

  va_end(args);

  return result;
}

/***************************/

int write_katcl(struct katcl_line *l)
{
  int wr;

  if(l->l_odone < l->l_owant){
    wr = write(l->l_fd, l->l_output + l->l_odone, l->l_owant - l->l_odone);
    if(wr < 0){
      switch(errno){
        case EAGAIN :
        case EINTR  :
          return 0;
        default :
          l->l_error = errno;
          return -1;
      }
    } else {
      l->l_odone += wr;
    }
  }

  if(l->l_odone >= l->l_owant){
    l->l_odone = 0;
    l->l_owant = 0;
    return 1;
  }

  /* returns zero if still more to do */

  return 0;
}

int flushing_katcl(struct katcl_line *l)
{
  return (l->l_odone < l->l_owant) ? (l->l_owant - l->l_odone) : 0;
}

/***************************/

int error_katcl(struct katcl_line *l)
{
  return l->l_error;
}

void dump_katcl(struct katcl_line *l, FILE *fp)
{
  unsigned int i, j;

  fprintf(stderr, "input: used=%u, have=%u\n", l->l_iused, l->l_ihave);

  for(i = 0; i < l->l_ahave; i++){
    fprintf(fp, "arg[%u]: ", i);
    for(j = l->l_args[i].a_begin; j < l->l_args[i].a_end; j++){
      if(isprint(l->l_input[j])){
        fprintf(fp, "%c", l->l_input[j]);
      } else {
        fprintf(fp, "\\%02x", l->l_input[j]);
      }
    }
    fprintf(fp, "\n");
  }
}

#ifdef UNIT_TEST_LINE

int main()
{
  struct katcl_line *l;
  char *ptr;

  fprintf(stderr, "line.c test\n");

  l = create_katcl(STDIN_FILENO);
  if(l == NULL){
    fprintf(stderr, "main: unable to create katcl\n");
    return 1;
  }

  while(!read_katcl(l)){
    while(have_katcl(l) > 0){
      ptr = arg_string_katcl(l, 0);
      if(ptr && !strcmp(ptr, "?hello")){
        print_katcl(l, 1, "!goodbye");
        print_katcl(l, 1, "#mumble");
        while(!write_katcl(l));
      }

      dump_katcl(l, stdout);
    }
  }
  
  destroy_katcl(l, 1);

  return 0;
}

#endif

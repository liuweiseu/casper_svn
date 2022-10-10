/*****************************************************************************
 *
 * FILE: swapdata.c
 *
 *   Generic routines for big endian/little endian swapping.
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * jeff          9 Oct 2002    Original coding
 * lerner        6 Feb 2007    Added 'wapplib.h' and cleaned up for '-Wall'
 *
 *****************************************************************************/

#include "wapplib.h"



double swapd(double *d);
long swapl(long *l);
int swapi(int *l);
unsigned short swaps(unsigned short *s);
void in_swapus(unsigned short *s);
void in_swapf(float *f);
void in_swaps(short *s);
void in_swapd(double *d);
void in_swapl(long *ll);
void in_swapi(int *ii);
void set_swapd(double *p, double d);
void set_swapi(int *p, int l);



double swapd(double *d) {

  union {
    double d;
    unsigned char c[sizeof(double)];
  } u;
  unsigned char t;

  u.d = *d;

  t = u.c[0];
  u.c[0] = u.c[7];
  u.c[7] = t;

  t = u.c[1];
  u.c[1] = u.c[6];
  u.c[6] = t;

  t = u.c[2];
  u.c[2] = u.c[5];
  u.c[5] = t;

  t = u.c[3];
  u.c[3] = u.c[4];
  u.c[4] = t;

  return(u.d);
}



/* return but dont swap */

long swapl(long *l) {

  union {
    long l;
    unsigned char c[4];
  } u;
  unsigned char temp;

  u.l = *l;
  temp = u.c[0];
  u.c[0] = u.c[3];
  u.c[3] = temp;

  temp = u.c[1];
  u.c[1] = u.c[2];
  u.c[2] = temp;

  return(u.l);
}



int swapi(int *l) {

  union {
    int l;
    unsigned char c[4];
  } u;
  unsigned char temp;

  u.l = *l;
  temp = u.c[0];
  u.c[0] = u.c[3];
  u.c[3] = temp;

  temp = u.c[1];
  u.c[1] = u.c[2];
  u.c[2] = temp;

  return(u.l);
}



unsigned short swaps(unsigned short *s) {

  union {
    unsigned s;
    unsigned char c[2];
  } u;
  unsigned char temp;

  u.s = *s;
  temp = u.c[0];
  u.c[0] = u.c[1];
  u.c[1] = temp;

  return(u.s);
}



void in_swapus(unsigned short *s) {

  union SWAPS {
    unsigned short *s;
    unsigned char c[2];
  } *u;
  unsigned char t;

  u = (union SWAPS *) s;

  t = u->c[0];
  u->c[0] = u->c[1];
  u->c[1] = t;
}



void in_swapf(float *f) {

  union SWAPF {
    float f;
    unsigned char c[4];
  } *u;
  unsigned char t;

  u = (union SWAPF *) f;

  t = u->c[0];
  u->c[0] = u->c[3];
  u->c[3] = t;

  t = u->c[1];
  u->c[1] = u->c[2];
  u->c[2] = t;
}



void in_swaps(short *s) {

  union SWAPS {
    short s;
    unsigned char c[2];
  } *u;
  unsigned char t;

  u = (union SWAPS *) s;

  t = u->c[0];
  u->c[0] = u->c[1];
  u->c[1] = t;
}



void in_swapd(double *d) {

  union SWAPD {
    double d;
    unsigned char c[4];
  } *u;
  unsigned char t;

  u = (union SWAPD *) d;

  t = u->c[0];
  u->c[0] = u->c[7];
  u->c[7] = t;

  t = u->c[1];
  u->c[1] = u->c[6];
  u->c[6] = t;

  t = u->c[2];
  u->c[2] = u->c[5];
  u->c[5] = t;

  t = u->c[3];
  u->c[3] = u->c[4];
  u->c[4] = t;
}



void in_swapl(long *ll) {

  union SWAPLL {
    long l;
    unsigned char c[4];
  } *u;
  unsigned char t;

  u = (union SWAPLL *) ll;

  t = u->c[0];
  u->c[0] = u->c[3];
  u->c[3] = t;

  t = u->c[1];
  u->c[1] = u->c[2];
  u->c[2] = t;
}



void in_swapi(int *ii) {

  union SWAPII {
    int i;
    unsigned char c[4];
  } *u;
  unsigned char t;

  u = (union SWAPII *) ii;

  t = u->c[0];
  u->c[0] = u->c[3];
  u->c[3] = t;

  t = u->c[1];
  u->c[1] = u->c[2];
  u->c[2] = t;
}



void set_swapd(double *p, double d) {

  union {
    double d;
    unsigned char c[sizeof(double)];
  } u;
  unsigned char t;

  u.d = d;

  t = u.c[0];
  u.c[0] = u.c[7];
  u.c[7] = t;

  t = u.c[1];
  u.c[1] = u.c[6];
  u.c[6] = t;

  t = u.c[2];
  u.c[2] = u.c[5];
  u.c[5] = t;

  t = u.c[3];
  u.c[3] = u.c[4];
  u.c[4] = t;

  *p = u.d;
}



void set_swapi(int *p, int l) {

  union {
    int l;
    unsigned char c[4];
  } u;
  unsigned char temp;

  u.l = l;
  temp = u.c[0];
  u.c[0] = u.c[3];
  u.c[3] = temp;

  temp = u.c[1];
  u.c[1] = u.c[2];
  u.c[2] = temp;

  *p = u.l;
}

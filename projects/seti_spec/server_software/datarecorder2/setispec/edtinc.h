#ifndef _EDTINC_H_
#define _EDTINC_H_

//#include <sys/types.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <errno.h>

#include <linux/ioctl.h>

#include <pthread.h>
#include <stdlib.h>

typedef struct edt_device {
    unsigned int current_buf;
} EdtDev;

typedef struct edt_buffer {
    void * data;
    unsigned int buffer_size;
} edt_buffer;

// begin putting data into the buffer
int             edt_start_buffers(EdtDev *edt_p, unsigned int count);
// block until buffer is filled/data is ready return address of data in memory
void edt_wait_for_buffers(EdtDev *edt_p, edt_buffer * buf, int count);
// return the number of completed buffers
unsigned long        edt_done_count(EdtDev   *edt_p);
// error messages
void            edt_perror(char *str);

// TODO: delete references to these functions
// they are all edt specific and should be ignored
void				edt_flush_fifo(EdtDev *edt_p);



//stubs from edt.c
void InitEdt(int dsi);
EdtDev * edt_open(int dsi);
void  EdtCntl(EdtDev * edt_p, int action);
void  CloseEdt(int dsi);

//stubs from adc_to_edt_funcs.C
unsigned short read_vgc(EdtDev *edt_p, int chan);

void get_next_buf(edt_buffer *buf, int i);

#endif /*EDT_STUBS_H*/


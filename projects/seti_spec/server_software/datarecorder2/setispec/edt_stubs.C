#include "edtinc.h"
#include "udp_functions.h"

//stubs from edt.c
//initialize to read (open the socket etc.)
EdtDev * edt_open(int dsi) 
{
    //TODO create matching free
    EdtDev * newedtdev = (EdtDev *) malloc(sizeof(EdtDev));
    initialize_buffers();
    udp_open();
    return newedtdev;
}

// begin putting data into the buffer
// return 0 for success nonzero for error
// count=0 means freerunning mode
int edt_start_buffers(EdtDev *edt_p, unsigned int count)  
{
    int iret1,iret2;
    int *val = 0;
    launch_buf_threads();
    
    edt_p->current_buf=0;
    return 0;
}


void edt_wait_for_buffers(EdtDev *edt_p, edt_buffer * buf, int count) 
{
    get_next_buf(buf, edt_p->current_buf);
    fprintf(stderr, "returning spectra %i to mem thread\n", edt_p->current_buf);
    edt_p->current_buf = (edt_p->current_buf+1)%RINGBUFFSIZE;
    return;
}


// return the number of completed buffers
unsigned long edt_done_count(EdtDev   *edt_p) 
{
    return total_spectra;
}


// Print an error message after failure
void edt_perror(char *str)
{
    fprintf(stderr, str);
}


//close the socket, stop receiving
void  CloseEdt(int dsi) 
{
    join_buf_threads();
    udp_close();
    close_buffers();
}


//edt specific - ignore
// TODO : figure out what this should return
unsigned short read_vgc(EdtDev *edt_p, int chan) 
{
    return 0;
}
//edt specific - ignore
void				edt_flush_fifo(EdtDev *edt_p) {}
//edt specific - ignore
void  EdtCntl(EdtDev * edt_p, int action) {}

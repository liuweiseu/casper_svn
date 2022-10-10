/* #pragma ident "@(#)libpdma.h	1.12 08/12/02 EDT" */

/*
 * See /opt/EDTpcd/README.pdma
 */

#ifndef _LIBPDMA_H_
#define _LIBPDMA_H_

#define PDMA_MAX_SIZE ((4*1024*1024) - (256 * 1024))

int	pdma_init(EdtDev *edt_p, int bufsize, int numbufs, u_char **bufs) ;
void 	pdma_close(EdtDev *edt_p) ;
int 	pdma_set_size(EdtDev *edt_p, int size) ;
int	pdma_write_databuf(EdtDev *edt_p, int bufnum, void *buf, int size) ;
void 	pdma_start_dma(EdtDev *edt_p) ;
void	pdma_flush_fifo(EdtDev *edt_p) ;
void	pdma_abort_dma(EdtDev * edt_p) ;
void	pdma_dmaint_enable(EdtDev * edt_p, int enable) ;


#endif /* _LIBPDMA_H */

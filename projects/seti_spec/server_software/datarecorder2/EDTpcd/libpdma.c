/* #pragma ident "@(#)libpdma.c	1.17 04/23/03 EDT" */

/*
 * See /opt/EDTpcd/README.pdma
 */

#include "edtinc.h"
#include "libpdma.h"
#include <sys/mman.h>


static volatile u_int  *dma_sglist ;		/* mmap to sglist 	   */
static volatile u_int   dma_sglist_copy[PDMA_SIZE/4] ;
static volatile u_int  *dmacmd = NULL ;		/* mmap to SG_NXT_CNT_CTRL */
static volatile u_int   dmacmd_copy ;
static volatile u_int   dmacmd_master_copy ;
static volatile u_int  *dmacfg ;		/* mmap to EDT_DMA_INTCFG  */
static volatile u_int   dmacfg_copy ;
static volatile u_int  *dmacnt ;		/* mmap to EDT_DMA_CUR_CNT */
static volatile u_char *off_p ;			/* mmap to intfc regs      */
static volatile u_char *data_p ;		/* mmap to intfc regs      */
static volatile u_int   dma_count = 0 ;		/* num sglist entries	   */
static volatile u_int   pdma_size = 0 ;		/* max pdma size in bytes  */
static volatile u_int   pdma_regs ;		/* Linux requires 2 addrs  */
static 		int	pagesize ;

static u_char	      **pdma_databufs ;
static EdtDev	       *saved_edt_p = NULL ;


static u_int regswap(u_int val) ;




u_int
regswap(u_int val)
{
#ifndef __i386
    /* byteswap */
    val = ((val & 0xFF00FF00) >> 8) | ((val & 0x00FF00FF) << 8) ;

    /* shortswap */
    val = ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16) ;
#endif

    return val ;
}



int
pdma_init(EdtDev *edt_p, int bufsize, int numbufs, u_char **bufs)
{
    int i ;
    u_int mode ;
    volatile caddr_t mapaddr ;

    pagesize = getpagesize() ;

    if (bufsize > PDMA_MAX_SIZE)
    {
	fprintf(stderr, "pdma_init:  bufsize (%d) exceeds PDMA_MAX_SIZE (%d)\n",
						       bufsize, PDMA_MAX_SIZE) ;
	return -1 ;
    }

    if (numbufs != 1)
    {
	fprintf(stderr, "pdma_init:  number of buffers must be 1.\nContinuing with one buffer.\n") ;
	numbufs = 1 ;
    }


    /*
     * Map in the PCI registers and set a pointer to the DMA command register
     */

    if (dmacmd == NULL || edt_p != saved_edt_p)
    {
	mapaddr = (volatile caddr_t) edt_mapmem(edt_p, 0, 256) ;

	if ((int) mapaddr == 0)
	{
	    edt_perror("edt_mapmem failed") ;
	    return -1 ;
	}

	dmacmd  = (volatile u_int *) (mapaddr + (EDT_SG_NXT_CNT & 0xff)) ;
	dmacfg  = (volatile u_int *) (mapaddr + (EDT_DMA_INTCFG & 0xff)) ;
	dmacnt  = (volatile u_int *) (mapaddr + (EDT_DMA_CUR_CNT & 0xff)) ;

	
	off_p  = (volatile u_char *) mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = (volatile u_char *) mapaddr + (EDT_REMOTE_DATA & 0xff) ;
    }



    /*
     * Enable Programmed DMA mode, form a single ring buffer, and
     * map in the buffer's scatter-gather list
     */

#if defined(__linux__)
    dma_sglist = (u_int *) edt_alloc(PDMA_SIZE) ;
    edt_ioctl(edt_p, EDTS_PDMA_MODE, &dma_sglist) ;
#else
    mode = 1 ;
    edt_ioctl(edt_p, EDTS_PDMA_MODE, &mode) ;
#endif


    if (edt_configure_ring_buffers(edt_p, bufsize, numbufs, EDT_WRITE, bufs)
								       != 0)
    	return -1 ;

    pdma_databufs = edt_buffer_addresses(edt_p) ;
    edt_start_buffers(edt_p, 0) ;


    pdma_size = bufsize ;


#if defined(__sun)
    mapaddr = (volatile caddr_t) mmap((caddr_t)0, PDMA_SIZE,
    	PROT_READ|PROT_WRITE, MAP_SHARED, edt_p->fd, PDMA_OFFSET);

    if ((int) mapaddr == -1)
    {
	perror("mmap call");
	return -1 ;
    }
    dma_sglist = (volatile u_int *) mapaddr ;
#endif


    memcpy((u_char *)dma_sglist_copy, (u_char *)dma_sglist, PDMA_SIZE) ;

    saved_edt_p = edt_p ;

    edt_ioctl(edt_p, EDTG_PDMA_REGS, (u_int *) &pdma_regs) ;
    dmacmd_master_copy = pdma_regs ;

    return 0 ;
}


void
pdma_close(EdtDev *edt_p)
{
    int mode = 0 ;

    if (saved_edt_p == edt_p)
    {
#if defined(__sun)
	munmap((caddr_t) dma_sglist, PDMA_SIZE) ;
#endif
	edt_disable_ring_buffers(edt_p) ;
    }

    edt_ioctl(edt_p, EDTS_PDMA_MODE, &mode) ;
    edt_free((u_char *) dma_sglist) ;
}


int
pdma_set_size(EdtDev *edt_p, int size)
{
    int i ;


    /*
     * Return error if larger than pdma_size or not divisible by 4.
     */
    if (size > pdma_size || size != (size & ~0x3))
    	return -1 ;



    memcpy((u_char *)dma_sglist, (u_char *)dma_sglist_copy, PDMA_SIZE) ;



    dma_count = 0 ;
    for (i = 1; size > 0; i += 2)
    {
    	if (size > regswap(dma_sglist[i]))
	{
	    size -= regswap(dma_sglist[i]) ;
	    ++ dma_count ;
	}
	else
	{
	    dma_sglist[i] = regswap(size | EDT_LIST_PAGEINT) ;
	    ++ dma_count ;
	    break ;
	}
    }

    if ((dma_count << 3) > pagesize)
	dma_count = pagesize >> 3 ;



    return 0 ;
}


void
pdma_start_dma(EdtDev *edt_p)
{
    dmacmd_copy = dmacmd_master_copy ;

    dmacmd_copy &= ~0xffff ;
    dmacmd_copy |= (dma_count << 3) ;

    *dmacmd = regswap(dmacmd_copy) ;
}


void
pdma_flush_fifo(EdtDev * edt_p)
{
    unsigned char cmd;
    unsigned int dmy;

/* The PCI fifo is not used during output */
#if 0
    /* Turn off	the PCI	fifo */
    dmacfg_copy = regswap(*dmacfg) ;
    dmacfg_copy &= (~EDT_RFIFO_ENB);
    *dmacfg = regswap(dmacfg_copy) ;
    dmacfg_copy = regswap(*dmacfg) ;
#endif


    /* reset the interface fifos */
    *off_p = PCD_CMD & 0xff ;
    dmy = *off_p ;
    cmd = *data_p ;
    cmd &= ~PCD_ENABLE;
    *data_p = cmd ;
    dmy = *data_p ;
    cmd |= PCD_ENABLE;
    *data_p = cmd ;
    dmy = *data_p ;


/* The PCI fifo is not used during output */
#if 0
    /* Turn	on the PCI fifos, which	flushes	them */
    dmacfg_copy |= (EDT_RFIFO_ENB);
    *dmacfg = regswap(dmacfg_copy) ;
    dmacfg_copy = regswap(*dmacfg) ;
#endif
}

void
pdma_dmaint_enable(EdtDev * edt_p, int enable)
{
    if (enable)
	dmacmd_master_copy |= EDT_EN_MN_DONE ;
    else
	dmacmd_master_copy &= ~EDT_EN_MN_DONE ;
}

void
pdma_abort_dma(EdtDev * edt_p)
{

    dmacmd_copy = regswap(*dmacmd) ;

    dmacmd_copy |= EDT_DMA_ABORT ;
   *dmacmd = regswap(dmacmd_copy) ;
    dmacmd_copy = regswap(*dmacmd) ;
}

int
pdma_write_databuf(EdtDev *edt_p, int bufnum, void *buf, int size)
{
    if (size > PDMA_MAX_SIZE)
    {
	fprintf(stderr,
		"pdma_write_databuf: size (%d) exceeds PDMA_MAX_SIZE (%d)\n",
						       size, PDMA_MAX_SIZE) ;
	return -1 ;
    }

    memcpy(pdma_databufs[bufnum], buf, size) ;

    return size ;
}

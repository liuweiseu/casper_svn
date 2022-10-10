
/*********************************************
 * 
 * Edt Driver Wrapper Code 
 * Copyright EDT 2004
 *
 * Note - all functions called from the edt library must be declared asmlinkage, 
 * as must the functions in edt_lnx_kernel.c
 *
 *********************************************/

#ifndef _EDT_LNX_KERNEL_H
#define _EDT_LNX_KERNEL_H



#include <linux/types.h>



#include <linux/time.h>
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/time.h>

/* for asmlinkage */
#include <linux/linkage.h>

typedef struct
{

    unsigned long size;

    // for user_mem - kiobuf if USE_IOBUF, page_list in 2.6 from get_user_pages
    // if user_mem is false, mapping is array of page pointers

    void   *mapping;
    void   *vma;		// hold vma for get_user_pages
    unsigned long mapping_size;
    unsigned short npages; // number of pages in user space
    unsigned short offset;
    unsigned char user_mem;
    unsigned char own_mapping;
    unsigned char is_mapped;
    unsigned char rw;

} EdtMemMapping;


/**************************************************/
/* Edt wrapper functions around kernel calls */
/**************************************************/


/* typedefs for kernel structures - all code except edt_lnx_kernel doesn't  
   know about these */

typedef void *EdtWaitHandle;
typedef void *EdtKiobuf;
typedef void *EdtPciDev;
typedef void *EdtSpinLock_t;
typedef void *EdtPage;


typedef struct
{
    unsigned int address;
    unsigned int size;
} EdtMemSlot;


asmlinkage void    edt_printk (char *fmt, ...);

asmlinkage void    edt_free_irq (unsigned int irq, void *dev_id);

asmlinkage void   *edt_memcpy (void *dest, void *src, size_t length);
asmlinkage void   *edt_memset (void *dest, int c, size_t length);

asmlinkage int     edt_verify_area_read (const void *addr, unsigned long size);
asmlinkage int     edt_verify_area_write (const void *addr, unsigned long size);

asmlinkage unsigned long edt_copy_to_user (void *to, const void *from, unsigned long n);

asmlinkage unsigned long edt_copy_from_user (void *to, const void *from,
				  unsigned long n);

asmlinkage unsigned long
edt_copy_pages_to_user (void *to, EdtPage * from, unsigned long n);

asmlinkage unsigned long
edt_copy_pages_from_user (EdtPage * sg, void *from, unsigned long n);

asmlinkage void    edt_wake_up (EdtWaitHandle p);

asmlinkage void    edt_wake_up_interruptible (EdtWaitHandle p);

asmlinkage void    edt_interruptible_sleep_on (EdtWaitHandle p);

asmlinkage long    edt_interruptible_sleep_on_timeout (EdtWaitHandle p,
					    signed long timeout);

asmlinkage void    edt_spin_lock (EdtSpinLock_t * lock);

asmlinkage void    edt_spin_unlock (EdtSpinLock_t * lock);

asmlinkage void    edt_start_intr_critical (EdtSpinLock_t * lock, unsigned long *flags);

asmlinkage void    edt_end_intr_critical (EdtSpinLock_t * lock, unsigned long flags);

asmlinkage pid_t   edt_get_current_pid (void);
asmlinkage void *   edt_get_current(void);

asmlinkage unsigned long edt_virt_to_bus (volatile void *adr);
asmlinkage unsigned long edt_virt_to_phys (volatile void *adr);

asmlinkage int     edt_register_interrupt (int irq, void *dev_id, char *name);

asmlinkage void     edt_delay(int msecs);

/*********************************/
/* structure used to initialize each board */
/********************************/

typedef struct _board_values
{
    int     boardtype;
    EdtPciDev dev;
} BoardValues;

asmlinkage int
edt_map_pci_board (BoardValues * bvp,
		   unsigned char *irq_p,
		   caddr_t * pMem,
		   off_t * pMem2Size,
		   unsigned int *pMemPhys, unsigned int *pMemPhys2);

asmlinkage int     edt_init_module_type (int subid, BoardValues * bvp);


/********************************************/
/* Memory access functions                  */
/********************************************/

asmlinkage void   *edt_kmalloc (size_t);
asmlinkage void   *edt_kmalloc_dma (size_t);
asmlinkage void    edt_kfree (const void *ptr);
asmlinkage EdtPage edt_get_free_kernel_page (void);
asmlinkage EdtPage edt_get_free_page (unsigned int mask);
asmlinkage void    edt_free_page (EdtPage page);

asmlinkage unsigned long edt_get_free_pages (int size);
asmlinkage unsigned long edt_get_free_dma_pages (int size);

asmlinkage void    edt_free_pages (unsigned long addr, int size);

asmlinkage void    edt_vfree (void *addr);

asmlinkage void   *edt_vmalloc (unsigned int size);

/* If the vmalloced memory is for DMA, use these */

asmlinkage void   *edt_vmalloc_dma (unsigned int size);
asmlinkage void   edt_vfree_dma (void *addr, unsigned int size);


asmlinkage unsigned long edt_vmalloc_to_bus (unsigned long adr);



asmlinkage unsigned long edt_kvirt_to_phys (unsigned long adr);

asmlinkage unsigned long edt_kvirt_to_bus (void * adr);

asmlinkage unsigned long edt_uvirt_to_bus (unsigned long adr);


asmlinkage void    edt_open_wait_handle (EdtWaitHandle * h);

asmlinkage void    edt_close_wait_handle (EdtWaitHandle * h);

asmlinkage void    edt_init_spinlock (EdtSpinLock_t * lockp);

asmlinkage int     edt_map_user_sglist (EdtMemMapping * map, unsigned int *Sglist,
			     int size);

asmlinkage void    edt_unmap_user_sglist (EdtMemMapping * map);

asmlinkage void   *edt_mem_map_page_virtual (EdtMemMapping * map, int pageno);


/*
 * function to avoid race conditions while sleeping per
 * Linux Device Drivers 2nd Edition.
 */

asmlinkage int     edt_wait_event_interruptible_timeout (EdtWaitHandle * handle,
					      volatile unsigned short *arg1,
					      unsigned short arg2,
					      signed long timeout);
asmlinkage int     edt_wait_event_interruptible (EdtWaitHandle * handle,
				      volatile unsigned short *arg1,
				      unsigned short arg2);


asmlinkage int     edt_mem_map_open (EdtMemMapping * map, unsigned int maxsize);

asmlinkage void    edt_mem_map_release (EdtMemMapping * map);

asmlinkage int     edt_map_user_mem_map (int rw,
			      EdtMemMapping * kbhandlep,
			      unsigned long useraddr, size_t len);

asmlinkage unsigned long edt_mem_map_page_to_bus (EdtMemMapping * map, int pagen);

asmlinkage unsigned long edt_mem_map_byte_offset (EdtMemMapping * kbhandle);

asmlinkage unsigned long edt_mem_map_bus_address (EdtMemMapping * map, int pagen);

asmlinkage void    edt_unmap_mem_map (EdtMemMapping * map);

asmlinkage int     edt_mmap_vmap_address (void *vma,
			       unsigned MemBasePhys,
			       unsigned int Mem2BasePhys);


asmlinkage void    edt_do_gettimeofday (struct timeval *);
asmlinkage void    edt_pci_write_config (EdtPciDev dev, unsigned int addr, u32 value);
asmlinkage unsigned int edt_pci_read_config (EdtPciDev dev, unsigned int addr);

asmlinkage int     edt_HZ (void);		/* returns the kernels HZ value */

asmlinkage int edt_sprintf(char * buf, const char * fmt, ...);

asmlinkage int edt_kill_proc (void *tp, int i, int j);


#ifdef P53B

#define MODULE_NAME "p53b"

#else

#define MODULE_NAME "edt"

#endif

/* private fops functions called from edt_lnx_kernel.c */

asmlinkage int     edt_private_open (void *dev_ptr);
asmlinkage int     edt_private_release (void *dev_ptr);
asmlinkage ssize_t     edt_private_read (void *dev_ptr,
			  char *user_buf, size_t count, loff_t * seek);
asmlinkage ssize_t     edt_private_read_nodma (void *dev_ptr,
			  char *user_buf, size_t count, loff_t * seek);
asmlinkage ssize_t     edt_private_write (void *dev_ptr,
			   const char *user_buf, size_t count, loff_t * seek);
asmlinkage int     edt_private_ioctl (void *dev_ptr,
			   unsigned int cmd, unsigned long arg);
asmlinkage int     edt_private_mmap (void *dev_ptr, void *vma);

asmlinkage int     edt_private_init_module (BoardValues bv[], int nboards,
				 unsigned int memsize);

asmlinkage int     edt_intr(void *dev_id);
asmlinkage int     edt_intr_process(void *dev_id);

asmlinkage void    edt_private_cleanup_module (void);

asmlinkage void   *edt_get_dev_from_minor (int minor);

/* this routine returns the interrupt status(es) - flags ored together */

#define INT_TYPE_PGDONE    1  /* page done interrupt */
#define INT_TYPE_DEVINT    2  /* interrupt from interface */
#define INT_TYPE_RDY       4  /* scatter-gather empty */
   
asmlinkage int     edt_check_interrupt(void *dev_id);

/* Routines to retrieve the IRQs used by EDT boards for use by 
   other kernel routines (i.e. RTLinux)
*/

#define MAX_IRQS  16

typedef struct _irqdata {
    int irq;
    void *dev;
    char *name;
} EdtIrqData;

asmlinkage int edt_get_n_irqs(void);
asmlinkage int edt_get_irq_value( const int index);
asmlinkage void * edt_get_dev_from_irq(int irq);

/* these routines give access in the kernel to the most 
   recent buffer finished */

/* Note that the done buffer pointer will normally be user-memory 
   unless kernel_buffers is true */

asmlinkage void * edt_get_done_buffer(void *dev);
asmlinkage int edt_get_done(void *dev);
asmlinkage int edt_get_curbuf(void *dev);
asmlinkage int edt_get_use_kernel_buffers(void *dev);

 
#endif

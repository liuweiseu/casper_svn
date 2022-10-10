/* #pragma ident "@(#)edt_lnx_kernel.c	1.68 08/12/04 EDT" */

/* 
This file acts as the compiled linkage between the EDT driver object files, which 
   make no reference to the kernel source, and the Linux kernel on a particular system 
*/

/* #pragma ident "@(#)edt_lnx_kernel.c	1.39 10/16/02 EDT" */

#include "edtdef.h"

#ifdef USE_MODV

#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#endif

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/init.h>


#include "edt_lnx_kernel.h"

#define VERSION_CODE(vers,rel,seq) ( ((vers)<<16) | ((rel)<<8) | (seq) )

#include <linux/version.h>

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,0)

#include <linux/gfp.h>

#ifndef REMAP2_5
#define REMAP2_5
#endif

#define KERNEL_26

#define USE_SPIN_LOCK
#define pte_offset pte_offset_kernel

#elif LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)



#if LINUX_VERSION_CODE < VERSION_CODE(2,4,18)

#define USE_IOBUF

#include <linux/iobuf.h>
#define MAX_IOBUF_LEN 128*1024*1024

#endif
#define iminor(inode) (MINOR(inode->i_rdev))

#define KERNEL_24

#define irqreturn_t void

#ifdef REMAP2_5

#define pte_offset pte_offset_kernel

#endif

#else



#endif

#include <asm/uaccess.h>

#include <linux/mm.h>

#include <linux/pci.h>

#include <linux/interrupt.h>

#include <linux/types.h>

#include <linux/highmem.h>

#include <asm/io.h>

#include "edt_os_lnx.h"
#include "edtreg.h"
#include "libedt.h"

//#include "edtinc.h"

#ifndef page_to_bus
#define page_to_bus page_to_phys
#endif

#define EdtWaitHandleType wait_queue_head_t


char    kernel_version[] = UTS_RELEASE;


int     edt_major = 0;


/** Modify this value to get debug print in uvirt_to_phys */

int     edt_driver_debug = 1;


#ifdef USE_IOBUF

#ifndef page_to_bus
#define page_to_phys(page)      ((page - mem_map) << PAGE_SHIFT)
#define page_to_bus      page_to_phys

#endif

#endif

#define MAX_IRQS  16


static int n_irqs = 0;
EdtIrqData irq_list[MAX_IRQS];


/* VARARGS2 */
static char     prtbuf[512];
asmlinkage void
edt_printk(
	   char *fmt,...)
{
    va_list         ap;

	va_start(ap, fmt);
	(void) vsprintf(prtbuf, fmt, ap);
	va_end(ap);

	printk(KERN_DEBUG  "Edt: %s", prtbuf);
}

unsigned long edt_get_memsize(void)

{
  struct sysinfo val;

  si_meminfo(&val);

  edt_printk("totalram %d\n",val.totalram);
  edt_printk("sharedram %d\n",val.sharedram);
  edt_printk("freeram %d\n",val.freeram);
  edt_printk("bufferram %d\n",val.bufferram);
  edt_printk("totalhigh %d\n",val.totalhigh);
  edt_printk("freehigh %d\n",val.freehigh);
  edt_printk("mem_unit %d\n",val.mem_unit);

  return val.totalram;
}

asmlinkage int
edt_HZ()

{
	return HZ;

}

asmlinkage void
edt_delay(int msecs)

{
    set_current_state(TASK_INTERRUPTIBLE);

    schedule_timeout(msecs);
}

asmlinkage 
void edt_free_irq(unsigned int irq, void *dev_id)

{
  free_irq(irq,dev_id);
}

asmlinkage void *edt_kmalloc(size_t size)

{
  return kmalloc(size, GFP_KERNEL);
}

asmlinkage void *edt_kmalloc_dma(size_t size)

{
  return kmalloc(size, GFP_DMA);
}

asmlinkage void edt_kfree(const void * ptr)

{
  kfree(ptr);
}

asmlinkage char *
edt_kmalloc_strdup(const char *s)

{
    int l;
    char *news;

    if (!s)
	return NULL;

    l = strlen(s)+1;

    news = edt_kmalloc(l);
	 
    if (news != NULL) memcpy(news,s,l); 

    return news;	
}


asmlinkage EdtPage edt_get_free_page(unsigned int gfp_mask)
{
  return (EdtPage) __get_free_page(gfp_mask);
}

asmlinkage void edt_free_page(EdtPage addr)
{
  free_page((unsigned int) addr);
}

/*
 * Fix to allow large DMA.  - Mark 11/02 for TI
 *
 * Use get_free_pages() to allocate large chunks of memory for sglists.
 * This technique is similar to that used on Solaris.
 */

asmlinkage unsigned long 
edt_get_free_pages(int size)
{
  int i, pow2 = 1 ;

  for (i = 0; ; i++)
  {
      if (pow2 * PAGE_SIZE >= size)
	  break ;
      pow2 *= 2 ;
  }
      
  return __get_free_pages(GFP_KERNEL, i);
}

asmlinkage unsigned long edt_get_free_dma_pages(int size)
{
  int i, pow2 = 1 ;

  for (i = 0; ; i++)
  {
      if (pow2 * PAGE_SIZE >= size)
	  break ;
      pow2 *= 2 ;
  }

  return __get_free_pages(GFP_DMA, i);
}

asmlinkage void edt_free_pages(unsigned long addr, int size)
{
  int i, pow2 = 1;

  for (i = 0; ; i++)
  {
      if (pow2 * PAGE_SIZE >= size)
	  break ;
      pow2 *= 2 ;
  }

  free_pages(addr, i);
}

/*
 ********************************
 Routines to copy memory to/from user space 
 ********************************
*/
asmlinkage unsigned long edt_copy_to_user(void *to, const void *from, unsigned long n)

{
  return copy_to_user(to,from,n);
}

asmlinkage unsigned long edt_copy_from_user(void *to, const void *from, unsigned long n)

{
  return copy_from_user(to,from,n);
}

asmlinkage unsigned long
edt_copy_pages_to_user(void *to, EdtPage *sg, unsigned long n)

{
	int npages;
	int i;
	int extra = PAGE_MASK;
	unsigned char *to_p = to;
	
	npages = n >> PAGE_SHIFT;
	
	for (i=0;i<npages;i++)
	{
		copy_to_user(to_p, sg[i],PAGE_SIZE);
		to_p += PAGE_SIZE;
	}

	extra = n & ~PAGE_MASK;

	if (extra)
		copy_to_user(to_p, sg[npages], extra);
	
	return 0;

}

asmlinkage unsigned long
edt_copy_pages_from_user(EdtPage *sg, void * from, unsigned long n)

{
	int npages;
	int i;
	int extra = PAGE_MASK;
	unsigned char *from_p = from;
	
	npages = n >> PAGE_SHIFT;

	
	for (i=0;i<npages;i++)
	{
		copy_from_user(sg[i], from_p, PAGE_SIZE);
		from_p += PAGE_SIZE;
	}

	extra = n & ~PAGE_MASK;

	if (extra)
		copy_from_user(sg[npages], from_p, extra);
	
	return 0;

}

asmlinkage void edt_wake_up(EdtWaitHandle  h)

{
  EdtWaitHandleType * p = (EdtWaitHandleType *) h;

  wake_up(p);
}

asmlinkage void edt_wake_up_interruptible(EdtWaitHandle  h)

{
  EdtWaitHandleType * p = (EdtWaitHandleType *) h;
  wake_up_interruptible(p);
}

asmlinkage void edt_interruptible_sleep_on(EdtWaitHandle  h)
{
  EdtWaitHandleType * p = (EdtWaitHandleType *) h;
  interruptible_sleep_on(p);
}
 
asmlinkage long edt_interruptible_sleep_on_timeout(EdtWaitHandle  h,
          signed long timeout)

{
  EdtWaitHandleType * p = (EdtWaitHandleType *) h;

  long rc;

  rc = interruptible_sleep_on_timeout(p,timeout);

  return rc;
}


asmlinkage int edt_verify_area_read(const void * addr, unsigned long size)

{
  return verify_area(VERIFY_READ,addr,size);
}

asmlinkage int edt_verify_area_write(const void * addr, unsigned long size)

{
  return verify_area(VERIFY_WRITE,addr,size);
}


asmlinkage void edt_spin_lock(EdtSpinLock_t *lock)
{
#ifdef USE_SPIN_LOCK 
	spin_lock((spinlock_t *)lock);
#endif
}


asmlinkage void edt_spin_unlock(EdtSpinLock_t *lock)
{
#ifdef USE_SPIN_LOCK 

	spin_unlock((spinlock_t *)lock);
#endif
}


asmlinkage void edt_start_intr_critical(EdtSpinLock_t *lock, unsigned long *flags)

{
#ifdef USE_SPIN_LOCK 

	spin_lock_irqsave((spinlock_t *)lock, *flags);
#else

  save_flags(*flags);
  cli();
#endif

}

asmlinkage void edt_end_intr_critical(EdtSpinLock_t *lock, unsigned long flags)

{
#ifdef USE_SPIN_LOCK 
	spin_unlock_irqrestore((spinlock_t *)lock, flags);

#else
  sti();
  restore_flags(flags);
#endif

}

asmlinkage pid_t
edt_get_current_pid()

{
	if (current)
		return current->pid;
	else
		return 0;
}

/* (virt_to_phys only works for kmalloced kernel memory) */

static unsigned long
pgd_to_phys(pgd_t *pgd, unsigned long adr)

{
  pmd_t  *pmd;
  pte_t  *ptep, pte;
  unsigned long rc = 0;

  struct page *page;

  if (pgd_none(*pgd))
	return 0;
 
  pmd = pmd_offset(pgd, adr);
  if (pmd_none(*pmd))
	return 0;

  ptep = pte_offset(pmd, adr /* &(~PGDIR_MASK) */ );
  if (!ptep)
  {
  return 0 ;
  }
  pte = *ptep;

	
  if (pte_present(pte))
    {

	page = (struct page *) pte_page(pte);


#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)

#ifdef HIGHMEM
	  rc = virt_to_phys(page->virtual);
#else
	  rc = page_to_bus(page) | (adr & (PAGE_SIZE - 1)) ;
#endif

#else

	rc = virt_to_phys((void *) (page)) | (adr & (PAGE_SIZE -1 ));
	if (edt_driver_debug > 1)
		edt_printk("virt_to_phys: %08x\n", rc);


#endif


    }
  else
    {
	  edt_printk("uvirt_to_phys for %lx - pte_page not present\n",
				 adr);
    }

  return rc;
}

static unsigned long
pgd_to_bus(pgd_t *pgd, unsigned long adr)

{
  pmd_t  *pmd;
  pte_t  *ptep, pte;
  unsigned long rc = 0;

  struct page *page;

  if (pgd_none(*pgd))
	return 0;
 
  pmd = pmd_offset(pgd, adr);
  if (pmd_none(*pmd))
	return 0;

  ptep = pte_offset(pmd, adr /* &(~PGDIR_MASK) */ );
  if (!ptep)
  {
  return 0 ;
  }
  pte = *ptep;

	
  if (pte_present(pte))
    {

	page = (struct page *) pte_page(pte);


#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)

#ifdef HIGHMEM
	  rc = virt_to_phys(page->virtual);
#else
	  rc = page_to_bus(page) | (adr & (PAGE_SIZE - 1)) ;
#endif

#else

	rc = virt_to_bus((void *) (page)) | (adr & (PAGE_SIZE -1 ));
	
	if (edt_driver_debug > 1)
		edt_printk("virt_to_bus: %08x\n", rc);


#endif


    }
  else
    {
	  edt_printk("uvirt_to_bus for %lx - pte_page not present\n",
				 adr);
    }

  return rc;
}

asmlinkage unsigned long 
edt_uvirt_to_phys(unsigned long adr)
{
  pgd_t  *pgd;

  if (edt_driver_debug > 1)
	{

	  edt_printk( "uvirt_to_phys current %lx current->mm %x\n", 
				  current, current->mm);
	}

  pgd = pgd_offset(current->mm, adr);

  if (edt_driver_debug > 1)
	edt_printk( "uvirt_to_phys pgd %x\n", 
				pgd);

  return pgd_to_phys(pgd, adr);
}

asmlinkage unsigned long
edt_vmalloc_to_bus(unsigned long adr)

{
  pgd_t  *pgd;
  unsigned long retval;

  edt_printk("vmalloc to bus\n");

  pgd = pgd_offset_k(adr);

  retval = pgd_to_phys(pgd, adr);

  edt_printk("vmalloc to bus %x -> %x\n", adr, retval);

  return retval;

}


asmlinkage unsigned long 
edt_uvirt_to_bus(unsigned long adr)
{
  pgd_t  *pgd;

  if (edt_driver_debug > 1)
	{

	  edt_printk( "uvirt_to_bus current %lx current->mm %x\n", 
				  current, current->mm);
	}

  pgd = pgd_offset(current->mm, adr);

  if (edt_driver_debug > 1)
	edt_printk( "uvirt_to_bus pgd %x\n", 
				pgd);

  return pgd_to_bus(pgd, adr);
}

asmlinkage unsigned long 
edt_virt_to_bus(volatile void* adr)

{
  return virt_to_bus(adr);
}

asmlinkage unsigned long 
edt_virt_to_phys(volatile void* adr)

{
  return virt_to_phys(adr);
}
 
asmlinkage unsigned long 
edt_kvirt_to_phys(unsigned long adr)
{
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  
	pgd_t *pgd;

	pgd = pgd_offset_k(adr);

	return pgd_to_phys(pgd, adr);

#else

    return edt_uvirt_to_phys(VMALLOC_VMADDR(adr));
#endif

}


asmlinkage extern int edt_intr(void *dev_id);

static irqreturn_t
edt_intr_func(int irq, void *dev_id, struct pt_regs * regs)

{
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,0)
    return edt_intr (dev_id);
#else
    edt_intr (dev_id);
#endif
}

asmlinkage 
int edt_get_n_irqs()

{
    return n_irqs;
}

asmlinkage 
int edt_get_irq_value(int index)
{
    if (index >= 0 && index < n_irqs)
	return irq_list[index].irq;
    /* out of range error */
    return -1;
}

asmlinkage void * edt_get_dev_from_irq(int irq)

{
    int i;
    for (i=0;i<n_irqs;i++)
    {
	edt_printk("Searching for irq %d == %d at %d dev = %p\n",
		   irq,irq_list[i].irq,i,irq_list[i].dev);
	if (irq == irq_list[i].irq)
	    return irq_list[i].dev;
    }
    return NULL;
}

void
edt_clear_irq_list()

{
    n_irqs = 0;
}

asmlinkage int
edt_take_interrupt(int irq, void *dev_id)

{
    edt_free_irq(irq,dev_id);
    return 0;
}

asmlinkage int
edt_restore_os_interrupt(int irq, void *dev_id)

{
	int i, rc;
    for (i=0;i<n_irqs;i++)
	if (irq == irq_list[i].irq)
	{
	    if (irq_list[i].dev != dev_id)
	    { 
		edt_printk("edt_restore_os_interrupt dev %x != list %x\n",
			dev_id, irq_list[i].dev);
		return -1;
	    }
	    rc = request_irq (irq, edt_intr_func, SA_SHIRQ | SA_INTERRUPT, 
	        irq_list[i].name,
	         dev_id);
	    return rc;
	}

     edt_printk("Irq %d not found in irq_list\n",irq);
     return -1;
}

asmlinkage int
edt_register_interrupt (int irq, void *dev_id, char *name)
{

    
    int     rc = 0;

    irq_list[n_irqs].irq = irq;
    irq_list[n_irqs].dev = dev_id;
    irq_list[n_irqs].name = name;    
   
    edt_printk("adding irq %d to list at %d\n", irq, n_irqs);
 
    n_irqs++;


    rc = request_irq (irq, edt_intr_func, SA_SHIRQ | SA_INTERRUPT, name,
	      dev_id);

    edt_printk ("register irq %d dev %p name %s returns %d\n",
	    irq, dev_id, name, rc);

    return rc;
}

static u32 addresses[] = {
    PCI_BASE_ADDRESS_0,
    PCI_BASE_ADDRESS_1,
    0,
    PCI_BASE_ADDRESS_2,
    PCI_BASE_ADDRESS_3,
    PCI_BASE_ADDRESS_4,
    PCI_BASE_ADDRESS_5,
    0
};

asmlinkage int
edt_map_pci_board(BoardValues *bp,
				  unsigned char *irq_p,
				  caddr_t *pMem,
				  off_t *pMem2Size,
				  u_int *pMemPhys,
				  u_int *pMem2Phys)
				  
{
  int i;

  
if (edt_driver_debug > 1)
  edt_printk("edt_map_pci_board\n");


    for (i = 0; addresses[i]; i++)
    {
		u32     curr, mask;

		pci_read_config_dword(bp->dev, addresses[i], &curr);
		//		cli();
		pci_write_config_dword(bp->dev, addresses[i], ~0);
		pci_read_config_dword(bp->dev, addresses[i], &mask);
		pci_write_config_dword(bp->dev, addresses[i], curr);
		//sti();
		pci_read_config_byte(bp->dev, PCI_INTERRUPT_LINE, irq_p);

	
		*irq_p = ((struct pci_dev *) bp->dev)->irq;


		if (!mask)
		{
			break;
		}

		if (i == 0)
		{
		    *pMem = ioremap(curr, 256);
		    *pMemPhys = curr ;
if (edt_driver_debug > 1)
		edt_printk("edt_map_pci_board mem0 %x %x\n",*pMem,*pMemPhys) ;
		}
		else
		{
		    *pMem2Size = ~(mask)+1;
		    *pMem2Phys = curr ;
if (edt_driver_debug > 1)
		edt_printk("edt_map_pci_board mem1 Size = %x %x\n",*pMem2Size,*pMem2Phys) ;
		}

    }

#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)

	pci_enable_device(bp->dev) ;
	pci_set_master(bp->dev);

#endif

	
	return 0;
}

asmlinkage int
edt_init_module_type(int subid, BoardValues * bvp)

{
    int     boardsfound = 0;

	struct pci_dev *dev = NULL;

	while ((dev = pci_find_device(0x123d, subid, dev)))
	  {
		bvp->dev = dev;
		bvp->boardtype = subid;
		bvp++;
		boardsfound++;
	  }

	return boardsfound;
}


asmlinkage void edt_vfree(void * addr)

{

#ifdef NO_VMALLOC

	kfree(addr);
	
#else

	// printk("vfree %p\n", addr);

  vfree(addr);

#endif

}

asmlinkage void *edt_vmalloc(unsigned int size)

{

	void *p;

#ifdef NO_VMALLOC

	return kmalloc(size, GFP_KERNEL);
#else

#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)

	p =  vmalloc_32(size);

	// printk("vmalloc %p (%d bytes)\n", p, size);

	return p;

#else
	p =  vmalloc(size);

	return p;

#endif

#endif
}

asmlinkage void
edt_init_spinlock(EdtSpinLock_t *lockp)

{
  *((spinlock_t *) lockp) = SPIN_LOCK_UNLOCKED;

}


asmlinkage void
edt_open_wait_handle(EdtWaitHandle *h)

{
	EdtWaitHandleType ** p = (EdtWaitHandleType **) h;

#if LINUX_VERSION_CODE >= VERSION_CODE(2,3,0)

	*p = (EdtWaitHandleType *) edt_vmalloc(sizeof(EdtWaitHandleType));
	init_waitqueue_head(*p);

#else
	*p = (EdtWaitHandleType *) vmalloc(sizeof(EdtWaitHandle));
	**p = NULL ;
#endif
}

asmlinkage void
edt_close_wait_handle(EdtWaitHandle *h)

{

  EdtWaitHandleType ** p = (EdtWaitHandleType **) h;

  vfree(*p);

  *p = NULL;
  
}



#define KIOBUF_VMALLOC

#ifdef USE_IOBUF

static int
nkiobufs_required(int maxsize)
{

	int n = (maxsize+MAX_IOBUF_LEN-1);
		       
	
	n /= MAX_IOBUF_LEN;
	
	
	if (n <= 0) n = 1;

	return n;

}

asmlinkage int
edt_kiobuf_open (struct kiobuf ***kbh, unsigned int maxsize)
{


    // pointer to array of pointers to structs
    int n;
	int rc;
	struct kiobuf *kbp;

	n = nkiobufs_required(maxsize);

	
    // allocate array of pointers to structs
    *kbh = kmalloc(n * sizeof(struct kiobuf *), GFP_KERNEL);

	
    // pass array of pointers to structs
#ifdef USE_KIOVEC_SZ


    {
	int nbhs = KIO_MAX_SECTORS;
	alloc_kiovec_sz(n,*kbh,&nbhs);
	edt_printk("edt_kiobuf_open alloc_kiovec_sz n=%d *kbh = %x nbh = %d\n", n, *kbh, nbh);
    }
#else

    rc = alloc_kiovec(n,*kbh);
	edt_printk("edt_kiobuf_open alloc_kiovec n=%d *kbh = %x rc = %d\n", 
	n, *kbh, rc);
#endif

	kbp = (*kbh)[0];

	edt_printk("kbp = %d kbp->nr_pages = %d\n",
		kbp, kbp->nr_pages);
	return n;

}

asmlinkage void
edt_kiobuf_release (struct kiobuf ***kbh, int nkiob)
{

    int i;
	struct kiobuf *kbp;

    for (i=0;i<nkiob;i++)
    {
	kbp = (*kbh)[0];
        if (kbp->nr_pages)
	    unmap_kiobuf(kbp);	
	
    }
    // pass array of pointers to structs
#ifdef USE_KIOVEC_SZ

    {
	int nbhs = KIO_MAX_SECTORS;
	free_kiovec_sz(nkiob,*kbh,&nbhs);
    }	
#else

    free_kiovec(nkiob,*kbh);
#endif

    kfree(*kbh) ;

}


asmlinkage int	edt_map_user_kiobuf(int rw,     
						struct kiobuf ***kbh,
			  unsigned long va, size_t len, int *nkiob)
{

    // array of pointers to structs

    struct kiobuf **kbp;

    int bytes_remaining = len ;
    int map_len, map_va = va ;
    int count = 0, ret ;
    //int i ;


	if (*nkiob < nkiobufs_required(len))
	  {
		edt_kiobuf_release(kbh, *nkiob);
		*nkiob = edt_kiobuf_open(kbh, len);
	  }
    kbp = * kbh;

    do {

	map_len = (bytes_remaining > MAX_IOBUF_LEN) ? MAX_IOBUF_LEN 
	    					    : bytes_remaining ;
	//edt_printk("count %d map_len %d map_va %x\n", count, map_len, map_va);
	ret = map_user_kiobuf(rw, kbp[count], map_va, map_len); 

	if (ret != 0)
	{
	    edt_printk("FATAL ERROR in edt_map_user_kiobuf(): errno %d map_va %d map_len %d \n", 
				   ret,
				   map_va,
				   map_len
				   );
	    return ret ;
	}

	bytes_remaining -= map_len ;
	map_va += map_len ;
	++ count ;

    } while (bytes_remaining) ;

#if 0
    for (i = 0; i < 8; i++)
	edt_printk("%d: nr_pages %d\n", i, kbp[i]->nr_pages) ;
#endif

    return 0 ;

}


asmlinkage void	edt_unmap_kiobuf(EdtKiobuf kbhandle, int nkiob)
{


    // array of pointers to structs
    struct kiobuf **kbp = (struct kiobuf **) kbhandle;
    int i ;

    for (i = 0; i < nkiob; i++)
    {
	  if (edt_driver_debug > 1)
	    edt_printk("Unmap kiobuf %p\n", kbp[i]);

	  if (kbp[i]->nr_pages)
	    unmap_kiobuf(kbp[i]);
    }

}

#endif


#include <linux/pagemap.h>


asmlinkage void * edt_memcpy(void *dest, void *src, size_t length)

{
	return memcpy(dest, src, length);
}

asmlinkage void * edt_memset(void *dest, int value, size_t length)

{
	return memset(dest, value, length);
}


asmlinkage int edt_kill_proc(void * tp, int i, int j)

{
  struct task_struct *ptask = (struct task_struct *) tp;

	if (ptask)
	  return kill_proc(ptask->pid, i, j);

	return -1;
}

asmlinkage void *edt_get_current()

{
  return current;
}

asmlinkage int edt_sprintf(char * buf, const char * fmt, ...)

{
	va_list ap;

	va_start(ap, fmt);

	return vsprintf(buf, fmt, ap);

}

extern int edt_remap_page_range(struct vm_area_struct *vma,
	unsigned long from, unsigned long phys_addr,
	unsigned long size, pgprot_t prot)

{
#ifdef REMAP2_5
  return remap_page_range(vma, from, phys_addr, size, prot);
#else
  return remap_page_range(from, phys_addr, size, prot);
#endif
}

asmlinkage void
edt_do_gettimeofday(struct timeval *tv)

{

	do_gettimeofday(tv);

}

asmlinkage void
edt_pci_write_config(EdtPciDev dev, unsigned int addr, u32 value)
{
    pci_write_config_dword(dev, addr, value) ;
}

asmlinkage unsigned int
edt_pci_read_config(EdtPciDev dev, unsigned int addr)
{
    u32 value ;
    pci_read_config_dword(dev, addr, &value) ;
    return(value) ;
}


#if LINUX_VERSION_CODE < VERSION_CODE(2,6,0)


#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)

#define EdtWaitHandleType wait_queue_head_t

#define __wait_event_interruptible_timeout(wq, condition, ret, timo)	\
do {									\
	wait_queue_t __wait;						\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
                        if ((ret = schedule_timeout(timo)) == 0)        \
                        {                                               \
                            break;                                      \
                        }                                               \
                        else                                            \
                        {                                               \
			    continue;					\
                        }                                               \
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

#else

#define EdtWaitHandleType struct wait_queue *

#define __wait_event_interruptible_timeout(wq, condition, ret, timo)	\
do {									\
	struct wait_queue __wait;					\
									\
	__wait.task = current;						\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		current->state = TASK_INTERRUPTIBLE;			\
		mb();							\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
                        if ((ret = schedule_timeout(timo)) == 0)        \
                        {                                               \
                            ret = -ETIMEDOUT;                           \
                            break;                                      \
                        }                                               \
                        else                                            \
                        {                                               \
			    continue;					\
                        }                                               \
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

#endif

#define wait_event_interruptible_timeout(wq, condition, timo)		\
({									\
    int __ret = timo;							\
    if (!(condition))							\
	__wait_event_interruptible_timeout(wq, condition, __ret, timo);	\
    __ret;								\
})

#endif

asmlinkage int
edt_wait_event_interruptible (EdtWaitHandle * handle, volatile unsigned short *arg1,
			      unsigned short arg2)
{
    EdtWaitHandleType *p = (EdtWaitHandleType *) handle;
    int rc = 0; 

    rc = wait_event_interruptible ((*p), *arg1 != arg2);


    return rc;
}


asmlinkage int
edt_wait_event_interruptible_timeout (EdtWaitHandle * handle,
				      volatile unsigned short *arg1,
				      unsigned short arg2,
				      signed long timeout)
{
    EdtWaitHandleType *p = (EdtWaitHandleType *) handle;

    int rc; 

    rc = wait_event_interruptible_timeout ((*p), *arg1 != arg2, timeout);

    if (rc == 0)
	rc = -ETIMEDOUT;
    else if (rc > 0)
	rc = 0;

    return rc;
}

/*********************************************
 *
 *
 *
 ********************************************/

#define VM_OFFSET(vma) ((vma)->vm_pgoff << PAGE_SHIFT)

asmlinkage int
edt_mmap_vmap_address(void * vma_ptr,
						   unsigned int MemBasePhys,
						   unsigned int Mem2BasePhys)

{
	int rc = 0;
	struct vm_area_struct *vma  = (struct vm_area_struct *) vma_ptr; 
	u_long offset ;

   	int vmsize = vma->vm_end - vma->vm_start;

	offset = VM_OFFSET(vma) ;
	if (offset < 0x10000)
	{
  
		rc = edt_remap_page_range(vma, vma->vm_start, 
				(u_long) MemBasePhys + offset,
					 vmsize, vma->vm_page_prot);
	}
	else /* if (offset <= 0x800000)*/
	{
	   
		offset -= 0x10000 ;
		rc = edt_remap_page_range(vma, vma->vm_start, 
				(u_long) Mem2BasePhys + offset,
					 vmsize, vma->vm_page_prot);

	}

	return rc;

}

asmlinkage unsigned long edt_kiobuf_bus_address(EdtKiobuf kbhandle, int pagen)

{

#ifdef USE_IOBUF

#ifndef page_to_bus
#define page_to_phys(page)      ((page - mem_map) << PAGE_SHIFT)
#define page_to_bus      page_to_phys

#endif

    // array of pointers to structs
    struct kiobuf **kbp = (struct kiobuf **) kbhandle;
    int count = 0 ;

    // kiobufs are limited to 32*1024 pages
    while (pagen >= 32*1024)
    {
	++ count ;
	pagen -= 32*1024 ;
    }

    return page_to_bus(kbp[count]->maplist[pagen]);

#else
    return 0;
#endif
}

asmlinkage unsigned long edt_kiobuf_byte_offset(EdtKiobuf kbhandle)

{
#ifdef USE_IOBUF

  // array of pointers to structs
  struct kiobuf **kbp = (struct kiobuf **) kbhandle;

  if (kbp)
	return kbp[0]->offset;

#endif

  return 0;
}


int 
edt_driver_open(struct inode * inode, struct file * filp)
{
    int     num = iminor(inode);
    void *dev;		/* device information */

    dev = edt_get_dev_from_minor(num);

    /* and use filp->private_data to point to the device data */
    filp->private_data = dev;
    
	edt_private_open(dev);

	return 0;


}

int 
edt_driver_release(struct inode * inode, struct file * filp)
{
	
	
	void *edt_p;		/* device information */

    edt_p = filp->private_data;
	
	edt_private_release(edt_p);



    return 0;
}


ssize_t
edt_driver_read(struct file * filp,
				char *user_buf, size_t count, loff_t * seek)


{
    void *edt_p = filp->private_data;
	
	return edt_private_read(edt_p,user_buf, count, seek);

}


ssize_t 
edt_driver_write(struct file * filp,
				 const char *user_buf, size_t count, loff_t * offset)


{
    void *edt_p = filp->private_data;

	return edt_private_write(edt_p,user_buf,count, offset);

}

 
int 
edt_driver_ioctl(struct inode * inode, struct file * filp,
				 unsigned int cmd, unsigned long arg)
{
    void *edt_p = filp->private_data;

	return edt_private_ioctl(edt_p,cmd,arg);

}



int     edt_driver_mmap(struct file * filp,
				struct vm_area_struct *vma)

{

    void *edt_p = filp->private_data;

	return edt_private_mmap(edt_p,vma);


}


/*
 * The different file operations
 */


struct file_operations edt_fops = {
    read:edt_driver_read,
    write:edt_driver_write,
    ioctl:edt_driver_ioctl,
    open:edt_driver_open,
    release:edt_driver_release,
	mmap:edt_driver_mmap
};


static int
edt_find_boards (BoardValues bv[])
{
    int     nboards = 0;

    /* */

    /* P11w */


#ifdef P53B

    nboards += edt_init_module_type (P53B_ID, &bv[nboards]);

#else

    nboards += edt_init_module_type (P11W_ID, &bv[nboards]);


    nboards += edt_init_module_type (P16D_ID, &bv[nboards]);

    nboards += edt_init_module_type (PDV_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVA_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVA16_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVFOX_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVFCI_AIAG_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVFCI_USPS_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVK_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVFOI_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDV44_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVCL_ID, &bv[nboards]);
    nboards += edt_init_module_type (PDVAERO_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP_RGB_ID, &bv[nboards]);
   
    nboards += edt_init_module_type (PCDA_ID, &bv[nboards]);
    nboards += edt_init_module_type (PSS4_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGS4_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCD20_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCD40_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCD60_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP20_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP40_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP60_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP_ECL_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCD_16_ID, &bv[nboards]);
    nboards += edt_init_module_type (PSS16_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGS16_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCDA16_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCDFOX_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCDFCI_SIM_ID, &bv[nboards]);
    nboards += edt_init_module_type (PCDFCI_PCD_ID, &bv[nboards]);
    nboards += edt_init_module_type (PGP_THARAS_ID, &bv[nboards]);

#endif

    return nboards;
}


// This should match the definition in edtreg.h
#define PDMA_SIZE      0x2000

/**************************************
 * Routines for mapping sg lists from user space
 *
 *************************************/

// This should match the definition in edtreg.h
#define PDMA_SIZE      0x2000


#ifdef OLD_SGLIST

asmlinkage void *
edt_map_user_sglist(unsigned int *Sglist)
{
#ifdef USE_IOBUF
    int result ;
    struct kiobuf *kb ;

#ifdef USE_KIOVEC_SZ
    int nbhs = KIO_MAX_SECTORS;
	result = alloc_kiovec_sz(1, &kb,&nbhs) ;
#else
    result = alloc_kiovec(1, &kb) ;
#endif
    if (result)
    {
	edt_printk(
	    "Error result %d in edt_map_user_sglist from alloc_kiovec\n",
	    result) ;
	return 0 ;
    }

    result = map_user_kiobuf(1, kb, (u_int) Sglist, PDMA_SIZE) ;
    if (result)
    {
	edt_printk(
	    "Error result %d in edt_map_user_sglist from map_user_kiobuf\n",
	    result) ;
	edt_printk("Sglist 0x%08x\n", Sglist) ;

	return 0 ;
    }

    return kb ;
#else
	return NULL;
#endif
}

asmlinkage void
edt_unmap_user_sglist(void *iobuf)
{
#ifdef USE_IOBUF
    struct kiobuf *kb = (struct kiobuf *) iobuf ;

    unmap_kiobuf(kb) ;

#ifdef USE_KIOVEC_SZ
	{
		int nbhs = KIO_MAX_SECTORS;
    	free_kiovec_sz(1, &kb,&nbhs) ;
	}
#else
    free_kiovec(1, &kb) ;
#endif

#endif
}

#else

asmlinkage int
edt_map_user_sglist (EdtMemMapping * map, unsigned int *Sglist, int size)
{

    return edt_map_user_mem_map (READ, map, (unsigned long) Sglist, size);

}

#endif

#ifdef USE_IOBUF
void *
edt_sglist_page(void *iobuf, int pageno)
{
    struct kiobuf *kb = (struct kiobuf *) iobuf ;

    return kmap(kb->maplist[pageno]) ;
}
#endif

asmlinkage EdtPage
edt_get_free_kernel_page (void)
{
    EdtPage addr =  (EdtPage) __get_free_page (GFP_KERNEL);

#ifdef PRINT_MEMALLOC
    edt_printk("__get_free_page(GFP_KERNEL) addr %p\n", addr);
#endif

    return addr;
}

asmlinkage void   *
edt_mem_map_page_virtual (EdtMemMapping * map, int pageno)
{

#ifdef USE_IOBUF

    edt_printk
	("Error in mem_map_page_to_virtual %d greater than npages %d\n",
	 pageno, map->npages);

    return (void *) NULL;

#else
    struct page **pages = (struct page **) map->mapping;

    return page_address (pages[pageno]);

#endif
}


/***************************************
 *
 *  create a page list mapping 
 *
 ***************************************/

asmlinkage int
edt_map_user_mem_map (int rw,
		      EdtMemMapping * map,
		      unsigned long useraddr, size_t size)
{


#ifdef USE_IOBUF

    // array of pointers to structs
    struct kiobuf **kbp;

    int     bytes_remaining = size;
    int     map_len, map_va = useraddr;
    int     count = 0, ret;

    //int i ;

    int     nkiob = map->npages;

	edt_printk("map_user_mem_map map->npages %d map->mapping %x\n", map->npages, map->mapping);


    if (nkiob < nkiobufs_required (size))
      {
	  edt_kiobuf_release ((struct kiobuf ***)&(map->mapping), nkiob);
	  nkiob = edt_kiobuf_open ((struct kiobuf ***)&(map->mapping), size);
      }

    kbp = (struct kiobuf **) map->mapping;
	map->npages = nkiob;

	edt_printk("map_user_mem_map map->npages %d map->mapping %x\n", map->npages, map->mapping);
    do
      {

	  map_len = (bytes_remaining > MAX_IOBUF_LEN) ? MAX_IOBUF_LEN
	      : bytes_remaining;
	  edt_printk("map_user_kiobuf kbp[count] %x rw %d count %d map_len %d map_va %x\n", 
				 kbp[count], rw, 
				 count, map_len, map_va);

	  
	  ret = map_user_kiobuf (rw, kbp[count], map_va, map_len);

	  if (ret != 0)
	    {
	    edt_printk("FATAL ERROR from map_user_kiobuf in edt_map_user_mem_map(): \nerrno %d map_va %d map_len %d \n", 
				   ret,
				   map_va,
				   map_len
				   );
		return ret;
	    }

	  bytes_remaining -= map_len;
	  map_va += map_len;
	  ++count;

      }
    while (bytes_remaining);

    map->npages = nkiob;
    return 0;


#else

    struct page **pages;
    int     len;
    int     rc;
	//struct vm_area_struct  *vma;


    /* FIX - figure out initial offset */
    len = size / PAGE_SIZE;

    len = (((useraddr & ~PAGE_MASK) + size + ~PAGE_MASK) / PAGE_SIZE);

    pages = (struct page **) edt_get_free_pages (len * sizeof (struct page *));
    if (pages == NULL)
    {
	printk("edt_map_user_mem_map:  out of memory for sglist\n");
	return -ENOMEM;
    }


    memset(pages,0, len * sizeof (struct page *));

    
    down_read (&current->mm->mmap_sem);

#ifdef PRINT_MEMALLOC
    vma = find_vma(current->mm, useraddr);

	printk("vma %p for address %08x\n", vma, useraddr);
#endif

    rc = get_user_pages (current, current->mm, useraddr, len, rw,	/* write */
			 0,	/* force */
			 pages, NULL);
    up_read (&current->mm->mmap_sem);
    if (rc <= 0)
    {
	printk("edt_map_user_mem_map:  get_user_pages error %d\n", rc);
	edt_free_pages ((unsigned long)pages, len * sizeof (struct page *));
	return -ENOMEM;
    }

#ifdef PRINT_MEMALLOC
    edt_printk("mapping %d pages rw = %d\n", len, rw);
	dump_page_state(pages[0]);
#endif
    map->mapping = pages;
    map->npages = len;
    map->own_mapping = 1;
    map->is_mapped = 1;
    map->rw = rw;
    map->offset = useraddr & ~PAGE_MASK;
    return 0;

#endif

}

asmlinkage void
edt_mem_map_release (EdtMemMapping * map)
{
#ifdef USE_IOBUF

    struct kiobuf **kbp;
    int     nkiob = map->npages;

    kbp = (struct kiobuf **) map->mapping;

    edt_printk ("mem_map_release %x npages = %d\n", kbp, nkiob);
    
    edt_kiobuf_release (&kbp, nkiob);

    map->mapping = NULL;
    map->npages = 0;
    map->is_mapped = 0;

#else

#ifdef PRINT_MEMALLOC
    edt_printk ("mem_map_release %p\n", map);
#endif

    if (!map)
	{
	edt_printk("Error null map variable\n");
	return;
	}
#if 1
    if (map->mapping)
      {
	  if (map->own_mapping)
	    {
		int     i;

		struct page **pages = (struct page **) map->mapping;

#ifdef PRINT_MEMALLOC
		edt_printk("Page 0 state\n");	
		dump_page_state(pages[0]);
#endif

		for (i = 0; i < map->npages; i++)
		  {
		      if (map->rw)
				  set_page_dirty(pages[i]);
		      put_page (pages[i]);
	    
		  }

	    }

      }
    else
	return;

#endif

    
    edt_free_pages ((unsigned long)map->mapping, (map->npages * sizeof (struct page *)));

    map->mapping = NULL;
    map->is_mapped = 0;
#endif

}

asmlinkage unsigned long
edt_mem_map_page_to_bus (EdtMemMapping * map, int pagen)
{

#ifdef USE_IOBUF

    struct kiobuf **kbp = (struct kiobuf **) map->mapping;
    int     count = 0;

    // kiobufs are limited to 32*1024 pages
    while (pagen >= 32 * 1024)
      {
	  ++count;
	  pagen -= 32 * 1024;
      }

    if (count < map->npages)
	return (unsigned long) page_to_bus (kbp[count]->maplist[pagen]);
    else
      {
	  edt_printk
	      ("Error in mem_map_page_to_bus %d greater than npages %d\n",
	       pagen, map->npages);

	  return 0;
      }
#else

    struct page **pages = (struct page **) map->mapping;

//	edt_printk("page->bus pages[%d] = %p bus = %p\n",
//		pagen, pages[pagen], page_to_bus(pages[pagen]));

    return page_to_bus (pages[pagen]);


#endif


}

/***********************************************
 * Use vmalloc to get large DMA buffer in kernel
 ***********************************************/

asmlinkage void *
edt_vmalloc_dma(unsigned int size)
{
	void *p = edt_vmalloc(size);
	int dsize = size;
	unsigned long adr = (unsigned long) p;
	
	if (p)
	{
		while (dsize > 0)
		{
			SetPageReserved(vmalloc_to_page((void *) adr));
			adr += PAGE_SIZE;
			dsize -= PAGE_SIZE;
		}
	}
	
#ifdef PRINT_MEMALLOC
	edt_printk("vmalloc_dma %p size %d\n", p,size);
#endif
	return p;	

}

asmlinkage void
edt_vfree_dma(void *p, unsigned int size)
{

	int dsize = size;
	unsigned long adr = (unsigned long) p;
	
	if (p)
	{
		while (dsize > 0)
		{
			ClearPageReserved(vmalloc_to_page((void *) adr));
			adr += PAGE_SIZE;
			dsize -= PAGE_SIZE;
		}

		edt_vfree(p);
	}

#ifdef PRINT_MEMALLOC
	edt_printk("vfree_dma %p size %d\n", p,size);
#endif
}

/* cloned from meye.c */
asmlinkage unsigned long
edt_kvirt_to_bus(void * adr)
{

	unsigned long kva;
	unsigned long return_addr;
	
	kva = (unsigned long) page_address(vmalloc_to_page((void *) adr));

	kva |= (unsigned long) adr & (PAGE_SIZE-1);

	return_addr =  __pa(kva);

#ifdef PRINT_MEMALLOC
	edt_printk("kvirt_to_bus %x -> %x kva = %x virt_to_bus -> %x vmalloc_to_page %x page_address %x\n",
		adr, return_addr, kva, virt_to_bus(adr), vmalloc_to_page(adr), page_address(vmalloc_to_page(adr)));
#endif

	return return_addr;

}

#define VM_OFFSET(vma) ((vma)->vm_pgoff << PAGE_SHIFT)
static int
init_edt(void)
{
  int nboards = 0;
  int result = 0;
  int memsize = 0;

  BoardValues bv[16];

  nboards = edt_find_boards(bv);

  memsize = edt_get_memsize();

  if (nboards)
	{
	  result = register_chrdev(edt_major, MODULE_NAME, &edt_fops);

	
        if (result < 0)
        {
		edt_printk("edt: can't get major %d\n", edt_major);
		return result;
        }


        if (edt_major == 0)
		edt_major = result;	/* dynamic */

	edt_printk("EDT boards = %d\n", nboards);

	return edt_private_init_module(bv, nboards, memsize);

	}
  else
	{
	  edt_printk("No EDT Boards Found...\n");
	  
	  return 0;


	}


}


/*********************************************
 *
 *
 *
 ********************************************/

static void 
cleanup_edt(void)
{

    unregister_chrdev(edt_major, MODULE_NAME );

	edt_private_cleanup_module();

}

module_init (init_edt);
module_exit (cleanup_edt);


/* #pragma ident "@(#)edt_rtlinux.c	1.6 02/20/02 EDT" */

/*******************************************************************************
 *
 * edt_rtlinux.c:  RTLinux support for EDT DMA drivers.
 *
 * RTLinux support is built into three EDT driver files: edt_rtlinux.c,
 * edt_lnx.c and edt_lnx_kernel.c.
 *
 * Building the RTLinux EDT driver:
 *
 *	Performing a normal Linux EDT driver build on a computer running
 *	RTLinux will configure the driver for RTLinux.  Note that the driver
 *	will not load on regular Linux.
 *
 * Structure of the RTLinux code:
 *
 * 	edt_rtlinux.c isolates calls to RTLinux kernel libraries, and
 *	it must be compiled in /usr/rtlinux/examples/edt_rtlinux/.
 *	Once compiled it is copied over to the linux/driver directory
 *	and linked with edt.o.
 *
 *	edt_lnx.c makes edt_rtlinux_* calls defined in edt_rtlinux.c.
 *	At driver load time, if USE_RTLINUX is defined, edt_lnx.c
 *	registers interrupts through RTLinux instead of Linux by calling
 *	edt_rtlinux_request_irq(int irq) for each uniq IRQ assigned to
 *	EDT boards.  This causes the RTLinux interrupt handler to call
 *	edt_rtlinux_intr_handler() for interrupt processing.
 *
 *      edt_lnx.c also calls edt_rtlinux_start_intr_thread() at this time
 *	to create an interrupt processing thread which runs under control
 *	of the RTLinux real-time scheduler.  The new thread,
 *	edt_rtlinux_intr_thread() defined in edt_lnx.c,	blocks on a semaphore
 *	waiting to be awakened by edt_rtlinux_intr_handler().
 *
 *	edt_rtlinux_intr_handler() calls edt_rt_test_interrupts(), defined
 *	in edt_lnx.c, which loops through each board and tests and clears
 *	the interrupt.
 *
 *	Interrupting boards have an interrupt counter incremented,
 *	and the semaphore that the scheduled interrupt thread is waiting
 *	on is posted.  edt_rtlinux_intr_thread() then waits for
 *	edt_rtlinux_intr_handler() to finish before it is scheduled for
 *	execution.
 *
 *	When edt_rtlinux_intr_thread() wakes up it traverses each EDT
 *	board and compares the number of interrupt with the number of
 *	calls to edt_intr() (the standard Linux interrupt processing routine).
 *	edt_intr() is called until all interrupts for that board have been
 *	processed.
 *	
 *	After all outstanding interrupts for every EDT board are processed,
 *	edt_rtlinux_intr_thread() blocks on the semaphore and waits for
 *	new interrupt activity.
 *
 *	Caveats:
 *
 *	edt_rtlinux_intr_thread() runs when scheduled by the RTLinux
 *	real-time scheduler, which reschedules processes on the default
 *	quantum of 10 miliseconds (100 times per second).  If your expected
 *	interrupt frequency is higher than 100 Hz, then you must adjust
 *	the RTLinux real-time scheduling quantum to suit.
 *
 *	Contacts:
 *
 *	Written by Mark Mason at Engineering Design Team:
 *
 *		(503) 690-1234
 *		tech@edt.com or mark@edt.com
 *
 *
 ******************************************************************************/

#include <rtl.h>
#include <rtl_sched.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

int edt_rtlinux_set_prio(int priority) ;
void *edt_rtlinux_intr_thread(void *) ;
int edt_rt_test_interrupts(void) ;

static pthread_t edt_rtlinux_intr_thread_id ;
static int edt_rtlinux_irqs[32] ;
static int edt_rtlinux_irq_count = 0 ;
static sem_t edt_rtlinux_sem ;



unsigned int
edt_rtlinux_intr_handler(unsigned int irq, struct pt_regs *regs)
{
    int oldpri ;

    oldpri = edt_rtlinux_set_prio(100) ;

    edt_rt_test_interrupts() ;

    rtl_hard_enable_irq(irq) ;

    edt_rtlinux_set_prio(oldpri) ;

    return 0;
}


int
edt_rtlinux_start_intr_thread(void)
{
    sem_init(&edt_rtlinux_sem, 1, 0) ;

    return pthread_create(&edt_rtlinux_intr_thread_id,  NULL, 
		       edt_rtlinux_intr_thread, NULL);
}

int
edt_rtlinux_request_irq(int irq)
{
    edt_rtlinux_irqs[edt_rtlinux_irq_count++] = irq ;
    return rtl_request_irq(irq, edt_rtlinux_intr_handler) ;
}

int
edt_rtlinux_stop_intr_thread(void)
{
    int i ;

    for (i = 0; i < edt_rtlinux_irq_count; ++i)
	rtl_free_irq(edt_rtlinux_irqs[i]) ;

    pthread_delete_np(edt_rtlinux_intr_thread_id) ;

    sem_destroy(&edt_rtlinux_sem) ;

    return 0 ;
}

void
edt_rtlinux_sem_wait(void)
{
    sem_wait(&edt_rtlinux_sem) ;
}

void
edt_rtlinux_sem_post(void)
{
    sem_post(&edt_rtlinux_sem) ;
}

int
edt_rtlinux_set_prio(int priority)
{
    int policy, oldpriority ;
    struct sched_param param ;

    pthread_getschedparam(pthread_self(), &policy, &param) ;
    oldpriority = param.sched_priority ;

    param.sched_priority = priority ;
    pthread_setschedparam(pthread_self(),  policy, &param);

    return oldpriority ;
}

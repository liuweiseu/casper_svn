
/* #pragma ident "@(#)simple_getdata.c    1.14 04/30/99 EDT" */

/*
 * simple_getdata.c: 
 *
 *  An example program to show use of library for ring buffer mode.
 *  This program performs continuous input from the edt dma device
 *  and optionally writes to a disk file or device.
 *
 *  Just use edt_read() if you just have a simple read to do.
 *
 * To compile:
 *  cc -O -DSYSV -o simple_getdata simple_getdata.c -L. -ledt -lthread
 *
 *  A data source must be provided to the EDT board.
 *
 */

#include "edtinc.h"
#include <stdlib.h>

void
usage()
{
    printf("usage: simple_getdata\n") ;
    printf("    -u <unit>   - specifies edt board unit number\n") ;
    printf("    -v        - verbose\n") ;
    printf("    -o outfile  - specifies output filename\n") ;
}

int done = 0;
int bufsize = 0x100000;
int numbufs = 4;
int nwaits = 0;
int exited = 0;

void
capture_thread(void * arg)

{
    EdtDev * edt_p = (EdtDev *) arg;
    double t1, t2;
    int status = 0;
    u_char *p;

    t1 = edt_timestamp();

    if (edt_configure_ring_buffers(edt_p, bufsize, 4, EDT_READ, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }
    while (!done)
    {
    
    if (status == 0)
        edt_start_buffers(edt_p,1);

    p = edt_wait_for_buffers(edt_p,1);

    status = edt_get_wait_status(edt_p);

    printf("buf = %x status %d\n", p, status);

    nwaits++;

    }

    t2 = edt_timestamp();
    printf("elapsed %f\n", t2-t1);

    exited = 1;
}

int
main(int argc, char **argv)
{
    EdtDev *edt_p ;
    FILE   *outfile = NULL ;
    char   *outfile_name = NULL ;
    int     unit = 0, i;
    int     verbose = 0 ;
    u_char *buf ;
    int bufsize = 1024*1024;
    int loops = 100;
	int loop = 0;
    
    int timeout = -1;
    int do_user = 0;
    int timeout_ok = 0;
    int channel = 0;
    thread_t thread;
    int rc = 0;
    int loopsleep = 50;

    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'c':
            ++argv;
            --argc;
            channel = atoi(argv[1]);
            break ;

        case 'u':
            ++argv;
            --argc;
            unit = atoi(argv[1]);
            break ;

        case 'v':
            verbose = 1 ;
            break ;

	case 'w':
	    do_user = 1;
	    break;

	case 'S':
            ++argv;
            --argc;
            loopsleep = atoi(argv[1]);
            break ;

        case 'o':
            ++argv ;
            --argc ;
            outfile_name = argv[1] ;

        if ((outfile = fopen(outfile_name, "wb")) == NULL)
        {
        perror(outfile_name) ;
        exit(1) ;
        }

            break ;

            case 's':
            ++argv ;
            --argc ;
            bufsize = strtol(argv[1],0,0);
            break;    

            case 'l':
            ++argv ;
            --argc ;
            loops = strtol(argv[1],0,0);
            break;    

            case 't':
            ++argv ;
            --argc ;
            timeout = strtol(argv[1],0,0);
            break;    

           case 'k':
            timeout_ok = !timeout_ok;
           break;

        default:
            usage() ;
            exit(1) ;
        }
        --argc ;
        ++argv ;
    }

    if (argc > 1) { usage(); exit(1); }

    if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
    {
        perror("edt_open") ;
        exit(1) ;
    }

    edt_set_timeout_ok(edt_p,timeout_ok);

    if (timeout >= 0)
	edt_set_rtimeout(edt_p, timeout);

    rc = pthread_create(&thread, 0, capture_thread, edt_p);
   
    loop = 0;
 
    while (!exited)
    {

	edt_msleep(loopsleep);

	loop ++;

	if (do_user)
		edt_user_dma_wakeup(edt_p);

	if (loop > loops && loops != 0)
		done = 1;

    }

    pthread_join(thread,NULL);

    printf("buffers completed:  application %d  driver %d\n",
                edt_p->donecount, edt_done_count(edt_p)) ;

    edt_close(edt_p) ;

    exit(0) ;
}

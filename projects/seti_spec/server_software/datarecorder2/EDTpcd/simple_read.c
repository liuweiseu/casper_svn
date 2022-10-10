/* #pragma ident "@(#)simple_getdata.c	1.14 04/30/99 EDT" */

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

#define NUMBUFS 1

void
usage()
{
    printf("usage: simple_getdata\n") ;
    printf("    -u <unit>   - specifies edt board unit number\n") ;
    printf("    -v	    - verbose\n") ;
    printf("    -o outfile  - specifies output filename\n") ;
}
void
init_buf(u_char *buf, int size);

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
	int numbufs = NUMBUFS;

	u_char **buffers;
	u_char *testbuf;
	int kernel_buffers = -1;

	int checking = 0;


    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'u':
            ++argv;
            --argc;
            unit = atoi(argv[1]);
            break ;

        case 'v':
            verbose = 1 ;
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

		case 'C':
		  checking = 1;
		  break;

	case 's':
            ++argv ;
            --argc ;
            bufsize = strtol(argv[1],0,0);
		break;	
		case 'k':
			++argv;
			--argc;
			kernel_buffers = strtol(argv[0],0,0);
			break ;

	case 'l':
            ++argv ;
            --argc ;
            loops = strtol(argv[1],0,0);
			break;	

        default:
            usage() ;
            exit(1) ;
        }
        --argc ;
        ++argv ;
    }

    if (argc > 1) { usage(); exit(1); }

    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
        perror("edt_open") ;
        exit(1) ;
    }
	if (kernel_buffers != -1)
	{

		edt_ioctl(edt_p, EDTS_DRV_BUFFER, &kernel_buffers);

	}

    buffers = (u_char **) edt_alloc(sizeof(u_char *) * numbufs);

    for (i=0;i<numbufs;i++)
	    buffers[i] = edt_alloc(bufsize);

	testbuf = edt_alloc(bufsize);
	init_buf(testbuf, bufsize);

    edt_flush_fifo(edt_p) ;         /* Flush the input fifo */

    for (i = 0; (i < loops || loops == 0); i++)
    {
	  buf = buffers[i % numbufs];
	  
	  edt_read(edt_p, buf, bufsize);

	  if (outfile) fwrite(buf, 1, bufsize, outfile) ;

	  putchar('.') ;
	  fflush(stdout) ;
	  if (checking)
	  {
	  		  if (memcmp(buf, testbuf, bufsize))
		  printf("Buffers differ\n");
		memset(buf,0,bufsize);
	  }

    }
    
    edt_close(edt_p) ;

    exit(0) ;
}
/*
 * initialize buffer with 16-bit incremented data
 * could replace with read of data from file, other device, or ???
 */
void
init_buf(u_char *buf, int size)
{
    u_short word = 0;
    u_short *p, *endp;

    endp = (u_short *) (buf + size);
    p = (u_short *) buf;

    while (p < endp)
    {
    	*p++ = word++;
    }
}

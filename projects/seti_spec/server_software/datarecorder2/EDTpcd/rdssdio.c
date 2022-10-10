#include "edtinc.h"
#include <stdlib.h>
#ifndef _NT_
#include <signal.h>
#endif
/* struct timeval starttime, endtime; */

EdtDev *read_p ;

/*
#define	BUF_SIZE	256*1024 
*/
#define	BUF_SIZE	1024*1024

#define	READBUFS	16


#include "chkbuf.h"

int
edt_rdssdio_test_continuous(EdtDev *read_p, 
						    unsigned int loopcount,
							unsigned int bufsize)

{
				 
  unsigned char *buf;
  unsigned int totalerrors = 0;
  unsigned int rc;

  unsigned int loop;
  int boff;
  
  unsigned char *align;
  
  /*
	 * get a buffer for temporay use
	 */
  if ((align = (unsigned char *)malloc(bufsize)) == 0) 
	{
	  printf("malloc failed for aligned array\n");
	  exit(1);
	}

		
  edt_start_buffers(read_p, READBUFS);
	
  /* increment loopcount because we throw out the first one */

  if (loopcount > 0)
	loopcount++;
  
  for(loop = 0 ; loop < loopcount || loopcount == 0 ; loop++)
	{
			
	  buf = edt_wait_for_buffers(read_p,1);
			

	  if (edt_ring_buffer_overrun(read_p))
		putchar('o');

   	  if (loop < loopcount - 2)
		edt_start_buffers(read_p, 1);
	  
	  if (loop > 0)
		{
		  boff = bitoffset(buf);
			  
		  cp_shift(buf, align, boff, bufsize);
			  
		  rc  = chkbuf(align, bufsize, loop, 1);
		
		  totalerrors += rc;

		  if (rc == 0)
			putchar('.');
		  else
			putchar ('x');

		  fflush(stdout);
		}
	}
  
  free(align);

  return totalerrors;

}

int
main(int argc, char **argv)
{
	int i, loop ;
	unsigned char *rtmpp[READBUFS] ;
	unsigned char *align ;
	int continuous = 1;

	int rc;

	int channel = 0;

	int loopcount = 1 ;
	int unit = 0 ;
	int bufsize = BUF_SIZE;
	int timeout = 5000 ;	/* default timeout 5 seconds */
	int verbose = 1;
	FILE *output_file = NULL;

	int boff;

    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
		switch (argv[0][1])
		{
		  case 'u':
			++argv;
			--argc;
			unit = atoi(argv[0]);
			break;

		  case 'l':
			++argv;
			--argc;
			loopcount = atoi(argv[0]);
			break;

		  case 'n':
			++argv;
			--argc;
			channel = atoi(argv[0]);
			break;

		  case 't':
			++argv;
			--argc;
			timeout = atoi(argv[0]);
			break;

		  case 'b':
			++argv;
			--argc;
			bufsize = atoi(argv[0]);
			break;

		case 'c':
		  continuous = 1;
		  break;

		  
		case 's':
		  continuous = 0;
		  break;

		case 'o':
		  
		  ++argv;
		  --argc;
		  output_file = fopen(argv[0],"wb");
		  break;

		  case 'q':
			verbose = 0;
			break;
		
		  default:
			puts("Usage: rdssdio [-v (verbose)] [-u unit_no] [-t timeout] [-b buffer size]\n");
			exit(0);
		}
		argc--;
		argv++;
    }

	if ((read_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
	{
		edt_perror(EDT_INTERFACE) ;
		puts("Hit return to exit") ;
		getchar() ;
		return(1) ;
	}

	/* don't set RFIFO on every read and write */
	edt_set_autodir(read_p, 0) ;

	edt_set_timeout_action(read_p, EDT_TIMEOUT_BIT_STROBE);
	
	if (verbose)
	printf("Setting Read timeout to %d\n", timeout);

	edt_set_rtimeout(read_p,timeout);
	
	if (bufsize < 128000)
	{
		if (verbose)
		  printf("buffer size too small for efficent DMA - %d bytes\n", bufsize);
	}
	if (verbose)
	  printf("buffer size = %d\n", bufsize);


	if (verbose)
	  printf("configure %d ring buffers to read to buf size %d\n",
		READBUFS, bufsize) ;
    if (edt_configure_ring_buffers(read_p, bufsize, READBUFS, 
				EDT_READ, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }

	/*
	 * get a buffer for temporay use
	 */
	if ((align = (unsigned char *)malloc(bufsize)) == 0) 
	{
		printf("malloc failed for aligned array\n");
		exit(1);
	}


	if (continuous)
	  {
	
		rc = edt_rdssdio_test_continuous(read_p,
									loopcount,
									bufsize);

		if (verbose)
		  printf("\n%d errors in %d buffers (%g bytes)\n",
				 rc, loopcount, 
				 (double) loopcount * (double) bufsize);

				 
	  }

	else

	  {

	  for(loop = 0 ; loop < loopcount || loopcount == 0 ; loop++)
		{
	  
		  edt_start_buffers(read_p, READBUFS) ; 

		  /* read all the buffers allocatied */
		  for( i=0; i< READBUFS; i++)
			{
			  u_int timevals[2];
			  int timeouts = edt_timeouts(read_p);
			  timevals[0] = timevals[1] = 0;
			  rtmpp[i] = edt_wait_buffers_timed(read_p, 1, timevals) ;
			  printf("read (time=%u.%06u) %d %p\n", timevals[0], timevals[1] / 1000,i, rtmpp[i]) ;
			  if (timeouts != edt_timeouts(read_p))
				{
				  printf("Timeout - good bits in last word = %d\n",
						 edt_get_timeout_goodbits(read_p));

				}

				
			}

		  if (output_file)
			{
			  boff = bitoffset(rtmpp[0]);
			  cp_shift(rtmpp[0], align, boff, bufsize);
			  fwrite(align, bufsize, 1, output_file);
			}

		  /*
		   * don't check buffer 0 since it could have bad
		   * data in the FIFO at startup
		   */
		  for(i=0; i< READBUFS; i++)
			{

			  if (verbose)
				{
				  printf("raw data read buffer %d\n", i);
				  dumpblk(rtmpp[i], 0, bufsize);
				}

			  boff = bitoffset(rtmpp[i]);
			  cp_shift(rtmpp[i], align, boff, bufsize);
			  if (output_file)
				{
				  fwrite(rtmpp[i], bufsize, 1, output_file);
				}
			  if (verbose)
				{
				  printf("corrected data read buffer %d\n", i);
				  dumpblk(align, 0, bufsize);
				}
				
			  chkbuf(align, bufsize, i, 1);
			}


		}
	  }

	if (verbose)
	  printf("read done %d \n",
		 edt_done_count(read_p)) ;

	if (output_file)
	  fclose(output_file);

	edt_close(read_p) ;
	exit(0) ;
	return(0) ;
}

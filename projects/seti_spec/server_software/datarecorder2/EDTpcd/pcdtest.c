/* #pragma ident "@(#)pcdtest.c	1.22 05/08/03 EDT" */

#include "edtinc.h"

#ifndef _NT_
#include <signal.h>
#endif

int funct_test(EdtDev *edt_uut, EdtDev *edt_td);
int stat_test(EdtDev *edt_uut, EdtDev *edt_td);
int dma_read_test(EdtDev *edt_uut,  EdtDev *edt_td, int *haltflag);
int dma_write_test(EdtDev *edt_uut, EdtDev *edt_td);
void print_x_header(FILE *xfile);
int test_char(EdtDev *edt_p, int wptr, int rptr, int fptr, char *errormsg) ;



#define	DATABUFSIZE	0x10000
#define	BUFSIZE	0x100000
#define	BUFS	4
#define	READ	0
#define	WRITE	1

#define	USAGE "usage: %s [-l loopcount] pcd_unit_to_be_test_driver unit_under_test_pcd \n"


int i,x	;
EdtDev *edt_td;	/* file	descriptor for test driver pcd */
EdtDev *edt_uut;	/* file	descriptor for unit under test file */

#ifndef _NT_
void got_sigint(sig)
int sig;
{
	printf("interrupted - exiting\n") ;
	edt_close(edt_td) ;
	edt_close(edt_uut) ;
	exit(0) ;
}

char * bitload_name = "./bitload";

#else

char * bitload_name = ".\\bitload";

#endif

int dbgsave = 0 ;
int dbgcnt = 0 ;
int try_resync = 0 ;
uchar_t use_pcd_src = 0 ;

int backio;

int
main(argc,argv)
int argc;
char *argv[];
{

    int	uut_no ;
    int	td_no ;
    char *progname ;
    int	readval	;
    char cmd[100] ;
	int loop ;
	int loops_todo = 1 ;
	int total_errors = 0 ;

    progname = argv[0];

    /* check args */
    if (argc < 3)
    {
	    fprintf(stderr,USAGE,progname);
	    exit(1);
    }

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
		switch (argv[0][1])
		{
		  case 'l':
			++argv;
			--argc;
			loops_todo = atoi(argv[0]);
			break;
		  case 'd':
			dbgsave = 1 ;
			break ;
		  case 'r':
			try_resync = 1 ;
			break ;
		  /* pmc board enable p2 io */
		  case 'p':
			backio = 1;
			break;  
		  case 's':
			use_pcd_src = 0x80 ;
			break;  
		}
		argc--;
		argv++;
    }

    td_no = atoi(argv[0]) ;
    (void) sprintf(cmd,	"%s  -u %d pcdtest.bit", bitload_name, td_no);
    system(cmd)	;

    uut_no = atoi(argv[1]) ;
    if (use_pcd_src)
	(void) sprintf(cmd,	"%s  -u %d pcd_src.bit", bitload_name, uut_no);
    else
	(void) sprintf(cmd,	"%s  -u %d pcd_looped.bit", bitload_name, uut_no);
    system(cmd)	;



    if ((edt_td = edt_open(EDT_INTERFACE, td_no)) == NULL)
    {
		edt_perror("edt_open(pcd_td)") ;
		return(2) ;
    }

    if ((edt_uut = edt_open(EDT_INTERFACE, uut_no)) == NULL)
    {
		edt_perror("edt_open(pcd_uut)") ;
		return(2) ;
    }

    /* turn off	all i/o	drivers	 and test if the register took it */
    edt_intfc_write_short(edt_td,PCDT_IODIR,0xffff) ;
    if ( (readval = edt_intfc_read_short(edt_td,PCDT_IODIR)) != 0xffff)
	printf("%s: failed to set io drivers to	tristate - register was	%04x s/b 0xffff\n",
		progname, readval);

#ifndef _NT_
	sigset(SIGINT, got_sigint) ;
#endif

    for(loop = 0 ; loop < loops_todo || loops_todo == 0 ; loop++)
    {
		static loopcount = 0 ;
		static int error = 0;
		int errs ;
		u_short cmd;

		printf("SETUP: test driver is pcd%d;  uut is pcd%d\n\n", td_no, uut_no);

		/* init tester, turn off enable compare */
		/* edt_intfc_write_short(edt_td, PCDT_CMD, 0x0 | use_pcd_src) ; */
		cmd = 0;
		if (backio) cmd |= PCD_EN_BACK_IO;
		edt_intfc_write_short (edt_td, PCDT_CMD, 
									  (u_short)(cmd | use_pcd_src));

		printf("TEST FUNCT REGISTER:\t\t\t");
		if ( (errs = test_char(edt_td, PCDT_FUNCT, PCDT_FUNCT, PCDT_STAT,
					"xilinx funct failed")) == 0)
		{
			puts("PASSED") ;
		}
		else
		{
			printf("FAILED: %d errors\n", errs);
		}
		error +=  errs ;


		edt_intfc_write_short(edt_td,PCDT_IODIR,0xf30f) ;
		/* set uut for input */
		edt_intfc_write_short(edt_uut,PCD_DIRREG,0xccf0) ;

		error += funct_test(edt_uut, edt_td);
		error += stat_test(edt_uut, edt_td);

		errs = 0;
		dbgcnt = loopcount ;
		error += dma_read_test(edt_uut, edt_td, &errs);
		/* exit	if DMA did not complete	*/
		if (errs != 0)
		{
			error += errs ;
			printf("TEST HALTED: can not continue if DMA does not complete\n");
			puts("Hit return to continue");
			getchar() ;
			exit(4) ;
		}

		/*
		error += dma_cont_read_test(uut_no, edt_td->tdfd, 100) ;
		*/

		/*
		 * turn	the io buffers around and tristate the xilinx data outputs
		 * on the test driver pcd will happen automatically on the write.
		 */
		edt_intfc_write_short(edt_td, PCDT_CMD, (u_short)(PCDTEST_ENCHK | use_pcd_src)) ;
		edt_intfc_write_short(edt_td ,PCDT_IODIR,0xf3f0) ;

		error += dma_write_test(edt_uut, edt_td) ;
		loopcount++ ;
		printf("\n\t%d Loops,\t\t%d Errors\n\n", loopcount, error) ;
    }

	puts("Closing the EDT boards") ;
	edt_close(edt_td) ;
	edt_close(edt_uut) ;

    return(0);
}


/*
 * test	the function outputs ouf the unit under	test
 * returns the number of errors	encountered
 */
int
funct_test(EdtDev *edt_uut, EdtDev *edt_td)
{
	int i;
	unsigned char shdbe, rdbk, was;
	int errcnt = 0;

	printf("TEST FUNCT SIGNALS OUT:\t\t\t");
	fflush(stdout) ;
	for(i=0; i<4; i++)
	{
	    shdbe = 1 << i ;
	    pcd_set_funct(edt_uut, shdbe) ;
	    rdbk = pcd_get_funct(edt_uut) ;

	    if ( (was =	(edt_intfc_read_short(edt_td, PCDT_STAT) & PCDTEST_STAT_MSK))
							!= shdbe)
	    {
		printf(" function bit test failed was %x should	be %x readback %x\n",
			was, shdbe, rdbk);
		getchar();
		errcnt++;
	    }
	}
	for(i=0; i<4; i++)
	{
	    shdbe = (~(1 << i))	& 0xf ;
	    pcd_set_funct(edt_uut, shdbe) ;
	    rdbk = pcd_get_funct(edt_uut) ;

	    if ( (was =	(edt_intfc_read_short(edt_td, PCDT_STAT)& PCDTEST_STAT_MSK))
							!= shdbe)
	    {
		printf(" function bit test failed was %x should	be %x readback %x\n",
			was & 0xf, shdbe & 0xf,	rdbk & 0xf);
		errcnt++;
	    }
	}
	if (errcnt == 0)
	    printf("PASSED\n");
	else
	    printf("FAILED\n");
	return(errcnt);
}

int got_event[4] ;

void got_stat1_event(void *p)
{
	got_event[0] = 1 ;
}
void got_stat2_event(void *p)
{
	got_event[1] = 1 ;
}
void got_stat3_event(void *p)
{
	got_event[2] = 1 ;
}
void got_stat4_event(void *p)
{
	got_event[3] = 1 ;
}


/*
 * test	the stat inputs	ouf the	unit under test
 * returns the number of errors	encountered
 */
int
stat_test(EdtDev *edt_uut, EdtDev *edt_td)
{
	unsigned char rdbk;
	int errcnt = 0;
	int errcnt_save ;
	u_char polarity;
	u_char shdbe ;
	u_char was ;

	edt_intfc_write(edt_td, PCDT_FUNCT, 0) ;

	rdbk = edt_intfc_read(edt_td, PCDT_FUNCT) ;

	/*
	 * set the stat	line polarity register.
	 */
	polarity = 0x00	;
	pcd_set_stat_polarity(edt_uut, polarity) ;
	polarity = pcd_get_stat_polarity(edt_uut) ;


	/*
	 * Tell	driver to enable stat line interrupts and map
	 * the four stat lines into events which will be sent to
	 * this	process.
	 */

	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT1, got_stat1_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT2, got_stat2_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT3, got_stat3_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT4, got_stat4_event,edt_uut,0);
	edt_msleep(100);

	printf("TEST STAT events IN:\t\t\t");
	fflush(stdout) ;
	shdbe = 1 ;
	for(i=0; i < 4; i++)
	{
	    got_event[i] = 0 ;

	    edt_intfc_write(edt_td, PCDT_FUNCT, shdbe) ;

	    rdbk = edt_intfc_read(edt_td, PCDT_FUNCT) ;

	    edt_msleep(100) ;
	    was = pcd_get_stat(edt_uut) ;
	    if ( (was =	(was & PCDTEST_FUNCT_MSK)) != shdbe)
	    {
			printf(" stat bit test failed was %x should be %x readback %x again %x\n",
				was, shdbe, rdbk, pcd_get_stat(edt_uut));
			errcnt++;
	    }
#ifndef __hpux
	    edt_msleep(100) ;
		if (!got_event[i])
		{
			printf("didn't get event for stat %d\n", i+1) ;
			errcnt++ ;
		}
#endif
	    shdbe = (shdbe << 1) | 1 ;
	}

	if (errcnt == 0)
	    printf("PASSED\n");
	else
	    printf("FAILED\n");

	printf("TEST STAT events IN (inverse polarity):\t");
	fflush(stdout) ;

	errcnt_save = errcnt ;

	edt_intfc_write(edt_td, PCDT_FUNCT, 0x0f) ;

	rdbk = edt_intfc_read(edt_td, PCDT_FUNCT) ;

	/*
	 * set the stat	line polarity register.
	 */
	polarity = 0x0f	;
	pcd_set_stat_polarity(edt_uut, polarity) ;
	polarity = pcd_get_stat_polarity(edt_uut) ;

	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT1, got_stat1_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT2, got_stat2_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT3, got_stat3_event,edt_uut,0);
	edt_set_event_func(edt_uut, EDT_EVENT_PCD_STAT4, got_stat4_event,edt_uut,0);
	edt_msleep(100) ;

	shdbe = 0xf ;
	for(i = 0; i < 4; i++)
	{
		got_event[i] = 0 ;
	    shdbe = (shdbe << 1) & 0x0f ;

	    edt_intfc_write(edt_td, PCDT_FUNCT, shdbe) ;

	    rdbk = edt_intfc_read(edt_td, PCDT_FUNCT) ;

	    edt_msleep(100) ;
	    was = pcd_get_stat(edt_uut) ;
	    if ( (was =	(was & PCDTEST_FUNCT_MSK)) != shdbe)
	    {
			printf(" stat bit test failed was %x should be %x readback %x again %x\n",
				was, shdbe, rdbk, pcd_get_stat(edt_uut));
			errcnt++;
	    }
#ifndef __hpux
	    edt_msleep(100) ;
		if (!got_event[i])
		{
			printf("didn't get event for stat %d\n", i+1) ;
			errcnt++ ;
		}
		fflush(stdout) ;
#endif
	}


	if (errcnt - errcnt_save == 0)
	    printf("PASSED\n");
	else
	    printf("FAILED\n");

	return(errcnt);
}


/*
 * print all the xilinx	header data until a line feed is followed by a one
 */
void
print_x_header(FILE *xfile)
{
	int c; /* current character */
	int linefeed = 0; /* set when last character was a linefeed */

	printf("Raw bits file header information\n");
	while( ((c = getc(xfile)) != EOF) && !((linefeed == 1) && (c ==	'1')))
	{
		if ( c == '\n')
			linefeed = 1;
		else
			linefeed = 0;
		putchar(c);
	}
	/* push	the first bit back */
	ungetc(c, xfile);
}



int err_printed	;
int resync_printed	;
/*
 * compare the two unsigned shorts passed count	the bits that are different
 * return 1 if the words miscompare, 0 if the same.
 * print the first 16 bad compares
 */
int
compare_short(index, shdbe, was, cnts)
int index;
unsigned short was, shdbe ;
int *cnts; /* array of 16 ints */
{
    unsigned short err_msk;
    int	bit;

    if ((err_msk = was ^ shdbe)	!= 0 )
    {
	if (err_printed	< 8)
	{
	    printf("index %d (byte 0x%x) was %x shouldbe %x	mask %x\n", 
		index, index * 2, was,	shdbe, err_msk);
	    err_printed++ ;
	}
	for(bit=0; bit!=16; bit++)
	{
	    if(err_msk & 0x1)
	    {
		cnts[bit]++;
	    }
	    err_msk = err_msk >>1;
	}
	return(1) ;
    }
    else
    {
	return(0);
    }
}

/*
 * check data in a buffer against the test driver generator algorithm
 * count the number of errors in each bit and print. return the	total words incorrect.
 */


int
good_data(buf, size)
unsigned short *buf;
int size;
{
    int	bit_err[16] ;
    int	bit, i,	errcnt;
    unsigned short shdbe;

    for(bit=0; bit!=16;	bit++)
	bit_err[bit] = 0;

    /* check the data */
    errcnt = 0 ;
    err_printed	= 0 ;
	resync_printed = 0 ;
    shdbe = 0 ;	 /* first id the same as start */
    for(i=0; i!=size; i++)
    {
		errcnt += compare_short(i, shdbe, buf[i], bit_err);
		/* TODO - should we sync up if just missing or extra word? */
		if (try_resync && (buf[i] == shdbe - 1
			|| buf[i] == shdbe - 2
			|| buf[i] == shdbe + 1
			|| buf[i] == shdbe + 2))
		{
			int j ;
			int k ;
			int k_tmp ;
			if (!resync_printed)
			{
				k = i - 4 ;
				k_tmp = shdbe + 4 ;
				if (k < 0)
				{
					k = 0 ;
					k_tmp = 0 ;
				}
				printf("WARNING - trying to resync data compare\n") ;
				printf("shdbe at %08x:\n",i * 2) ;
				for(j = 0 ; j < 8 ; j++)
				{
					printf("%04x ",k_tmp-j) ;
				}
				printf("\n") ;
				printf("was:\n") ;
				for(j = 0 ; j < 8 ; j++)
				{
					printf("%04x ",buf[k+j]) ;
				}
				printf("\n") ;
			}
				
			shdbe = buf[i] ;
		}
		shdbe--	;
    }
	if (errcnt && dbgsave)
	{
		FILE *tmpf ;
		char tmpname[20] ;
		sprintf(tmpname, "pcderrs.%d",dbgcnt) ;
		printf("debug - saving bad data in %s\n",tmpname) ;
		tmpf = fopen(tmpname,"wb") ;
		if (tmpf)
		{
			fwrite(buf,size*2,1,tmpf) ;
			fclose(tmpf) ;
		}
	}
	memset(buf,0,size) ;

    /* if non zero errors print	error array */
    if (errcnt != 0)
    {
	for(bit=0; bit!=16; bit++)
	{
	    printf("%d - %d, ",	bit, bit_err[bit]);
	}
	printf("\n");

#ifdef PRINTERR
	ibuf = (unsigned int *)buf ;
	for(i=0; i!=20;	i++)
	{
	    for( bit=0;	bit!=16; bit++)
	    {
		printf("%04x ",	buf[(i*	16)+bit]);
	    }
	    printf("\n");
	    for( bit=0;	bit!=8;	bit++)
	    {
		printf("%08x  ", ibuf[(i* 8)+bit]);
	    }
	    printf("\n");
	}
#endif
    }

    return(errcnt);
}

/* return true if little_endian */
int little_endian()
{
	u_short test ;
	u_char *byte_p ;
	byte_p = (u_char *)&test ;
	*byte_p++ = 0x11 ;
	*byte_p = 0x22 ;
	if (test == 0x1122) return(0) ;
	else return(1) ;
}

/*
 * dma_read_test sets up a dma read transfer of	65k words, test	if it waits
 * until the data is enabled, starts the data, tests if	it completes, and then
 * checks the data. returns the	number of errors.
 */
int
dma_read_test(EdtDev *edt_uut,  EdtDev *edt_td, int *haltflag)
{
	int	errcnt = 0;
	int	errs ;
	unsigned short *buf	; /* pointer to	the buffer */

	/* turn off	data generation	*/
	edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(0 | use_pcd_src)) ;

	/* reset fifo */
	edt_intfc_write(edt_uut, PCD_CMD, PCD_ENABLE ) ;
	edt_intfc_write(edt_uut, PCD_CMD, 0x00 ) ;
	edt_intfc_write(edt_uut, PCD_CMD, PCD_ENABLE ) ;

	if (little_endian())
		edt_intfc_write(edt_uut, PCD_CONFIG, 0) ;
	else
		edt_intfc_write(edt_uut, PCD_CONFIG, PCD_BYTESWAP) ;

	/* set tester counter to 0x0 */
	edt_intfc_write_short(edt_td,PCDT_START,0x0) ;

#if 0
	/* reset dma fifo */
	dma_cmd = PCD_RST_FIFO ;
	ioctl(edt_uut->fd, PCDS_DMA_CMD, &dma_cmd);
#endif


	/*
	 * Have edt_configure_ring_buffers allocate	the ring buffers by
	 * passing the last	argument as NULL.  Otherwise pass in an	array
	 * of pointers to the buffers.
	 */
	if (edt_configure_ring_buffers(edt_uut, DATABUFSIZE*2, 2, EDT_READ, NULL) == -1)
	{
		perror("edt_configure_ring_buffers") ;
		exit(1)	;
	}

	fputs("DMA READ TEST; DATA DISABLED:\t\t", stdout);
	fflush(stdout) ;
	/* start the read */
	edt_start_buffers(edt_uut, 1)	;
	edt_msleep(100) ;


	if (edt_check_for_buffers(edt_uut, 1))
	{
		printf("FAILED:	read completed while test driver data disabled\n");
		errcnt++ ;
	}
	else
	{
		puts("PASSED");
	}

	/* start data in tester */
	edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_ENABLE | use_pcd_src) ) ;
	fputs("DMA READ TEST; DATA ENABLED:\t\t", stdout) ;
	fflush(stdout) ;
	edt_msleep(100) ;
	if (buf = (unsigned short *) edt_check_for_buffers(edt_uut, 1))
	{
		puts("PASSED") ;
		fputs("DMA READ TEST: check data\t\t", stdout);
		fflush(stdout) ;
		if (( errs = good_data(buf, DATABUFSIZE) ) == 0)
		{
			puts("PASSED");
			/*
				     * print the initial data if not commented ou
				    for(i=0; i!=16; i++)
				    {
					printf("%04x ",	buf[i]);
				    }
				    printf("\n");
				    */
		}
		else
		{
			printf("FAILED: %d words of	%d incorrect\n", errs, DATABUFSIZE);
			errcnt += errs ;
		}
	}
	else
	{
		printf("FAILED:	no data	received after 3 seconds\n");
		*haltflag = 1;
		errcnt++ ;
	}
	edt_disable_ring_buffers(edt_uut) ;

	return(errcnt) ;
}


/*
 * dma_write_test sets up a dma	write transfer of 65k words, test if it	waits
 * until the data is enabled, starts the data, tests if	it completes, and then
 * checks the data. returns the	number of errors.
 */
int
dma_write_test(EdtDev *edt_uut, EdtDev *edt_td)
{
    int	errcnt = 0;
    int	i;
    unsigned short errs, init ;
    unsigned short *buf	; /* pointer to	the buffer */

    /* set swap byte */
    if (little_endian())
		edt_intfc_write(edt_uut, PCD_CONFIG, 0) ;
    else
		edt_intfc_write(edt_uut, PCD_CONFIG, PCD_BYTESWAP) ;
    /*
     * Have edt_configure_ring_buffers allocate	the ring buffers by
     * passing the last	argument as NULL.  Otherwise pass in an	array
     * of pointers to the buffers.
     */
    if (edt_configure_ring_buffers(edt_uut, DATABUFSIZE*2, 2, EDT_WRITE, NULL) ==	-1)
    {
	edt_perror("edt_configure_ring_buffers") ;
	exit(1)	;
    }
    buf	= (unsigned short *) edt_next_writebuf(edt_uut) ;


    /*
     * initialize the buffer
     */
    init = 0;
    for(i=0; i!=DATABUFSIZE; i++)
    {
	    buf[i] = init /* | 0x1 */  ;
	    init-- ;
    }


    fputs("DMA WRITE TEST; DATA DISABLED:\t\t", stdout);
	fflush(stdout) ;

    /* reset check check circuits  set DNR */
    edt_intfc_write(edt_td, PCDT_START,	0) ;
    edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_SETDNR | use_pcd_src)) ;
    edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_ENCHK | PCDTEST_SETDNR | use_pcd_src)) ;
    /*
     * print the start of the buffer move the close comment up
    for(i=0; i!=16; i++)
    {
		printf("%04x ",	buf[i]);
    }
    printf("\n");
    */

    edt_start_buffers(edt_uut, 1)	;
    if (edt_check_for_buffers(edt_uut, 1))
    {
		puts("FAILED: write completed with data disabled (DNR failed)");
		errcnt++ ;
    }
    else
    {
		puts("PASSED") ;
    }


    fputs("DMA WRITE TEST; DATA ENABLED:\t\t", stdout) ;
	fflush(stdout) ;

    edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_ENCHK | PCDTEST_SETDNR | use_pcd_src)) ;

    /* delay to	let start pass through to test driver refernce counter */
    edt_msleep(100);
    if (( errs = edt_intfc_read_short(edt_td,PCDT_BITE)) != 0)
    {
		printf("TEST DRIVER FAILED: biterr is %04x not 0000 during reset\n", errs);
		printf("command	reg = %02x stat	= %02x biterr addr %08x\n",
			edt_intfc_read(edt_td, PCDT_CMD),
			edt_intfc_read(edt_td, PCDT_STAT),
			edt_intfc_read_short(edt_td, PCDT_BITE)) ;
    }

    /* enable error register */
    edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_ENCHK | PCDTEST_SETDNR | PCDTEST_ENCMP | use_pcd_src)) ;
    edt_msleep(100);

    /* start data from pcd */

    edt_intfc_write(edt_td, PCDT_CMD, (uchar_t)(PCDTEST_ENCHK | PCDTEST_ENCMP | use_pcd_src)) ;

    edt_msleep(100) ;
    if (edt_check_for_buffers(edt_uut, 1))
    {
		puts("PASSED");
    }
    else
    {
		puts("FAILED:  no data output after 2 seconds");
		errcnt++ ;
    }

    fputs("DMA WRITE TEST: check data\t\t", stdout);
	fflush(stdout) ;
    if (( errs = edt_intfc_read_short(edt_td, PCDT_BITE)) == 0)
    {
		puts("PASSED");
    }
    else
    {
		printf("FAILED:	bits %04x incorrect\n",	errs);
		errcnt ++ ;
    }

    return(errcnt) ;
}

int
test_char(EdtDev *edt_p, int wptr, int rptr, int fptr, char *errormsg)
{
	int i;
	unsigned char value;
	unsigned char read;
	int err	= 0;

	/* printf("testing %s at %x %x\n",errormsg,wptr,rptr) ;	*/
	for(i=0; i!=8; i++)
	{
	    value = 0x1	<< i;

	    edt_intfc_write(edt_p,wptr,value) ;
	    /* flash the bus high - stat is read only */
	    if (fptr !=	0) edt_intfc_write(edt_p,fptr,0xff) ;
	    edt_msleep(10);
	    read = edt_intfc_read(edt_p,rptr) ;
	    if (read !=	value)
	    {
		err++;
		printf("ERROR -	%s r offset %02x  w/r, was %04x	s/b %04x\n",
			 errormsg, rptr, read, value );
		getchar();
	    }
	}
	for(i = 0; i != 8; i++)
	{
	    value = (~(0x1 << i));

	    edt_intfc_write(edt_p,wptr,value) ;
	    /* flash the bus high - stat is read only */
	    if (fptr !=	0) edt_intfc_write(edt_p,fptr,0xff) ;
	    edt_msleep(10);
	    read = edt_intfc_read(edt_p,rptr) ;
	    if (read !=	value)
	    {
		err++;
		printf("ERROR -	%s r offset %02x  w/r, was %04x	s/b %04x\n",
			 errormsg, rptr, read, value );
	    }
	}
    return(err);
}

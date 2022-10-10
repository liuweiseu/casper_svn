/* #pragma ident "@(#)pcdtest_bb.c	1.12 05/07/04 EDT" */

#include "edtinc.h"

#ifndef _NT_
#include <signal.h>
#endif

int     funct_test(EdtDev * edt_uut, EdtDev * edt_td);
int     stat_test(EdtDev * edt_uut, EdtDev * edt_td);
int     dma_read_test(EdtDev * edt_uut, EdtDev * edt_td, int *haltflag);
int     dma_write_test(EdtDev * edt_uut, EdtDev * edt_td);

int extended_dma_test(EdtDev * edt_w, EdtDev * edt_r);


#define	DATABUFSIZE	0x10000
#define	BUFSIZE		0x100000
#define	BUFS		4
#define	READ		0
#define	WRITE		1

#define	USAGE "usage: %s [-l loop] pcd_unit_to_be_test_driver unit_under_test_pcd\n"


int     i, x;
EdtDev *edt_td = NULL ;		/* file descriptor for test driver pcd */
EdtDev *edt_uut = NULL ;	/* file descriptor for unit under test file */

#ifndef _NT_
void
got_sigint(sig)
    int     sig;
{
    printf("interrupted - exiting\n");
    edt_close(edt_td);
    edt_close(edt_uut);
    exit(0);
}

char   *bitload_name = "./bitload";

#else

char   *bitload_name = ".\\bitload";

#endif

int     dbgsave = 0;
int     dbgcnt = 0;
int     try_resync = 0;
uchar_t use_pcd_src = 0;

int     backio;
int     read_only = 0;
int     write_only = 0;

int
main(int argc, char *argv[])
{
    int     uut_no;
    int     td_no;
    int     uut_id;
    int     td_id;
    char   *progname;
    char    cmd[128];
    int     loop;
    int     loops_todo = 1;
    int     total_errors = 0;

    progname = argv[0];

    /* check args */
    if (argc < 3)
    {
	fprintf(stderr, USAGE, progname);
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
	    dbgsave = 1;
	    break;
	case 'r':
	    try_resync = 1;
	    break;
	    /* pmc board enable p2 io */
	case 'p':
	    backio = 1;
	    break;
	case 's':
	    use_pcd_src = 0x80;
	    break;
	case 'R':
	    read_only = 1;
	    break;
	case 'W':
	    write_only = 1;
	    break;
	}
	argc--;
	argv++;
    }

    if (read_only)
	uut_no = atoi(argv[0]);
    else if (write_only)
	td_no = atoi(argv[0]);
    else
    {
	td_no = atoi(argv[0]);
	uut_no = atoi(argv[1]);
    }



    /*
     * Obtain device id to determine how to configure.
     */
    if (!read_only)
    {
	if ((edt_td = edt_open(EDT_INTERFACE, td_no)) == NULL)
	{
	    edt_perror("edt_open(pcd_td)");
	    return (2);
	}
	td_id  = edt_device_id(edt_td) ;
	edt_close(edt_td) ;

	if (td_id >= 0x11 && td_id <= 0x13)
	{
	    if (use_pcd_src)
		sprintf(cmd, "%s -u %d pcd_looped.bit", bitload_name, td_no);
	    else
		sprintf(cmd, "%s -u %d pcd_src.bit", bitload_name,td_no);
	}
	else if (td_id == 0x26)
	    sprintf(cmd, "%s -u %d gp_pcd.bit", bitload_name, td_no);
	else if (td_id == 0x40)
	    sprintf(cmd, "%s -u %d ss_pcd.bit", bitload_name, td_no);
	else if (td_id == 0x44)
	    sprintf(cmd, "%s -u %d pcda.bit", bitload_name, td_no);

	puts(cmd) ;
	if (edt_system(cmd))
	    exit(1) ;

	if ((edt_td = edt_open(EDT_INTERFACE, td_no)) == NULL)
	{
	    edt_perror("edt_open(pcd_td)");
	    return (2);
	}
	if (td_id == 0x26 || td_id == 0x40 || td_id == 0x44)
	{
	    if (use_pcd_src)
		edt_reg_or(edt_td, PCD_CONFIG, PCD_SEL_RXT) ;/* Use pcd_looped*/
	    else
		edt_reg_and(edt_td, PCD_CONFIG, ~PCD_SEL_RXT) ;/* Use pcd_src */
	}
	edt_flush_fifo(edt_td) ;
    }

    if (!write_only)
    {
	if ((edt_uut = edt_open(EDT_INTERFACE, uut_no)) == NULL)
	{
	    edt_perror("edt_open(pcd_uut)");
	    return (2);
	}
	uut_id = edt_device_id(edt_uut);
	edt_close(edt_uut) ;


	if (uut_id >= 0x11 && uut_id <= 0x13)
	{
	    if (use_pcd_src)
		sprintf(cmd, "%s -u %d pcd_src.bit", bitload_name, uut_no);
	    else
		sprintf(cmd, "%s -u %d pcd_looped.bit", bitload_name, uut_no);
	}
	else if (uut_id == 0x26)
	    sprintf(cmd, "%s -u %d gp_pcd.bit", bitload_name, uut_no);
	else if (uut_id == 0x40)
	    sprintf(cmd, "%s -u %d ss_pcd.bit", bitload_name, uut_no);
	else if (uut_id == 0x44)
	    sprintf(cmd, "%s -u %d pcda.bit", bitload_name, uut_no);

	puts(cmd) ;
	if (edt_system(cmd))
	    exit(1) ;

	if ((edt_uut = edt_open(EDT_INTERFACE, uut_no)) == NULL)
	{
	    edt_perror("edt_open(pcd_uut)");
	    return (2);
	}
	if (uut_id == 0x26 || uut_id ==0x40 || uut_id == 0x44)
	{
	    if (use_pcd_src)
		edt_reg_and(edt_uut, PCD_CONFIG, ~PCD_SEL_RXT);/* Use pcd_src */
	    else
		edt_reg_or(edt_uut, PCD_CONFIG, PCD_SEL_RXT);/* Use pcd_looped*/
	}
	edt_flush_fifo(edt_uut) ;
    }



#ifndef _NT_
    sigset(SIGINT, got_sigint);
#endif

    for (loop = 0; loop < loops_todo || loops_todo == 0; loop++)
    {
	static  loopcount = 0;
	static int error = 0;
	int     errs;

	printf("\n\nSETUP: test driver is pcd%d;  unit under test is pcd%d\n",
							    td_no, uut_no) ;
	if (use_pcd_src)
	{
	    puts("Using clock SRC logic on unit under test.") ;
	    puts("Using clock LOOPED logic on test driver.\n") ;
	}
	else
	{
	    puts("Using clock LOOPED logic on unit under test.") ;
	    puts("Using clock SRC logic on test driver.\n") ;
	}


	/*
	 * Set direction register for funct and stat lines;
	 * not needed for CDa, but not harmful.
	 */
	if (edt_td)  edt_intfc_write_short(edt_td,  PCD_DIRREG, 0xccf0);
	if (edt_uut) edt_intfc_write_short(edt_uut, PCD_DIRREG, 0xccf0);


	if (edt_uut && edt_td)
	{
	    error += funct_test(edt_uut, edt_td);

	    error += stat_test(edt_uut, edt_td);


	    errs = 0;
	    dbgcnt = loopcount;
	    error += dma_read_test(edt_uut, edt_td, &errs);
	    /* exit if DMA did not complete */
	    if (errs != 0)
	    {
		error += errs;
		puts("TEST HALTED: can't continue if DMA does not complete\n");
		exit(4);
	    }


	    error += dma_write_test(edt_uut, edt_td);
	}


	/* if (edt_uut) */
	{
	    puts("EXTENDED DMA READ TEST:");
	    /*
	     * puts("hit return to start") ;
	     * fflush(stdout);
	     * getchar() ;
	     */
	    error += extended_dma_test(edt_td, edt_uut);
	}

	/* if (edt_td) */
	{
	    puts("EXTENDED DMA WRITE TEST:");
	    /*
	     * puts("hit return to start") ;
	     * fflush(stdout);
	     * getchar() ;
	     */
	    error += extended_dma_test(edt_uut, edt_td);
	}

	loopcount++;
	printf("\n\t%d Loops,\t\t%d Errors\n\n", loopcount, error);
    }

    puts("Closing the EDT boards");
    if (edt_td)  edt_close(edt_td);
    if (edt_uut) edt_close(edt_uut);

    return (0);
}



/*
 * Test	the function outputs ouf the unit under	test;
 * returns the number of errors encountered.
 */
int
funct_test(EdtDev * edt_uut, EdtDev * edt_td)
{
    int     i;
    unsigned char shdbe, rdbk, was;
    int     errcnt = 0;

    printf("TEST FUNCT SIGNALS OUT:\t\t\t");
    fflush(stdout);
    for (i = 0; i < 4; i++)
    {
	shdbe = 1 << i;
	pcd_set_funct(edt_uut, shdbe);
	rdbk = pcd_get_funct(edt_uut);

	if ((was = pcd_get_stat(edt_td)) != shdbe)
	{
	    printf("function bit test failed was %x should be %x readback %x\n",
		 was, shdbe, rdbk);
	    getchar();
	    errcnt++;
	}
    }
    for (i = 0; i < 4; i++)
    {
	shdbe = (~(1 << i)) & 0xf;
	pcd_set_funct(edt_uut, shdbe);
	rdbk = pcd_get_funct(edt_uut);

	if ((was = pcd_get_stat(edt_td)) != shdbe)
	{
	    printf("function bit test failed was %x should be %x readback %x\n",
		 was & 0xf, shdbe & 0xf, rdbk & 0xf);
	    errcnt++;
	}
    }
    if (errcnt == 0)
	printf("PASSED\n");
    else
	printf("FAILED\n");
    return (errcnt);
}



int     got_event[4];

void
got_stat1_event(void *p)
{
    got_event[0] = 1;
}

void
got_stat2_event(void *p)
{
    got_event[1] = 1;
}

void
got_stat3_event(void *p)
{
    got_event[2] = 1;
}

void
got_stat4_event(void *p)
{
    got_event[3] = 1;
}


/*
 * Test	the stat inputs	ouf the	unit under test;
 * returns the number of errors encountered.
 */
int
stat_test(EdtDev * edt_uut, EdtDev * edt_td)
{
    unsigned char rdbk;
    int     errcnt = 0;
    int     errcnt_save;
    u_char  polarity;
    u_char  shdbe;
    u_char  was;

    pcd_set_funct(edt_td, 0);
    rdbk = pcd_get_funct(edt_td);

    /*
     * set the stat line polarity register.
     */
    polarity = 0x00;
    pcd_set_stat_polarity(edt_uut, polarity);
    polarity = pcd_get_stat_polarity(edt_uut);


    /*
     * Tell driver to enable stat line interrupts and map the four stat lines
     * into events which will be sent to this process.
     */

    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT1, got_stat1_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT2, got_stat2_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT3, got_stat3_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT4, got_stat4_event, edt_uut,0);
    edt_msleep(100);

    printf("TEST STAT events IN:\t\t\t");
    fflush(stdout);
    shdbe = 1;
    for (i = 0; i < 4; i++)
    {
	got_event[i] = 0;

	pcd_set_funct(edt_td, shdbe);

	rdbk = pcd_get_funct(edt_td);

	edt_msleep(100);
	was = pcd_get_stat(edt_uut);
	if ((was = (was & PCDTEST_FUNCT_MSK)) != shdbe)
	{
	    printf
		(" stat bit test failed was %x should be %x readback %x again %x\n",
		 was, shdbe, rdbk, pcd_get_stat(edt_uut));
	    errcnt++;
	}
	edt_msleep(100);
#ifndef __hpux
	if (!got_event[i])
	{
	    printf("didn't get event for stat %d\n", i + 1);
	    errcnt++;
	}
#endif
	shdbe = (shdbe << 1) | 1;
    }

    if (errcnt == 0)
	printf("PASSED\n");
    else
	printf("FAILED\n");

    printf("TEST STAT events IN (inverse polarity):\t");
    fflush(stdout);

    errcnt_save = errcnt;

    pcd_set_funct(edt_td, 0x0f);
    rdbk = pcd_get_funct(edt_td);

    /*
     * set the stat line polarity register.
     */
    polarity = 0x0f;
    pcd_set_stat_polarity(edt_uut, polarity);
    polarity = pcd_get_stat_polarity(edt_uut);

    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT1, got_stat1_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT2, got_stat2_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT3, got_stat3_event, edt_uut,0);
    edt_set_event_func(edt_uut,EDT_EVENT_PCD_STAT4, got_stat4_event, edt_uut,0);
    edt_msleep(100);

    shdbe = 0xf;
    for (i = 0; i < 4; i++)
    {
	got_event[i] = 0;
	shdbe = (shdbe << 1) & 0x0f;

	pcd_set_funct(edt_td, shdbe);

	rdbk = pcd_get_funct(edt_td);

	edt_msleep(100) ;
	was = pcd_get_stat(edt_uut);
	if ((was = (was & PCDTEST_FUNCT_MSK)) != shdbe)
	{
	    printf(
	     " stat bit test failed was %x should be %x readback %x again %x\n",
	     was, shdbe, rdbk, pcd_get_stat(edt_uut));
	    errcnt++;
	}
	edt_msleep(100);
	if (!got_event[i])
	{
	    printf("didn't get event for stat %d\n", i + 1);
	    errcnt++;
	}
	fflush(stdout);
    }


    if (errcnt - errcnt_save == 0)
	printf("PASSED\n");
    else
	printf("FAILED\n");

    return (errcnt);
}


int     err_printed;
int     resync_printed;

/*
 * compare the two unsigned shorts passed count	the bits that are different
 * return 1 if the words miscompare, 0 if the same. print the first 16 bad
 * compares
 */
int
compare_short(index, shdbe, was, cnts)
    int     index;
    unsigned short was, shdbe;
    int    *cnts;		/* array of 16 ints */
{
    unsigned short err_msk;
    int     bit;

    if ((err_msk = was ^ shdbe) != 0)
    {
	if (err_printed < 8)
	{
	    printf("index %d (byte 0x%x) was %x shouldbe %x	mask %x\n",
		   index, index * 2, was, shdbe, err_msk);
	    err_printed++;
	}
	for (bit = 0; bit != 16; bit++)
	{
	    if (err_msk & 0x1)
	    {
		cnts[bit]++;
	    }
	    err_msk = err_msk >> 1;
	}
	return (1);
    }
    else
    {
	return (0);
    }
}

/*
 * check data in a buffer against the test driver generator algorithm count
 * the number of errors in each bit and print. return the	total words
 * incorrect.
 */


int
good_data(unsigned short *buf, int size)
{
    int     bit_err[16];
    int     bit, i, errcnt;
    unsigned short shdbe;

    for (bit = 0; bit != 16; bit++)
	bit_err[bit] = 0;

    /* check the data */
    errcnt = 0;
    err_printed = 0;
    resync_printed = 0;
    shdbe = 0;			/* first id the same as start */
    for (i = 0; i != size; i++)
    {
	errcnt += compare_short(i, shdbe, buf[i], bit_err);
	/* TODO - should we sync up if just missing or extra word? */
	if (try_resync && (buf[i] == shdbe - 1
			   || buf[i] == shdbe - 2
			   || buf[i] == shdbe + 1 || buf[i] == shdbe + 2))
	{
	    int     j;
	    int     k;
	    int     k_tmp;

	    if (!resync_printed)
	    {
		k = i - 4;
		k_tmp = shdbe + 4;
		if (k < 0)
		{
		    k = 0;
		    k_tmp = 0;
		}
		printf("WARNING - trying to resync data compare\n");
		printf("shdbe at %08x:\n", i * 2);
		for (j = 0; j < 8; j++)
		{
		    printf("%04x ", k_tmp - j);
		}
		printf("\n");
		printf("was:\n");
		for (j = 0; j < 8; j++)
		{
		    printf("%04x ", buf[k + j]);
		}
		printf("\n");
	    }

	    shdbe = buf[i];
	}
	shdbe--;
    }
    if (errcnt && dbgsave)
    {
	FILE   *tmpf;
	char    tmpname[20];

	sprintf(tmpname, "pcderrs.%d", dbgcnt);
	printf("debug - saving bad data in %s\n", tmpname);
	tmpf = fopen(tmpname, "wb");
	if (tmpf)
	{
	    fwrite(buf, size * 2, 1, tmpf);
	    fclose(tmpf);
	}
    }
    memset(buf, 0, size);

    /* if non zero errors print error array */
    if (errcnt != 0)
    {
	for (bit = 0; bit != 16; bit++)
	{
	    printf("%d - %d, ", bit, bit_err[bit]);
	}
	printf("\n");

#ifdef PRINTERR
	ibuf = (unsigned int *) buf;
	for (i = 0; i != 20; i++)
	{
	    for (bit = 0; bit != 16; bit++)
	    {
		printf("%04x ", buf[(i * 16) + bit]);
	    }
	    printf("\n");
	    for (bit = 0; bit != 8; bit++)
	    {
		printf("%08x  ", ibuf[(i * 8) + bit]);
	    }
	    printf("\n");
	}
#endif
    }

    return (errcnt);
}

/* return true if little_endian */
int
little_endian()
{
    u_short test;
    u_char *byte_p;

    byte_p = (u_char *) & test;
    *byte_p++ = 0x11;
    *byte_p = 0x22;
    if (test == 0x1122)
	return (0);
    else
	return (1);
}

/*
 * dma_read_test sets up a dma read transfer of	65k words, test	if it waits
 * until the data is enabled, starts the data, tests if	it completes, and
 * then checks the data. returns the	number of errors.
 */
int
dma_read_test(EdtDev * edt_uut, EdtDev * edt_td, int *haltflag)
{
    int     i, b ;
    int     errcnt = 0;
    int     errs;
    unsigned short *buf;	/* pointer to the input buffer */
    unsigned short **obufs;	/* pointer to the output ring buffers */
    unsigned short **ibufs;	/* pointer to the input ring buffers */
    unsigned short counter ;

    /* reset fifo */
    edt_flush_fifo(edt_td);
    edt_flush_fifo(edt_uut);

    pcd_set_byteswap(edt_td, 0);
    pcd_set_byteswap(edt_uut, 0);

    /* Output test buffers */
    if (edt_configure_ring_buffers(edt_td, DATABUFSIZE * 2, 2, EDT_WRITE, NULL)
	== -1)
    {
	perror("edt_configure_ring_buffers");
	exit(1);
    }
    obufs = (unsigned short **) edt_buffer_addresses(edt_td) ;
    for (b = 0; b < 2; b++)
    {
	counter = 0 ;

	for (i = 0; i < DATABUFSIZE; i++)
	    obufs[b][i] = counter-- ;
    }

    	
    /* Input buffers */
    if (edt_configure_ring_buffers(edt_uut, DATABUFSIZE * 2, 2, EDT_READ, NULL)
	== -1)
    {
	perror("edt_configure_ring_buffers");
	exit(1);
    }

    ibufs = (unsigned short **) edt_buffer_addresses(edt_uut) ;
    for (b = 0; b < 2; b++)
    	memset(ibufs[b], 0, DATABUFSIZE * 2) ;

    fputs("DMA READ TEST; DATA DISABLED:\t\t", stdout);
    fflush(stdout);

    edt_flush_fifo(edt_uut) ;
    edt_flush_fifo(edt_td) ;

    /* start the read */
    edt_start_buffers(edt_uut, 1);
    edt_msleep(100);


    if (edt_check_for_buffers(edt_uut, 1))
    {
	puts("FAILED:  read completed while test driver data disabled");
	errcnt++;
    }
    else
    {
	puts("PASSED");
    }

    /* start data in tester */
    edt_start_buffers(edt_td, 2) ;
    fputs("DMA READ TEST; DATA ENABLED:\t\t", stdout);
    fflush(stdout);
    edt_msleep(100) ;
    if (buf = (unsigned short *) edt_check_for_buffers(edt_uut, 1))
    {
	puts("PASSED");
	fputs("DMA READ TEST: check data\t\t", stdout);
	fflush(stdout);
	if ((errs = good_data(buf, DATABUFSIZE)) == 0)
	{
	    puts("PASSED");
	}
	else
	{
	    printf("FAILED: %d words of	%d incorrect\n", errs, DATABUFSIZE);
	    errcnt += errs;
	}
    }
    else
    {
	printf("FAILED:	no data	received after 100 msec\n");
	*haltflag = 1;
	errcnt++;
    }
    edt_disable_ring_buffers(edt_uut);
    edt_disable_ring_buffers(edt_td);

    return (errcnt);
}


/*
 * dma_write_test sets up a dma	write transfer of 65k words, test if it	waits
 * until the data is enabled, starts the data, tests if	it completes, and
 * then checks the data. returns the	number of errors.
 */
int
dma_write_test(EdtDev * edt_uut, EdtDev * edt_td)
{
    int     errcnt = 0;
    int     i, b;
    unsigned short errs, init;
    unsigned short *buf;	/* pointer to the buffer */
    unsigned short **ibufs;	/* pointers to the input test buffers */

    /* set swap byte */
    pcd_set_byteswap(edt_td, 0);
    pcd_set_byteswap(edt_uut, 0);


    /* Configure test input ring buffer */
    if (edt_configure_ring_buffers
	(edt_td, DATABUFSIZE * 2, 2, EDT_READ, NULL) == -1)
    {
	edt_perror("edt_configure_ring_buffers edt_td");
	exit(1);
    }

    ibufs = (unsigned short **) edt_buffer_addresses(edt_td) ;
    for (b = 0; b < 2; b++)
    	memset(ibufs[b], 0, DATABUFSIZE * 2) ;

    if (edt_configure_ring_buffers
	(edt_uut, DATABUFSIZE * 2, 2, EDT_WRITE, NULL) == -1)
    {
	edt_perror("edt_configure_ring_buffers edt_uut");
	exit(1);
    }
    buf = (unsigned short *) edt_next_writebuf(edt_uut);


    /*
     * initialize the buffer
     */
    init = 0;
    for (i = 0; i != DATABUFSIZE; i++)
    {
	buf[i] = init /* | 0x1 */ ;
	init--;
    }


    fputs("DMA WRITE TEST; DATA DISABLED:\t\t", stdout);
    fflush(stdout);

    edt_flush_fifo(edt_td) ;
    edt_flush_fifo(edt_uut) ;

    edt_start_buffers(edt_uut, 1);
    edt_msleep(100) ;
    if (edt_check_for_buffers(edt_uut, 1))
    {
	puts("FAILED: write completed with data disabled (DNR failed)");
	errcnt++;
    }
    else
    {
	puts("PASSED");
    }


    fputs("DMA WRITE TEST; DATA ENABLED:\t\t", stdout);
    fflush(stdout);

    /* start data from pcd */
    edt_start_buffers(edt_td, 2);

    edt_msleep(100);

    if (buf = (unsigned short *) edt_check_for_buffers(edt_uut, 1))
    {
	puts("PASSED");
    }
    else
    {
	puts("FAILED:  no data output after 2 seconds");
	errcnt++;
    }

    fputs("DMA WRITE TEST: check data\t\t", stdout);
    fflush(stdout);
    if ((errs = good_data(buf, DATABUFSIZE)) == 0)
    {
	puts("PASSED");
    }
    else
    {
	printf("FAILED:	bits %04x incorrect\n", errs);
	errcnt++;
    }

    edt_disable_ring_buffers(edt_uut);
    edt_disable_ring_buffers(edt_td);

    return (errcnt);
}


/*
 * extended_dma_test transfers 100MB of known data between edt_w and edt_r.
 */
int
extended_dma_test(EdtDev * edt_w, EdtDev * edt_r)
{

    int     i, counter = 0;
    int     loops = 100;
    u_short *inbuf, **bufs, *testbuf;
    int     bufsize = 1024 * 1024;
    u_int   offset;
    double  deltatime;

    testbuf = (u_short *) edt_alloc(bufsize * 2);
    for (i = 0; i < (bufsize / 2); i++)
    {
	testbuf[i] = i;
	testbuf[i + (bufsize / 2)] = i;
    }


    if (edt_r)
	edt_configure_ring_buffers(edt_r, bufsize, 4, EDT_READ, NULL);
    if (edt_w) 
	edt_configure_ring_buffers(edt_w, bufsize, 2, EDT_WRITE, NULL);

    if (edt_w)
    {
	bufs = (u_short **) edt_buffer_addresses(edt_w);
	for (i = 0; i < bufsize / 2; i++)
	{
	    bufs[0][i] = counter;
	    bufs[1][i] = counter++;
	}
    }


    if (edt_r) edt_flush_fifo(edt_r);
    if (edt_w) edt_flush_fifo(edt_w);
    if (edt_r) edt_start_buffers(edt_r, loops+10);
    if (edt_w) edt_start_buffers(edt_w, loops+10);
    if (edt_r) edt_wait_for_buffers(edt_r, 1) ;

    edt_dtime();

    for (i = 0; loops == 0 || i < loops; i++)
    {
	if (edt_r)
	{
	    inbuf = (u_short *) edt_wait_for_buffers(edt_r, 1) ;
	    offset = inbuf[0];

	    if (offset < (u_int)(bufsize / 2))
	    {
		if (memcmp(inbuf, &testbuf[offset], bufsize) == 0)
		{
		    putchar('.');
		    fflush(stdout);
		}
		else
		{
		    FILE   *fp;

		    if (edt_r) edt_abort_dma(edt_r);
		    if (edt_w) edt_abort_dma(edt_w);
		    fp = fopen("bad", "wb");
		    fwrite(inbuf, 1, bufsize, fp);
		    fclose(fp);
		    fp = fopen("good", "wb");
		    fwrite(&testbuf[offset], 1, bufsize, fp);
		    fclose(fp);
		    puts("output to files \"good\" and \"bad\"; data should match");
		    puts("\t\t\t\t\tFAILED") ;
		    fflush(stdout) ;
		    return 1 ;
		}
	    }
	}
	else
	{
	    edt_wait_for_buffers(edt_w, 1) ;
	    putchar('o') ;
	    fflush(stdout) ;
	}
    }
    deltatime = edt_dtime();


    if (edt_w && !edt_r)
    {
	puts("\n") ;
	edt_msleep(1000) ;
    }
    if (edt_w) edt_disable_ring_buffers(edt_w) ;
    if (edt_r) edt_disable_ring_buffers(edt_r) ;
    edt_free((u_char *) testbuf) ;

    if (edt_r)
    {
	printf("\n%d bytes in %lf seconds for %lf bytes/sec\n\n",
	   bufsize * i, deltatime, ((double) (bufsize * i)) / deltatime);

	puts("\t\t\t\t\tPASSED") ;
	fflush(stdout) ;
    }

    return 0 ;
}

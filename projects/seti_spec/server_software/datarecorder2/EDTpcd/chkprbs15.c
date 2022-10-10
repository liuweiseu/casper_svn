/* #pragma ident "@(#)chkprbs15.c	1.37 10/27/04 EDT" */

#include "edtinc.h"

EdtDev *edt_p[16];
int firstchan = -1;

int     loops = 8;
int     unit = 0;
int     numbufs = 16;
int     bufsize = 128 * 1024;
int     testbufsize = 32 * 32767;  /* Test up to 4MB data buffers */
u_short chan_enable_bits = 0;
u_short ssd16_chen;
int     numchans = 16;
int     enable_mask = 0;
int     nobitload = 0;
int     noinitpcd = 0;
int     code_invert = 0;
int     verbose = 0;
int     gotsync = 0;
int     multiple_channel_speeds = 0;
int	devid;
int	is_ssd16 = 0;
int     wc;
char   *cfg_file = "chk16.cfg";
char    cmd[128];
int next_index[16];

#ifdef _NT_
#define current_dir ".\\"
#else
#define current_dir "./"
#endif

void
usage()
{
    puts("usage: chkprbs15");
    puts("    -u <unit>     - specifies edt board input unit number");
    puts("    -D <seconds>  - specify a delay from config load to DMA start");
    puts("    -c <chan>     - specifies first channel when used with -n, or");
    puts("                   or multiple -c arguments enable selected channels");
    puts("    -n <nchan>    - specifies number of channels to test");
    puts("    -l <loops>    - minimum number of buffers to check per channel (0 = run forever)");
    puts("    -b <size>[k][m] - specifies size of input buffers in bytes:");
    puts("                      k following <size> multiplies by 1024;");
    puts("                      m following <size> multiplies by 1024*1024.");
    puts("    -o <outfile>  - save output to <outfile>");
    puts("    -R <file.cfg> - use a different configuration file other than chk16.cfg");
    puts("    -s            - stop on error");
    puts("    -N            - no bitload flag passed to initpcd");
    puts("    -v            - verbose");
    puts("    -d            - dump a few words of each buffer");
    puts("    -I            - skip reading configuration file - useful for running");
    puts("                    multiple instances of chkprbs15 on separate channel sets.");
    puts("    -i            - check for inverted prbs15 code");
    puts("    -m            - multiple channel speeds");
    puts("    -e <enable>   - A hex value to override the chan enable register");
}

/*
 * calculate the next 32 bit number for a prbs15 code input 15 bit number
 */

unsigned int 
calnext(unsigned int in, int invert)
{
    unsigned int out = 0;
    unsigned int mask = 0x1;
    int     a, b, loop;

    /*
     * printf("in with %08x invert=%d\n", in, invert);
     */
    for (loop = 0; loop < 32; loop++)
    {
	if (loop < 15)
	    a = in >> (loop) & 0x1;
	else
	    a = out >> (loop - 15) & 0x1;

	if (loop < 14)
	    b = in >> (loop + 1) & 0x1;
	else
	    b = out >> (loop - 14) & 0x1;

	if ((a ^ b) == invert)
	    out |= mask << loop;
	/*
	 * printf("-%d(%d)(%d)$%04x$%08x$",loop,a,b,mask,out);
	 */
    }
    /*
     * printf("\nout with %08x", out); getchar();
     */
    return (out);
}

/*
 * fill out a lookup table for checking a prbs buffer address _> next data ->
 * nextaddress ... etc
 */

/* memory for the 16 bit look up table */
unsigned int *testbuf = NULL;

/*
 * fill a buffer with prbs15 code lastval is the last 32 bit word of the
 * previous buffer returns the lastvalue in this buffer
 */
u_int 
fill_buf(ptr, length, lastval, invert)
    unsigned int *ptr;
    int     length;
    unsigned int lastval;
    unsigned int invert;
{
    unsigned int gennxt, tmp;
    int     count = 0;

    gennxt = lastval >> 17;

    while (count < length)
    {
	tmp = calnext(gennxt, invert);
	gennxt = tmp >> 17;
	*ptr = tmp;
	ptr++;
	count++;
    }
    return (tmp);
}

void
print_binary(FILE *fp, u_int word)
{
    int     bit;
    u_int   mask = 0x80000000;

    fprintf(fp, " ");
    for (bit = 0; bit < 32; bit++)
    {
	if (word & mask)
	    fprintf(fp, "1");
	else
	    fprintf(fp, "0");

	mask = mask >> 1;
    }
}

int 
chkbuf(unsigned int *ptr, int length, int next_i, int loop, int chan)
{
    unsigned int tmp;
    int i, test_index;
    int     error = 0;
    int     biterr = 0;
    int     errcnt;
    FILE   *badfp = NULL;
    static int badval_count = 0;
    static int badsync = 0;
    u_int   badval = 0xffffffff;

    if (code_invert)
	badval = ~badval;

    /* If the next index is -1 then synchronize the data with the test buffer */
    if (next_i == -1)
    {
	if (ptr[0] == badval)
	{
	    if (++badval_count >= 10)
	    {
		puts("Too many illegal values.  Aborting test.\n");
		exit(1);
	    }
	    printf("Illegal data value:  0x%x", badval);
	    return -1;
	}
	for (i = 0; i < testbufsize/4; i++)
	{
	    if (ptr[0] == testbuf[i])
	    {
		next_i = i;
		next_index[chan] = next_i;
		badsync = 0;
		gotsync = 1;
		return 0;
	    }
	}

	printf("    Bad sync chan %d.  Data: %08x %08x %08x %08x\n", chan,
				  ptr[0], ptr[1], ptr[2], ptr[3]);
	return -1;
    }

    badval_count = 0;

    if (memcmp(testbuf + next_i, ptr, length * 4))
    {
        unsigned int *p = &ptr[0];
        unsigned int *b = &testbuf[next_i];
	int count = 0;
    	for (i = 0; i < length; i++)
	{
	    if (*p != *b)
	    {
		printf("chan %d offset %d: input %x s/b %x\n", chan, i, *p, *b);
		if (++count > 10)
		    break;
	    }
	    p++;
	    b++;
	}
	biterr = 1;
    }
    return (biterr);
}



/******************************************************************************
 *                                                                            *
 * chkprbs15:
 *
 *                                                                            *
 ******************************************************************************/

int
main(int argc, char **argv)
{
    int     chan_loop[16]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int     chan_errors[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int     i, chan;
    u_int *buf;
    int     sec_delay = 0;
    int     dump_out = 0;
    unsigned int lastv;
    int     totbufs = 0;
    int     toterrs = 0;
    int     stoponerr = 0;
    FILE   *outfp = NULL;
    FILE   *fp = NULL;
    int curtime  = time(0);
    int lasttime = time(0);
    int done = 0;


    while   (argc > 1 && argv[1][0] == '-')
    {
	switch (argv[1][1])
	{
	    case 'u':
	    ++argv;
	    --argc;
	    unit = atoi(argv[1]);
	    break;

	    case 'R':
	    ++argv;
	    --argc;
	    cfg_file = argv[1];
	    break;

	    case 'e':
	    ++argv;
	    --argc;
	    enable_mask = strtol(argv[1], NULL, 16);
	    break;

	    case 'l':
	    ++argv;
	    --argc;
	    loops = atoi(argv[1]);
	    break;

	    case 'b':	/* Input buffer size in bytes */
	    {
	        int multiplier = 1;
		char *p;

		++argv;
		--argc;

		if ((p = strchr(argv[1], 'k')) || (p = strchr(argv[1], 'K')))
		{
		    *p = '\0';
		    multiplier = 1024;
		}
		else if ((p = strchr(argv[1],'m')) || (p = strchr(argv[1],'M')))
		{
		    *p = '\0';
		    multiplier = 1024 * 1024;
		}
		
		bufsize = strtoul(argv[1], NULL, 0);
		bufsize *= multiplier;
		break;
	    }

	    case 'c':
	    {
		static chan_args = 0;
		int chan;

		++argv;
		--argc;
		++chan_args;
		if (chan_args > 1)
		    numchans = 0;
		chan = atoi(argv[1]);
		if (chan < 0 || chan > 15)
		{
		    fprintf(stderr, "Illegal channel number: %d\n", chan);
		    exit(1);
		}
		chan_enable_bits |= 1 << chan;
		break;
	    }

	    case 'n':
	    ++argv;
	    --argc;
	    numchans = atoi(argv[1]);
	    break;

	    case 'D':
	    ++argv;
	    --argc;
	    sec_delay = atoi(argv[1]);
	    break;

	    case 'm':
	    multiple_channel_speeds = 1;
	    break;


	    case 'N':
	    nobitload = 1;
	    break;

	    case 's':
	    stoponerr = 1;
	    break;

	    case 'v':
	    verbose = 1;
	    break;

	    case 'd':
	    dump_out = 1;
	    break;

	    case 'i':
	    code_invert = 1;
	    break;

	    case 'I':
	    noinitpcd = 1;
	    break;

	    case 'o':
	    ++argv;
	    --argc;
	    if ((outfp = fopen(argv[1], "wb")) == NULL)
	    {
		perror(argv[1]);
		exit(1);
	    }
	    break;

	    default:
	    usage();
	    exit(1);
	}

	--argc;
	++argv;
    }

    /* Calculate test buffer size from buffer size */
    if (bufsize > testbufsize)
    {
	int tmp = testbufsize;

    	for (i = 2; bufsize > tmp; i++)
	    tmp = i * testbufsize;

	testbufsize = tmp;
    }

    if (chan_enable_bits == 0)
	chan_enable_bits = 0xffff;


    /* Enable numchans in chan_enable_bits */
    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	for (i = 1; i < numchans; i++)
	    chan_enable_bits |= (1 << (chan + i));
	break;
    }

    /* fill test buffer, initial lastval and endval should be the same */
    printf("bufsize %d testbufsize %d\n", bufsize, testbufsize);
    testbuf = (u_int *) edt_alloc(testbufsize * 2);
    lastv = fill_buf(testbuf, (testbufsize*2)/4, 1 << 17, code_invert);
    fp = fopen("testbuf", "wb");
    if (fp)
    {
	fwrite(testbuf, 1, testbufsize*2, fp);
	fclose(fp);
    }

    /* read config file */
    if (noinitpcd == 0)
    {
	sprintf(cmd, "%sinitpcd %s -u %d -f %s",
		current_dir,
		(nobitload) ? "-N" : "",
		unit, cfg_file);
	puts(cmd);
	if (edt_system(cmd) != 0)
	{
	    fprintf(stderr, "Cannot load config file \"%s\".  Exiting\n",
							       cfg_file);
	    exit(1);
	}
    }

    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	next_index[chan] = -1;

	if ((edt_p[chan] = edt_open_channel(EDT_INTERFACE, unit, chan)) == NULL)
	{
	    perror("edt_open_channel");
	    exit(1);
	}

#ifdef UCD
	if (chan == firstchan)
	{
	    edt_reg_write(edt_p[firstchan], SSD16_LSBL, 0xff);
	    printf("LSBL %x\n", edt_reg_read(edt_p[firstchan], SSD16_LSBL));
	    edt_reg_write(edt_p[firstchan], 0x01010001, 0x01);
	    printf("USB-2 enable:  %x\n", edt_reg_read(edt_p[firstchan],
							   0x01010001));
	    edt_reg_write(edt_p[firstchan], 0x01010010, 0x0f);
	    edt_reg_write(edt_p[firstchan], 0x01010100, 0x00);
	}
#endif

	if (firstchan == -1)
	    firstchan = chan;

	edt_set_firstflush(edt_p[chan], EDT_ACT_NEVER);
	edt_startdma_action(edt_p[chan], EDT_ACT_NEVER);

	if (edt_configure_ring_buffers(edt_p[chan], bufsize, numbufs,
				       EDT_READ, NULL) == -1)
	{
	    perror("edt_configure_ring_buffers");
	    exit(1);
	}
    }

    devid = edt_device_id(edt_p[firstchan]);
    if (devid == PSS16_ID || devid == PGS16_ID || devid == PCDA16_ID)
	is_ssd16 = 1;

    if (is_ssd16)
    {
	edt_reg_write(edt_p[firstchan], SSD16_CHEN, 0);
	edt_reg_write(edt_p[firstchan], SSD16_LSB, 0xffff);
    }

    if (sec_delay != 0)
        edt_msleep(sec_delay * 1000);

    edt_flush_fifo(edt_p[firstchan]);


    if (is_ssd16)
	edt_reg_write(edt_p[firstchan], PCD_CMD, 0);

    if (is_ssd16)
    {
	ssd16_chen = edt_reg_read(edt_p[firstchan],SSD16_CHEN);
	if (enable_mask)
	{
	    if (verbose)
		printf("\nChannel enable mask %x\n", enable_mask);
	    ssd16_chen |= enable_mask;
	}
	else
	    ssd16_chen |= chan_enable_bits;
	edt_reg_write(edt_p[firstchan], SSD16_CHEN, ssd16_chen);

	if (verbose)
	    printf("Channel enable register %x\n",
		edt_reg_read(edt_p[firstchan],SSD16_CHEN));
    }

    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
	edt_start_buffers(edt_p[chan], numbufs);

    if (is_ssd16)
	edt_reg_write(edt_p[firstchan], PCD_CMD, 8);


    while (!done)
    {
	/* Print errors and buffers every second */
	curtime = time(0);
	if (gotsync && curtime > lasttime)
	{
	    lasttime = curtime;
	    if (verbose)
	    {
		char separator = '(';

		fputs("errs: ", stdout);
		for (chan = 0; chan < 16; chan++)
		{
		    printf("%c%d", separator, chan_errors[chan]);
		    separator = ',';
		}
		puts(")");

		separator = '(';
		fputs("bufs: ", stdout);
		for (chan = 0; chan < 16; chan++)
		{
		    printf("%c%d", separator, chan_loop[chan]);
		    separator = ',';
		}
		puts(")\n");
	    }
	    else
	    {
		printf("Total errs=%d bufs=%d; Channel errs(",
					    toterrs, totbufs);
		for (i = 0; i < 16; ++i)
		{
		    if (chan_enable_bits & (1 << i))
		    {
			if (chan_errors[i])
			    putchar('Y');
			else
			    putchar('N');
		    }
		    else
			putchar('x');
		}
		fputs(") bufs(", stdout);
		for (i = 0; i < 16; ++i)
		{
		    if (chan_enable_bits & (1 << i))
		    {
			if (chan_loop[i])
			    putchar('Y');
			else
			    putchar('N');
		    }
		    else
			putchar('x');
		}
		fputs(")\r", stdout);
	    }

	    fflush(stdout);
	}

	for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
	{
	    static int init = 1;

	    if (init)
	    {
		int j;

		/* Throw away first buffer */
		for (j = 0; j < 16; j++) if (chan_enable_bits & (1 << j))
		{
		    edt_wait_for_buffers(edt_p[j], 1);
		    edt_start_buffers(edt_p[j], 1);
		}
		init = 0;
	    }

	    if (multiple_channel_speeds)
		buf = (u_int *)edt_check_for_buffers(edt_p[chan], 1);
	    else
		buf = (u_int *)edt_wait_for_buffers(edt_p[chan], 1);

	    if (buf)
	    {
		int errs ;

		errs = chkbuf(buf, bufsize / 4, next_index[chan],
				       chan_loop[chan], chan);

		chan_errors[chan] += errs;
		toterrs += errs;
		totbufs++;
		chan_loop[chan]++;

		wc = bufsize / 4;
		if (errs == -1)
		    next_index[chan] = -1;
		else
		    next_index[chan] = (next_index[chan] + wc)%(testbufsize/4);

		if (dump_out)
		{
		    u_short *sbuf = (u_short *) buf;

		    wc = bufsize / 2;
		    printf("raw data %04x %04x %04x %04x %04x %04x %04x %04x\n",
		        sbuf[0], sbuf[1], sbuf[2], sbuf[3],
		        sbuf[wc - 4], sbuf[wc - 3], sbuf[wc - 2], sbuf[wc - 1]);

		    wc = bufsize / 4;
		    printf("raw data %08x %08x %08x %08x %08x %08x %08x %08x\n",
		        buf[0], buf[1], buf[2], buf[3],
		        buf[wc - 4], buf[wc - 3], buf[wc - 2], buf[wc - 1]);

		    fflush(stdout);
		}



		if (outfp)
		{
		    *buf = ((unsigned) 0xdead << 16) | chan;
		    fwrite(buf, 4, bufsize / 4, outfp);
		}
		buf[0] = 0xdeadbeef;

		if ((stoponerr == 1) && (chan_errors[chan] != 0))
		    break;

		edt_start_buffers(edt_p[chan], 1);
	    }
	}


	if (loops != 0)
	{
	    for (chan = 0; chan < 16; chan++) if(chan_enable_bits & (1 << chan))
	    {
		if (chan_loop[chan] < loops)
		    break;
	    }
	    if (chan == 16)
		done = 1;
	}
	/* Can't do this because edt_msleep(1) waits for 18ms*/
	/* edt_msleep(1); */
    }
    
    puts("\nDone.");
    if (verbose)
    {
	char separator = '(';

	fputs("errs: ", stdout);
	for (chan = 0; chan < 16; chan++)
	{
	    printf("%c%d", separator, chan_errors[chan]);
	    separator = ',';
	}
	puts(")");

	separator = '(';
	fputs("bufs: ", stdout);
	for (chan = 0; chan < 16; chan++)
	{
	    printf("%c%d", separator, chan_loop[chan]);
	    separator = ',';
	}
    }
    else
    {
	printf("Total errs=%d bufs=%d; Channel errs(",
				    toterrs, totbufs);
	for (i = 0; i < 16; ++i)
	{
	    if (chan_enable_bits & (1 << i))
	    {
		if (chan_errors[i])
		    putchar('Y');
		else
		    putchar('N');
	    }
	    else
		putchar('x');
	}
	fputs(") bufs(", stdout);
	for (i = 0; i < 16; ++i)
	{
	    if (chan_enable_bits & (1 << i))
	    {
		if (chan_loop[i])
		    putchar('Y');
		else
		    putchar('N');
	    }
	    else
		putchar('x');
	}
    }
    puts(")\n");

    fflush(stdout);

    puts("Closing channels\n");
    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	edt_close(edt_p[chan]);
    }

    exit(0);
}

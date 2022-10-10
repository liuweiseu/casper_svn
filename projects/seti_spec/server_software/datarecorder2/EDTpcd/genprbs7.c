
#include "edtinc.h"

EdtDev *edt_p[16];
int firstchan = -1;

int     loops = 8;
int     unit = 0;
int     numbufs = 3;
int     bufsize = 32 * 1024 * 127;	/* fixed buffer size has to be at prbs7
				 * repeat interval
				 */
u_short chan_enable_bits = 0;
u_short ssd16_chen;
int     numchans = 16;
int     enable_mask = 0;
int     nobitload = 0;
int     noinitpcd = 0;
int     verbose = 0;
int     wc;
char   *cfg_file = "gen16.cfg";
char    cmd[128];

#ifdef _NT_
#define current_dir ".\\"
#else
#define current_dir "./"
#endif

void
usage()
{
    puts("usage: genprbs7");
    puts("    -u <unit>     - Specifies edt board input unit number");
    puts("    -c <chan>     - Specifies first channel when used with -n, or");
    puts("                    multiple -c arguments enable channel sets.");
    puts("    -n <nchan>    - Specifies number of channels to test.");
    puts("    -l <loops>    - Number of buffers to output (0 = run forever).");
    puts("    -R <file.cfg> - Use a different configuration file other than gen16.cfg.");
    puts("    -N            - No bitload flag passed to initpcd.");
    puts("    -v            - Verbose.");
    puts("    -I            - Skip reading configuration file - useful for running.");
    puts("                    multiple instances on separate channel sets.");
    puts("    -i            - Send inverted prbs7 code.");
    puts("    -e <enable>   - A hex value to override the chan enable register.");
}

/*
 * calculate the next 32 bit number for a prbs7 code input 7 bit number
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
	if (loop < 7)
	    a = in >> (loop) & 0x1;
	else
	    a = out >> (loop - 7) & 0x1;

	if (loop < 6)
	    b = in >> (loop + 1) & 0x1;
	else
	    b = out >> (loop - 6) & 0x1;

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
 * fill a buffer with prbs7 code lastval is the last 32 bit word of the
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

    gennxt = lastval >> 25;

    while (count < length)
    {
	tmp = calnext(gennxt, invert);
	gennxt = tmp >> 25;
	*ptr = tmp;
	ptr++;
	count++;
    }
    return (tmp);
}


/******************************************************************************
 *                                                                            *
 * genprbs7:
 *
 *                                                                            *
 ******************************************************************************/

int
main(int argc, char **argv)
{
    int     chan_loop[16]   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int     i, chan;
    u_int *buf;
    int     sec_delay = 0;
    int     code_invert = 0;
    unsigned int lastval[16];
    int     totbufs = 0;
    int curtime  = time(0);
    int lasttime = time(0);
    int done = 0;
    u_int **wbufadd = NULL;


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

	    case 'N':
	    nobitload = 1;
	    break;

	    case 'v':
	    verbose = 1;
	    break;

	    case 'i':
	    code_invert = 1;
	    break;

	    case 'I':
	    noinitpcd = 1;
	    break;

	    default:
	    usage();
	    exit(1);
	}

	--argc;
	++argv;
    }


    /* Enable numchans in chan_enable_bits */
    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	for (i = 1; i < numchans; i++)
	    chan_enable_bits |= (1 << (chan + i));
	break;
    }

    /* read config file */
    if (noinitpcd == 0)
    {
	sprintf(cmd, "%sinitpcd %s -u %d -f %s",
		current_dir,
		(nobitload) ? "-N" : "",
		unit, cfg_file);
	puts(cmd);
	system(cmd);
    }

    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	lastval[chan] = 0;

	if ((edt_p[chan] = edt_open_channel("pcd", unit, chan)) == NULL)
	{
	    perror("edt_open_channel");
	    exit(1);
	}

	if (firstchan == -1)
	    firstchan = chan;

	edt_set_firstflush(edt_p[chan], EDT_ACT_NEVER);
	edt_startdma_action(edt_p[chan], EDT_ACT_NEVER);

	if (edt_configure_ring_buffers(edt_p[chan], bufsize, numbufs,
					      EDT_WRITE, NULL) == -1)
	{
	    perror("edt_configure_ring_buffers");
	    exit(1);
	}


	/* get write buffer addresses */
	wbufadd = (u_int **) edt_buffer_addresses(edt_p[chan]);

	/* fill first buffer, initial lastval and endval should be the same */
	lastval[chan] = fill_buf(wbufadd[0], bufsize/4, (chan + 1) << 25,
							    code_invert);

	printf("fill channel %d initval %08x, firstval %08x, lastval %08x\n",
		       chan, (chan + 1) << 25, wbufadd[0][0], lastval[chan]);
	for(i=0; i< 128; i++)
	{
		printf("%08x ", wbufadd[0][i]);
		if ((i+1)%8 == 0) printf("\n");
	}
	printf("\n");

	if (numbufs > 1)
	{
	    /* copy the first buffer into the remaining */
	    for (i = 1; i < numbufs; i++)
		memcpy(wbufadd[i], wbufadd[0], bufsize);
	}
    }


    if (noinitpcd == 0)
    {
	edt_reg_write(edt_p[firstchan], SSD16_CHEN, 0);

	if (sec_delay != 0)
	    edt_msleep(sec_delay * 1000);

	edt_flush_fifo(edt_p[firstchan]);

	edt_reg_write(edt_p[firstchan], SSD16_LSB, 0xffff);
	edt_reg_write(edt_p[firstchan], PCD_CMD, 0);
    }
    else
    {
	for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
	    pcd_flush_channel(edt_p[chan], chan);
    }

    ssd16_chen = edt_reg_read(edt_p[firstchan],SSD16_CHEN);
    printf("\nChannel enable mask %x\n", enable_mask);
    if (enable_mask)
	ssd16_chen |= enable_mask;
    else
	ssd16_chen |= chan_enable_bits;
    edt_reg_write(edt_p[firstchan], SSD16_CHEN, ssd16_chen);

    printf("\nChannel enable register %x\n",
	    edt_reg_read(edt_p[firstchan],SSD16_CHEN));

    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
	edt_start_buffers(edt_p[chan], numbufs);

    if (noinitpcd == 0)
	edt_reg_write(edt_p[firstchan], PCD_CMD, 8);


    while (!done)
    {
	/* Print buffer count every second */
	curtime = time(0);
	if (curtime > lasttime)
	{
	    lasttime = curtime;
	    if (verbose)
	    {
		char separator = '(';

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
		printf("Total bufs=%d; Channel bufs(", totbufs);
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
	    if (buf = (u_int *)edt_check_for_buffers(edt_p[chan], 1))
	    {
		totbufs++;
		chan_loop[chan]++;

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
	edt_msleep(1);
    }

    printf("\nDone.\n\nTotal bufs=%d; Channel bufs(", totbufs);
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
    puts(")");

    puts("Closing channels\n");
    for (chan = 0; chan < 16; chan++) if (chan_enable_bits & (1 << chan))
    {
	edt_close(edt_p[chan]);
    }

    exit(0);
}

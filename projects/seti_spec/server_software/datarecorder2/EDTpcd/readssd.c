#include "edtinc.h"
#include <stdlib.h>

#define	SSD_MSB		0x04
#define	BYTE_SWAP	0x01

EdtDev *edt_p;
EdtDev *td_p;
int	fd;
int	ret;
int	count;
int	do_msb;
int	docount = 1;
int	bits = 4;
int	doflush = 0;
int	verbose = 0;
int	noload = 0;
u_char	option;
u_char	config;
u_char	command;
u_char	crcflag[4] = {0, 0, 0, 0};
u_short channels = 1;
u_short *buf_p;
u_short *saved_ptr;
u_short lastcrc[4];

void
usage()
{
    puts("usage: readssd [-d filename] [-m] [-u unit number] [-n num] size");
    puts("-d datafile specifies output file for data");
    puts("-c perform 16-bit count-up instead of crc");
    puts("-m sets most significant bit of word is first bit(default is least)");
    puts("-n sets number of bits per channel 1(default), 2, or 4");
    puts("-u sets scd device number - default is 0");
    puts("-t sets scd test device number - default is 1");
    puts("-s tester_speed where:");
    puts("\t0 - no clock divide");
    puts("\t1 - divide by 2");
    puts("\t2 - divide by 4");
    puts("\t3 - divide by 8 (the default)");
    puts("size is the size of read in words (16 bits)");
}


int
next_crc(crc)
    u_short crc;
{
    int     loop;
    u_short nxt;

    if (docount)
    {
	switch (crc)
	{
	case 0xfedc:
	    crc = 0x3210;
	    break;
	case 0xedcb:
	    crc = 0x210f;
	    break;
	case 0xdcba:
	    crc = 0x10fe;
	    break;
	case 0xcba9:
	    crc = 0x0fed;
	    break;
	case 0xba98:
	    crc = 0xfedc;
	    break;
	case 0xa987:
	    crc = 0xedcb;
	    break;
	case 0x9876:
	    crc = 0xdcba;
	    break;
	case 0x8765:
	    crc = 0xcba9;
	    break;
	case 0x7654:
	    crc = 0xba98;
	    break;
	case 0x6543:
	    crc = 0xa987;
	    break;
	case 0x5432:
	    crc = 0x9876;
	    break;
	case 0x4321:
	    crc = 0x8765;
	    break;
	case 0x3210:
	    crc = 0x7654;
	    break;
	case 0x210f:
	    crc = 0x6543;
	    break;
	case 0x10fe:
	    crc = 0x5432;
	    break;
	case 0x0fed:
	    crc = 0x4321;
	    break;

	default:
	    fprintf(stderr,
		    "next_crc: Unexpected value 0x%x.  Failed.\n", crc);
	    return (-1);
	    break;
	}
    }
    else
    {
	for (loop = 0; loop < 16; loop++)
	{
	    nxt = (crc & 0x1) ^ ((crc >> 15) & 0x1);
	    crc = (crc << 1) | nxt;
	    /*
	     * if (crc == 0) crc = 0x1 ; else crc = crc << 1 ;
	     */
	}
    }
    return (crc);
}

u_char	skip_values[4] = {0, 0, 0, 0};

int
getNextValue(bits, channel, channels, ptr, bufsize)
    int     bits;
    u_short channel;
    int     channels;
    u_short **ptr;
    u_short *bufsize;
{
    int     i;
    u_short value = 0;
    u_short tmp_val;
    u_short j;

    if (verbose)
	printf("getNext1: bits %d channel %d channels %d ptr %p bufsize %p\n",
	       bits, channel, channels, *ptr, bufsize);

    for (i = 0; i < bits; i++)
    {
	if ((*ptr) >= bufsize)
	    return -1;		/* overflow check */

	/* find next data word in this channel */
	if (channels > 1 || option != 0x08)
	{
	    while (*(*ptr) != channel)
	    {
		(*ptr) += 2;

		if ((*ptr) >= bufsize)
		    return -1;	/* overflow check */
	    }

	    /* found channel, now increment to it's dataword */
	    saved_ptr = (*ptr);
	    (*ptr)++;
	}
	else
	    saved_ptr = (*ptr);

	if ((*ptr) >= bufsize)
	    return -1;		/* overflow check */


	/*
	 * Now (*ptr) points to next dataword in this channel. Reverse bit
	 * order if necessary.
	 */
	if (verbose)
	    printf("before value 0x%x\n", *(*ptr));

	if (do_msb)
	    tmp_val = *(*ptr);
	else
	{
	    tmp_val = 0;
	    for (j = 1; j;)
	    {
		tmp_val |= ((*(*ptr) & j) ? 1 : 0);
		j <<= 1;
		if (j)
		    tmp_val <<= 1;
	    }
	}

	if (verbose)
	    printf("value 0x%x\n", tmp_val);


	if (docount)
	{
	    u_short retval;

	    if (verbose)
	    {
		printf("getNext2:  raw 0x%x possibly reversed 0x%x\n",
		       *(*ptr), tmp_val);
		printf("getNext3:  returning 0x%x\n", *(*ptr));
	    }
	    retval = *(*ptr);
	    /* increment to the next channel address */
	    (*ptr)++;
	    return retval;
	}
	if (verbose)
	    printf("bits pass %d: raw value %x reversed(?) value %x\n",
		   i, *(*ptr), tmp_val);

	(*ptr)++;

	switch (bits)
	{
	case 1:
	    value = tmp_val;

	    break;

	case 2:
	    /* special case on first 2 bits of first word */
	    if (i == 0)
	    {
		value |= ((tmp_val & 0x8000) ? 1 : 0);
		value <<= 1;
		value |= ((tmp_val & 0x4000) ? 1 : 0);
		value <<= 1;
		j = 0x1000;
	    }
	    else
	    {
		value <<= 1;
		j = 0x4000;
	    }

	    /* grab every other bit until the end */
	    while (j)
	    {
		value |= ((tmp_val & j) ? 1 : 0);

		j >>= 2;
		if (j)
		    value <<= 1;
	    }

	    break;

	case 4:
	    /* First, grab the first MS bits of the first word... */
	    if (i == 0)
	    {
		for (j = 0x8000; j != 0x800; j >>= 1)
		{
		    value |= ((tmp_val & j) ? 1 : 0);
		    value <<= 1;
		}
		j = 0x100;
	    }
	    else
	    {
		value <<= 1;
		j = 0x1000;
	    }

	    /* ...then grab the forth bit of every nibble */
	    for (; j;)
	    {
		value |= ((tmp_val & j) ? 1 : 0);

		j >>= 4;
		if (j)
		    value <<= 1;
	    }

	    break;
	}
	if (verbose)
	    printf("bits pass %d:\t\t\t\t\t\ttmp_val %x value %x\n",
		   i, tmp_val, value);
    }

    return (value);
}

int
main(int argc, char **argv)
{
    u_short channel;
    FILE   *outfile;
    char    filename[100];
    u_char  funct;
    u_char  speed = 3;
    int     unit = 0;
    int     test_unit = 1;
    char    syscmd[40];

    /*
     * set default options
     */
    do_msb = 0;
    outfile = 0;

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'm':
	    do_msb = 1;
	    break;

	case 'c':
	    docount = 1;
	    break;

	case 'f':
	    doflush = 1;
	    break;

	case 'v':
	    verbose = 1;
	    break;

	case 'N':
	    noload = 1;
	    break;

	case 'd':
	    ++argv;
	    --argc;
	    strcpy(filename, argv[0]);
	    if ((outfile = fopen(filename, "wb")) == NULL)
	    {
		perror(filename);
		exit(1);
	    }
	    break;

	case 'u':
	    ++argv;
	    --argc;
	    unit = atoi(argv[0]);
	    break;

	case 't':
	    ++argv;
	    --argc;
	    test_unit = atoi(argv[0]);
	    break;

	case 's':
	    ++argv;
	    --argc;
	    speed = atoi(argv[0]);
	    break;

	case 'n':
	    ++argv;
	    --argc;
	    bits = atoi(argv[0]);
	    if ((bits != 1) && (bits != 2) && (bits != 4))
	    {
		puts("illgal number of bits per channel - must be 1, 2 or 4");
		exit(1);
	    }
	    break;

	default:
	    usage();
	    exit(0);
	}
	argc--;
	argv++;
    }

    if (argc < 1)
    {
	usage();
	exit(0);
    }
    count = atoi(argv[0]);
    if (count < 8)
    {
	printf("must specify count\n");
	exit(1);
    }

    printf("reading %d words\n", count);

    if (outfile)
	printf("output to %s\n", filename);

    if (do_msb)
	puts("most significant bit is first bit in");
    else
	puts("least significant bit is first bit in");

    if (!noload)
    {
	sprintf(syscmd, "bitload  -u %d ssd4.bit", unit);
	system(syscmd);
	sprintf(syscmd, "bitload  -u %d ssdtest.bit", test_unit);
	system(syscmd);
    }

    edt_p = edt_open(EDT_INTERFACE, unit);
    if (edt_p == NULL)
    {
	sprintf(filename, "/dev/pcd%d", unit);
	perror(filename);
	exit(1);
    }

    td_p = edt_open(EDT_INTERFACE, test_unit);
    if (td_p == NULL)
    {
	sprintf(filename, "/dev/pcd%d", test_unit);
	perror(filename);
	exit(1);
    }
    edt_intfc_write(td_p, SSDT_FUNC, speed);
    edt_close(td_p);



    /* discover which rbt file is loaded:  ssd.rbt, ssd2.rbt, or ssd4.rbt */
    option = edt_intfc_read(edt_p, PCD_OPTION);
    if (option == 0x08)
    {
	puts("OPTION:  ssd.rbt, single channel 10 MHz maximum clock rate.\n");
	channels = 1;
    }
    else if (option == 0x09)
    {
	puts("OPTION:  ssd2.rbt, two channel 5 MHz maximum clock rate.\n");
	channels = 2;
    }
    else if (option == 0x0a)
    {
	puts("OPTION:  ssd4.rbt, four channel 1.25 MHz maximum clock rate.\n");
	channels = 4;
    }
    else if (option == 0x0d)
    {
	puts("OPTION:  fastssd.rbt, 1 channel 100 MHz maximum clock rate.\n");
	channels = 1;
    }
    else
    {
	printf("OPTION:  Illegal .rbt file; option number 0x%x.  Exiting.\n\n",
	       option);
	exit(1);
    }

    edt_intfc_write(edt_p, PCD_CONFIG, 0xff);

    /* set up number of bits per channel and bit order */
    funct = edt_intfc_read(edt_p, PCD_FUNCT);
    funct &= 0xf8;
    if (do_msb)
	funct |= SSD_MSB;

    switch (bits)
    {
    case 2:
	funct |= 0x1;
	break;

    case 4:
	funct |= 0x3;
	break;
    default:
	break;
    }

    edt_intfc_write(edt_p, PCD_FUNCT, funct);
    edt_intfc_write(edt_p, PCD_CONFIG, PCD_BYTESWAP);

    if (edt_little_endian())
	edt_intfc_write(edt_p, PCD_CONFIG, 0);
    else
	edt_intfc_write(edt_p, PCD_CONFIG, PCD_BYTESWAP);

    edt_configure_ring_buffers(edt_p, count * 2, 4, EDT_READ, NULL);

    edt_flush_fifo(edt_p);


    /* do a no-latency startup of dma.	Only need one buffer. */
    edt_start_buffers(edt_p, 4);
    /* use second buffer if flush */
    if (doflush)
	buf_p = (u_short *) edt_wait_for_buffers(edt_p, 2);
    else
	buf_p = (u_short *) edt_wait_for_buffers(edt_p, 1);

    if (outfile)
    {
	fwrite(buf_p, 1, (count * 2), outfile);
	printf("writing %d to %s\n", count * 2, filename);
	fclose(outfile);
    }
    if (buf_p[0] > 4)
    {
	printf("first word %x, incrementing\n", buf_p[0]);
	count--;
	buf_p++;
    }
    /* check the data */
    for (channel = 0; channel < channels; channel++)
    {
	u_short *ptr = buf_p;
	int	value;
	int	crc;
	int	value_ctr = 0;

	while ((value =
	  getNextValue(bits, channel, channels, &ptr, buf_p + count)) != -1)
	{
	    if (crcflag[channel])
	    {
		if ((crc = next_crc(lastcrc[channel])) == -1)
		{
		    continue;
		}
		if (verbose)
		    printf("Testing ch %d value 0x%x s/b 0x%x ptr %p offset %x\n",
			   channel + 1, value, crc, ptr, (u_int) ptr - (u_int) buf_p);
		if (value != crc)
		{
		    printf(
			   "ch[%d] data failed: address 0x%x was %04x s/b %04x offset %x\n",
			   channel + 1, saved_ptr - buf_p, value, crc, (u_int) ptr - (u_int) buf_p);
		}
		/*
		 * if we have a bad bit set to what we expected
		 */
		if (next_crc(value) == -1)
		    value = crc;
	    }
	    else
		crcflag[channel] = 1;

	    lastcrc[channel] = value;
	    value_ctr++;
	}

	printf("Channel %d:  %d values tested\n", channel + 1, value_ctr);
    }
    return 0;
}

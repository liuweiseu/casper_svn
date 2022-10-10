/* #pragma ident "@(#)rdwr16.c	1.54 03/25/04 EDT" */

#include "edtinc.h"
#include "chkbuf.h"
#include <stdlib.h>


#define WRITE 1
#define READ  1
#define NUMBUFS 4
#define MEM_16		1
#define MEM_12		2
#define MEM_16FREE	3
#define MEM_12FREE	4

FILE *markfp ;
/*
 * Data pattern types
 */
#define COUNT_8BIT	0
#define COUNT_16BIT	1
#define COUNT_32BIT	2
#define COUNT_64BIT	3
#define GPSSD4_TEST	4
#define COUNT_64NIB	5
#define ONOFF_8BIT	6
#define ONOFF_32BIT	7


EdtDev *edt_r[16] ;
#if WRITE
EdtDev *edt_w[16] ;
#endif
int loops 	 = 10 ;
int runit 	 = 0 ;
int wunit 	 = 1 ;
int numbufs  	 = NUMBUFS ;
int firstchan  	 = 0 ;
int numchans  	 = 16 ;
int no_initpcd 	 = 0 ;
int nobitload  	 = 0 ;
int nobitshift 	 = 0 ;
int verbose	 = 0 ;
int test_data	 = 0 ;
int bufsize  	 = 128*1024 ;
int wc, llc ;
int read_select  = 0 ;
int write_select = 0 ;
int select_count = 0 ;
int bitswaps = 0 ;
char *rd_cfg_file = "rd16.cfg" ;
char *wr_cfg_file = "wr16.cfg" ;
char *exp_rd_cfg_file = "rdslow.cfg" ;
char *exp_cfg_file = "wrslow.cfg" ;
char *exp_gen_file = "gen16.cfg" ;
char *mem_cfg_file = "mem16.cfg" ;
char *runit_file ;
char *wunit_file ;
u_char *writebufs[16][NUMBUFS] ;
u_int *testbuf    ;
u_int *testbuf2    ;
char cmd[128] ;

void func_byteswap(EdtDev *edt_p, int val) ;
void func_shortswap(EdtDev *edt_p, int val) ;
void func_msb_first(EdtDev *edt_p, int val) ;

void
usage()
{
    puts("usage: rdwr16") ;
    puts("    -r <runit>   - specifies edt board input unit number") ;
    puts("    -w <wunit>   - specifies edt board output unit number") ;
    puts("    -n <nchan>   - specifies number of channels to test") ;
    puts("    -o <outfile> - save output to <outfile>") ;
    puts("    -N	   - no bitload flag passed to initpcd") ;
    puts("    -v	   - verbose") ;
    puts("    -x	   - experimental out") ;
}

/******************************************************************************
 *                                                                            *
 * rdwr16:  open 16 channels on runit for input and 16 channels on wunit for  *
 *          output, then test buffer transfers over a loopback cable.         *
 *                                                                            *
 ******************************************************************************/

/******************************************************************************
 *
 * left_shift:  Shift a byte buffer left 1 to 7 bits.
 *
 ******************************************************************************/

u_char *cmpbufs[8] ;
u_char val ;

void
left_shift(u_char *from, u_char *to, int shift, int blen)
{
    u_short reg, tmp;
    int     i;

    /*
     * printf("cp_shift from %08x to %08x shift %d length %d\n", from, to,
     * shift, blen);
     */
    reg = (*from << 8);
    from++;

    for (i = 0; i < blen - 1; i++)
    {
	reg |= *from;
	from++;
	tmp = reg;
	*to = (u_char) ((tmp << shift) >> 8) & 0xff;
	to++;
	reg = reg << 8;
    }
}

/******************************************************************************
 *
 * setup_cmpbufs:  Create a set of 8 buffers filled with 8-bit counters and
 *		   shifted N bits to the left, where N is the buffer 0 - 7.
 *		   These are used to compare bit-shifted buffers coming in
 *		   over a serial channel.
 *
 ******************************************************************************/

void
setup_cmpbufs(bufsize)
{
    int i ;
    /*
     * get  buffers for temporay use
     */
    for (i = 0; i < 8; i++)
    {
	if ((cmpbufs[i] = (u_char *) edt_alloc(bufsize * 2)) == 0)
	{
	    printf("malloc failed for aligned array\n");
	    exit(1);
	}
    }
    /* build preshifted buffers */

    for (i = 0, val = 0; i < bufsize * 2; i++)
	cmpbufs[0][i] = val++ ;

    for (i = 1; i < 8; i++)
    {
	left_shift(cmpbufs[0], cmpbufs[i], i, bufsize * 2);
    }
}

#if 0
u_longlong_t *cmpbufs_64nib[64] ;

/******************************************************************************
 *
 * setup_cmpbufs_64nib:  Create a set of 64 buffers filled with 64-bit nibble
 *		         binary counters and shifted N bits to the left, where
 *			 N is the buffer 0 - 63.  These are used to compare
 *			 bit-shifted buffers coming in over a serial channel.
 *
 ******************************************************************************/

void
setup_cmpbufs_64nib(bufsize)
{
    int i ;

    /*
     * get  buffers for temporay use
     */
    for (i = 0; i < 64; i++)
    {
	if ((cmpbufs_64nib[i] = (u_longlong_t *) edt_alloc(bufsize * 2)) == 0)
	{
	    edt_perror("malloc failed for 64-bit aligned array");
	    exit(1);
	}
    }

    /* build preshifted buffers */

    for (i = 0, val = 0; i < bufsize * 2; i++)
	cmpbufs_64nib[0][i] = val++ ;

    for (i = 1; i < 8; i++)
    {
	cp_shift(cmpbufs[0], cmpbufs[i], i, bufsize * 2);
    }
}
#endif

/******************************************************************************
 *
 * get_cmpdata:  Determine and return which of the above shifted buffers
 *		 to use for comparison.
 *
 ******************************************************************************/
static u_char *savedata[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

u_char *
get_cmpdata(u_char * testdata, int chan)
{
    u_char *tmpp;
    u_char *testp;
    int     boff;
    int     cnt = 0 ;

    boff = bitoffset(testdata);
    /* printf("<%d> ",boff) ; */

    /* find first starting point in correct cmpbuf */
    if (!savedata[chan])
    {

	tmpp = cmpbufs[boff];
	testp = testdata;
	while (*tmpp != *testp)
	{
	    tmpp++;
	    if (cnt++ > bufsize)
		return(0) ;
	}

	savedata[chan] = tmpp;
    }
    else
	tmpp = savedata[chan];

    return (tmpp);
}


/******************************************************************************
 *
 * check_data:  compares an incrementing byte array with preshifted cmpbufs.
 *		See above.
 *
 ******************************************************************************/

int
check_4bit_count(u_char * testdata, int bufsize, int chan, int verbose)
{
    u_longlong_t *testp = (u_longlong_t *) testdata;
    u_longlong_t testval = *testp;
    u_int *tp = (u_int *) testdata ;
    int     i, j;
    int     foundcount = 0 ;
    int     errors = 0;

    /*
     * Check for all 16 nibble values.
     */
    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 16; j++)
        {
    	    if (((testval >> (j*4)) & 0xf) == i)
	    {
	    	++ foundcount ;
		break ;
	    }
        }
    }

    if (foundcount != 16)
    {
    	++ errors ;
	if (verbose)
#ifdef _NT_
	    printf("missing nibble values in buffer: %016I64x\n\n", testval);
#else
	    printf("missing nibble values in buffer: %016llx\n\n", testval);
#endif
    }
    else
    {
    	/*
	 * First longword should repeat throughout buffer.
	 */
	for (i = 0; i < bufsize/8; i++)
	{
	    if (*testp != testval)
	    {
	    	++ errors ;
		if (verbose)
#ifdef _NT_
		    printf("%016I64x s/b %016I64x\n\n",
#else
		    printf("%016llx s/b %016llx\n\n",
#endif
					   *testp, testval);
	    }
	    ++ testp ;
	}
    }

    return (errors);
}

/******************************************************************************
 *
 * check_data:  compares an incrementing byte array with preshifted cmpbufs.
 *		See above.
 *
 ******************************************************************************/

int
check_data(u_char * testdata, int bufsize, int chan, int verbose)
{
    u_char *tmpp;
    u_char *testp;
    int     j;
    int     errors = 0;

    testp = testdata;
    tmpp = get_cmpdata(testdata, chan);
/*
printf("%02x %02x %02x %02x %02x %02x\n",
	testp[0],
	testp[1],
	testp[2],
	testp[3],
	testp[4],
	testp[5]) ;
*/
    if (!tmpp)
    {
	printf("Can't get cmpbuf\n") ;
	return(1) ;
    }

#ifdef _NT_
    if (memcmp(testp, tmpp, bufsize))
#else
    if (bcmp(testp, tmpp, bufsize))
#endif
    {
	++ errors ;
	savedata[chan] = 0 ;

	if (verbose)
	{
	    for (j = 0; j < 10; j++)
	    {
		if (*tmpp != *testp)
		{
			printf("byte %d (0x%x) %02x s/b %02x\n", 
				j, j, *testp, *tmpp);
/*
		    if (errors++ > 10)
		    {
			puts("") ;
			break ;
		    }
*/
		}
		tmpp++;
		testp++;
	    }
	}
    }
    return (errors);
}


/******************************************************************************
 *
 * check_channel:  checks channel 0 for 0 , others for ff.
 *
 ******************************************************************************/

int
check_channel(u_char * testdata, int bufsize, int chan, int verbose)
{
    u_char *testp;
    int     errors = 0;
    int i ;
    u_char testval ;
    if (chan == 0) testval = 0x00 ; 
    else testval = 0xff ;
    testp = testdata ;

    for(i = 0 ; i < bufsize ; i++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    savedata[chan] = 0 ;

	    if (verbose)
	    {
		if (errors < 10)
		printf("byte %d (0x%x) %02x s/b %02x\n",i,i,*testp,testval) ;
	    }
	}
	testp++ ;
    }
    return (errors);
}
 

/******************************************************************************
 *
 * check_count:  check against incrementing 32 bit values.
 *
 ******************************************************************************/
int
check_count(u_int * testdata, int bufsize, int chan, int verbose)
{
    u_int *testp;
    u_int   cnt = bufsize / 4 - 1 ;
    int     errors = 0;
    u_int	    testval ;
    u_int	    firstval ;
    u_int     ix ;

    testp = testdata;
    firstval = *testdata ;
    testval = firstval ;
    for(ix = 0 ; ix < cnt ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10)
	    {
		if (verbose)
		    printf("word %d %08x s/b %08x\n",ix,*testp,testval) ;
	    }
	    else
		break ;
	}
	++testp ;
	++testval ;
    }
#if 0
    for(ix = 1 ; ix <= 32 ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (verbose && errors < 10) 
		printf("word %ld %08x s/b %08x\n",ix,*testp,testval) ;
	    /* testval = *testp ;*/
	}
	++testp ;
	++testval ;
    }
    testp = testdata + (cnt - 32);
    firstval = *testp ;
    testval = firstval ;
    for(ix = cnt - 32 ; ix < (long)cnt ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (verbose && errors < 10) 
	    printf("word %ld %08x s/b %08x\n",ix,*testp,testval) ;
	    /* testval = *testp ;*/
	}
	++testp ;
	++testval ;
    }
    if (verbose)
	printf("(%08x-%08x)",firstval,*testp) ;
#endif
    return (errors);
}

u_longlong_t
inc_nib64(u_longlong_t val)
{
    u_longlong_t nib ;

    for (nib = (u_longlong_t) 0x1; val & nib; nib <<= 4)
	val ^= nib ;

    return val ^= nib ;
}


/******************************************************************************
 *
 * check_count64:  check 64-bit one bit per nibble incrementing values
 *
 ******************************************************************************/
int
check_count64(u_longlong_t *testdata, int bufsize, int chan, int verbose)
{
    u_longlong_t *testp;
    u_int   cnt = bufsize / 8 - 1 ;
    int     errors = 0;
    u_longlong_t	    testval ;
    u_longlong_t	    firstval ;
    u_int     ix ;

    testdata += 4 ; 	/* skip first several values munged by cp_shift64() */
    testp = testdata;
    firstval = *testdata ;
    testval = firstval ;

    for(ix = 4 ; ix < cnt ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10) 
	    {
		/* if (verbose) */
#ifdef _NT_
		    printf("Fword64 %ld %016I64x s/b %016I64x\n",
					    ix, *testp, testval);
#else
		    printf("Fword64 %ld %016llx s/b %016llx\n",
					  ix, *testp, testval);
#endif
	    }
	    else
		break ;
	}
	++testp ;
	testval = inc_nib64(testval) ;
    }
    return (errors);
}


/******************************************************************************
 *
 * check_mem:  Check first and last 32 buffer elements for incrementing
 *	       12 or 16 bit values.  If memtype is MEM_12, the MS 4 bits
 *	       are the channel number and the data is 12 bits.  Otherwise
 *	       the data is 16 bits.
 *
 ******************************************************************************/

int
check_mem(u_short * testdata, int bufsize, int chan, int verbose, int memtype)
{
    u_short *testp;
    u_short   cnt = bufsize / 2 - 1 ;
    int     errors = 0;
    int	    testval ;
    int	    firstval ;
    int     ix ;

    testp = testdata;
    firstval = *testdata ;
    testval = firstval ;
    if (memtype == MEM_12)
    {
	if ((testval & 0xf000) != (chan << 12))
	{
	    ++ errors ;
	    printf("incorrect channel nibble %04x s/b %04x\n",
		testval & 0xf000, chan << 12) ;
	}
    }
    for(ix = 0 ; ix < 32 ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10) 
	    printf("word %d %04x s/b %04x\n",ix,*testp,testval) ;
testval = *testp ;
	}
	++testp ;
	++testval ;
    }
    testp = testdata + (cnt - 32);
    firstval = *testp ;
    testval = firstval ;
    if (memtype == MEM_12)
    {
	if ((testval & 0xf000) != (chan << 12))
	{
	    ++ errors ;
	    printf("incorrect channel nibble %04x s/b %04x\n",
		testval & 0xf000, chan << 12) ;
	}
    }
    for(ix = cnt - 32 ; ix < cnt ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10) 
	    printf("word %d %04x s/b %04x\n",ix,*testp,testval) ;
testval = *testp ;
	}
	++testp ;
	++testval ;
    }
    if (verbose)
	printf("(%08x-%08x)",firstval,*testp) ;
    return (errors);
}


/******************************************************************************
 *
 * check_chancnt:  Check first and last 32 buffer elements for incrementing
 *	           28 bit values.   The MS 4 bits are the channel number.
 *
 ******************************************************************************/

int
check_chancnt(u_int * testdata, int bufsize, u_int chan, int verbose)
{
    u_int *testp;
    u_int   cnt = bufsize / 4 - 1 ;
    u_int     errors = 0;
    u_int	    testval ;
    u_int	    firstval ;
    u_int     ix ;

    testp = testdata;
    firstval = *testdata ;
    testval = firstval ;
    if ((testval & 0xf0000000) != (chan << 28))
    {
	++ errors ;
	printf("incorrect channel nibble %08x s/b %08x\n",
	    testval, chan << 28) ;
    }
    for(ix = 0 ; ix < 32 ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10) 
	    printf("word %d 0x%x %08x s/b %08x\n",ix,*testp,testval) ;
testval = *testp ;
	}
	++testp ;
	++testval ;
    }
    testp = testdata + (cnt - 32);
    firstval = *testp ;
    testval = firstval ;
    if ((testval & 0xf0000000) != (chan << 28))
    {
	++ errors ;
	printf("incorrect channel nibble %08x s/b %08x\n",
	    testval, chan << 28) ;
    }
    for(ix = cnt - 32 ; ix < cnt ; ix++)
    {
	if (*testp != testval)
	{
	    ++ errors ;
	    if (errors < 10) 
	    printf("word %d 0x%x %08x s/b %08x\n",ix,*testp,testval) ;
testval = *testp ;
	}
	++testp ;
	++testval ;
    }
    if (verbose)
	printf("(%08x-%08x)",firstval,*testp) ;
    return (errors);
}


/******************************************************************************
 *
 * mem_stat:  This appears to take 12 or 16 bit incrementing buffers and
 *	      characterize the amount of data loss between successive values.
 *	      This would be used when the data in the onboard Xilinx
 *	      data generator is incrementing the data value on each clock
 *	      instead of on each read, and bus contention is causing
 *	      counter values to be skipped.
 *
 ******************************************************************************/
u_int timebins[1000] ;
void
mem_stat(u_short *testdata, int bufsize, unsigned chan, int verbose,int memtype)
{
    u_short *testp;
    u_int   cnt = bufsize / 2 - 1 ;
    double  total = 0.0 ;
    u_int   tmpval ;
    u_int   lastval ;
    u_int   min = 0xffff ;
    u_int   max = 0 ;
    u_int   delta ;
    long     ix ;

    testp = testdata;
    lastval = *testp++ ;
    if (memtype == MEM_12)
	lastval &= 0xfff ;
    for(ix = 0 ; ix < 1000 ; ix++)timebins[ix] = 0 ;
    for(ix = 0 ; ix < (long)cnt - 1 ; ix++)
    {
	tmpval = *testp++ ;
	if (memtype == MEM_12)
	{
	    if ((tmpval & 0xf000) != (chan << 12))
	    {
		printf("incorrect channel nibble %04x s/b %04x\n",
		    tmpval & 0xf000, chan << 12) ;
	    }
	    tmpval &= 0xfff ;
	}
	if (tmpval >= lastval)
	    delta = tmpval - lastval ;
	else
	{
	    if (memtype == MEM_12)
		delta = (tmpval + 0x1000) - lastval ;
	    else
		delta = (tmpval + 0x10000) - lastval ;
	
	}
	if (delta > max)
	    max = delta ;
	if (delta < min)
	    min = delta ;
	if (delta < 1000)
	    timebins[delta]++ ; 
	lastval = tmpval ;
	total += delta ;
    }
    for(ix = 0 ; ix < 1000 ; ix++)
    {
	if (timebins[ix])
	    printf("<%ld %d>",ix,timebins[ix]) ;
    }
    printf("(L%d H%d A%.2f)",
	min,max,total / (float)(cnt-1)) ;
}

int
main(argc, argv)
int argc;
char **argv;
{
    u_short *ptr16 ;
    u_int *ptr32 ;
    int i, chan, loop;
    int j ;
    int boff = 0 ;
    u_char *buf ;
    u_int *wbuf = NULL;
    u_int *savebuf = NULL;
    u_short *sbuf ;
    int exp_out = 0 ;
    int toss_first = 0 ;
    int exp_gen = 0 ;
    int check_cnt = 0 ;
    int dump_out = 0 ;
    int interactive = 1 ;
    int data_pattern_type = 0 ;
    int dma_out = 0 ;
    int mem_test = 0 ;
    int mem_type = 0 ;
    u_int wval ;
    FILE *outfp = NULL ;
    int     errors = 0;

#if 0
    u_longlong_t longtmp ;
    u_int inttmp
#endif

    runit_file = rd_cfg_file ;
    wunit_file = wr_cfg_file ;


    while (argc > 1 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
        case 'r':
            ++argv;
            --argc;
            runit = atoi(argv[1]);
	    read_select = 1 ;
	    select_count++ ;
            break ;

        case 'w':
            ++argv;
            --argc;
            wunit = atoi(argv[1]);
	    write_select = 1 ;
	    select_count++ ;
            break ;

        case 'R':
            ++argv;
            --argc;
            rd_cfg_file = argv[1] ;
	    runit_file = rd_cfg_file ;
	    printf("rd_cfg_file %s\n", rd_cfg_file) ;
            break ;

        case 'W':
            ++argv;
            --argc;
            wr_cfg_file = argv[1] ;
	    wunit_file = wr_cfg_file ;
	    printf("wr_cfg_file %s\n", wr_cfg_file) ;
            break ;

        case 'l':
            ++argv;
            --argc;
            loops = atoi(argv[1]);
            break ;

        case 'c':
            ++argv;
            --argc;
            firstchan = atoi(argv[1]);
            break ;


        case 'b':
            ++argv;
            --argc;
            bitswaps = atoi(argv[1]);
	    printf("\nbitswap %d\n\n", bitswaps) ;
            break ;

        case 'a':
            interactive = 0;
            break;


        case 'n':
            ++argv;
            --argc;
            numchans = atoi(argv[1]);
            break ;

        case 'Z':
            no_initpcd = 1 ;
            break ;

        case 'N':
            nobitload = 1 ;
            break ;

        case 'p':
            ++argv;
            --argc;
            data_pattern_type = atoi(argv[1]);
            break ;

        case 'S':
            nobitshift = 1 ;
            break ;

        case 'v':
            verbose = 1 ;
            break ;

        case 'x':
            exp_out = 1 ;
	    runit_file = exp_rd_cfg_file ;
	    wunit_file = exp_cfg_file ;
            break ;

        case 'g':
	    runit_file = exp_gen_file ;
            exp_gen = 1 ;
            break ;

        case 'X':
            check_cnt = 1 ;
            break ;

        case 'd':
            dump_out = 1 ;
            break ;

	case 'D':
	    dma_out = 1 ;
	    wunit_file = wr_cfg_file ;
	    break ;

	case 't':
	    toss_first = 1 ;
	    break ;

	case 'o':
            ++argv;
            --argc;
	    if ((outfp  = fopen(argv[1], "wb")) == NULL)
	    {
		perror(argv[1]) ;
		exit(1) ;
	    }
            break ;
        case 'm':
            ++argv;
            --argc;
            mem_test = 1 ;
	    mem_type = atoi(argv[1]) ;
	    switch(mem_type)
	    {
	     case MEM_12:
		puts(
	      "\nGenerating and testing 4-bit channel number + 12-bit counter");
		break;
	     case MEM_16:
		puts("\nGenerating and testing 16-bit counter");
		break;
	     case MEM_12FREE:
		puts(
      "\nGenerating and testing 4-bit channel number + 12-bit clocked counter");
		break;
	     case MEM_16FREE:
		puts("\nGenerating and testing 16-bit clocked counter");
		break;
	    }
	    runit_file = mem_cfg_file ;
	    toss_first = 0 ;
            break ;

        default:
            usage() ;
            exit(1) ;
        }
        --argc ;
        ++argv ;
    }
    if (data_pattern_type == GPSSD4_TEST)
    {
	runit_file = "rd4.cfg" ;
	wunit_file = "ssdtest.cfg" ;
        numchans = 4 ;
    }

    if (select_count == 0)
    {
	printf("must select -r, -w or both\n") ;
	exit(1) ;
    }

    if (write_select)
    {
	if (!dma_out && !exp_out && data_pattern_type != GPSSD4_TEST)
	{
		puts(
     "With -w must have -D (dma_out) or -x (exp_out) or -p 4 (GPSSD4_TEST)\n") ;
		exit(1) ;
	}
    }


    if (firstchan + numchans > 16)
	numchans = 16 - firstchan ;

    /* load the appropriate bitfile(s) */
    if (read_select && !no_initpcd)
    {
#ifdef _NT_
	sprintf(cmd, ".\\initpcd %s %s -u %d -f %s",
			     (verbose)   ? "-v" : "",
			     (nobitload) ? "-N" : "",
			        runit, runit_file) ;
#else
	sprintf(cmd, "./initpcd %s %s -u %d -f %s",
			     (verbose)   ? "-v" : "",
			     (nobitload) ? "-N" : "",
			        runit, runit_file) ;
#endif
	puts(cmd) ;
	if (edt_system(cmd) != 0)
	{
	    fprintf(stderr, "Unit %d: bitload or initpcd failed.  Exiting.\n", runit) ;
	    exit(1) ;
	}
    }
    if (write_select && !no_initpcd)
    {
#ifdef _NT_
	sprintf(cmd, ".\\initpcd %s %s -u %d -f %s",
			     (verbose)   ? "-v" : "",
			     (nobitload) ? "-N" : "",
			        wunit, wunit_file) ;
#else
	sprintf(cmd, "./initpcd %s %s -u %d -f %s",
			     (verbose)   ? "-v" : "",
			     (nobitload) ? "-N" : "",
			        wunit, wunit_file) ;
#endif
	puts(cmd) ;
	if (edt_system(cmd) != 0)
	{
	    fprintf(stderr, "Unit %d: bitload or initpcd failed. Exiting.\n", wunit) ;
	    exit(1) ;
	}
    }

    if (write_select && dma_out)
    {
	/*
	 * Allocate and initialize a set of write buffers with incrementing
	 * u_chars.  Use these buffers when configuring write ring buffers.
	 */
 /* writebufs = (u_char ***) edt_alloc(numchans * numbufs * sizeof(u_char*)) ;*/
	for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	{
	    for (i = 0; i < numbufs; i++)
	    {
		writebufs[chan][i] = edt_alloc(bufsize) ;
	    }
	}

	wval = 0 ;

       /***********************************************************************
        *
        * DMA output pattern selection:  use "-p N" to select one of the output
        * buffer patterns below.  Default N == 0.
        *
        ***********************************************************************/

	switch (data_pattern_type)
	{
         case COUNT_8BIT:
	    puts(
	    "\nDMA output data pattern COUNT_8BIT:  8-bit incrementing counter.\n");

	    puts("Constructing output buffers:") ;
	    for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	    {
		printf("%x ", chan) ;
		fflush(stdout) ;
		for (i = 0; i < numbufs * bufsize; i++)
		    writebufs[chan][i/bufsize][i % bufsize] = i & 0xff ; 
	    }
	    break ;

	 case COUNT_16BIT:
	    puts(
	  "\nDMA output data pattern COUNT_16BIT:  16-bit incrementing counter.\n");

	    puts("Constructing output buffers:") ;
            check_cnt = 1 ;
	    for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	    {
		for (i = 0; i < numbufs * bufsize; i += 2)
		{
		    ptr16 = (u_short *)(&writebufs[chan][i/bufsize][i%bufsize]);
		    *ptr16 = wval++ ;
		}
	    }
	    break ;


	 case COUNT_64BIT:
	 {
	     u_longlong_t longtmp ;

	    puts(
	  "\nDMA output data pattern COUNT_64BIT:  64-bit incrementing counter.\n");

	    puts("Constructing output buffers:") ;
	     for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	     {
		 wval = 0 ;

		 for (i = 0; i < numbufs * bufsize; i += 8)
		 {
		    u_longlong_t *ptr64 ;
		    longtmp = 0 ;

		    for (j = 0 ; j < 8 ; j++)
		    {
			if (wval & (1 << j))
			    longtmp |= ((u_longlong_t)1 << (j*8)) ;
		    }
		    ptr64 =
			 (u_longlong_t *)(&writebufs[chan][i/bufsize][i%bufsize]);
		    *ptr64 = longtmp ;
		    wval++ ;
		 }
	     }
	     break ;
	 }

	 case ONOFF_8BIT:
	    puts(
       "\nDMA output data pattern ONOFF_8BIT:  8-bit alternating 0x00 and 0xff.\n");

	    puts("Constructing output buffers:") ;
	    for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	    {
		for(j = 0 ; j < numbufs ; j++)
		{
		    for(i = 0 ; i < 4 ; i++)
			writebufs[chan][j][i] = 0xff ;

		    for(i = 0 ; i < 4 ; i++)
			writebufs[chan][0][i+4] = 0x00 ;
		}
	    }
	    break ;
	 case ONOFF_32BIT:
	    puts(
"\nDMA output data pattern ONOFF_32BIT:  32-bit alternating 0x0 and 0xffffffff.\n");

	    puts("Constructing output buffers:") ;
	    for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	    {
		for (i = 0; i < numbufs * bufsize; i += 4)
		{
		    ptr32 = (u_int *)(&writebufs[chan][i/bufsize][i%bufsize]) ;
		    *ptr32 = wval ;
			if (wval == 0) wval = 0xffffffff ;
			else wval = 0 ;
		}
	    }
	    break ;

	 case COUNT_64NIB:
	 {
	    u_longlong_t llval = 0 ;
	    u_longlong_t *ptr64 ;

	    puts(
"\nDMA output data pattern COUNT_64NIB:  64-bit counter with 1 bit per nibble\n");

	    puts("Constructing output buffers:") ;
            check_cnt = 1 ;
	    for(chan = firstchan ; chan < firstchan + numchans ; chan++)
	    {
		for (i = 0; i < numbufs * bufsize; i += 8)
		{
		    ptr64 = (u_longlong_t *)
				       (&writebufs[chan][i/bufsize][i%bufsize]);
		    *ptr64 = llval ;
		    llval = inc_nib64(llval) ;
		}
	    }
	    break ;
	 }
	}
    }

    wc = bufsize / 4 ;
    llc = bufsize / 8 ;
    if (read_select && data_pattern_type == COUNT_8BIT)
    {
	setup_cmpbufs(bufsize) ;
    }



    for (chan = firstchan; chan < firstchan + numchans; chan++)
    {
	if (read_select)
	{
	    if ((edt_r[chan] = edt_open_channel("pcd", runit, chan)) == NULL)
	    {
		perror("edt_open_channel") ;
		exit(1) ;
	    }

	    if (bitswaps)
	    {
		func_byteswap(edt_r[chan], bitswaps & 0x01) ;
		func_shortswap(edt_r[chan], bitswaps & 0x02) ;
		func_msb_first(edt_r[chan], bitswaps & 0x04) ;
	    }

	    if (edt_configure_ring_buffers(edt_r[chan], bufsize, numbufs,
					       EDT_READ, NULL) == -1)
	    {
		perror("edt_configure_ring_buffers") ;
		exit(1) ;
	    }

	}

	if (write_select && data_pattern_type != GPSSD4_TEST)
	{
	    if ((edt_w[chan] = edt_open_channel("pcd", wunit, chan)) == NULL)
	    {
		perror("edt_open_channel") ;
		exit(1) ;
	    }
	    if (bitswaps)
	    {
		func_byteswap(edt_w[chan], bitswaps & 0x08) ;
		func_shortswap(edt_w[chan], bitswaps & 0x10) ;
		func_msb_first(edt_w[chan], bitswaps & 0x20) ;
	    }

	    if (dma_out)
	    {
		if (verbose)
		{
		    u_int *buf ;

		    buf = (u_int *) writebufs[chan][0] ;
		    puts("\nOutput ring buffers initialized (before config ring):\n") ;
		    for(i = 0 ; i < 16 ; i++)
			printf("%08x ",buf[i]) ;
		    puts("") ;
		}
		if (edt_configure_ring_buffers(edt_w[chan], bufsize, numbufs,
					   EDT_WRITE, writebufs[chan]) == -1)
		{
		    perror("edt_configure_ring_buffers") ;
		    exit(1) ;
		}
		if (verbose)
		{
		    u_int **bufs ;

		    bufs = (u_int **) edt_buffer_addresses(edt_w[chan]) ;
		    puts("\nOutput ring buffers initialized:\n") ;
		    for(i = 0 ; i < 16 ; i++)
			printf("%08x ",bufs[0][i]) ;
		    puts("") ;
		}
	    }
	}
    }

    if (read_select)
    {
	testbuf = (u_int *)edt_alloc(bufsize*2) ;

	if (data_pattern_type == COUNT_64NIB)
	    testbuf2 = (u_int *)edt_alloc(bufsize*2) ;
    }


    if (write_select && data_pattern_type != GPSSD4_TEST)
    {
	if (exp_out)
	    edt_reg_write(edt_w[firstchan],PCD_CMD,8) ;
	else
	{
	    edt_reg_write(edt_w[firstchan],SSD16_CHEN,0) ;
	    if (interactive)
	    {
		printf("return to flush write") ;
		getchar();
	    }
	    edt_flush_fifo(edt_w[firstchan]) ;
	    for (chan = firstchan; chan < firstchan + numchans; chan++)
	    {
		edt_set_firstflush(edt_w[chan],EDT_ACT_NEVER) ;
		edt_startdma_action(edt_w[chan], EDT_ACT_NEVER) ;
	    }
	    edt_reg_write(edt_w[firstchan],PCD_CMD,0) ;
	    /* edt_reg_write(edt_w[firstchan],SSD16_CHEN,0xffff) ;*/
	}
    }


    if (read_select)
    {
	edt_reg_write(edt_r[firstchan],SSD16_CHEN,0) ;
	if (interactive)
	{
	    printf("return to flush read") ;
	    getchar();
	}
	edt_flush_fifo(edt_r[firstchan]) ;
	for (chan = firstchan; chan < firstchan + numchans; chan++)
	{
	    edt_set_firstflush(edt_r[chan],EDT_ACT_NEVER) ;
	    edt_startdma_action(edt_r[chan], EDT_ACT_NEVER) ;
	}
	    edt_reg_write(edt_r[firstchan],SSD16_LSB,0) ;
	edt_reg_write(edt_r[firstchan],PCD_CMD,0) ;
	edt_reg_write(edt_r[firstchan],SSD16_CHEN,0xffff) ;
    }

    if (mem_test)
    {
	/* for mem test setting CHEN enables channel */
	/* lsbfirst 0 - 16 bit counter, lsbfirst 1 - 12 bit counter 4bit chan */
	/* polarity - 0 - increment counter on read - 1 - increment on clock */
	switch (mem_type)
	{
	case MEM_16:
	    edt_reg_write(edt_r[firstchan],SSD16_LSB,0) ;
	    edt_reg_write(edt_r[firstchan],SSD16_CHEDGE,0) ;
	    break ;
	case MEM_12:
	    edt_reg_write(edt_r[firstchan],SSD16_LSB,0xffff) ;
	    edt_reg_write(edt_r[firstchan],SSD16_CHEDGE,0) ;
	    break ;
	case MEM_16FREE:
	    edt_reg_write(edt_r[firstchan],SSD16_LSB,0) ;
	    edt_reg_write(edt_r[firstchan],SSD16_CHEDGE,0xffff) ;
	    break ;
	case MEM_12FREE:
	    edt_reg_write(edt_r[firstchan],SSD16_LSB,0xffff) ;
	    edt_reg_write(edt_r[firstchan],SSD16_CHEDGE,0xffff) ;
	    break ;
	}
    }

    for (chan = firstchan; chan < firstchan + numchans; chan++)
    {

        if (write_select && dma_out)
	{
	     edt_start_buffers(edt_w[chan], loops + 100) ; 
	}
	if (read_select)
	{
	    edt_start_buffers(edt_r[chan], loops + 100) ; 
	}
    }
    if (read_select)
    {
	if (interactive)
	{
	    printf("return to enable read") ;
	    getchar();
	}
	edt_reg_write(edt_r[firstchan],PCD_CMD,8) ;
    }
    if (write_select)
    {
	if (interactive)
	{
	    printf("return to enable write") ;
	    getchar();
	}
	if (exp_out)
	{
	    edt_reg_write(edt_w[firstchan],PCD_CMD,4) ;
	    if (!read_select)
	    {
		printf("return to exit") ; getchar();
		edt_close(edt_w[firstchan]) ;
		exit(1) ;
	    }
	    puts("\nXilinx generated data output:  32-bit counter\n");
	}
        else if (data_pattern_type == GPSSD4_TEST)
	{
	    puts("\nGPSSD4_TEST:  4 bit counter generated by ssdtest.bit\n");
	}
	else
	{
	    u_int enb = 0 ;
	    edt_reg_write(edt_w[firstchan],PCD_CMD,8) ;
	    edt_reg_write(edt_w[firstchan],SSD16_CHEN,0xffff) ;
	}
    }

/*
markfp = fopen("/tmp/markout", "w") ;
*/

    /* Wait for things to settle into steady-state. Throw away 1st buffer. */
    /* edt_msleep(5000) ; */
    if (read_select && toss_first)
    {
u_char *buf ;
	puts("\nThrowing away first buffer from each channel...") ;
/*while(1)*/
{
	for (i = 0; i < 16; i++)
	{
	    int curchan = i % 16 ;

	    /* Only loop on selected channels */
	    if (curchan < firstchan || curchan >= firstchan + numchans)
		continue ;
	    buf = edt_wait_for_buffers(edt_r[curchan],1) ;

	}
}
    }


    if (interactive)
    {
	printf("return to continue") ;
	getchar();
    }
    puts("Go") ;
if (read_select || dma_out)
{
    for (i = 0, loop = 0; loops == 0 || loop < loops; i++)
    {
	int curchan = i % 16 ;


	/* Only loop on selected channels */
	if (curchan < firstchan || curchan >= firstchan + numchans)
	    continue ;


	if (read_select)
	{
	    int bufcheck = -1; /* is there a more appropriate value for this? */
	    int buffer_number ;


	    /* buf = edt_wait_for_buffers(edt_r[curchan],1) ; */
	    buf = edt_last_buffer(edt_r[curchan]) ;
	    if (curchan == (firstchan + numchans - 1))
		buffer_number = edt_done_count(edt_r[curchan]) ;

	    if (curchan == firstchan)
		printf("\n%d: ",loop) ;
	    printf(" %x-", curchan) ;

	    /*
	    if (curchan == 0)
		fwrite(buf, 1, bufsize, markfp) ;
	    */

	    /* 4-bit check */
	    if (data_pattern_type == GPSSD4_TEST)
	    {
		bufcheck = check_4bit_count(buf, bufsize, curchan, verbose) ;
	    }

	    /* 8-bit check */
	    if (data_pattern_type == COUNT_8BIT)
	    {
		bufcheck = check_data(buf, bufsize, curchan, verbose) ;
	    }

	    /* 32 bit check */
	    if (exp_out || (data_pattern_type == COUNT_32BIT))
	    {
		u_int *twbuf;

		boff = bitoffset32((unsigned int*)buf);

		if (boff != 0)
		{
		    cp_shift32((unsigned int*)buf, testbuf, boff, wc);
		    wbuf = (u_int *)testbuf ;
		}
		else wbuf = (u_int *)buf ;
		savebuf = wbuf ;
		if (check_cnt)
		    buf = (u_char *) wbuf ;

		if (verbose && curchan == firstchan)
		{
		    twbuf = (u_int *) buf ;

		    printf("\nboff %d before %x %x %x %x / after %x %x %x %x\n",
				boff, twbuf[0], twbuf[1], twbuf[2], twbuf[3],
				wbuf[0], wbuf[1], wbuf[2], wbuf[3]) ;
		}
		if(dump_out)
		    printf(
			"shft %02d: %08x %08x %08x %08x %08x %08x %08x %08x\n", 
			boff,
			wbuf[0],wbuf[1],wbuf[2],wbuf[3],
			wbuf[wc - 4],wbuf[wc - 3],wbuf[wc - 2], wbuf[wc - 1]) ;
	    }

	    /* 64 bit check; 1 bit per nibble */
	    if (data_pattern_type == COUNT_64NIB)
	    {
		u_longlong_t *llbuf, *lltbuf ;

		boff = bitoffset64((u_int *)buf);

		lltbuf = (u_longlong_t *) buf ;

		if (boff != 0)
		{
		    cp_shift64((u_longlong_t *)buf, (u_longlong_t *)testbuf,
							     boff, llc);
		    llbuf = (u_longlong_t *)testbuf ;
		}
		else
		    llbuf = (u_longlong_t *)buf ;

		if (verbose && curchan == firstchan)
		{
		    printf(
#ifdef _NT_
	   "\nboff %d\nbefore %016I64x %016I64x %016I64x %016I64x\nafter %016I64x %016I64x %016I64x %016I64x\n",
#else
	   "\nboff %d\nbefore %016llx %016llx %016llx %016llx\nafter %016llx %016llx %016llx %016llx\n",
#endif
			    boff,
			    lltbuf[0], lltbuf[1], lltbuf[2], lltbuf[3],
			    llbuf[0], llbuf[1], llbuf[2], llbuf[3]) ;
		}

		if (check_cnt)
		    buf = (u_char *) llbuf ;
	    }

	    if (check_cnt)
	    {
		if (exp_gen)
		    bufcheck = 
			check_chancnt((u_int *)buf, bufsize, curchan, verbose) ;
	    	else if (data_pattern_type == COUNT_64NIB)
		{
		    /*
		    if (curchan == firstchan)
		    {
			puts("Saving buffer to /tmp/markout") ;
			fwrite(buf, 1, bufsize, markfp) ;
		    }
		    */
		    bufcheck = check_count64((u_longlong_t *) buf, bufsize,
							curchan, verbose);
		}
		else
		    bufcheck = 
			check_count((u_int *)buf, bufsize, curchan, verbose) ;
	    }

	    if (mem_test)
	    {
		if (mem_type == MEM_12FREE || mem_type == MEM_16FREE)
		{
		    mem_stat((u_short *)buf,
			bufsize, curchan, verbose, mem_type) ;
		}
		else
		{
		    bufcheck = check_mem((u_short *)buf,
				bufsize, curchan, verbose, mem_type) ;
		}
	    }

	    if (dump_out)
	    {
		if (mem_test)
		{
		sbuf = (u_short *)buf ;
		printf("raw data %04x %04x %04x %04x %04x %04x %04x %04x\n", 
		    sbuf[0],sbuf[1],sbuf[2],sbuf[3],
		    sbuf[wc - 4],sbuf[wc - 3],sbuf[wc - 2], sbuf[wc - 1]) ;
		}
		else
		{
		wbuf = (u_int *)buf ;
		printf("raw data %08x %08x %08x %08x %08x %08x %08x %08x\n", 
		    wbuf[0],wbuf[1],wbuf[2],wbuf[3],
		    wbuf[wc - 4],wbuf[wc - 3],wbuf[wc - 2], wbuf[wc - 1]) ;
		}
	    }

	    if (!exp_out || check_cnt)
	    {
		if (bufcheck == 0)
		    fputs(".", stdout) ;  
		else
		{
		    fputs("X", stdout) ;  
		    ++ errors ;

		    if (verbose)
		    {
			wbuf = (u_int *)buf ;
			printf("\n%08x %08x %08x %08x %08x %08x %08x %08x\n", 
			wbuf[0],wbuf[1],wbuf[2],wbuf[3],
			wbuf[wc - 4],wbuf[wc - 3],wbuf[wc - 2], wbuf[wc - 1]) ;
		    }
		}
	    }

	    if (curchan == (firstchan + numchans - 1))
		printf("  dmabuf-%d", buffer_number) ;

	    fflush(stdout);

	    if (outfp)
	    {
		if (exp_out)
		{
		    ptr32 = (u_int *)buf ;
		    /*
		    *ptr32 = ((unsigned)0xdead << 16) | curchan ;
		    */
		    fwrite(buf, 4, bufsize/4, outfp) ;
		    printf("writing with shift %d at %x\n",boff,bufsize) ;
		    ptr32 = (u_int *)savebuf ;
		    /*
		    *ptr32 = ((unsigned)0xbeef << 16) | curchan ;
		    fwrite(ptr32, 4, bufsize/4, outfp) ;
		    */
		}
		else
		{
		    ptr32 = (u_int *)buf ;
		    *ptr32 = ((unsigned)0xdead << 16) | curchan ;
		    fwrite(buf, 1, bufsize, outfp) ;
		    if (!mem_test)
		    {
			if (dma_out)
			{
			boff = bitoffset(buf) ;
			printf("writing with shift %d\n",boff) ;
			cp_shift((unsigned char *)buf, (unsigned char*)testbuf, boff, bufsize/4);
			}
			else
			{
			boff = bitoffset32((unsigned int*)buf) ;
			printf("writing with shift %d\n",boff) ;
			cp_shift32((unsigned int*)buf, testbuf, boff, bufsize/4);
			}
		    ptr32 = (u_int *)testbuf ;
		    *ptr32 = ((unsigned)0xbeef << 16) | curchan ;

			fwrite(testbuf, 1, bufsize, outfp) ;
		    }
		}
	    }
	buf[0] = 0xde;
	buf[1] = 0xad;
	buf[2] = 0xbe;
	buf[3] = 0xef;
	}
	if (write_select && !read_select && data_pattern_type != GPSSD4_TEST)
	{
	    buf = edt_wait_for_buffers(edt_w[curchan], 1) ;
	    if (curchan == firstchan)
		printf("\n%d: ",loop) ;
	    if (curchan < 10)
		putchar('0' + curchan) ;
	    else
		putchar('a' + (curchan - 10)) ;
	    fflush(stdout) ;
	}
	if (curchan == firstchan+numchans-1)
	{
	    loop++ ;
	    if (loops != 0 && loop > loops)
		break ;
	}
    }
}
else
{
	printf("return to exit") ;
	getchar();
}

    puts("") ;

    puts("Closing channels") ;
    for (chan = firstchan; chan < firstchan + numchans; chan++)
    {
	if (read_select)
	{
	    edt_close(edt_r[chan]) ;
	}

	if (write_select && (data_pattern_type != GPSSD4_TEST))
	{
	    edt_close(edt_w[chan]) ;
	}
    }

    printf("\n%d errors\n\n", errors) ;
/*
fclose(markfp) ;
*/
    exit(0) ;
}


void
func_byteswap(EdtDev *edt_p, int val)
{
    u_char  config;

    config = edt_intfc_read(edt_p, PCD_CONFIG);

    if (val)
	config |= PCD_BYTESWAP;
    else
	config &= ~PCD_BYTESWAP;

    edt_intfc_write(edt_p, PCD_CONFIG, config);
}

void
func_shortswap(EdtDev *edt_p, int val)
{
    u_char  config;

    config = edt_intfc_read(edt_p, PCD_CONFIG);

    if (val)
	config |= PCD_SHORTSWAP;
    else
	config &= ~PCD_SHORTSWAP;

    edt_intfc_write(edt_p, PCD_CONFIG, config);
}


void
func_msb_first(EdtDev *edt_p, int val)
{
    u_char funct = pcd_get_funct(edt_p);

    if (val)
	funct |=  0x04 ;
    else
	funct &= ~0x04 ;

    pcd_set_funct(edt_p, funct);
}



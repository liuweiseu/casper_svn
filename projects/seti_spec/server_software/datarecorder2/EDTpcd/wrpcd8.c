#include "edtinc.h"
#include <stdlib.h>
#ifndef _NT_
#include <signal.h>
#endif

#include "edt_vco.h"
EdtDev *write_p ;

#define	BUF_SIZE	256*1024 
#define	READBUFS	8
#define SSD_REF_XTAL    10000000.0
/*
 * address for idle bytes, low byte is 0x18 high is 0x19
 */
#define SSD_LOW_IDLE	0x01010018
#define SSD_HI_IDLE	0x01010019


/*
 * dump 64 longs
 */
static
void
dumpblk(ptr, index, maxindex)
u_char *ptr;
int index;
int maxindex;
{
	int *p;
	int i;

	/* test index for limits */
	if (index < 0) index = 0;
	if (index + 256 > maxindex) index = maxindex - 256;
	index = index & 0xfffffffc ; /* make an even word boundary */

	p = (int *)(&ptr[index]);
	for(i=0; i<64; i++)
	{
		if ( i % 8 == 0) printf("\n%d -\t", index + (i*4));
		printf("%08x ", *p);
		p++;
	}
	printf("\n");
}

int
main(int argc, char **argv)
{
	int i, loop ;
	int channel = 1;
	unsigned char do_preload = 1;
	unsigned char *wbuf_p ;
	unsigned char *wtmpp ;
	unsigned char conf;
	unsigned char funct;
	unsigned char tmpc;
	unsigned char byteswap = 0;
	unsigned char shortswap = 0;
	unsigned char shiftr = 0;
	unsigned char testout = 0;
	unsigned char verbose = 0;
	int unit = 0 ;
	int bufsize = BUF_SIZE;
	int timeout = 5000 ;	/* default timeout 5 seconds */
	int bitload = 1 ; /* flag - load bit files if 1 (default) */

	int oneshot = 0;

		/* l and x are not implemented on this unit */
		/* {m,n,v,r,h,l,x} */
	edt_pll clkset = {10,60,1,4,1,1,1}; /* default 15MHz */
	int use_clkset = 0;
	double xtal = XTAL20;
	int override_xtal = 0;
	char cmd[80] ;
	double freq = 0;
	double vco = 0;

	int backio=0;
	double target_freq = 15000000.0; 
	
    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
		switch (argv[0][1])
		{

		case '1':
		  oneshot = !oneshot;
		  break;

		case 'P':
			do_preload = !do_preload;
			break;

		case 'u':
			++argv;
			--argc;
			unit = atoi(argv[0]);
			break;

		  case 't':
			++argv;
			--argc;
			timeout = atoi(argv[0]);
			break;

		  case 'c':

			++argv;
			--argc;
			channel = atoi(argv[0]);
			break;

		  case 'b':
			++argv;
			--argc;
			bufsize = atoi(argv[0]);
			break;

		  case 'o':
			testout = 1;
			break;

		  case 'r':
			shiftr = 1;
			break;

		  case 's':
			shortswap = 1;
			break;

		  case 'z':
			byteswap = 1;
			break;


		  case 'N':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.n = atoi(argv[0]);
			break;

		  case 'M':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.m = atoi(argv[0]);
			break;

		  case 'R':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.r = atoi(argv[0]);
			break;

		  case 'V':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.v = atoi(argv[0]);
			break;

		  case 'L':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.l = atoi(argv[0]);
			break;

		  case 'H':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.h = atoi(argv[0]);
			break;

		  case 'X':
			++argv;
			--argc;
			use_clkset = 1;
			clkset.x = atoi(argv[0]);
			break;
		  case 'B':
			bitload = 0;
			break;

		  case 'v':
			verbose = 1;
			break;
			/* pmc board enable p2 io */
		  case 'p':
		    backio = 1;
			break;
		  case 'F':
			++argv;
			--argc;
			target_freq = atof(argv[0]);
			break;

			case 'S':
			++argv;
			--argv;

			xtal = atof(argv[0]);
			override_xtal = 1;
			if (xtal < 100)
				xtal *= 1000000.0;
		break;

		  default:
			puts("Usage: wrpcd8 [-v[erbose]] [-u unit_no] [-t timeout] [-b buffer size] -B(don't load bitfile)\n") ;
			exit(0);
		}
		argc--;
		argv++;
    }

	if (bitload)
	{
		if (verbose)
		  printf("Loading bitfile pcd8_src.bit\n");
		sprintf(cmd,"./bitload -u %d pcd8_src.bit",unit) ;
		system(cmd) ;
	}

	if ((write_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
	{
		edt_perror(EDT_INTERFACE) ;
		puts("Hit return to exit") ;
		getchar() ;
		return(1) ;
	}

	if (!override_xtal)
	{
		switch(write_p->devid)
		{
			case PCD20_ID: xtal = XTAL20;
			case PCD_16_ID: xtal = XTAL20;
			break;
			case PCD40_ID: xtal = XTAL40;
			break;
			case PCD60_ID: xtal = XTAL60;
			break;

		}

	}
	/* set the direction bits and reset fifos */
	/* makes sure the buffers are enabled the correct way */
	/* TURN on the pll */	

	edt_set_funct_bit(write_p, EDT_FUNCT_PLLCLK);

	edt_reg_write(write_p, PCD_CMD, 0) ;

	if (backio) 
		edt_reg_or(write_p, PCD_STAT_POLARITY, PCD_EN_BACK_IO);
	else
		edt_reg_and(write_p, PCD_STAT_POLARITY, ~PCD_EN_BACK_IO);

	/* setting output clock */

	if (use_clkset)
	{
	if (verbose) printf("clock parameters - \n");
	if (verbose) printf("\tAV9110 refernce divide(-M) = %d\n", clkset.m);
	if (verbose) printf("\tAV9110 VCO feedback divide(-N) = %d\n", clkset.n);
	if (verbose) printf("\tAV9110 VCO feedback prescale divide(-V) = %d\n", clkset.v);
	if (verbose) printf("\tAV9110 VCO output  divide(-R) = %d\n", clkset.r);
    if (verbose) printf("\tXilinx low speed divide(-L)  = %d\n", clkset.l);
        if (verbose) printf("\tXilinx odd divide(-H) = %d\n", clkset.h);
        if (verbose) printf("\tXilinx tclk prescaler(-X) = %d\n", clkset.x);
        vco = (SSD_REF_XTAL*clkset.v*clkset.n)/(clkset.m);
        if (vco < 45e+06 || vco > 2.50e+08)
                if (verbose) printf("VCO FREQUENCY OUT OF RANGE at %g Hz\n", vco);
        if (verbose) printf("VCO Frequency at %g Hz\n", vco);
        freq = vco/(2*clkset.r * clkset.x * clkset.h * clkset.l) ;
        if (verbose) printf("setting write unit output clock %g Hertz\n", freq);
        edt_set_out_clk(write_p, &clkset);
	}    
	else
	{
		if (verbose) printf("target frequency: %g Hertz\n", target_freq);
		freq = edt_find_vco_frequency(write_p,target_freq, xtal, &clkset, verbose);
		/*if (verbose)*/ printf("setting write unit output clock %g Hertz\n", freq);
		edt_set_out_clk(write_p, &clkset);
	}

    /* set the data and clock low to select fifo clock as 2x with AV9170 */
	/*
	edt_clr_funct_bit(write_p, EDT_FUNCT_CLK);
	edt_clr_funct_bit(write_p, EDT_FUNCT_DATA);
	*/

	if (verbose) printf("setting timeout to %d ms\n",timeout) ;
	
	(void)edt_set_wtimeout(write_p, timeout) ;

	edt_set_autodir(write_p, 0) ;

	/* set byte swap  if required */
	if (verbose) printf("swap address %08x, BSWAP %02x, SSWAP %02x\n",
		PCD_CONFIG, PCD_BYTESWAP, PCD_SHORTSWAP);
	conf = edt_reg_read(write_p, PCD_CONFIG);
	if (verbose) printf("config register was %02x\n", conf);
	conf = conf & ~(PCD_BYTESWAP | PCD_SHORTSWAP);
	if ( byteswap )
		conf |= PCD_BYTESWAP;
	if ( shortswap )
		conf |= PCD_SHORTSWAP;
	edt_reg_write(write_p, PCD_CONFIG, conf) ;
	conf = edt_reg_read(write_p, PCD_CONFIG);
	if (verbose) printf("config register was %02x\n", conf);

	/*
	 *set funct byte if required ishift right is FUNCT[0], test out is
	 * FUNCT[2]
	 */
	funct = edt_reg_read(write_p, PCD_FUNCT);
	if (verbose) printf("funct register was %02x\n", funct);
	funct &= 0xf0;
	if ( shiftr )
		funct |= 0x01;
	if ( testout )
		funct |= 0x04;
	edt_reg_write(write_p, PCD_FUNCT, funct ) ;
	funct = edt_reg_read(write_p, PCD_FUNCT);
	if (verbose) printf("funct register was %02x\n", funct);
	

	/* set charachters for idle */
	edt_reg_write(write_p, SSD_LOW_IDLE, 0xff) ;
	edt_reg_write(write_p, SSD_HI_IDLE, 0xff) ;

	edt_flush_fifo(write_p) ;

	if (verbose) printf("after flush fifo return to start, ^C to stop\n") ; 
	/* getchar(); */


	if (bufsize < 128000)
	{
		if (verbose) printf("buffer size to small for efficent DMA - %d bytes\n", bufsize);
	}
	if (verbose) printf("buffer size = %d\n", bufsize);


	if (verbose) printf("configure 3 ring buffers to write from buf size %d\n",bufsize) ;
    if (edt_configure_ring_buffers(write_p, bufsize, 3, 
				EDT_WRITE, NULL) == -1)
    {
        perror("edt_configure_ring_buffers") ;
        exit(1) ;
    }



	tmpc = 0;
	/* initialize the write buffers to inc data */
	for(loop=0; loop< 3; loop++)
	{
		wbuf_p = edt_next_writebuf(write_p) ;
		wtmpp = wbuf_p;
		for(i=0; i< bufsize; i++)
		{
			*wtmpp = tmpc++; wtmpp++;
		}
		if (verbose) 
			dumpblk(wbuf_p, 0, bufsize);
	}

	if (oneshot)
	  {
		edt_start_buffers(write_p,1);
		edt_wait_for_buffers(write_p,1);
		putchar('.') ;
		fflush(stdout) ;
	  }
	else
	  {

		int nbufs = 0;
		edt_dtime();
		if (do_preload)
			edt_start_buffers(write_p, 3) ;
		else
			edt_start_buffers(write_p, 1);
		/* forever until the user control c */
		while (1)
		  {
			edt_wait_for_buffers(write_p, 1) ;
			edt_start_buffers(write_p, 1) ;
			putchar('.') ;
			if (nbufs++ % 16 == 0) {
				if (nbufs > 1)
				{
					double speed = edt_dtime() / 16.0;
					printf("\nSpeed %12f bytes/sec\n",
						1.0 / (speed / (bufsize)));
				}
				else
					edt_dtime();
			}

			fflush(stdout) ;
				 
		  }
	  }
	/* never get here */
	edt_close(write_p) ;
	exit(0) ;
	return(0) ;
}

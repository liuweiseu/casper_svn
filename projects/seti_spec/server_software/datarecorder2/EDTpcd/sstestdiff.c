
#include "edtinc.h"
usage()
{
    printf("Usage: sstestdiff\n") ;
    printf("-u unit		unit to test\n") ;
    printf("-i    		test PIO (need loopback)\n") ;
    printf("-s msleep	msecs to collect each channel\n") ;
    printf("-r 			run test even if board id is wrong\n") ;
    printf("-x 			do not set pll (must be set externally)\n") ;
}

#define ALOOP	1    
#define RLOOP	2    
#define DLOOP	0xc    
#define LOSSTAT	4
#define AISSTAT 0x13
#define LXT0	0xc0
#define LXT1	0xe0
#define BITCNT	0x2c
#define PIO	0x2c
#define COUNTS	0x30

#define E1CTL	0x2c
#define E1CNT	0x30

#define E3CTL	0x2d
#define E3CNT	0x40

#define ECLCTL	0x58
#define ECLCNT	0x50
#define ECLDIR	0x22	/* also pll debug select */

/* for CTLREGs */
#define TST_RESET	0x80 /* resets test pattern */
#define TST_ERRINS	0x40 /* inserts bit errors */
#define CNT_RESET	0x20 /* reset counters */
#define CNT_LATCH	0x10 /* latch counters */

u_char stat_sv[16][16] ;


edt_df_collect(EdtDev *edt_p, u_int mask, u_int msecs) 
{
    int i ;
    int j ;
    printf("collecting differential\n") ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    edt_intfc_write(edt_p, ECLCTL , CNT_RESET) ;
	    edt_intfc_write(edt_p, ECLCTL, i) ;
	    if (msecs)
		edt_msleep(msecs) ;
	    edt_intfc_write(edt_p, ECLCTL, CNT_LATCH+i) ;
	    for (j = 0 ; j < 16 ; j++)
	    {
		stat_sv[i][j] = edt_intfc_read(edt_p,ECLCNT+j) ;
	    }
	}
    }
}


edt_status(EdtDev *edt_p, u_int mask)
{
    int i ;
    u_int bits ;
    u_int errs ;
    u_int *tmpp ;
    int firststat = 1 ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    tmpp = (u_int *)&stat_sv[i][0] ;
	    printf("%d: ",i) ;
	    bits = *tmpp++ ;
	    printf("bits: %d ",bits) ;
	    errs = *tmpp++ ;
	    printf("errs: %d ",errs) ;
	    printf("\n") ;
	}
    }
    return(errs) ;
}


EdtDev *edt_p ;

u_int rd_id = 0 ;


piotest()
{
u_int rval = 0 ;
u_int rval1 = 0 ;
u_int wval = 0 ;
u_int i = 0 ;
u_int err = 0 ;

    /* set pio bit at 2c bit 1 */
    edt_intfc_write(edt_p, PIO, 0xff) ;

    /* start with 0 */
    edt_intfc_write(edt_p, 0x3a, 0) ;
    edt_intfc_write(edt_p, 0x3b, 0) ;

    for(i=0; i<8; i++)
    {
        wval = 1 << i ;
        edt_intfc_write(edt_p, 0x3a, wval) ;
        edt_msleep(10) ; 
        rval = edt_intfc_read(edt_p, 0x38) ;
        if (rval != wval) {
            printf(" wval = 00%02x, rval = %02x%02x\n", wval, rval1,rval) ;
	    printf("\nERROR!!! ERROR!!! ERROR!!! rval does not = wval\n") ; 
            err++; break ;
        }
        rval1 = edt_intfc_read(edt_p, 0x39) ;
        if (rval1 != 0) {
            printf(" wval = 00%02x, rval = %02x%02x\n", wval, rval1,rval) ;
	    printf("\nERROR!!! ERROR!!! ERROR!!! rval does not = wval\n") ; 
            err++; break ;
        }
        printf(" wval = 00%02x, rval = %02x%02x\n", wval, rval1,rval) ;

    }
    /* set 3a back to 0 */
    edt_intfc_write(edt_p, 0x3a, 0) ;

    for(i=0; i<8; i++)
    {
        wval = 1 << i ;
        edt_intfc_write(edt_p, 0x3b, wval) ;
        edt_msleep(10) ;
        rval = edt_intfc_read(edt_p, 0x38) ;
        if (rval != 0) {
            printf(" wval = %02x00, rval = %02x%02x\n", wval, rval1,rval) ;
	    printf("\nERROR!!! ERROR!!! ERROR!!! rval does not = wval\n") ; 
            err++; break ;
        }
        rval1 = edt_intfc_read(edt_p, 0x39) ;
        if (rval1 != wval) {
            printf(" wval = %02x00, rval = %02x%02x\n", wval, rval1,rval) ;
	    printf("\nERROR!!! ERROR!!! ERROR!!! rval does not = wval\n") ; 
            err++; break ;
        }
        printf(" wval = %02x00, rval = %02x%02x\n", wval, rval1,rval) ;
    }
    if ( err == 0)
        printf("\nTEST PASSED\n") ;
}

main(int argc,char **argv)
{

    int unit = 0 ;
    int pio = 0 ;
    int mscollect = 1000 ;
    u_int mask = 0xffff ;
    int runanyway = 0 ;
    int channel = 0 ;
    int extpll = 0; /* skip the setting of the pll if 1 */

    
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

	  case 's':
		++argv;
		--argc;
		mscollect = atoi(argv[0]);
		break;

	  case 'i':
		pio = 1 ;
		break;

	  case 'x':
		extpll = 1 ;
		break;

	  case 'r':
		runanyway = 1;
		break;
	  default:
		usage();
		exit(0);
	}
	argc--;
	argv++;
    }
    
    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
	char str[64] ;
	sprintf(str, "%s%d", EDT_INTERFACE, unit) ;
	perror(str) ;
	exit(1) ;
    }


    if (extpll == 0)
    {
	/* set pll #1 to 2.048 Mhz */
#ifdef _NT_

	system(".\\set_ss_vco 1 1 ") ;
#else
	system("./set_ss_vco 1 1 ") ;
#endif

    }

    rd_id = edt_intfc_read(edt_p, 0x7f) ;
    if(rd_id == 0x0)
    {
        printf ("\n You are testing a PCISS-RS422 board. Is this correct?\n") ;
        printf ("\n Hit return to continue\n") ;
        getchar() ;
    }
    else if (rd_id == 0x1)
    {
        printf ("\n You are testing a PCISS-LVDS board. Is this correct?\n") ;
        printf ("\n Hit return to continue\n") ;
        getchar() ;
    }
    else
    {
 	printf("Unknown board id (%02x) \n", rd_id);
  	if (runanyway == 0) exit(0);
    }

    if (pio) {
        piotest() ;
    } else {
        /* test ecl/differential */
        /* disable pio 2c bit 1 */
        edt_intfc_write(edt_p, PIO, 0x00) ;

	printf("testing all 16 channels\n") ;
	mask = 0xffff ; 
	edt_df_collect(edt_p, mask, mscollect) ;
	edt_status(edt_p, mask) ;
        printf(" Install loopback and hit return ") ; getchar() ;
        piotest() ;
    }
    return(0) ;
}


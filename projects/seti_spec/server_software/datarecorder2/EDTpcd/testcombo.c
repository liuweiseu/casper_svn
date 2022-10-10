
#include "edtinc.h"
usage()
{
    printf("Usage: testcombo\n") ;
    printf("-u unit		unit to test\n") ;
    printf("-p [0|1]	pass 0 or 1\n") ;
    printf("-s msleep	msecs to collect each channel\n") ;
}

#define ALOOP	1    
#define RLOOP	2    
#define DLOOP	0xc    
#define LOSSTAT	4
#define AISSTAT 0x13
#define LXT0	0xc0
#define LXT1	0xe0
#define BITCNT	0x2c
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

edt_lxt_aloop(EdtDev *edt_p, u_int mask) 
{
    u_char tmp ;
    u_char lxt0_bits ;
    u_char lxt1_bits ;
    lxt0_bits = mask & 0xff ;
    lxt1_bits = (mask >> 8) & 0xff ;
    tmp = edt_intfc_read(edt_p, LXT0+ALOOP) ;
    edt_intfc_write(edt_p, LXT0+ALOOP, tmp |= lxt0_bits) ;
    tmp = edt_intfc_read(edt_p, LXT1+ALOOP) ;
    edt_intfc_write(edt_p, LXT1+ALOOP, tmp |= lxt1_bits) ;
}
edt_lxt_noloop(EdtDev *edt_p, u_int mask) 
{
    u_char tmp ;
    u_char lxt0_bits ;
    u_char lxt1_bits ;
    lxt0_bits = mask & 0xff ;
    lxt1_bits = (mask >> 8) & 0xff ;
    tmp = edt_intfc_read(edt_p, LXT0+ALOOP) ;
    edt_intfc_write(edt_p, LXT0+ALOOP, tmp &= ~lxt0_bits) ;
    tmp = edt_intfc_read(edt_p, LXT0+DLOOP) ;
    edt_intfc_write(edt_p, LXT0+DLOOP, tmp &= ~lxt0_bits) ;
    tmp = edt_intfc_read(edt_p, LXT0+RLOOP) ;
    edt_intfc_write(edt_p, LXT0+RLOOP, tmp &= ~lxt0_bits) ;

    tmp = edt_intfc_read(edt_p, LXT1+ALOOP) ;
    edt_intfc_write(edt_p, LXT1+ALOOP, tmp &= ~lxt1_bits) ;
    tmp = edt_intfc_read(edt_p, LXT1+DLOOP) ;
    edt_intfc_write(edt_p, LXT1+DLOOP, tmp &= ~lxt1_bits) ;
    tmp = edt_intfc_read(edt_p, LXT1+RLOOP) ;
    edt_intfc_write(edt_p, LXT1+RLOOP, tmp &= ~lxt1_bits) ;
}

edt_e1_collect(EdtDev *edt_p, u_int mask, u_int msecs) 
{
    int i ;
    int j ;
    u_short xmt_enb ;
    u_short los ;
    u_int bits ;
    u_int errs ;
    u_int bpv ;
    u_int *tmpp ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    /* 3a lsb xmt enb, 3b msb xmt enb */
	    /* edt_intfc_write(edt_p, E1CTL , TST_RESET | CNT_RESET) ; */
	    /* enable only the one transmit */
	    if (i < 4)
	    {
		/* 4 transmits to 0 */
	    	xmt_enb = 1 << (i + 4) ;
	    }
	    else if (i < 8)
	    {
		/* 0 transmits to 4 */
	    	xmt_enb = 1 << (i - 4) ;
	    }
	    else if (i < 12)
	    {
		/* 12 transmits to 8 */
	    	xmt_enb = 1 << (i + 4) ;
	    }
	    else if (i < 16)
	    {
		/* 8 transmits to 12 */
	    	xmt_enb = 1 << (i - 4) ;
	    }
xmt_enb ^= 0xffff ;
edt_intfc_write(edt_p, 0x3a, xmt_enb & 0xff) ;
edt_intfc_write(edt_p, 0x3b, (xmt_enb >> 8) & 0xff) ;
	    edt_intfc_write(edt_p, E1CTL , CNT_RESET) ;
	    edt_intfc_write(edt_p, E1CTL, i) ;
		edt_msleep(msecs) ;
	    edt_intfc_write(edt_p, E1CTL, CNT_LATCH+i) ;
	    for (j = 0 ; j < 16 ; j++)
	    {
		stat_sv[i][j] = edt_intfc_read(edt_p,E1CNT+j) ;
	    }
tmpp = (u_int *)&stat_sv[i][0] ;
bits = *tmpp++ ;
errs = *tmpp++ ;
los = *tmpp++ ;
los &= 0xffff ;
bpv = *tmpp++ ;
printf("%d: now los %x bits %d err %d bpv %d\n",
	i,
	los, bits, errs,bpv) ;
	}
    }
}
edt_e3_collect(EdtDev *edt_p, u_int mask, u_int msecs) 
{
    int i ;
    int j ;
printf("collecting e3\n") ;
    for (i = 0 ; i < 4 ; i++)
    {
	if (mask & (1 << i))
	{
	    edt_intfc_write(edt_p, E3CTL , CNT_RESET) ;
	    edt_intfc_write(edt_p, E3CTL, i) ;
	    if (msecs)
		edt_msleep(msecs) ;
	    edt_intfc_write(edt_p, E3CTL, CNT_LATCH+i) ;
	    for (j = 0 ; j < 16 ; j++)
	    {
		stat_sv[i][j] = edt_intfc_read(edt_p,E3CNT+j) ;
	    }
	}
    }
}

edt_df_collect(EdtDev *edt_p, u_int mask, u_int msecs) 
{
    int i ;
    int j ;
printf("collecting differential\n") ;
    for (i = 0 ; i < 8 ; i++)
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
#if 0
	    if (firststat)
	    {
	        tmpp = (u_int *)&stat_sv[i][8] ;
		printf("los %08x\n",*tmpp) ;
		/* firststat = 0 ;*/
	    }
#endif
	    tmpp = (u_int *)&stat_sv[i][0] ;
	    printf("%d: ",i) ;
	    bits = *tmpp++ ;
	    printf("bits: %d ",bits) ;
	    errs = *tmpp++ ;
	    printf("errs: %d ",errs) ;
#if 0
	    los = *tmpp++ ;
	    printf("los: (%08x) ",los) ;
	    lcv = *tmpp++ ;
	    printf("lcv/bpv: %d ",lcv) ;
	    printf("err %6.3f",
		(float)errs / (float)bits * 1000000000.0) ;
#endif
	    printf("\n") ;
	}
    }
    return(errs) ;
}


main(int argc,char **argv)
{

    EdtDev *edt_p ;
    int unit = 0 ;
    int mssleep = 1000 ;
    int mscollect = 1000 ;
    u_int mask = 0xffff ;
    int passno = 0 ;
    int channel = 0 ;
    int t1e1 = 1;
    int t3e3 = 1;
    int diff = 1;
    
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

	  case '1':
		t1e1 = 0;
		break;

	  case '3':
		t3e3 = 0;
		break;

	  case 'd':
		diff = 0;
		break;

	  case 's':
		++argv;
		--argc;
		mscollect = atoi(argv[0]);
		break;

	  case 'S':
		++argv;
		--argc;
		mssleep = atoi(argv[0]);
		break;

	  case 'p':
		++argv;
		--argc;
		passno = atoi(argv[0]);
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

    /* set plls */
    /* E1/T1 2048 */
#ifdef _NT_

    system(".\\set_ss_vco 1 3 > null") ;
    /* E3/T3 34368 */
    system(".\\set_ss_vco 4 2 > null") ;
    /* ECL/Differential 8448 */
    system(".\\set_ss_vco 3 1 > null") ;
    system(".\\set_ss_vco 0 1 > null") ;

#else

    system("./set_ss_vco 1 3 > /dev/null") ;
    /* E3/T3 34368 */
    system("./set_ss_vco 4 2 > /dev/null") ;
    /* ECL/Differential 8448 */
    system("./set_ss_vco 3 1 > /dev/null") ;
    system("./set_ss_vco 0 1 > /dev/null") ;


#endif

    edt_lxt_noloop(edt_p, 0xffff) ;
    /* edt_lxt_aloop(edt_p, 0xffff) ;*/

    if (t1e1)
    /* test t1/e1 */
    {
	/* each pass has different jumpers installed to change loopbacks */
	if (passno == 0)
	{
	    /* receive on 0-3, 8-11 */
	    printf("pass 0 - t1/e1 receive channels 0-3, 8-11\n") ;
	    mask = 0x0f0f ;
	}
	else
	{
	    /* receive on 4-7, 12-15 */
	    printf("pass 1 - t1/e1 receive channels 4-7, 12-15\n") ;
	    mask = 0xf0f0 ;
	}
	edt_e1_collect(edt_p, mask, mscollect) ;
	/* edt_status(edt_p, mask) ;*/
    }
    if (t3e3)
    /* test t3/e3 */
    {
	/* each pass has different jumpers installed to change loopbacks */
	if (passno == 0)
	{
	    /* receive on 1,3 */
	    printf("pass 0 - t3/e3 receive channels 1 and 3\n") ;
	    mask = 0x000a ;
	}
	else
	{
	    /* receive on 0,2 */
	    printf("pass 1 - t3/e3 receive channels 0 and 2\n") ;
	    mask = 0x0005 ;
	}
	edt_e3_collect(edt_p, mask, mscollect) ;
	edt_status(edt_p, mask) ;
    }
    if (diff)
    /* test ecl/differential */
    {
	/* each pass has different jumpers installed to change loopbacks */
	if (passno == 0)
	{
	    /* set direction register */
	    edt_intfc_write(edt_p, ECLDIR, 0xc) ;
	    /* receive on 0-3 */
	    printf("pass 1 - receive channels 0-3\n") ;
	    mask = 0x000f ;
	}
	else
	{
	    /* set direction register */
	    edt_intfc_write(edt_p, ECLDIR, 0x3) ;
	    /* receive on 4-7 */
	    printf("pass 0 - receive channels 4-7\n") ;
	    mask = 0x00f0 ;
	}
	edt_df_collect(edt_p, mask, mscollect) ;
	edt_status(edt_p, mask) ;
    }
    return(0) ;
}

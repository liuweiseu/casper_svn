
#include "edtinc.h"
usage()
{
    printf("Usage: lxt\n") ;
    printf("-u unit		unit to test\n") ;
    printf("-s msleep	msecs to sleep between status checks\n") ;
    printf("-S msleep	msecs to sleep between output\n") ;
    printf("-l 		local analog loopback\n") ;
    printf("-d 		local digital loopback\n") ;
    printf("-r		remote loopback\n") ;
    printf("-n		no loopback\n") ;
    printf("-R		reset before checks\n") ;
    printf("-b		bitcounts only\n") ;
    printf("-c mask		set channel mask (defaults to all)\n") ;
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

/* for CMDREG */
#define BIT_ERRINS	0x40
#define BIT_RESET	0x20
#define BIT_LATCH	0x10

u_short atoh(char *p)
{
    u_int val = 0 ;
    int     i;

    for (i = 0; i < 4; i++)
    {
	if (*p)
	val <<= 4;
	if (*p >= '0' && *p <= '9')
	    val |= (*p - '0');
	else if (*p >= 'A' && *p <= 'F')
	    val |= (*p - 'A') + 0xa;
	else if (*p >= 'a' && *p <= 'f')
	    val |= (*p - 'a') + 0xa;
	else
	    break;
	++p;
    }
    return (val);
}

u_char stat_sv[16][16] ;

edt_lxt_reset(EdtDev *edt_p, u_int mask)
{
    int i ;
    int j ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    for (j = 0 ; j < 16 ; j++)
		stat_sv[i][j] = 0 ;
	}
    }
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
edt_lxt_dloop(EdtDev *edt_p, u_int mask) 
{
    u_char tmp ;
    u_char lxt0_bits ;
    u_char lxt1_bits ;
    lxt0_bits = mask & 0xff ;
    lxt1_bits = (mask >> 8) & 0xff ;
    tmp = edt_intfc_read(edt_p, LXT0+DLOOP) ;
    edt_intfc_write(edt_p, LXT0+DLOOP, tmp |= lxt0_bits) ;
    tmp = edt_intfc_read(edt_p, LXT1+DLOOP) ;
    edt_intfc_write(edt_p, LXT1+DLOOP, tmp |= lxt1_bits) ;
}
edt_lxt_rloop(EdtDev *edt_p, u_int mask) 
{
    u_char tmp ;
    u_char lxt0_bits ;
    u_char lxt1_bits ;
    lxt0_bits = mask & 0xff ;
    lxt1_bits = (mask >> 8) & 0xff ;
    tmp = edt_intfc_read(edt_p, LXT0+RLOOP) ;
    edt_intfc_write(edt_p, LXT0+RLOOP, tmp |= lxt0_bits) ;
    tmp = edt_intfc_read(edt_p, LXT1+RLOOP) ;
    edt_intfc_write(edt_p, LXT1+RLOOP, tmp |= lxt1_bits) ;
}

edt_lxt_collect(EdtDev *edt_p, u_int mask, u_int msecs)
{
    int i ;
    int j ;
    u_int *tmpp ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    edt_intfc_write(edt_p, BITCNT , BIT_RESET) ;
	    edt_intfc_write(edt_p, BITCNT, i) ;
	    if (msecs)
		edt_msleep(msecs) ;
	    edt_intfc_write(edt_p, BITCNT, BIT_LATCH+i) ;
	    for (j = 0 ; j < 16 ; j++)
	    {
		stat_sv[i][j] += edt_intfc_read(edt_p,0x30+j) ;
	    }
	}
    }
}
edt_lxt_status(EdtDev *edt_p, u_int mask)
{
    int i ;
    int j ;
    u_int *tmpp ;
    int firststat = 1 ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    if (firststat)
	    {
		tmpp = (u_int *)&stat_sv[i][8] ;
		printf("los %08x\n",*tmpp) ;
	        firststat = 0 ;
	    }
	    tmpp = (u_int *)&stat_sv[i][0] ;
	    printf("%d: ",i) ;
	    printf("bits: %d ",*tmpp++) ;
	    printf("errs: %d ",*tmpp++) ;
	    /* printf("los: %08x ",*tmpp++) ;*/
	    tmpp++ ;
	    printf("bipolar: %d ",*tmpp++) ;
	    printf("\n") ;
	}
    }
}

edt_lxt_bitcnt(EdtDev *edt_p, u_int mask)
{
    int i ;
    int j ;
    int printed = 0 ;
    u_int *tmpp ;
    printf("bitcounts:\n") ;
    for (i = 0 ; i < 16 ; i++)
    {
	if (mask & (1 << i))
	{
	    tmpp = (u_int *)&stat_sv[i][0] ;
	    printf("%d ",*tmpp) ;
	}
	if (++printed == 8) printf("\n") ;
    }
    printf("\n") ;
}

main(int argc,char **argv)
{

    EdtDev *edt_p ;
    int unit = 0 ;
    int mssleep = 1000 ;
    int mscollect = 100 ;
    u_int mask = 0xffff ;
    int do_aloop = 0 ;
    int do_dloop = 0 ;
    int do_rloop = 0 ;
    int do_noloop = 0 ;
    int do_reset = 0 ;
    int do_bitcnt = 0 ;
    int channel = 0 ;
    
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

	  case 'S':
		++argv;
		--argc;
		mssleep = atoi(argv[0]);
		break;

	  case 'c':
		++argv;
		--argc;
		mask = atoh(argv[0]);
		break;

	  case 'a':
		do_aloop = 1 ;
		break ;

	  case 'd':
		do_dloop = 1 ;
		break ;

	  case 'n':
		do_noloop = 1 ;
		break ;

	  case 'r':
		do_rloop = 1 ;
		break ;

	  case 'b':
		do_bitcnt = 1 ;
		break ;

	  case 'R':
		do_reset = 1 ;
		break ;

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
    /* E1/T1 2048 */
    system("./set_ss_vco 1 3 > /dev/null") ;

    edt_lxt_reset(edt_p, mask) ;
    edt_lxt_noloop(edt_p, mask) ;

    if (do_aloop)
    {
	edt_lxt_noloop(edt_p, mask) ;
	edt_lxt_aloop(edt_p, mask) ;
    }
    if (do_dloop)
    {
	edt_lxt_noloop(edt_p, mask) ;
	edt_lxt_dloop(edt_p, mask) ;
    }

    if (do_noloop)
    {
	edt_lxt_noloop(edt_p, mask) ;
    }

#if 0
    printf("no loopback\n");
    edt_lxt_reset(edt_p, mask) ;
    edt_lxt_noloop(edt_p, mask) ;
    edt_lxt_collect(edt_p, mask, mscollect) ;
    edt_lxt_status(edt_p, mask) ;

    printf("digital loopback\n") ;
    edt_lxt_reset(edt_p, mask) ;
    edt_lxt_noloop(edt_p, mask) ;
    edt_lxt_dloop(edt_p, mask) ;
    edt_lxt_collect(edt_p, mask, mscollect) ;
    edt_lxt_status(edt_p, mask) ;

    printf("analog loopback\n") ;
    edt_lxt_reset(edt_p, mask) ;
    edt_lxt_noloop(edt_p, mask) ;
    edt_lxt_dloop(edt_p, mask) ;
    edt_lxt_collect(edt_p, mask, mscollect) ;
    edt_lxt_status(edt_p, mask) ;
#endif

    if (mssleep)
    {
	u_int  dtmp ;
	u_short los ;
	(void)edt_dtime();
	for(;;)
	{
	    dtmp = (u_int)(edt_dtime() * 1000.0) ;
	    printf("%04d ms ",dtmp) ;
	    if (do_reset)
		edt_lxt_reset(edt_p, mask) ;
	    edt_lxt_collect(edt_p, mask, mscollect) ;
	    if (do_bitcnt)
		edt_lxt_bitcnt(edt_p, mask) ;
	    else
		edt_lxt_status(edt_p, mask) ;
	    if (mssleep)
		edt_msleep(mssleep) ;
	}
    }
    return(0) ;
}

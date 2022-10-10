/* #pragma ident "@(#)pcd_watchstat.c	1.2 05/13/99 EDT" */

#include "edtinc.h"
#if defined(__sun) || defined(sgi)

#include <sys/filio.h> 

#endif
#include <signal.h>

void
gotsig(int sig)
{
	puts("INTERRUPT") ;
#ifndef _NT_
	system("stty echo icanon min 4") ;
#endif
	exit(0) ;
}


int i,x ;

main(int argc,char **argv)
{

    EdtDev *edt_p ;
	u_int  dma_cfg = 0;
	u_int  last_dma_cfg ;
	u_int  pcd_cmd = 0 ;
	u_int  pcd_funct = 0 ;
	u_int  pcd_stat = 0 ;
	u_int  pcd_stat_polarity = 0 ;
	u_int  last_pcd_cmd = 0 ;
	u_int  last_pcd_funct = 0 ;
	u_int  last_pcd_stat = 0 ;
	u_int  last_pcd_stat_polarity = 0 ;
u_int last_count ;
u_int pcd_count ;
u_int last_sgnxt ;
u_int pcd_sgnxt ;
u_int pcd_dpstat ;
u_int last_dpstat ;
    int    unit = 0 ;
    int    mssleep = 10 ;
    char   *progname;
	u_int  dtmp ;
    u_int  count =0 ;

	/* command processing variables */
	char s[32] ;
	u_int arg ;
	int ch ;

    progname = argv[0];
    
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
			mssleep = atoi(argv[0]);
			break;

		  default:
			puts("Usage: watchstat [-u unit] [-s msleep]\n") ;
			exit(0);
		}
		argc--;
		argv++;
    }
    printf("unit %d mssleep %d\n", unit, mssleep) ;

    
    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
		char str[64] ;
		sprintf(str, "%s%d", EDT_INTERFACE, unit) ;
		perror(str) ;
		exit(1) ;
    }

    (void)edt_dtime();
    	
#ifdef __sun

	sigset(SIGINT, gotsig) ;


    system("stty -echo -icanon min 1") ;
#endif
    for(;;)
    {
		pcd_cmd = pcd_get_cmd(edt_p) ;
		pcd_funct = pcd_get_funct(edt_p) ;
		pcd_stat = pcd_get_stat(edt_p) ;
		pcd_stat_polarity = pcd_get_stat_polarity(edt_p) ;
		dma_cfg = edt_reg_read(edt_p, EDT_DMA_CFG) ;
		pcd_count = edt_get_bytecount(edt_p) ;
		pcd_sgnxt = edt_reg_read(edt_p, EDT_SG_NXT_CNT) ;
		pcd_dpstat = edt_reg_read(edt_p, PCD_DATA_PATH_STAT) ;


		if (last_pcd_cmd != pcd_cmd || last_pcd_stat != pcd_stat 
			|| last_count != pcd_count 
			|| last_sgnxt != pcd_sgnxt
			|| last_dpstat != pcd_dpstat
								|| last_pcd_funct != pcd_funct 
								|| last_pcd_stat_polarity != pcd_stat_polarity 
								|| last_dma_cfg != dma_cfg)
		{
			last_pcd_cmd = pcd_cmd ;
			last_pcd_funct = pcd_funct ;
			last_pcd_stat = pcd_stat ;
			last_pcd_stat_polarity = pcd_stat_polarity ;
			last_dma_cfg = dma_cfg ;
			last_count = pcd_count ;
			last_sgnxt = pcd_sgnxt ;
			last_dpstat = pcd_dpstat ;

			dtmp = (u_int)(edt_dtime() * 1000.0) ;

			printf(
				"ms %04d cmd %04x funct %04x stat %04x statpol %04x cfg %08x cnt %08x sgnxt %08x dpstat %04x\n",
				dtmp,
				pcd_cmd,
				pcd_funct,
				pcd_stat,
				pcd_stat_polarity,
				dma_cfg,
				pcd_count,
				pcd_sgnxt,
				pcd_dpstat
				) ;
		}
		if (mssleep)
			edt_msleep(mssleep) ;

		/* Command processing code */
#if defined(__sun) || defined(sgi)

		ioctl(0, FIONREAD, &count) ;
#endif

		if (count)
		{
			ch = getchar() ;

			if (ch == ':')
			{
				fputs(": ", stdout) ;
				fflush(stdout) ;

				ch = getchar() ;
				putchar(ch) ;
				fflush(stdout) ;

				system("stty echo") ;
				switch(ch)
				{
				case 'f':
					gets(s) ;
					sscanf(s, "%x", &arg) ;
					pcd_set_funct(edt_p, (u_char) arg) ;
					printf("funct set to 0x%x readback 0x%x\n",
									arg, pcd_get_funct(edt_p)) ;
					break ;
				case 'p':
					gets(s) ;
					sscanf(s, "%x", &arg) ;
					pcd_set_stat_polarity(edt_p, (u_char) arg) ;
					break ;
				case 'c':
					gets(s) ;
					sscanf(s, "%x", &arg) ;
					pcd_set_cmd(edt_p, (u_char) arg) ;
					break ;
				case 'q':
					system("stty echo icanon min 4") ;
					exit(0) ;
				case '\r':
					break ;
				}
				system("stty -echo") ;
			}
			else
				putchar('\n') ;
			/* Make sure new line is output */
			last_pcd_cmd = ~last_pcd_cmd ;
			last_pcd_funct = ~last_pcd_funct ;
		}
    }
    return(0) ;
}

/* example of programmed io */

#include "edtinc.h"

EdtDev *edt_p ;

u_char mmap_intfc_read(volatile caddr_t mapaddr, u_int desc) ;
void mmap_intfc_write(volatile caddr_t mapaddr, u_int desc, u_char val) ;

main(argc, argv)
int argc;
char **argv;
{
	int i, unit = 0 ;
	int do_ioctl = 0 ;
	int verbose = 0 ;
	u_char ftmp ;
	u_char ftmp2 ;
	u_char stmp ;
	double dtime ;
	int loop = 100000 ;
	volatile caddr_t mapaddr ;
	volatile caddr_t mapaddr1 ;
	u_int *tstaddr ;


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

		  case 'i':
			do_ioctl = 1 ;
			break;

		  case 'v':
			verbose = 1 ;
			break;

		  case 'l':
			++argv;
			--argc;
			loop = atoi(argv[0]);
			break;

		  default:
			puts("Usage: testmmap [-u unit_no] [-l loop] [-i] [-v]") ;
			puts("-i to test ioctl vs mmap") ;
			puts("-v for verbose") ;
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
	if (do_ioctl)
	{
		printf("starting ioctl test %d\n",loop) ;
		(void)edt_dtime();
for(i = 0 ; i < 10 ; i++)
printf("%d: %x\n",i,edt_intfc_read(edt_p,i)) ;
		for(i = 0 ; i < loop ; i++)
		{
			ftmp = i % 4 ;
			(void)edt_reg_write(edt_p, PCD_FUNCT,ftmp) ;
			if (verbose)
			{
				stmp = edt_reg_read(edt_p, PCD_STAT) ;
				printf("%d %x %x\n", i, ftmp, stmp) ;
			}
		}
		dtime = edt_dtime() ;
	}
	else
	{
		printf("setting up mmap\n") ;
		mapaddr = (volatile caddr_t)edt_mapmem(edt_p, 0, 256) ;
		  if ((int)mapaddr == 0)
		  {
		    printf("edt_mapmem failed\n") ;
		    exit(2);
		  }
		tstaddr = (u_int *)mapaddr ;
printf("mapaddr %x\n",mapaddr) ;
		for(i = 0 ; i < 10 ; i++)
		{
		printf("%08x\n",*tstaddr++) ;
		}
		mapaddr1 = (volatile caddr_t)edt_mapmem(edt_p, 0x10000, 0x4000) ;
		if ((int)mapaddr1 == 0)
		{
		    printf("edt_mapmem of second region failed\n") ;
		}
		else
		{
printf("mapaddr1 %x\n",mapaddr1) ;
		    tstaddr = (u_int *)mapaddr1 ;
		    for(i = 0 ; i < 10 ; i++)
		    {
			printf("%08x\n",*tstaddr++) ;
		    }
		}
		(void)edt_dtime();
		for(i = 0 ; i < loop ; i++)
		{
			ftmp = i % 4 ;
			(void)mmap_intfc_write(mapaddr, PCD_FUNCT,ftmp) ;
			if (verbose)
			{
				stmp = mmap_intfc_read(mapaddr, PCD_STAT) ;
				printf("%d %x %x\n", i, ftmp, stmp) ;
			}
		}
		dtime = edt_dtime() ;
		printf("done\n") ;
	}
	printf("\n%8.0f tests/sec (%2.2f usec/test)\n",
		(double)(loop) / dtime,
		(dtime / (double)(loop)) * 1000000.0) ;


	edt_close(edt_p) ;
	exit(0) ;
	return(0) ;
}

typedef	volatile u_char v_char ;

u_char
mmap_intfc_read(caddr_t mapaddr, u_int desc)
{
	v_char *off_p ;
	v_char *data_p ;
	u_char val ;
	/* interface must set offset then read data */
	off_p = (v_char *)mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = (v_char *)mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	val = *off_p ;
	/* edt_msleep(10) ;*/
	val = *data_p ;
	return(val) ;
}

void
mmap_intfc_write(caddr_t mapaddr, u_int desc, u_char val)
{
	v_char *off_p ;
	v_char *data_p ;
	u_char tmp ;
	/* interface must set offset then read data */
	off_p = (v_char *)mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = (v_char *)mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	tmp = *off_p ;
	*data_p = val ;
}

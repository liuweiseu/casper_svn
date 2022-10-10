#include "edtinc.h"
#include <stdlib.h>

#define	SSD_MSB		0x04
#define	BYTE_SWAP 	0x01

EdtDev         *edt_p;
EdtDev         *td_p;
int             fd;
int             ret;
int             do_msb;
int             verbose = 0;
int             noload = 0;
int		count ;
u_char          option;
u_short        *buf_p;

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
	puts("-s tester_speed where:") ;
	puts("\t0 - no clock divide") ;
	puts("\t1 - divide by 2") ;
	puts("\t2 - divide by 4") ;
	puts("\t3 - divide by 8 (the default)") ;
	puts("size is the size of read in words (16 bits)");
}



int
main(int argc, char **argv)
{
	int             i;
	FILE           *outfile;
	FILE           *rawfile;
	char            filename[100];
	char            rfilename[100];
	u_char          funct;
	int				speed = 3 ;
	int             unit = 0;
	int             test_unit = 1;
	char			syscmd[40] ;
	u_short tmpx ;
	u_short tmpy ;
	u_short tmpz ;
	u_short shdbe ;
	u_int ltmp0 ;
	u_int ltmp1 ;
	int errs ;
	int shift ;

	/*
	 * set default options
	 */
	do_msb = 0;
	outfile = 0;
	rawfile = 0;

	--argc;
	++argv;
	while (argc && argv[0][0] == '-')
	{
		switch (argv[0][1])
		{
		case 'm':
			do_msb = 1;
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
			sprintf(rfilename,"%s.raw",filename) ;
			if ((outfile = fopen(filename, "wb")) == NULL)
			{
				perror(filename);
				exit(1);
			}
			if ((rawfile = fopen(rfilename, "wb")) == NULL)
			{
				perror(rfilename);
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
		printf("must specify count\n") ;
		exit(1) ;
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
		sprintf(syscmd,"bitload  -u %d ssd.bit",unit) ;
		system(syscmd) ;
		sprintf(syscmd,"bitload  -u %d ssdout1.bit",test_unit) ;
		system(syscmd) ;
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

	edt_intfc_write(td_p, SSDT_FUNC, speed) ;
	edt_close(td_p) ;


	edt_intfc_write(edt_p, PCD_CONFIG, 0xff);

	/* set up bit order */
	funct = edt_intfc_read(edt_p, PCD_FUNCT) ;
	funct &= 0xf8;
	if (do_msb)
		funct |= SSD_MSB;

	edt_intfc_write(edt_p, PCD_FUNCT, funct) ;
	edt_intfc_write(edt_p, PCD_CONFIG, PCD_BYTESWAP) ;

	edt_configure_ring_buffers(edt_p, count * 2, 4, EDT_READ, NULL) ;

	edt_flush_fifo(edt_p) ;


	/* do a no-latency startup of dma.  Only need one buffer. */
	edt_start_buffers(edt_p, 4);
	buf_p = (u_short *)edt_wait_for_buffers(edt_p, 1);

	if (rawfile)
	{
		fwrite(buf_p, 1, (count * 2), rawfile);
		printf("writing %d to %s\n", count * 2, filename);
		fclose(rawfile);
	}
	/* check the data */
	/* find shift */
	ltmp0 = buf_p[0] << 16 | buf_p[1] ;
	ltmp1 = buf_p[2] << 16 | buf_p[3] ;
	for (shift = 0 ; shift < 16 ; shift++)
	{
	    tmpx = (ltmp0 >> 16) & 0xffff ;
	    tmpy = ltmp0 & 0xffff ;
	    tmpz = (ltmp1 >> 16) & 0xffff ;
	    /* find what shift is needed */
	    if (tmpy - tmpx == 1 && tmpz - tmpy == 1)
	    {
		printf("data shifted %d bits\n",shift) ;
		break ;
	    }
	    ltmp0 <<= 1 ;
	    if (ltmp1 & 0x80000000) ltmp0 |= 1 ;
	    ltmp1 <<= 1 ;
	}
	shdbe = tmpx ;
	printf("checking data from %04x\n",shdbe) ;
	errs = 0 ;
	for(i = 0 ; i < count ; i++)
	{
	    ltmp0 = (buf_p[i] << 16) | buf_p[i+1] ;
	    ltmp0 <<= shift ;
	    tmpx = (ltmp0 >> 16) & 0xffff ;
	    if (outfile)
	    {
		fwrite(&tmpx, 1, 2, outfile);
	    }
	    if (tmpx != shdbe)
	    {
		printf("%04x s/b %04x\n",tmpx,shdbe) ;
		printf("%04x %04x %04x [%04x] %04x %04x %04x\n",
			buf_p[i-3],
			buf_p[i-2],
			buf_p[i-1],
			buf_p[i],
			buf_p[i+1],
			buf_p[i+2],
			buf_p[i+3]) ;
		errs++ ;
	    }
	    shdbe++ ;
	}
	fclose(outfile);
	printf("last word %x checked %d errs %d\n",tmpx,count,errs) ;
	exit(0);
}





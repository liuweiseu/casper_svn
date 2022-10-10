#include "edtinc.h"
#include <stdlib.h>

#define	SSD_MSB		0x04
#define	BYTE_SWAP 	0x01

EdtDev         *edt_p;
int             fd;
int             ret;
int             count;
int             do_msb;
int             verbose = 0;
int				noload ;
int				do_speed ;
int				buffers_inuse ;
u_short        *buf_p;
#define NUMBUFS	4

u_short next_val(u_short val) ;

void
usage()
{
	puts("usage: writessd [-d filename] [-m] [-u unit number] [-n num] size");
	puts("-d datafile specifies input file for data");
	puts("-m sets most significant bit of word is first bit(default is least)");
	puts("-u sets scd device number - default is 0");
	puts("-s tester_speed where:") ;
	puts("\t0 - no clock divide") ;
	puts("\t1 - divide by 2") ;
	puts("\t2 - divide by 4") ;
	puts("\t3 - divide by 8 (the default)") ;
	puts("-w selects 16 bit counter instead of 4") ;
	puts("size is the size of read in words (16 bits)");
}

int
main(int argc, char **argv)
{
	int             i;
	FILE           *infile;
	char            filename[100];
	u_char          funct;
	u_char			speed = 3 ;
	int             unit = 0;
	char			syscmd[40] ;
	int				loops ;
	int				loopcount ;
	int				loop = 0 ;
	u_short 	*ptr ;
	u_short		*endptr ;
	u_short val ;

	/*
	 * set default options
	 */
	do_msb = 0;
	infile = 0;
	loops = 0 ;
	loopcount = 1 ;

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
			if ((infile = fopen(filename, "rb")) == NULL)
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

		case 'l':
			++argv;
			--argc;
			loopcount = atoi(argv[0]);
			break;

		case 's':
			++argv;
			--argc;
			do_speed = 1 ;
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

	printf("writing %d words\n", count);

	if (infile)
		printf("input from %s\n", filename);

	if (do_msb)
		puts("most significant bit is first bit in");
	else
		puts("least significant bit is first bit in");

	if (!noload)
	{
		sprintf(syscmd,"bitload  -u %d fastssdout.bit",unit) ;
		system(syscmd) ;
	}

	edt_p = edt_open(EDT_INTERFACE, unit);
	if (edt_p == NULL)
	{
		sprintf(filename, "/dev/pcd%d", unit);
		perror(filename);
		exit(1);
	}

	edt_intfc_write(edt_p, PCD_CONFIG, 0xff);

	/* set up number of bits per channel and bit order */
	funct = edt_intfc_read(edt_p, PCD_FUNCT) ;
	funct &= 0xf8;
	if (do_msb)
		funct |= SSD_MSB;

	/* set speed */
	printf("return to write %d",speed) ; getchar() ;
	edt_intfc_write(edt_p, EDT_OUT_SCALE, speed) ;

	edt_intfc_write(edt_p, PCD_FUNCT, funct) ;
	edt_intfc_write(edt_p, PCD_CONFIG, PCD_BYTESWAP) ;

    edt_configure_ring_buffers(edt_p, count * 2, NUMBUFS, EDT_WRITE, NULL) ;

	/* edt_flush_fifo(edt_p) ;*/

	buffers_inuse = 0 ;
	printf("loopcount %d\n",loopcount) ;
	val = 0 ;
	while (loop < loopcount || loopcount == 0)
	{
		if (buffers_inuse >= NUMBUFS)
		{
			/* bufs in use - wait for next */
			edt_wait_for_buffers(edt_p, 1) ;
			--buffers_inuse ;
		}
		/* get pointer to first output buffer */
		buf_p = (u_short *)edt_next_writebuf(edt_p);

		/* fill in first buffer */
		if (infile)
		{
			ret = fread(buf_p, 1, (count * 2), infile);
			printf("writing %d to %s\n", count * 2, filename);
		}
		endptr = buf_p + count ;
		ptr = buf_p;

		while (ptr < endptr)
		{
			*ptr++ = val++ ;
		}


		/* ddb */
		endptr = buf_p + count ;
		ptr = buf_p;
		while (ptr < endptr)
		{
			*ptr++ = 0x100 ;
			*ptr++ = 0x101 ;
			*ptr++ = 0x102 ;
			*ptr++ = 0x103 ;
			*ptr++ = 0x104 ;
			*ptr++ = 0x105 ;
			*ptr++ = 0x106 ;
			*ptr++ = 0x107 ;
		}
		ptr = buf_p ;
		if (loop == 0)
		{
		    printf("sending:\n") ;
		    for(i = 0 ; i < 512 ; i++)
			printf("%04x ",ptr[i]) ;
		    printf("\n") ;
		}
		 
		/* do a no-latency startup of dma.  Only need one buffer. */
		edt_start_buffers(edt_p, 1);
		putchar('.') ;
		fflush(stdout) ;
		++buffers_inuse ;
		loop++ ;
	}
	/* 
	 * wait for buffers to finish 
	 */
	while (buffers_inuse)
	{
		edt_wait_for_buffers(edt_p, 1) ;
		--buffers_inuse ;
	}

	printf("return to close") ; getchar();
	edt_close(edt_p) ;
	return 0;
}



u_short
next_val(u_short val)
{
	u_short         old = val;

	switch (val)
	{
	case 0xfedc:
		val = 0x3210;
		break;
	case 0xedcb:
		val = 0x210f;
		break;
	case 0xdcba:
		val = 0x10fe;
		break;
	case 0xcba9:
		val = 0x0fed;
		break;
	case 0xba98:
		val = 0xfedc;
		break;
	case 0xa987:
		val = 0xedcb;
		break;
	case 0x9876:
		val = 0xdcba;
		break;
	case 0x8765:
		val = 0xcba9;
		break;
	case 0x7654:
		val = 0xba98;
		break;
	case 0x6543:
		val = 0xa987;
		break;
	case 0x5432:
		val = 0x9876;
		break;
	case 0x4321:
		val = 0x8765;
		break;
	case 0x3210:
		val = 0x7654;
		break;
	case 0x210f:
		val = 0x6543;
		break;
	case 0x10fe:
		val = 0x5432;
		break;
	case 0x0fed:
		val = 0x4321;
		break;
	default:
		printf("next_val:  unexpected value 0x%x\n", old);
		return (-1);
		break;
	}
	return (val);
}


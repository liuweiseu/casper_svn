#include <stdio.h>
#include <math.h>

#include <string.h>

#include <stdlib.h>

#include "edtinc.h"

#include "edt_vco.h"
#include "edt_ss_vco.h"

#define E3FREQ	34368000.0
#define XTAL	10368100.0

/*
 * offsets in the 8 bit indirect space 78P2344JAT serial pins
 */
#define TDK78P2344_EN_D	0x0101004A
#define TDK78P2344_CTL	0x0101004C
#define TDK78P2344_LOS	0x01010048
/* bits in the data enable register */
#define	TDK78P2344_ENCH	0x0f
/* bits in the control register */
#define	TDK78P2344_EN_TX 0x0f
#define	TDK78P2344_ENABLE 0x10
#define	TDK78P2344_SDI	0x20
#define	TDK78P2344_SCK	0x40
#define	TDK78P2344_CS	0x80
/* bits in the status register */
#define	TDK78P2344_INTR	0x0f
#define	TDK78P2344_SDO	0x20
/* Page bamks in the TDK78P2344 */
#define	GLOBAL_BANK	0x0
#define	CHAN_BANK_BASE	0x1
/* sub addresses in the global bank */
#define TDK78P2344_MSCR	0x0
#define TDK78P2344_INTC	0x1
/* sub addresses in each per port bank */
#define TDK78P2344_MDCR	0x0
#define TDK78P2344_STAT	0x1
#define TDK78P2344_JACR	0x3


void
edt_write_78P2344_s_bit(EdtDev * edt_p, u_int bit)
{
	/* assume the sck  and sdi are low  */
	/* always set serial enable (CS) */
	if (bit == 0)
	{
		edt_reg_or( edt_p, TDK78P2344_CTL, TDK78P2344_CS );
	}
	else
	{
		edt_reg_or( edt_p, TDK78P2344_CTL, TDK78P2344_CS | TDK78P2344_SDI );
	}
	/* set sck */
	edt_reg_or( edt_p, TDK78P2344_CTL, TDK78P2344_SCK );
	/* clear sdi and sck together (data is latched on the rising edge */
	edt_reg_and(edt_p, TDK78P2344_CTL, ~(TDK78P2344_SDI | TDK78P2344_SCK));
}

u_int
edt_read_78P2344_s_bit(EdtDev * edt_p)
{
	u_int tdk_stat;
	/* read the first bit before clocking to the next */
	tdk_stat = edt_reg_read(edt_p, TDK78P2344_LOS);
	/* assume SCK is low to start - always set serial enable high */
	edt_reg_or( edt_p, TDK78P2344_CTL, TDK78P2344_CS | TDK78P2344_SCK );
	edt_reg_and(edt_p, TDK78P2344_CTL, ~(TDK78P2344_SDI | TDK78P2344_SCK));
	if ((tdk_stat & TDK78P2344_SDO) == 0)
	{
		return(0);
	}
	else
	{
		return(1);
	}
}

void
edt_78p2344_serial_add(EdtDev * edt_p, u_int bank, u_int sub_add)
{
	int i;

	/* read write bit is already sent */
	for(i=0; i<3; i++)
	{
		edt_write_78P2344_s_bit(edt_p, (sub_add & 0x1));
		sub_add >>= 1;
	}
	for(i=0; i<4; i++)
	{
		edt_write_78P2344_s_bit(edt_p, (bank & 0x1));
		bank >>= 1;
	}
}

void
edt_write_78p2344_reg(EdtDev * edt_p, u_int bank, u_int sub_add, unsigned char data)
{
	int i;

	/* output read/write bit as 0 (write) */
	edt_write_78P2344_s_bit(edt_p, 0);
	/* output address */
	edt_78p2344_serial_add(edt_p, bank, sub_add);
	for(i=0; i<8; i++)
	{
		edt_write_78P2344_s_bit(edt_p, (data & 0x1));
		data >>= 1;
	}
	/* drop serial enable */
	edt_reg_and(edt_p, TDK78P2344_CTL, ~(TDK78P2344_CS));
}

unsigned char
edt_read_78p2344_reg(EdtDev * edt_p, u_int bank, u_int sub_add)
{
	int i;
	unsigned char data = 0;

	/* output read/write bit as 1 (read) */
	edt_write_78P2344_s_bit(edt_p, 1);
	/* output address */
	edt_78p2344_serial_add(edt_p, bank, sub_add);
	for(i=0; i<8; i++)
	{
		data >>= 1;
		if ( (edt_read_78P2344_s_bit(edt_p)) != 0)
		{
			data |= 0x80;
		}
	}
	/* drop serial enable */
	edt_reg_and(edt_p, TDK78P2344_CTL, ~(TDK78P2344_CS));
	return(data);
}

void
tdk78P2344_e3_init(EdtDev *edt_p, int loopback)
{
	int i;
	/* set glogal register 0 for e3, endec on and sense on falling edge */
	edt_write_78p2344_reg(edt_p, 0, TDK78P2344_MSCR, 0xAC);
	edt_write_78p2344_reg(edt_p, 0, TDK78P2344_INTC, 0x82); /* intr is los */
	/* for each channel */
	for(i=0; i<4; i++)
	{
		if (loopback == 0)
			edt_write_78p2344_reg(edt_p, i+1, TDK78P2344_MDCR, 0x21);
		else
			edt_write_78p2344_reg(edt_p, i+1, TDK78P2344_MDCR, 0x29);
		edt_write_78p2344_reg(edt_p, i+1, TDK78P2344_JACR, 0x8c);
	}
}

void
tdk78P2344_dump(EdtDev *edt_p)
{
	int i;

	printf("Global registers: MSCR = %02x ", edt_read_78p2344_reg(edt_p, 0, TDK78P2344_MSCR));
	printf("INTC = %02x\n", edt_read_78p2344_reg(edt_p, 0, TDK78P2344_INTC));
	/* for each channel */
	for(i=0; i<4; i++)
	{
		printf("\tChannel %d registers: MDCR = %02x ", i, edt_read_78p2344_reg(edt_p, i+1, TDK78P2344_MDCR));
		printf("STAT = %02x ", edt_read_78p2344_reg(edt_p, i+1, TDK78P2344_STAT));
		printf("JACR = %02x\n", edt_read_78p2344_reg(edt_p, i+1, TDK78P2344_JACR));
	}
}

void
usage(char *pname)

{

	fprintf(stderr, "usage: %s [-u unit] [-v] [-d] [-R] [-l] [-D]\n", pname);
	fprintf(stderr, "\t-v for Verbose\n");
	fprintf(stderr, "\t-R disable main command reset\n");
	fprintf(stderr, "\t-D disable TDK register init\n");
	fprintf(stderr, "\t-d dump TDK register only\n");
	fprintf(stderr, "\t-l sets analog loopback\n");

	exit(1);
}



int main(int argc, char **argv)
{

	int verbose = 0;
	int dump = 0;
	int delay = 0;
	int reset = 1;
	int reg_default = 0;
	int loopback = 0;

	double xtal = XTAL;
	double target = E3FREQ;
	double actual;

	edt_pll pll;

	EdtDev *edt_p;

	int unit = 0;

	char *pname = argv[0];
 
	while (argc > 1 && argv[1][0] == '-')
	{
		if (argv[1][1] == '-')
		{
			argc--;
			argv++;
			break;
		}

		switch (argv[1][1])
		{
			case 'u':
				++argv;
				--argc;
				unit = atoi(argv[1]);
				break ;

			case 'R':
				reset = 0;
				break;

			case 'D':
				reg_default = 1;
				break;

			case 'd':
				dump = 1;
				break;

			case 't': /* delay  seconds before return (to let the part aquire signal */
				++argv;
				--argc;
				delay = atoi(argv[1]);
				break;

			case 'l':
				loopback = 1;
				break;

			case 'v':
				verbose = 1;
				break;

			default:
				usage(pname);
				break;

		}

		++argv;
		--argc;
	}

	

	if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, 0)) != NULL)
	{
		if (dump != 1)
		{
			/* reset tdk78p2344 and the turn off data */
			edt_reg_write(edt_p, TDK78P2344_CTL, 0);

			if (reset == 1)
			{
				/* hardware reset lxt3108 and the rest of the interface chip */
				edt_reg_write(edt_p, PCD_CMD, 0);

				edt_msleep(1); /* reset low for a while */
				/* unreset chip and lxt3108 */
				edt_reg_write(edt_p, PCD_CMD, 0x8);
			}
			
			/*  set pll 2 for e3 clock times 1 */
			actual = edt_find_vco_frequency_ics307(NULL,target,
						xtal, &pll, verbose);

			edt_set_out_clk_ics307(edt_p, &pll, 2);

			/* unreset tdk */
			edt_reg_write(edt_p, TDK78P2344_CTL, TDK78P2344_ENABLE); 
			/* enable the data */
			edt_reg_write(edt_p, TDK78P2344_EN_D, TDK78P2344_ENCH); 

			if (reg_default == 0)
				tdk78P2344_e3_init(edt_p, loopback);
		}

		tdk78P2344_dump(edt_p);
	
		if (delay != 0)
			edt_msleep(delay * 1000);
		

		edt_close(edt_p);
	}
	else
		fprintf(stderr, "No board found (unit=%d)\n", unit);

	return 0;
}


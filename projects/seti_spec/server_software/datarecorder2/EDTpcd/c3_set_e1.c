#include <stdio.h>
#include <math.h>

#include <string.h>

#include <stdlib.h>

#include "edtinc.h"

#include "edt_vco.h"
#include "edt_ss_vco.h"

#define E1FREQ	2048000.0
#define XTAL	10368100.0

/*
 * offsets in the 8 bit indirect space for LXT3108 address and data registers
 */
#define	MAXCHIPS	2
#define LXT3108_ADD	0x010100C0
#define LXT3108_CMD	0x010100C1
/* bits in the command register */
#define	LXT3108_ENABLE	0x01
/* Data read write register (add 1 * chip_number for each LXT3108) */
#define LXT3108_BASE	0x010100C2
/* Page bamks in the LXT3108 */
#define	GLOBAL_BANK	0x0
#define	CHAN_BANK_BASE	0x1
#define	ALL_CHAN_WRITE	0x9

void
edt_write_lxt3108(EdtDev * edt_p, u_int chip, u_int add, unsigned char data)
{
	edt_reg_write(edt_p, LXT3108_ADD, add & 0xff);
	edt_reg_write(edt_p, LXT3108_BASE + (chip & 0xf), data);
}

unsigned char
edt_read_lxt3108(EdtDev * edt_p, u_int chip, u_int add)
{
	edt_reg_write(edt_p, LXT3108_ADD, add & 0xff);
	return(edt_reg_read(edt_p, LXT3108_BASE + (chip & 0xf)));
}

void
edt_write_lxt3108_ppr(EdtDev * edt_p, u_int chip, u_int ppr_bank, u_int add, unsigned char data)
{
	edt_write_lxt3108(edt_p, chip, 0x0, (unsigned char) ppr_bank) ;
	edt_write_lxt3108(edt_p, chip, add & 0xff, data) ;
}

unsigned char
edt_read_lxt3108_ppr(EdtDev * edt_p, u_int chip, u_int ppr_bank, u_int add)
{
	edt_write_lxt3108(edt_p, chip, 0x0, (unsigned char) ppr_bank) ;
	return(edt_read_lxt3108(edt_p, chip, add & 0xff)) ;
}

void
lxt3108_e1_init(EdtDev *edt_p, u_int chip, u_int ppr_bank, int loopback)
{
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x02, 0x00); /* disable receiver during setup */
	if (loopback == 1)
		edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x01, 0x03); /* master to e1  and loopback */
	else
		edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x01, 0x01); /* master to e1 */

	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x03, 0x14); /* txcon */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x04, 0x80); /* rxcon */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x05, 0x82); /* termination 120 ohm */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x10, 0x80); /* ler */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x11, 0x00); /* ier */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x1c, 0x0f); /* cr1 */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x1d, 0x00); /* cr2 */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x33, 0x40); /* ralc */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x34, 0x00); /* rwfctrl_1 */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x3C, 0x11); /* rwfctrl_2 */
	edt_write_lxt3108_ppr(edt_p, chip, ppr_bank, 0x02, 0x01); /* enable receiver */
}

void
lxt3108_dump_ppr(EdtDev *edt_p, u_int chip, u_int ppr_bank)
{
	u_int reg;
	unsigned char rd_byte;

	if ( (ppr_bank>0) && (ppr_bank < 9) )
	{
		printf("Dump PPR bank %d of chip %d\n", ppr_bank, chip);
		for( reg=0; reg<0x3d; reg++)
		{
			rd_byte = edt_read_lxt3108_ppr(edt_p, chip, ppr_bank, reg);
			printf("%02x=%02x ", reg, rd_byte);
			if (((reg+1) % 8) == 0)
				printf("\n");
		}
		printf("\n");
	}
	else
	{
		printf("Dump PPR bank %d is out of range 1 to 8\n", ppr_bank);
	}
}

void
usage(char *pname)

{

	fprintf(stderr, "usage: %s [-u unit] [-v] [-R] [-l]\n", pname);
	fprintf(stderr, "\t-v for Verbose\n");
	fprintf(stderr, "\t-R disable main command reset\n");
	fprintf(stderr, "\t-l sets analog loopback\n");

	exit(1);
}



int main(int argc, char **argv)
{

	int verbose = 0;
	int reset = 1;
	int loopback = 0;

	double xtal = XTAL;
	double target = E1FREQ;
	double actual;

	edt_pll pll;

	EdtDev *edt_p;

	int unit = 0;
	int i;
	unsigned char version;
	unsigned char clad;

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
		/* reset lxt3108 */
		edt_reg_write(edt_p, LXT3108_CMD, 0);

		if (reset == 1)
		{
			/* hardware reset lxt3108 and the rest of the interface chip */
			edt_reg_write(edt_p, PCD_CMD, 0);

			edt_msleep(1); /* reset low for a while */
			/* unreset chip and lxt3108 */
			edt_reg_write(edt_p, PCD_CMD, 0x8);
		}
		
		/*  set pll 0 and 3 for e1 clock times 1 */
		actual = edt_find_vco_frequency_ics307(NULL,target,
					xtal, &pll, verbose);

		edt_set_out_clk_ics307(edt_p, &pll, 0);
		edt_set_out_clk_ics307(edt_p, &pll, 3);

		/* unreset lxt3108 */
		edt_reg_write(edt_p, LXT3108_CMD, LXT3108_ENABLE);

		/* write the CLAD register with e1 x 1 */
		for(i = 0; i < MAXCHIPS; i++)
		{
			edt_write_lxt3108_ppr(edt_p, i, GLOBAL_BANK, 0x11, 0x8c);
		}
		edt_msleep(1); /* let the clock adaptor sync up */


		/* read the chip version bytes */
		for(i = 0; i < MAXCHIPS; i++)
		{
			version = edt_read_lxt3108_ppr(edt_p, i, GLOBAL_BANK, 0x1);
			printf("LXT3108 chip %d - version %02x\n", i, version);
		}

		/* set all receive channels to e1 by writting to bank 9 of both chips */
		/* check by printing registers for channel 0 */
		for(i = 0; i < MAXCHIPS; i++)
		{
			lxt3108_e1_init(edt_p, i, ALL_CHAN_WRITE, loopback);
			lxt3108_dump_ppr(edt_p, i, CHAN_BANK_BASE);
		}
  
		/* set for global read/write  */
		edt_write_lxt3108(edt_p, 0, 0x0, 0) ;

		edt_close(edt_p);
	}
	else
		fprintf(stderr, "No board found (unit=%d)\n", unit);

	return 0;
}


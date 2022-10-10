/* #pragma ident "@(#)set_ss_vco.c	1.16 10/21/04 EDT" */

#include <stdio.h>
#include <math.h>

#include <string.h>

#include <stdlib.h>

#include "edtinc.h"

#include "edt_vco.h"
#include "edt_ss_vco.h"


void
usage(char *pname)

{

	fprintf(stderr, "usage: %s [-u unit] [-c channel] [-d no divide by 2] <target freq id> <clock channel>\n", pname);
	fprintf(stderr, " id = freq (Mhz)\n");
	fprintf(stderr, "  0 =  1.544 \n");
	fprintf(stderr, "  1 =  2.048 \n");
	fprintf(stderr, "  2 =  6.312 \n");
	fprintf(stderr, "  3 =  8.448 \n");
	fprintf(stderr, "  4 = 34.368 \n");
	fprintf(stderr, "  5 = 44.736 \n");
	fprintf(stderr, "usage: %s [-u unit] [-c channel] [-d no divide by 2] -F <target freq> <clock channel>\n", pname);

	exit(1);
}

/* Test clock frequencies against a set of target frequencies */
/* Only report output if clock frequency is within 25 ppm (0.000025) */
/* of target */

#define MAX_N_TARGETS 10

/* targets for SS Project */

double targets[MAX_N_TARGETS] = 
{
	1544000.0,
	2048000.0,
	6312000.0,
	8448000.0,
	34368000.0,
	44736000.0,
	0.0
};


int main(int argc, char **argv)
{

	int i;
	int verbose = 1;

	double xtal = 10368100.0;

	double actual;

	double delta;

	double target = 0.0;
	
	edt_pll pll;


	EdtDev *edt_p;

	int unit = 0;
	int channel = 0;

	int freq_id = 0;
	int clock_channel = 0;
	int target_set = 0;

	int divide_by_2 = 1;
	int raw = 0;
	char *pname = argv[0];

	/* EDTucd uses a 40Mhz clock */
	if (strncmp(EDT_INTERFACE, "ucd", 3) == 0)
	    xtal = 40000000.0;

	/* take target from command line someday */

	/* take target from command line someday */

 
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
		case 'c':
           ++argv;
            --argc;
            channel = atoi(argv[1]);
            break ;

		case 'x':
			++argv;
			--argc;
			xtal = atof(argv[1]);
			break;

		case 'd':
		  divide_by_2 = 0;
		  break;

		case 'F':
			++argv;
			--argc;
			target = atof(argv[1]);
			target_set = 1;
			break;

		case 'v':
		  verbose = 1;
		  break;

		case 'r':
		  raw = 1;
		  break;

		default:
		  usage(pname);
		  
		  break;

		}

		++argv;
		--argc;
		
	}

	
	if (target_set)
	  {
		if (argc > 1)
			clock_channel = strtol(argv[1], 0, 0);
		
	  }
	else if (argc >= 2) {

		freq_id = strtol(argv[1], 0, 0);
		clock_channel = strtol(argv[2], 0,0);

		if (freq_id >= 0 && freq_id < 6)
			target = targets[freq_id];

	} 
	else
	{

		usage(pname);
		exit(1);

	}

	
   if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
    {
    }

	/* compute edt_pll values for target frequency */

   
	if (target != 0.0)
	{

		if (raw)
		  actual = edt_find_vco_frequency_ics307_raw(NULL,target,
					xtal, &pll, verbose);
		else if (divide_by_2 && target <= 100000000.0)
		  actual = edt_find_vco_frequency_ics307(NULL,target,
					xtal, &pll, verbose);
		else
		  actual = edt_find_vco_frequency_ics307_nodivide(NULL,target,
														  xtal,&pll,verbose);

		delta = fabs(target  - actual);

		delta /= target;

		if (edt_p)
		{
			edt_set_out_clk_ics307(edt_p, &pll, clock_channel);
					

			edt_close(edt_p);
		}

	}
	else
	{

		for (i=0;i<6;i++)
		{

			target = targets[i];

			actual = edt_find_vco_frequency_ics307(NULL,target,
						xtal, &pll, verbose);

			delta = fabs(target  - actual);

			delta /= target;

			if (verbose)
			{
			printf("Clock %8.3f target %10.6f actual %10.6f delta (ppm) %8.3f vdw %3d rdw %3d od %3d h %3d l %3d x %3d\n",
				xtal, 
				target / 1000000, actual / 1000000, delta * 1000000, pll.VDW, pll.RDW, pll.OD, pll.h, pll.l, pll.x);
			}


		}


	}



	return 0;
}


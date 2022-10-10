#include <stdio.h>
#include <math.h>

#include <string.h>

#include <stdlib.h>

#include "edtinc.h"

#include "edt_vco.h"
#include "edt_ss_vco.h"




printdata(double target, double actual, double delta)
{
	double ppm25;

		printf("crystal target  %10.6f actual %10.6f delta (ppm) %8.3f\n", 
			target / 1000000, actual / 1000000, delta * 1000000);
			ppm25 = target * 0.000025 ;
		printf("25ppm from actual %10.6f %10.6f\n",
			(actual - ppm25) / 1000000.0,
			(actual + ppm25) / 1000000.0) ;
		printf("50ppm from target %10.6f %10.6f\n",
			(target - ppm25 * 2.0) / 1000000.0,
			(target + ppm25 * 2.0) / 1000000.0) ;
}

		
void
usage(char *pname)

{

	fprintf(stderr, "usage: %s [-u unit]\n");
	exit(1);
}

/* Tests crystal and all pll channels, one frequency each  */
/* will pause for testing with frequency counter for each test */
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
	44376000.0,
	0.0
};


int main(int argc, char **argv)
{

	int verbose = 0;

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
	u_char pll_debug ;


	char *pname = argv[0];


	--argc;
	++argv;
	while (argc && argv[0][0] == '-')
	{
	    switch (argv[0][1])
	    {
	    case 'u':
		if(argc != 1)
		{
		++argv;
		--argc;
		unit = atoi(argv[0]);
		}
		break;
	    default:
		usage(pname) ;
	    	exit(1) ;
	    }
	    argc-- ;
	    argv++ ;
	}

	if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
	{
	}
	
	edt_intfc_write(edt_p, 0x22, pll_debug = 0x40) ;
		
		target = xtal;
	
		actual = edt_find_vco_frequency_ics307(NULL, target,
					xtal, &pll, verbose);
	
		delta = fabs(target - actual);

		delta /= target;
	
		printdata(target, actual, delta);

		printf("check frequency counter and hit enter to proceed\n") ;
		getchar();

		for (clock_channel = 0 ; clock_channel < 4 ; clock_channel++)
		{
	    /* set debug pin for output from this channel */
	    switch(clock_channel)
	    {
	    case 0:
	    	pll_debug = 0 ;
		break ;
	    case 1:
	    	pll_debug = 0x10 ;
		break ;
	    case 2:
	    	pll_debug = 0x20 ;
		break ;
	    case 3:
	    	pll_debug = 0x30 ;
		break ;
	    }
	
		
		edt_intfc_write(edt_p, 0x22, pll_debug ) ;
		
		for (freq_id = clock_channel + 1 ; freq_id < MAX_N_TARGETS ;)
		{
		target = targets[freq_id];
		if (target == 0.0) break ;

		actual = edt_find_vco_frequency_ics307(NULL,target,
				xtal, &pll, verbose);

		delta = fabs(target  - actual);
	
		delta /= target;

		printdata(target, actual, delta);

		edt_set_out_clk_ics307(edt_p, &pll, clock_channel);
#ifdef USE_SET_SS
		{
		char cmd[80] ;
		sprintf(cmd,"./set_ss_vco %d %d",freq_id,clock_channel) ;
		system(cmd) ;
		}
#endif
				
		printf("check frequency counter and hit enter to proceed\n") ;
		getchar();
		    
		break;
		}
	}
	edt_close (edt_p) ;

	return 0;
	


}

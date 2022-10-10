
#include "edtinc.h"
#include "edt_ss_vco.h"

#include <assert.h>

#include <math.h>
/*********************************************************

/* Mapping in edt_pll ftom av9110 to ics307 */

/* v = VCO Divider Word (VDW) range 4 - 511 */

/* r = Reference Divider Word (RDW) range  1-127 */

/* m = Output Divide (OD) values: 2 - 9, 10 */

#define HIGH_VCO_307 400000000
#define LOW_VCO_ics307 55000000
#define MIN_RDW_FREQ	200000

static int test_loops = 0;

typedef struct _target_ics307
{
    double  target;
    double  actual;

    edt_pll *pll;


    int     hits;
    int     finished;
}       target_ics307;


void
print_ics307_parameters(double target, double actual, double reference, edt_pll * pll)

{

    double  check;


    printf("ICS 307 computations for reference frequency %f\n", reference);

    printf("\nClosest frequency to target frequency %3.5f is %3.5f\n",
	   target, actual);
    printf("Parameters VDW=%d, RDW=%d, OD=%d, LX=%d\n",
	   pll->VDW, pll->RDW, pll->OD,pll->x);

    check = reference * (pll->VDW + 8) / ((pll->RDW + 2) * pll->OD * (pll->x+1));

	if (pll->l)
	{

	/* no final divide by 2 */
	check *= 2;

	printf("\n--- Note this requires a bitfile which supports no final divide by 2!!!  ---\n\n");

    printf("Computations:\nF  = Ref * 2 * (VDW+8) / ((RDW+2) * (OD) * (LX+1))\n");

    printf("%3.5f = %3.5f * 2 * (%d+8) / ((%d+2) * (%d) * (%d))\n",
      actual, reference, pll->VDW, pll->RDW, pll->OD, pll->x+1);

	}
	else
	{

    printf("Computations:\nF  = Ref * 2 * (VDW+8) / ((RDW+2) * (OD) * (LX+1) * 2)\n");

    printf("%3.5f = %3.5f * 2 * (%d+8) / ((%d+2) * (%d) * (%d) * 2)\n",
      actual, reference, pll->VDW, pll->RDW, pll->OD, pll->x+1);


	}

	printf("Check = %3.5f\n", check);

}


/*
 * foreach possible setting of the L (1 to 64) and X (1 to 256) dividers
 * check if the output frequency is closer to the target than the last
 * closest. Notice the last divide by 2 is fixed for clock symetry. note that
 * the F_LOW restriction only applies when either X or L is greater than 1.
 */

static void
div_out_ics307(int h, int vdw, int rdw, int od, double xtal, double av_out, 
			   target_ics307 * targ, 
			   int lastdivide)

{
    int     val;
    double  oddclkout;
    double  serialclk;
    int     minx = 1;
    int     maxx = 64 * 256;

	int divider = lastdivide;


    oddclkout = av_out / h;

    /* target xl value */

	if (lastdivide)
	{
		double  xl;

		xl = oddclkout / (targ->target * lastdivide);

		minx = (int) xl - 1;
		if (minx < 1)
		minx = 1;
		maxx = (int) xl + 1;

		if (maxx > 64 * 256)
		maxx = 64 * 256;

	}
	else
	{
		minx = maxx = 1;
		divider = 1;
	}

    for (val = minx; val <= maxx; val++)
    {

	if (val == 1 || ((oddclkout <= F_SS_LOW)))

	{
	    serialclk = oddclkout / (val * divider);

	    if ((targ->hits == 0) ||
		(fabs(targ->actual - targ->target) > fabs(serialclk - targ->target)))
	    {
		/* update the target structure */
		targ->actual = serialclk;
		targ->pll->VDW = vdw;
		targ->pll->RDW = rdw;
		targ->pll->OD = od;
		targ->pll->h = h;

		/* store x as a single 14 bit value */

		targ->pll->x = val-1;

		targ->hits++;

		if (targ->actual == targ->target)
		    targ->finished = 1;

	    }
	}

	test_loops++;

    }

}

/*
 * check all av9110 output frequencies for legality to enter the xilinx. If
 * ok call all possible high speed odd dividers
 */

static void
av_outputs_ics307(int vdw, int rdw, int od, double xtal, 
				  double av_out, target_ics307 * targ, 
				  int last_divide)

{
    if ((av_out) <= F_XILINX_307)
    {

	/*
	 * for all possible divsors of the high speed odd divider in the
	 * xilinx
	 */

	div_out_ics307(1, vdw, rdw, od, xtal, av_out, targ, last_divide);

    }
}



/*
 * do all n to find legal vco frequencies for given ref and v
 */
static void
all_vdw_ics307(int od, double xtal, double ref, target_ics307 * targ,
	       int last_divide)

{
    int     vdw = 4;
    int     rdw;
    double  vco;
    double  vref;

    double  workref;

    int     max_rdw = (int) (xtal / MIN_RDW_FREQ) - 2;

    vref = (vdw + 8) * xtal;

    while ((vdw < 511) && (2 * (vref / 128) <= HIGH_VCO_307))
    {
	vref = (vdw + 8) * xtal;

	/*
	 * for each legal vco freq check all values of r
	 */
	/*
	 * -debug printf("vco = %3.5f (m=%d,  n=%d, v=%d)\n", vco, m, n, v);
	 */

	workref = 2 * vref;

	for (rdw = 1; rdw < max_rdw; rdw++)
	{

	    vco = workref / (double) (rdw + 2);

	    if ((vco) >= LOW_VCO_ics307)
	    {
		if (vco <= HIGH_VCO_307)
		{
		    vco /= od;

		    av_outputs_ics307(vdw, rdw, od, xtal, vco, targ, last_divide);
		}
	    }
	    else
		break;

	    if (targ->finished)
		return;
	}


	vdw++;

    }
}

/*
 * print all possible vco frequencies for the refernce frequency passed
 * (xtal/m).
 */

static void
calc_vco_freq_ics307(double xtal, target_ics307 * targ, int last_divide)

{
    double  ref;
    int     od;

    /*
     * call n rates for v is 1 and 8
     */

	printf("calc vco freq last_divide = %d\n", last_divide);

    for (od = 2; od <= 10; od++)
    {
	if (od != 9)
	{

	    ref = xtal * 2 / od;

	    all_vdw_ics307(od, xtal, ref, targ, last_divide);

	    if (targ->finished)
		return;

	}
    }

}

double
edt_find_ss_vco_frequency(EdtDev * edt_p, double target,
			      double xtal, edt_pll * pll, int verbose, int nodivide)

{

    target_ics307 targ;

    edt_pll dummypll;

    int     minm, maxm;
	int div_value;



    if (!pll)
	pll = &dummypll;

    edt_dtime();


    if (xtal == 0)
    {
	switch (edt_p->devid)
	{
	case PSS16_ID:
	case PSS4_ID:
	case PGS16_ID:
	case PGS4_ID:
		xtal = XTAL_SS;
		break;

	default:
	    xtal = XTAL_SS;
	    fprintf(stderr, "WARNING -- SS vco set requires ICS307 PLL and PCISSgot invalid device ID (%d) -- using %f...?\n", edt_p->devid, XTAL_SS);

	    break;

	}
    }


    /*
     * if ( (minm = (int) (xtal/HI_REF)) < 3) minm = 3; if ( (maxm = (int)
     * (xtal/LOW_REF)) > 127) maxm = 127;
     */

    minm = 4;
    maxm = 511;

	memset(&targ, 0, sizeof(targ));

	targ.actual = 0;
    targ.hits = 0;		/* zero means no updates have been made */
    targ.finished = 0;
    targ.target = target;
	targ.actual = 0;

    targ.pll = pll;

	switch (nodivide)
	{
	case 0:
		div_value = 2;
		break;
	case 1:
		div_value = 1;
		break;
	case 2:
		div_value = 0;
		break;
	}


	targ.pll->l = nodivide;

    calc_vco_freq_ics307(xtal, &targ, div_value);

    if (verbose)
    {
		print_ics307_parameters(targ.target, targ.actual, xtal, pll);
		printf("Elapsed: %10.6f seconds\n", edt_dtime());
    }

    return targ.actual;

}


double
edt_find_vco_frequency_ics307(EdtDev * edt_p, double target,
				    double xtal, edt_pll * pll, int verbose)

{
	return edt_find_ss_vco_frequency(edt_p,target,xtal, pll, verbose,0);
}

double
edt_find_vco_frequency_ics307_nodivide(EdtDev * edt_p, double target,
				    double xtal, edt_pll * pll, int verbose)

{
	return edt_find_ss_vco_frequency(edt_p,target,xtal, pll, verbose,1);
}

double
edt_find_vco_frequency_ics307_raw(EdtDev * edt_p, double target,
				    double xtal, edt_pll * pll, int verbose)

{
	return edt_find_ss_vco_frequency(edt_p,target,xtal, pll, verbose,2);
}

/*
 * set phase lock loop clock
 */
double
edt_set_frequency_ics307(EdtDev * edt_p,
			 double ref_xtal,
			 double target,
			 int clock_channel,
			 int finaldivide)
{
    double  freq = 0;


    edt_pll clkset;

    freq = edt_find_ss_vco_frequency(edt_p, target, ref_xtal, &clkset,
					 FALSE, finaldivide);

    if (freq  > 0) 
		edt_set_out_clk_ics307(edt_p, &clkset, clock_channel);

    return freq;

}

/*
 * set phase lock loop clock
 */
double
edt_set_frequency_fcipcd(EdtDev * edt_p,
			 double target)
{
    double  freq = 0;
	int clock_channel = 0;
    edt_pll clkset;
	double ref_xtal = XTAL_SS;

    freq = edt_find_ss_vco_frequency(edt_p, target, ref_xtal, &clkset,
					 FALSE, 2);

    if (freq  > 0) 
		edt_set_out_clk_ics307(edt_p, &clkset, clock_channel);

    return freq;

}

/*
 * assume the ICS307 is already selected assume the ICS307 clock is low and
 * leave it low output the number of bits specified from the lsb until done
 */
static void
shift_ics307(EdtDev * edt_p, u_int data, u_int numbits)
{
    /*
     * printf("shift %d bits from bit8 of %08x\n", numbits, data);
     */
    while (numbits)
    {


	if ((data >> (numbits - 1)) & 0x01)
	{
	    edt_reg_or(edt_p, EDT_SS_PLL_CTL, EDT_SS_PLL_DATA);
	    /*
	     * printf("one ");
	     */
	}
	else
	{
	    edt_reg_and(edt_p, EDT_SS_PLL_CTL, ~EDT_SS_PLL_DATA);
	    /*
	     * printf("zero ");
	     */
	}

	/* clock it in */
	edt_reg_or(edt_p, EDT_SS_PLL_CTL, EDT_SS_PLL_CLK);
	edt_reg_and(edt_p, EDT_SS_PLL_CTL, ~EDT_SS_PLL_CLK);

	numbits--;
    }
    /*
     * printf("\n");
     */
}


/*
 * Array of values used as S2-S0 on ics307 ; index using OD value from
 * edt_pll structure
 */

static int SFromCLK1[11] = {-1, -1, 1, 6, 3, 4, 7, 5, 2, -1, 0};

/*
 * first 5 bits clocked in to ics307 c1 = 0 c0 = 0 TTL = 1 F1 = 1 F0 = 0
 */

#define ICS307_CAP_TTL_CLK2 0x06


void
edt_set_out_clk_ics307(EdtDev * edt_p, edt_pll * clk_data, int clock_channel)
{
    unsigned short opt_e = 0;

    u_int   out_scale;
    u_int   ref_scale;

    u_int   clk1_divide;

    u_int   strobe_bit;

	/* note that 0xffff is reserved for no final 
	   divide by 2 */

	if (clk_data->l != 2)
	{

		if (clk_data->l)
			opt_e = 0xffff;
		else
			opt_e = (clk_data->x) << EDT_X_DIVN_SHFT;

		switch (clock_channel)
		{
		case 0:
		out_scale = EDT_SS_PLL0_CLK;
		ref_scale = EDT_SS_PLL0_X;
		break;

		case 1:
		out_scale = EDT_SS_PLL1_CLK;
		ref_scale = EDT_SS_PLL1_X;
		break;
		case 2:
		out_scale = EDT_SS_PLL2_CLK;
		ref_scale = EDT_SS_PLL2_X;
		break;
		case 3:
		out_scale = EDT_SS_PLL3_CLK;
		ref_scale = EDT_SS_PLL3_X;
		break;
		default:
		assert(0);
		}

		edt_reg_write(edt_p, out_scale, opt_e & 0xff);
		edt_reg_write(edt_p, ref_scale, opt_e >> 8);

	}
    /* shift out data */

    /* fisrt 5 bits alwasy the same */


    shift_ics307(edt_p, ICS307_CAP_TTL_CLK2, 5);

    clk1_divide = SFromCLK1[clk_data->OD];

    shift_ics307(edt_p, clk1_divide, 3);

    shift_ics307(edt_p, clk_data->VDW, 9);
    shift_ics307(edt_p, clk_data->RDW, 7); 

	printf("Programming value %x\n",
		(5 << 19) | (clk1_divide<< 16) | (clk_data->VDW << 7) | (clk_data->RDW));

    switch (clock_channel)
    {
    case 0:
	strobe_bit = EDT_SS_PLL_STROBE0;
	break;
    case 1:
	strobe_bit = EDT_SS_PLL_STROBE1;
	break;
    case 2:
	strobe_bit = EDT_SS_PLL_STROBE2;
	break;
    case 3:
	strobe_bit = EDT_SS_PLL_STROBE3;
	break;
    default:
	assert(0);
    }
    edt_reg_or(edt_p, EDT_SS_PLL_CTL, strobe_bit);
    edt_reg_and(edt_p, EDT_SS_PLL_CTL, ~strobe_bit);

}


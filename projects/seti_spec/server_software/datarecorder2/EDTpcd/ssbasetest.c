/***************************************************************************
                          ssbasetest.c  -  test SS<->mezzanine board connections 
                             -------------------
 * Test program for use on the SS main card (with Seb's test mezzanine board)
 *  to test the connections  between the main board and the testing mezzanine board.
 * It works as follows:
 * After opening the card, we write some numbers into some xilinx registers (main xilinx)
 * which then are sent out across the connection and fall into some other registers
 * back on the mainboard's xilinx.  We then read what is in the destination register
 * and compare it to what we just wrote. If the mezzanine board is not actually connected,
 * there should be a certain pattern to the registers we read.
 * See basetest.txt for more info on the hardware of the board.
 *
 * Usage: sslooptest [-u UnitNumber] (see usage(), or ssbasetest -h)
 *
 * --Peter W (peter@edt.com)
 ***************************************************************************/

/* TODO:
 * 1a) create an edt_getopt library 
 * 		(nt doesn't conform to posix.2 (terminal stuff)).
 * 2e) clean up code so it fits in 80 column terminals. 
 * 		(well, most of it at least).
 * 2h) remove any unnecesary ifdef DEBUGs.
 * 3) do final testing.
 * 4) remove old (and should be commented as such) code.
 * + Perhaps add an option to pause after each error (so user can run pdb).
 * + add a "test basic xilinx functionality" which would write to registers 00->09
 *     and verify it can read them (0x00->0x09) back. This would detect if there is 
 *     a basic problem, like the xilinx not being loaded.
 *     (If xilinx isn't loaded, 0xcc is usually the default value of ALL registers).
 */


/* CHANGES:
 * Version 0.1.9 (2001-01-15):
 * 	Fixed straight through test so it checks "0x 0100 0100" (0x44) 
 * 		in addition to 0x00, 0x11, 0x22, and 0x88.
 *  testBoardPresence is now explicitly defined 
 *		(was defined as testBoardPresent...oops.) 
 *  testGP now returns tristate to its default (off) position.
 * Version 0.2.0
 *  testGP now thinks that register 1a should be 10 (was 90), which s/b right.
 * Version 0.2.1
 *  testStraightThrough now "walks a one" and a zero up the registers.
 *		It used to just test 0x11, 0x22, 0x44, etc.
 * Version 0.2.2
 * 	error messages now print in binary instead of hex. more readable.
 * Version 0.2.3: 
 *	fixed bug in testStraightThrough() (bug with testing the 0D register), 
 *	removed duplicate error message from testGP(). 
 *  made testGP() error message more pretty (lined up numbers, added newline).
 * Version 0.2.4: 
 * 	Dan decided that in the Gnd/Pwr test, 
 * 		reg 1a should read 0x90 (instead of 0x10). 
 *		So the testGP () function has been changed to reflect that. 
 *		(As of 2002-03-22).
 * Version 0.2.5 (2002-03-25):
 *	Cleaned up code a bit. Fixed some tab issues. Most stuff fits in 80 cols.
 *	Made testStraightThrough() more thorough. It now makes sure there aren't 
 *		any shorts between registers. It should be what Dave wanted now.
 *	Output for usage() and testGP() now fits in 80 columns (or should). 
 *	fixed output alignment and newlineage for testGCLK() and some others down there.
 * Version 0.2.6 (2002-03-26):
 * 	Changed tristate output messages; There is now only one note about tristate 
 * 		(in the beginning).
 * 	Removed "int verbose" argument on the functions, because it wasn't used.
 *	I'm now moving all messages regarding whether tests worked into main().
 * Version 0.2.7 (2002-08-07):
 * 	"Card is good." message now has a newline after it.
 * Version 0.2.10 (2004-08-20):
 *  bail if bitload fails.  print more bitload output. clean up comments.
 * Versio 0.2.11 (2004-09-1):
 *  code cleanup; some support for gs in testGP().
 *  
 */


/* INCLUDEs here: */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "edtinc.h"

/* DEFINEs here: */
#define SS_VERSION "0.2.11"

#define WRITEREG0      0x01010000
#define WRITEREGA      0x0101000a
#define WRITEREGD      0x0101000d

#define READREG0       0x01010010
#define READREGD       0x0101001d
#define READREG20      0x01010020

#define BUSMODEREG     0x0101001e
#define TRISTATEREG    0x0101000f

/* TRISTATE:
 * ff = enable
 * 00 = disable
 */

/* FUNCTION DECLARATIONS here. */
static void usage (char *progname);
int testBoardPresence (EdtDev *edt_p);
int testStraightThrough (EdtDev *edt_p, int cross_shorts);
int testBusmodes (EdtDev *edt_p);
int testGclk (EdtDev *edt_p);
int testGP (EdtDev *edt_p); 
char *print_binary(unsigned int number, char *buffer);


int main (int argc, char **argv)
{
	/* Declare Stuff Here: */
	EdtDev *edt_p;
	int unit = 0, errors = 0, testResult = 0;
	char devname[128]; 
	char errstr[64];
	char commandstr[40];
	int command_status;

	char *progname;
	char *unitstr = "0";

	int aflag = 1; /* run all tests (default) */
	int bflag = 0; /* run board-presence test */
	int sflag = 0; /* run straight-through test */
	int mflag = 0; /* run busmodes test */
	int gflag = 0; /* run g-clk test */
	int pflag = 0; /* run ground/power test */
	int vflag = 0; /* verbose */
	int Vflag = 0; /* version */
	int xflag = 1; /* check for cross-register shorts in testStraightThrough() */
	int hflag = 0; /* help */
	int bitflag = 1; /* run bitload. -B disable */
	int loops = 0; /* number of times to run test */

	progname = argv[0];
	--argc;
	++argv;

	/* while ((c = getopt (argc, argv, "u:absmgpvVh")) != -1) */
	/* switch (c) */
	/* I didn't use this b/c Win** doesn't have getopt. */
	while (argc > 0 && argv[0][0] == '-')
	{
		switch (argv[0][1])
		{
			case 'u':
				/* unit = atoi(optarg); */
				++argv;
				--argc;
				unitstr = argv[0];
				break;

			case 'a': /* The user wants to run all tests */
				/* (all tests is default, so this option would only negate -A if that were specified) */
				aflag = 1;
				break;

			case 'A': /* do not run all tests.  individual tests should now be specified */
				aflag = 0;
				break;

			case 'b':
				bflag = 1;
				break;

			case 'B':
				bitflag = 0; /* no bitload */
				break;

			case 's':
				sflag = 1;
				break;

			case 'm':
				mflag = 1;
				break;

			case 'g':
				gflag = 1;
				break;

			case 'p':
				pflag = 1;
				break;

			case 'v':
				if (!vflag) 
					vflag = 1;
				else if (vflag == 1)
					vflag = 2;
				break;

			case 'V':
				Vflag = 1;
				break;

			case 'x':
				xflag = 1; 
				break;

			case 'X':
				xflag = 0; 
				break;

			case 'l':
				if (argc > 1) {
					++argv;
					--argc;
					loops = atoi(argv[0]);
				} else {
					fprintf(stderr, "-l requires an integer argument specifying # loops\n");
				}
				break;

			case '-':
				/* note: if for some reason --HELP should be supported, 
				 * use strcasecmp() */
				if ( strcmp (argv[0], "--help") == 0 )
					hflag = 1;
				else
				{
					fprintf(stderr,"The only long option supported by this program is");
					fprintf(stderr," --help.\n");
					fprintf(stderr,"Your option was: %s.\n", argv[0]);
				}
				break;

			case 'h':
				hflag = 1;
				break;

			default:
				if ( strlen (argv[0]) <= 1 ) 
				{
					if (isprint (argv[0][1]))
					{
						fprintf (stderr, "Unknown option `-%c'.", argv[0][1]);
						fprintf(stderr, "  Try %s -h for help.\n", progname);
					}
					else
					{
						fprintf (stderr,"Unknown option character `\\x%x'.", argv[0][1]);
						fprintf (stderr, "  Try %s -h for help.\n", progname);
					}
				}
				else
				{
					fprintf(stderr, "Non-option argument '%s'.", argv[0]);
					fprintf(stderr, "  Try %s -h for help.\n", progname);
				}
				return 1;
				break;
		}
		--argc;
		++argv;
	}
	unit = edt_parse_unit(unitstr, devname, EDT_INTERFACE);	


	/* printf verbose information if desired. */	
	if (vflag == 2)
	{
		printf("unit is: %d.\n",unit);
		printf("Tests to run: "); 	
		if (hflag) 
		{
			printf("none, help message being displayed.");
			usage(progname);
		}
		else if (aflag) 
			printf("all.\n");	
		else {
			if (bflag) 
				printf("board presence. ");
			if (sflag) 
				printf("straight through. ");
			if (mflag) 
				printf("bus modes. ");
			if (gflag) 
				printf("GCLK ");
			if (pflag) 
				printf("gound/power pins.");
			printf("\n");
		}
	}
	else if (hflag) /* Or print help message */
		usage(progname);
	
	/* print version info if desired. */
	if (Vflag == 1) 
		printf("ssbasetest version: " SS_VERSION "\n"); 

	if (hflag || Vflag) /* help or version printed; we're done. */
		return 0;

	/* make sure the user will run _some_ test, if not, quit */
	if (!(aflag || bflag || sflag || mflag || gflag || pflag ))
	{
		fprintf(stderr,"No test specified; I quit! (Seek --help if needed).\n");
		return 1;
	}

	/* According to basetest.txt from Seb, we must first make sure 
	 *   pciss16 is loaded and so is basetest.bit. 
	 * Do that here: 
	 */
	if (vflag > 1) 
	{
		printf("Please make sure you are using a pciss16, or pciload it if not\n");
		sprintf(commandstr, "bitload -v -u %d basetest.bit", unit);
	} else if (vflag == 1)
		sprintf(commandstr,"bitload -u %d basetest.bit", unit);
	else 
		sprintf(commandstr,"bitload -q -u %d basetest.bit", unit);

	if (bitflag) {
		command_status = edt_system(commandstr);
		if (command_status)
		{
			fprintf(stderr, "bitload failed (exit status: %d); basetest cannot continue\n", command_status);
			exit(1);
		}
	}
	


	if (vflag)
		printf("Note: Tristate is only enabled for the Ground and Power test.\n\n");


	/* Open the EDT Card: */
	if ((edt_p = edt_open(devname,unit)) == NULL ) 
	{
		sprintf(errstr, "edt_open(%s%d)", devname, unit);
		edt_perror(errstr);
		return 1;
	}

	/* Now that the card is open for business, lets TEST SOME REGISTERS. ;) */
	/* Sebastion said it'd be best to write to all the write registers (00 -> 0E),
	 * then read all the read registers (10-1E) and compare the two.  After that, 
	 * set the tristate register (0F) and read all the read registers: they should 
	 * be in a certain pattern.
	 * Note: See pdb.c lines 680 -> 800 for info on the register addresses.
	 */

	/* Test if the board is connected: */
	if (aflag || bflag)
	{	
		if (vflag) 
			printf("Testing for mezzanine board presence: "); 
		testResult = testBoardPresence(edt_p);
		errors += testResult;

		if (!testResult) /* if testResult was 0 */
		{
			if (vflag)
				printf("GOOD.\n");
		}
		else if (testResult == 14)
		{
			fprintf(stderr,"Aggcch! No board connected! Exiting...\n");
			return 1;
		}
		else if ( (testResult > 0) && (testResult < 14))
		{
			fprintf(stderr, "It looks like the mezzanine board is half connected? ");
			fprintf(stderr, "Weird. %d of the 14 registers match the no-board pattern.\n", testResult);
		}
	}


	/* Now test the straight through pins: */
	if (aflag || sflag)
	{
		if (vflag)
			printf("Testing the straight-through pins  : "); 
		testResult = testStraightThrough(edt_p, xflag);
		errors += testResult;
		if (testResult)
		{
			fprintf(stderr,"WARNING: Errors were detected with the straight through pins;\n");
			fprintf(stderr,"         this may cause errors with the ground and power pins test.\n\n");
		}
		else if (vflag)
			printf("GOOD.\n");
	}

	/* Now test the busmodes; yes, all of them */
	if (aflag || mflag) 
	{
		if (vflag)
			printf("Testing the Bus Modes              : "); 
		testResult = testBusmodes(edt_p);
		errors += testResult;
		if (testResult) {
			fprintf(stderr,"ERROR: there was a problem with the busmodes.\n");
		} else if (vflag)
			printf("GOOD.\n");
	}	

	/* Now test that the GCLK is working. */
	if (aflag || gflag)
	{
		if (vflag)
			printf("Testing the GCLK                   : ");	
		testResult = testGclk(edt_p);
		errors += testResult;
		if (testResult) {
			fprintf(stderr,"ERROR: there was a problem with the glck.\n");
		} else if (vflag)
			printf("GOOD.\n");

	}

	/* Now test the ground and power pins. wee, almost done! :) */
	if (aflag || pflag)
	{	
		if (vflag)
			printf("Testing the Ground and Power pins  : "); 
		testResult = testGP(edt_p);
		errors += testResult;
		if ( testResult == 0 ) {
			if (vflag)
				printf("GOOD.\n");
		} else 
		{
			fprintf(stderr,"Well, there was an error with the ground and power pins,\n");
			fprintf(stderr,"with %d of 14 registers not being what they should be!\n\n", testResult);	
		}
	}

	if (vflag)
	{
		printf("Done with tests.\n");
		printf("Found %d errors total.\n", errors);
	}

	if (!errors)		
		puts("Card is good.");
	else 
		puts("Card is bad.");

	/* Close the EDT Card: */
	if (edt_close(edt_p) < 0) {
		sprintf(errstr, "edt_close(%s%d)", devname, unit);
		edt_perror(errstr);
		return 1;
	}

	return 0;
}



void usage (char *progname) {
	puts("");
	printf("%s: program to test mainboard/mezzanine connections.\n",progname);
	puts("");
	printf("Usage: %s [-] [u unit number] [a|A[b][s][m][g][p]] [v[v]] [V] [h]\n",
			progname);
	puts("Test program for use on the SS main card.");
	puts("For use with Sebastion's loop back mezzanine board to test the");
	puts("CMC connections between the main board and the mezzanine board.");
	puts("");
	puts("-u n				uses board of unit number n - default is 0");
	puts("");
	puts("Test Modes : choose \"a\" for all tests, or some combination of the others");
	puts("-A				do NOT run all tests (use with specific test(s) below)");
	puts("-a				run all tests");
	puts("-b				check for board presence");
	puts("-s				check the straight through pins");
	puts("-m				test the bus modes");
	puts("-g				test the GCLK"); 
	puts("-p				test the ground and power pins");
	puts("");
	puts("Other options:");
	puts("-v				print verbose messages. ");
	puts("					Use -v -v for greater effect.");
	puts("");
	puts("-x        Check for cross register shorts in the straight through pins- default.");
	puts("-X        Do NOT check for cross register shorts in straight through pins.");
	puts("-V				print version information");
	puts("-h 				print this help message");
	puts("-B				Do NOT run bitload");
	puts("");	
}



int testBoardPresence (EdtDev *edt_p) {

	/* int testBoardPresence (EdtDev *edt_p, int verbose) 
	 * This function will test to see if the mezzanine board is connected to the
	 * mainboard.  It works by reading all the read registers with the tristate 
	 * disabled, and seeing if their values match the pattern that 
	 * means there is no mezzanine board attached. 
	 * This is what read registers look like if no board is present: 
	 * ----------------------------------
	 * ||    Rd Reg    |     Output    ||
	 * ----------------------------------
	 *      10      |       66
	 *      11      |       cd
	 *      12      |       6c
	 *      13      |       73
	 *      14      |       66
	 *      15      |       c2
	 *      16      |       ce
	 *      17      |       b6
	 *      18      |       bb
	 *      19      |       bb
	 *      1a      |       6b
	 *      1b      |       ab
	 *      1c      |       aa
	 *      1d      |       3a
	 *
	 * 
	 *
	 * Returns 0 if board is present, or the number of register matching the
	 * "no board" pattern.  If it returns 14, there isn't a mezzanine board connected.
	 */

	unsigned int address;
	unsigned int noboard[14] = 
	{ 0x66, 0xcd, 0x6c, 0x73, 0x66, 0xc2, 0xce, 
		0xb6, 0xbb, 0xbb, 0x6b, 0xab, 0xaa, 0x3a  };
	unsigned int readreg[14];

	int i, numEqual;

	/* disable tristate: */
	edt_reg_write(edt_p,TRISTATEREG, 0x00);

	/* Read all the read registers into the readreg array: */
	address = READREG0;
	for (i = 0; i < 14; ++i) 
	{
		readreg[i] = edt_reg_read(edt_p, address);
		++address;
	}

	/* Check to see if the board is connected: 
	 *	(if our array matches the noboard array) 
	 * Should I write 0x00s to the registers first? 
	 * Actually, bitload does that for me. 
	 */
	numEqual = 0;
	for ( i=0; i < 14; ++i) 
	{
		if ( readreg[i] == noboard[i] ) 
		{
			if (numEqual == 0) 
				fprintf(stderr, "\n"); /* This is for cosmetic effect. */
			fprintf(stderr,"WARNING: Register %x was %x, matching the no-board pattern.\n", 
					i, readreg[i]);
			++numEqual;
		}
	}

	return numEqual; 
}


/* verify a range of registers matches a given value
 * check that all registers between start_addr and end_addr 
 * (except skip_addr if it is within that range) match the given value.
 * If prev_errors is 0, a newline is printed for cosmetic reasons.
 * returns: number of errors found

 * If one of the wires for this register is shorted somewhere with wires
 * corresponding to another register, this check should find it (b/c all other registers 
 * have been set to zero right now).
 * Warning: If there is a normal short (that shows up in a regular straight-through test),
 * it will be shown repeatedly by this function.  At that point, you'll want to disable the cross shorts test. */
int verify_registers(EdtDev *edt_p, u_int start_addr, u_int end_addr, u_int skip_addr, u_int value, int prev_errors)
{
	char buffer0[9], buffer1[9]; /* strings to hold binary formatted numbers */
	int retval = 0; /* number of mismatches */
	int rvalue; /* read value */
	int raddress; /* read address */

	for (raddress = start_addr; raddress < end_addr; ++raddress) { /* for each register address in range */
		if (raddress == skip_addr)
		{
			++raddress; /* bypass skip_addr */
		}
		rvalue = edt_reg_read(edt_p, raddress);
		if (rvalue != value) /* Print an error if mismatch */
		{
			if (prev_errors == 0)
				fprintf(stderr, "\n"); /*cosmetic*/
			fprintf(stderr, "ERROR: Register 0x%x was %s,\n", raddress & 0xff, print_binary(rvalue, buffer0));
			fprintf(stderr, "                     s/b %s\n\n", print_binary(value, buffer1));
			++retval;
		}
	}
	return retval;
}



int testStraightThrough (EdtDev *edt_p, int cross_shorts) {

	/* int testStraightThrough (EdtDev *edt_p, int cross_shorts);
	 *
	 * This function tests the stright through pins 
	 * (if everything works, it'll read in the read registers whatever 
	 * it writes in the corresponding write register). 
	 * For this to work correctly, we have to turn off tristate.
	 * To ensure there are no shorts between the pins, this function will 
	 * use a binary counter to figure out what to write (it'll count and 
	 * write between 00 and ff).
	 *
	 * This function will return 0 if all goes well, 1 otherwise.
	 * 
	 */


	unsigned int waddress, raddress;

#define NUM_OF_REGISTERS 14 /*between 0x00 and 0x0d*/

	 /* It seems most simple to store the values for the binary walk in an array. */
#define WVALUE_LENGTH 18 
	unsigned int wvalue[WVALUE_LENGTH] = 
	{ 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,   /*walk a one up*/
		0xff, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f }; /*walk a zero up*/

#define WVALUE2_LENGTH 14   /* This is for Ox0d: */
	unsigned int wvalue2[WVALUE2_LENGTH] = 
	{ 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,  /*walk a one up*/
		0x3f, 0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0x1f }; /*walk a zero up*/
	/* note that nothing in the previous line is above 3 on the high byte,
	 * because those two wires are not connected. */

	unsigned int rvalue; /* read value */
	int retval = 0; /* return value */
	int i; /* loop variable */
	char buffer0[9], buffer1[9]; /* strings to hold binary formatted numbers */

	/* disable tristate: */
	edt_reg_write(edt_p,TRISTATEREG, 0x00);


	/* write 0x00 to all the registers. */
	for (waddress = WRITEREG0; waddress <= WRITEREGD; ++waddress)
	{
		edt_reg_write(edt_p, waddress, 0x00);
	}


	/* warn if anything is not 0x00. */
	for (raddress = READREG0; raddress < READREGD; ++raddress)
	{
		rvalue = edt_reg_read(edt_p, raddress);
		if (rvalue != 0x00)
		{
			if (retval == 0)
				fprintf(stderr, "\n"); /*cosmetic.*/
			fprintf(stderr, "ERROR: I just wrote 0x00 to all registers, " 
					"but read register 0x%x was 0x%x!\n", raddress & 0xff, rvalue);
			++retval;
		}
	}

	/* nested loop: 
	 * 1 to check each W# writes/reads ok, the other to do the first loop for 
	 * each of the 14 registers.
	 */
	for (waddress = WRITEREG0; waddress < WRITEREGD; ++waddress) { /* for each write register */

		/* Test this register with all the write values: */
		for ( i = 0; i < WVALUE_LENGTH; ++i) {  /* for each write value */
			edt_reg_write(edt_p, waddress, wvalue[i]);
			raddress = waddress + 0x10;
			rvalue = edt_reg_read(edt_p, raddress);  	

			if ( rvalue != wvalue[i] ) { /*make sure we read what we wrote. */
				if (retval == 0)
					fprintf(stderr, "\n"); /*cosmetic.*/
				fprintf(stderr,"ERROR: Register 0x%x was %s,\n", waddress & 0xff, print_binary(rvalue,buffer0));
				fprintf(stderr,"                    s/b %s\n\n",print_binary(wvalue[i], buffer1)); 
				++retval;
			}

			/* Search for "cross register shorts" if desired. 
			 * This is done by default, but can be disabled if necessary for debugging. */
			/* Verify all other read registers are what they should be.
			 * If one of the wires for this register is shorted somewhere with wires
			 * corresponding to another register, this check should find it (b/c all other registers 
			 * have been set to zero right now).*/
			/* note that we skip the register connected to waddress because we've already checked it above */
			if (cross_shorts) {
				retval += verify_registers(edt_p, READREG0, READREGD, waddress + 0x10, 0x00, retval); 
			}


			/* Write 0x00 back to write reg. */
			edt_reg_write(edt_p, waddress, 0x00);
		}
	}

	/* Now test register 0d. 
	 * Note that the highest 2 bits/wires are not connnected,
	 * so we don't test them. wvalue2[] is the array for the 0D register. */
	for ( i = 0; i < WVALUE2_LENGTH; ++i) {
		edt_reg_write(edt_p, waddress, wvalue2[i]);
		raddress = waddress + 0x10;
		rvalue = edt_reg_read(edt_p, raddress);  	
		if ( rvalue != wvalue2[i] ) {
			if (retval == 0)
				fprintf(stderr, "\n"); /*cosmetic.*/
			fprintf(stderr,"ERROR: Register 0x%x was %s,\n",waddress & 0xff, print_binary(rvalue,buffer0));
			fprintf(stderr,"                    s/b %s\n\n",print_binary(wvalue2[i], buffer1)); 
			++retval;
		}

		if (cross_shorts) {
			retval += verify_registers(edt_p, READREG0, READREGD, waddress + 0x10, 0x00, retval); 
		}

		/* Write 0x00 back to the write reg. */
		edt_reg_write(edt_p, waddress, 0x00);
	}

	return retval;
}




int testBusmodes (EdtDev *edt_p) {

	/* This function will test to make sure the busmodes are working correctly.
	 * You can use this function to test 1 or all of the busmodes 
	 * (there are 4 of them).
	 * The modes are: Busmode 1, Busmode 2, Busmode 3, Busmode 4.  
	 * To choose which one to test, pass it as the mode integer, or 0 for all.
	 *
	 * Example: testBusmodes(edt_p, 2)  will see if Busmode 2 is working.
	 *
	 * Excerpt from basetest.txt (from Sebastion):
	 * To test special cases:
	 *     Disable tri-state.
	 *     To test busmode 1, busmode 3, and busmode 4, read register 0x1e.  
	 *     The first three bits should always stay low (i.e. you should 
	 *     only read 0x04 [0000 1000] or 0x00 [0000 0000]. 
	 *     These are busmode 4, busmode 3, and busmode 1.
	 *     Busmode 2 is connected to the last bit in register 0x0a.  
	 *     To test busmode 2, write that bit high and low (i.e. 0x80 and 0x00,
	 *     respectively).  The fourth bit in register 0x1e (busmode 2) will 
	 *     go high or low (0x04 or 0x00) in relation to the last bit in 0x0a.
	 *
	 * Returns: the number of errors encountered (so, 0 == success). 
	 */


	int retval = 0;
	char buffer0[9], buffer1[9], buffer2[9];
	unsigned int read;

	/* disable tristate: */
	edt_reg_write(edt_p,TRISTATEREG, 0x00);


	/* read the busmode register */	
#ifdef DEBUG
	printf("Testing busmodes 1, 3, and 4 now.\n");
#endif
	if ((read = edt_reg_read(edt_p, BUSMODEREG)) != 0x04 && read != 0x00) 
	{
		if (retval == 0)
			fprintf(stderr, "\n"); /*cosmetic.*/
		fprintf(stderr,"ERROR: There was a problem with busmode 1, 3 , or 4.\n");
		fprintf(stderr,"        Register 0x1e was    %s,\n", print_binary(read, buffer0));
		fprintf(stderr,"        but s/b              %s \n", print_binary(0x00, buffer1));
		fprintf(stderr,"        or  s/b              %s\n\n",print_binary(0x04, buffer2));
		++retval;
	}

	/* do other stuff to test busmode 2: */
	/* test bm2 high: */	
	edt_reg_write(edt_p, WRITEREGA, 0x80);
	if ((read = edt_reg_read(edt_p, BUSMODEREG)) != 0x04) { 
		if (retval == 0)
			fprintf(stderr, "\n"); /*cosmetic.*/
		fprintf(stderr,"ERROR: Problem with busmode 2.\n");
		fprintf(stderr,"       Wrote   %s to 0x%x,\n", print_binary(0x80, buffer0), WRITEREGA & 0xff);
		fprintf(stderr,"       and got %s from 0x%x instead of %s!\n\n",  
				print_binary(read,buffer1), BUSMODEREG & 0xff, print_binary(0x04,buffer2)); 
		++retval;
	}	
	/* test bm2 low: */	
	edt_reg_write(edt_p, WRITEREGA, 0x00);
	if ((read = edt_reg_read(edt_p, BUSMODEREG)) != 0x00) { 
		if (retval == 0)
			fprintf(stderr, "\n"); /*cosmetic.*/
		fprintf(stderr,"ERROR: Problem with busmode 2.\n");
		fprintf(stderr,"       Wrote %s to 0x%x,\n", print_binary(0x00, buffer0), WRITEREGA & 0xff);
		fprintf(stderr,"       and got %s from 0x%x instead of %s!\n\n",
				print_binary(read,buffer1), BUSMODEREG & 0xff, print_binary(0x00, buffer2)); 
		++retval;
	}	

	return retval;
}




int testGclk (EdtDev *edt_p) {

	/* This function will test to make sure the gclk working correctly.
	 *
	 * Excerpt from basetest.txt (from Sebastion):
	 * To test special cases:
	 *     Disable tri-state.
	 *     The gclk is connected to the first bit in register 0x0d.  
	 *     To test gclk, write that bit high and low (0x01 and 0x00). 
	 *     The first bit in register 0x20 will go high and low (0x01 or 0x00).
	 *  
	 * Returns: 0 for no errors, non-zero if errors. The value returned is the # of errrors.
	 */


	int retval = 0;
	unsigned int read;
	char buffer0[9], buffer1[9], buffer2[9];

	/* disable tristate: */
	edt_reg_write(edt_p,TRISTATEREG, 0x00);

	/* test gclk high: */	
	edt_reg_write(edt_p, WRITEREGD, 0x01);
	if ((read = edt_reg_read(edt_p, READREG20)) != 0x01) { 
		if (retval == 0)
			fprintf(stderr, "\n"); /*cosmetic.*/
		fprintf(stderr,"ERROR: Problem with gclk high.\n");
		fprintf(stderr,"       Wrote %s to 0x%x,\n", print_binary(0x01,buffer0), WRITEREGD & 0xff);
		fprintf(stderr,"       and got %s from 0x%x instead of %s!\n\n", 
				print_binary(read,buffer1), READREG20 & 0xff, print_binary(0x01,buffer2)); 
		++retval;
	}	

	/* test gclk low: */	
	edt_reg_write(edt_p, WRITEREGD, 0x00);
	if ((read = edt_reg_read(edt_p, READREG20)) != 0x00) { 
		if (retval == 0)
			fprintf(stderr, "\n"); /*cosmetic.*/
		fprintf(stderr,"ERROR: Problem with gclk low.\n");
		fprintf(stderr,"       Wrote %s to 0x%x,\n", print_binary(0x00, buffer0), WRITEREGD & 0xff);
		fprintf(stderr,"       and got %s from 0x%x instead of %s!\n\n", 
				print_binary(read,buffer1), READREG20 & 0xff, print_binary(0x00, buffer2)); 
		++retval;
	}	

	return retval;
}



int testGP (EdtDev *edt_p) {

	/* int testGP (EdtDev *edt_p) 
	 * This function will test to see if the ground and power pins are 
	 * working correctly. 
	 *
	 * Note that this is the only function that turns on tristate, 
	 * and since tristate-off is normal, this function will turn 
	 * tristate back off when it is done.
	 *
	 * Returns the number of registers that didn't match the gnd/pwr array. 
	 * So it should return 0 if everything works, non-zero on error.
	 */
	unsigned int gp[14] = 
	{ 0x89, 0x32, 0x93, 0x8c, 0x99, 0x24, 0x11, 0x09, 0x00, 0x00, 0x90, 0x04, 0x00, 0x04 }; 
	/* reg 1a changed from 0x10 to 0x90 2002-03-22. */
	
	/* register 0x12 on GS reads differently than SS,
	 * b/c fourth from LSB pin is user I/O rather than ground or power, so 
	 * the value can be 0x93 or 0x98 depending on what the value of pin is. 
	 * gp_reg_12 specifies the value that can be correct on GS and not SS. */
	unsigned int gp_reg_12 = 0x9b; 
	int is_gs; /* true (non-zero) if board edt_p is a GS card */
	unsigned int readreg[14];
	int i, numWrong = 0;
	char buffer0[9], buffer1[9];

	switch (edt_device_id(edt_p)) {
		case PGS4_ID: /* should handle GS 1 and GS 4 channel. */
		case PGS16_ID:
			is_gs = 1;
			break;
		default:
			is_gs = 0;
			break;
	}

	/* enable tristate: */
	edt_reg_write(edt_p,TRISTATEREG, 0xFF);

	/* Read all the read registers into the readreg array: */
	for (i = 0; i < 14; ++i) 
	{
		readreg[i] = edt_reg_read(edt_p, READREG0 + i);
	}

	/* Check to see if the ground (gnd) and power pins work: 
	 * (if our array matches the gp array)*/
	for (i = 0; i < 14; ++i) {
		if ( readreg[i] != gp[i] ) {
			if (i == 2 && is_gs) {
				if (readreg[i] != gp_reg_12 /* and we already know its != gp[i] from above */) {
					/* the value of register 0x12 is not one of
					 * the possible values for the GS card. */
					if (numWrong == 0)
						fprintf(stderr, "\n"); /* start at newline after "Testing Gnd/Power: " */
					fprintf(stderr,"ERROR: read register 0x1%x was %s,\n",i,print_binary(readreg[i], buffer0));
					fprintf(stderr,"                      but s/b %s.\n\n",print_binary(gp[i], buffer1)); 
					fprintf(stderr,"                       or s/b %s.\n\n",print_binary(gp_reg_12, buffer1)); 
					++numWrong;
				}
			} else {
				if (numWrong == 0)
					fprintf(stderr, "\n"); /* start at newline after "Testing Gnd/Power: " */
				fprintf(stderr,"ERROR: read register 0x1%x was %s,\n",i,print_binary(readreg[i], buffer0));
				fprintf(stderr,"                          s/b %s.\n\n",print_binary(gp[i], buffer1)); 
				++numWrong;
			}
		}

	}

	/* return tristate to its default (off) value. */
	edt_reg_write(edt_p,TRISTATEREG, 0x00);  

	return numWrong;
}




/**
 * Require: buffer length is 9.  Nothing else tested yet.
 */
char *print_binary (unsigned int number, char *buffer)
{
#define length 8 
#define num_spaces 1
#define total_length length+num_spaces
	int i, spaces_used;

	spaces_used = 0;
	for (i=0; (i+spaces_used)<total_length; ++i)
	{
		/* change (total_length) - (i+spaces_used) in next line to 
		 *  (i+spaces_used) if the endianness should end up wrong. */
		if ( ((i%4)==0) && (i>0) )
		{ 
			buffer[(total_length-1) - i] = ' '; 
			++spaces_used;
		}
		buffer[(total_length-1)-(i+spaces_used)] = ((number>>i)&0x1) ? '1' : '0';
	}
	buffer[total_length] = '\0'; /* end the string. */	

	return buffer;
}




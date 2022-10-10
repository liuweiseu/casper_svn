#pragma ident "@(#)README.c3	1.1 10/13/03 EDT"

The c3 or "Combo III" boards add:

	- programmable direction for e1/t1 and e3/t3 (no jumpers);
	- new e1/t1 and e3/t3 chips which require new setup code;
	- jitter attenuation for e3/t3 (e1/t1 still has this).

Three sets of test programs are included in the driver:


	c3_set_e3.c:	    Code to set up new e3/t3 chips.
	c3_e3prbs(.bat):    Output generated test data to cable.
	c3_e3prbs.pdb:	    "pdb" commands to configure board for output.
	c3_e3chk(.bat):	    Input and test generated data from cable.
	c3_e3chk.pdb:	    "pdb" commands to configure board for input.
	
	c3_set_e1.c:	    Code to set up new e1/t1 chips.
	c3_e1prbs(.bat):    Output generated test data to cable.
	c3_e1prbs.pdb:	    "pdb" commands to configure board for output.
	c3_e1chk(.bat):	    Input and test generated data from cable.
	c3_e1chk.pdb:	    "pdb" commands to configure board for input.
	
	c3_diffprbs(.bat):  Output generated test data to cable.
	c3_diffchk(.bat):   Input and test generated data from cable.
	diffchk.pdb:	    "pdb" commands to configure board for differential.

To run one of these sets of tests, connect cables from one SS/ComboIII to
another.  Use a standard 68-pin SCSI cable for the 68-pin connectors,
and use a straight through 15-pin cable for the e3 connectors.  

Then, one at a time, run "c3_e3prbs -u N" for output and run
"c3_e3chk -u M" for input.  The boards may reside on separate machines.
Substitute "e1" and "diff" for "e3" for the other two tests.

The tests run forever and print a buffer count unless errors are
detected on the input side.

From these scripts and .pdb files you can see how to configure
c3 boards for input and output on e3/t3, e1/t1, and differential
data streams.

Contact EDT for information not found here:  503-690-1234 or tech@edt.com.

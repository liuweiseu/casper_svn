/* #pragma ident "@(#)sslooptest.c	1.21 08/20/04 EDT" */

#include "edtinc.h"

int unit = 0, chan = 0, board_id, dev_id, external_loopback = 0;
char cmd[128];
EdtDev *edt_p;

/*
 * This program runs loopback tests on PCI-SS ECL, RS422 and combo boards
 * without requiring any loopback connectors.  No external data is required.
 */

/*
 * String defines containing initpcd config scripts.
 */

#define loopdiff_setup_cfg  "\
bitfile:		ssd16in.bit\n\
ssd16_chedge_reg:	0x0\n\
ssd16_chdir_reg:	0x0\n\
ssd16_chen_reg:		0xffff\n\
flush_fifo:		1\n\
byteswap_sun:		1\n\
shortswap_sun:		1\n\
byteswap_x86:		0\n\
shortswap_x86:		0\n\
intfc_reg:		0x22 0xf\n\
"

#define loopcombo_setup_cfg  "\
bitfile: 		combo16in.bit\n\
run_command:		set_ss_vco 1 0\n\
run_command:		set_ss_vco -F 10000000 1\n\
run_command:		set_ss_vco 4 2\n\
run_command:		set_ss_vco 1 3\n\
command_reg:		0x08\n\
funct_reg:		0x80\n\
ssd16_chen_reg:		0xffff\n\
ssd16_chdir_reg:	0x0000\n\
ssd16_chedge_reg:	0x0000\n\
flush_fifo:		1\n\
byteswap_x86:		0\n\
shortswap_x86:		0\n\
byteswap_sun:		1\n\
shortswap_sun:		1\n\
"

#define loopecl_setup_cfg  "\
bitfile:		eclssd16.bit\n\
ssd16_chedge_reg:	0x0\n\
ssd16_chdir_reg:	0x0\n\
ssd16_chen_reg:		0xffff\n\
flush_fifo:		1\n\
byteswap_sun:		1\n\
shortswap_sun:		1\n\
byteswap_x86:		0\n\
shortswap_x86:		0\n\
intfc_reg:		0x22 0xff\n\
"

#define loopcombo_e3_cfg  "\
# map e3 channel 0 -3 to dma 0-3 respectively\n\
intfc_reg:		60 18\n\
intfc_reg:		61 19\n\
intfc_reg:		62 1a\n\
intfc_reg:		63 1b\n\
# set line interface to e3 mode\n\
intfc_reg:		49 0\n\
# enable output clock and data\n\
intfc_reg:		4a ff\n\
# set analog loopback on each line interface\n\
intfc_reg:		2e 11\n\
intfc_reg:		2f 11\n\
"


#define loopcombo_e1_cfg  "\
#set e1 channel 0-15 to dma 0-15 respectively\n\
intfc_reg: 60 0\n\
intfc_reg: 61 1\n\
intfc_reg: 62 2\n\
intfc_reg: 63 3\n\
intfc_reg: 64 4\n\
intfc_reg: 65 5\n\
intfc_reg: 66 6\n\
intfc_reg: 67 7\n\
intfc_reg: 68 8\n\
intfc_reg: 69 9\n\
intfc_reg: 6a a\n\
intfc_reg: 6b b\n\
intfc_reg: 6c c\n\
intfc_reg: 6d d\n\
intfc_reg: 6e e\n\
intfc_reg: 6f f\n\
# lxt384 in analog loopback\n\
intfc_reg: c1 ff\n\
intfc_reg: e1 ff\n\
# enable lxt384 data\n\
intfc_reg: 3a ff\n\
intfc_reg: 3b ff\n\
"

#define loopcombo_diff_cfg  "\
# set diffin channels 0-7 to DMA 0-7 respectively\n\
intfc_reg: 60 10\n\
intfc_reg: 61 11\n\
intfc_reg: 62 12\n\
intfc_reg: 63 13\n\
intfc_reg: 64 14\n\
intfc_reg: 65 15\n\
intfc_reg: 66 16\n\
intfc_reg: 67 17\n\
# drive out data  loopback just occurs on xilinx inputs\n\
intfc_reg: 22 f\n\
"

#define hrcin_cfg  "\
bitfile:		sshrcin.bit\n\
flush_fifo:		1\n\
byteswap_x86:		0\n\
shortswap_x86:		0\n\
byteswap_sun:		1\n\
shortswap_sun:		1\n\
intfc_reg:		12 f0\n\
"

#define cda16diff_setup_cfg  "\
bitfile:		ssd16io.bit\n\
run_command:		set_ss_vco -F 10000000 0\n\
command_reg:		0x08\n\
funct_reg:		0x80\n\
ssd16_chedge_reg:	0x0\n\
ssd16_chdir_reg:	0x0\n\
ssd16_chen_reg:		0xffff\n\
flush_fifo:		1\n\
byteswap_sun:		1\n\
shortswap_sun:		1\n\
byteswap_x86:		0\n\
shortswap_x86:		0\n\
intfc_reg:		0x22 0xff\n\
"

#define sse_external_setup_cfg  "\
run_command:		sseload -b ssein\n\
"

#define sse_internal_setup_cfg  "\
run_command:		sseload -b ssein\n\
ssd16_chen_reg:		0x0003\n\
"

main(int argc, char **argv)
{
    FILE *fp ;

    while (argc > 1 && argv[1][0] == '-')
    {
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
	    chan = atoi(argv[1]) ;
            break ;

	case 'e':
            ++argv;
            --argc;
	    external_loopback = 1;
            break ;
        }
        --argc ;
        ++argv ;
    }

    printf("\nUsing unit %d (chan %d)\n", unit, chan) ;


    /*
     * Check pci interface xilinx ID for SS baseboard.
     */
    if ((edt_p = edt_open("pcd", unit)) == NULL)
    {
	sprintf(cmd, "pcd unit %d", unit) ;
	edt_perror(cmd) ;
	exit(1) ;
    }
    dev_id = edt_device_id(edt_p) ;
    edt_close(edt_p) ;

    if (dev_id != PSS4_ID && dev_id != PSS16_ID && dev_id != PCDA16_ID)
    {
	fprintf(stderr,
	    "\nIncorrect board type (0x%x) for sslooptest.  Exiting.\n\n",
	    dev_id) ;
	exit(1) ;
    }

    if (dev_id == PCDA16_ID)
    {
	/* Just use dev_id for cda16 */
	board_id = dev_id;
    }
    else
    {
	/*
	 * Load pciss16test.bit on the baseboard and read the daughterboard ID.
	 */
	if (dev_id == PSS4_ID)
	    sprintf(cmd, "./bitload -u %d pciss4test", unit) ;
	else
	    sprintf(cmd, "./bitload -u %d pciss16test", unit) ;
	puts(cmd) ;
	if (edt_system(cmd) != 0)
	{
	    fputs("\nBitload failed... Aborting.\n\n", stderr) ;
	}


	if ((edt_p = edt_open("pcd", unit)) == NULL)
	{
	    sprintf(cmd, "pcd unit %d", unit) ;
	    edt_perror(cmd) ;
	    exit(1) ;
	}

	board_id = edt_reg_read(edt_p, 0x0101007f) ;
	printf("board_id %x\n\n", board_id) ;
	edt_close(edt_p) ;
    }



    /*
     * Run the ID appropriate loop test.
     */
    switch (board_id)
    {
	case 0x0:
	case 0x1:
	{

	    if (board_id == 0)
		puts("Testing PCI-SS RS422 loopback\n") ;
	    else
		puts("Testing PCI-SS LVDS loopback\n") ;

	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopdiff_setup_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "./chkprbs15 -u %d -I -l 100", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;
	    break ;
	}
	case 0x4:
	{
	    if (external_loopback)
	    {
		int speed;

		puts("Testing PCI-SS SSE external loopback\n") ;

		if ((fp = fopen("test.cfg", "w")) == NULL)
		{
		    edt_perror("test.cfg") ;
		    exit(1) ;
		}
		fputs(sse_external_setup_cfg, fp) ;
		fclose(fp) ;

		sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
		puts(cmd) ;
		fflush(stdout) ;
		if (edt_system(cmd))
		{
		    edt_perror(cmd);
		    exit(0);
		}

		for (speed = 50; speed <= 400; speed += 10)
		{
		    sprintf(cmd, "./sseload -u %d -F %d", unit, speed);
		    puts(cmd) ;
		    fflush(stdout) ;
		    if (edt_system(cmd))
		    {
			edt_perror(cmd);
			exit(0);
		    }

		    puts("") ;
		    puts("") ;
		    sprintf(cmd, "./chkprbs15 -u %d -c %d -n 1 -l 500",
							   unit, chan);
		    puts(cmd) ;
		    fflush(stdout) ;
		    edt_system(cmd) ;

		    puts("") ;
		    puts("") ;
		    fflush(stdout) ;
		}
	    }
	    else
	    {
		int speed;

		puts("Testing PCI-SS SSE internal loopback\n") ;

		if ((fp = fopen("test.cfg", "w")) == NULL)
		{
		    edt_perror("test.cfg") ;
		    exit(1) ;
		}
		fputs(sse_internal_setup_cfg, fp) ;
		fclose(fp) ;

		sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
		puts(cmd) ;
		fflush(stdout) ;
		edt_system(cmd) ;

		for (speed = 50; speed <= 400; speed += 10)
		{
		    sprintf(cmd, "./sseload -u %d -F %d", unit, speed);
		    puts(cmd) ;
		    fflush(stdout) ;
		    edt_system(cmd) ;

		    puts("") ;
		    puts("") ;
		    sprintf(cmd, "./chkprbs15 -u %d -c 0 -n 2 -l 500",
							   unit, chan);
		    puts(cmd) ;
		    fflush(stdout) ;
		    edt_system(cmd) ;

		    puts("") ;
		    puts("") ;
		    fflush(stdout) ;
		}
	    }
	    break ;
	}
	case 0x8:
	{

	    puts("Testing PCI-SS ECL loopback\n") ;

	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopecl_setup_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "./chkprbs15 -u %d -I -l 100", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;
	    break ;
	}
	case 0x0f:
	case 0x0e:
	case 0x07:
	{
	    if (board_id == 0x0f)
		puts("Testing PCI-SS COMBO loopback\n") ;
	    else if (board_id == 0x03)
		puts("Testing PCI-SS COMBOII RS422 loopback\n") ;
	    else if (board_id == 0x07)
		puts("Testing PCI-SS COMBOII LVDS loopback\n") ;

	    puts("\n   COMBO BOARD MUST BE JUMPERED FOR INPUT\n\n") ;


	    printf("\nUsing unit %d\n\n", unit) ;

	    puts("") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    puts("		Configure SSCombo for loopcombo test.") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopcombo_setup_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;


	    puts("") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    puts("		Run E3 loopback test (4 channels).") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopcombo_e3_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    /* DMA and check data from DMA 0-3 1000 buffers each */
	    
	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -N -c 0 -n 4 -l 1000", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;


	    puts("") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    puts("		Run E1 loopback test (16 channels).") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopcombo_e1_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    /* Set e1 channel 0-15 to dma 0-15 respectively */
	    
	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -N -c 0 -n 16 -l 75", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;


	    puts("") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    puts("		Run differential loopback test (8 channels).") ;
	    puts("::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::") ;
	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(loopcombo_diff_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    /* DMA and check data from DMA 0-7 250 buffers each */
	    
	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -N -c 0 -n 8 -l 250", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;


	    break ;
	}
	case 0x05:
	{
	    puts("") ;
	    puts("Testing PCI-SS HRC loopback\n") ;
	    puts("") ;
	    puts("") ;
	    puts("Testing E4 Rate - 139Mbps\n") ;

	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(hrcin_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;


	    puts("") ;
	    puts("") ;
	    puts("Testing channel 0\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 0 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 1\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 1 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 2\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 2 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 3\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 3 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;


	    puts("") ;
	    puts("") ;
	    puts("") ;
	    puts("Testing STM-1 Rate - 155Mbps\n") ;

	    edt_p = edt_open("pcd", unit) ;
	    edt_intfc_write(edt_p, 0x13, 0x0f) ;
	    edt_close(edt_p) ;


	    puts("") ;
	    puts("") ;
	    puts("Testing channel 0\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 0 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 1\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 1 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 2\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 2 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    puts("Testing channel 3\n") ;
	    puts("") ;
	    sprintf(cmd, "chkprbs15 -u %d -I -c 3 -n 1 -l 500", unit) ;
	    edt_system(cmd) ;


	    puts("") ;
	    puts("") ;

	    break ;
	}
	case 0xd:
	    puts(
	    "Testing PCI-SS COMBOIII ECL loopback is unimplemented.  Exiting.\n");
	    break ;
	case 0x45:
	{

	    puts("Testing PCI-CDA 16 differential loopback\n") ;

	    if ((fp = fopen("test.cfg", "w")) == NULL)
	    {
		edt_perror("test.cfg") ;
		exit(1) ;
	    }
	    fputs(cda16diff_setup_cfg, fp) ;
	    fclose(fp) ;

	    sprintf(cmd, "./initpcd -u %d -f test.cfg", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    puts("") ;
	    sprintf(cmd, "./chkprbs15 -u %d -I -l 100", unit) ;
	    puts(cmd) ;
	    fflush(stdout) ;
	    edt_system(cmd) ;

	    puts("") ;
	    fflush(stdout) ;
	    break ;
	}
	default:
	    fprintf(stderr, "Unknown board type %d", unit) ;
	    puts("Exiting.") ;
	    puts("") ;
	    exit(1) ;
	    break ;
    }
}

#if 0
#  The codes we are using:
#
#  0 - RS422 I/O board
#  1 - LVDS I/O board
#  4 - reserved for SSE (high speed serial ECL) I/O
#  5 - HRC for E4/STM-1/OC-3 I/O board
#  6 - OCM board
#  7 - COMBOII I/O LVDS board
#  8 - ECL w/opt SSE I/O board
#  9 - TLK1501 I/O board
#  B - MICROWAVE board
#  D - COMBOIII ECL
#  E - COMBOII I/O RS422        board
#  F - COMBO I/O board
#endif

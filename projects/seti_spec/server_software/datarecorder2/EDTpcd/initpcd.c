/* #pragma ident "@(#)initpcd.c	1.19 08/13/03 EDT" */

/******************************************************************************
 *									      *
 * initpcd:  Configure a PCD board using an ascii configuration	file.	      *
 *									      *
 *	To add new commands, search for	the string "ADD".		      *
 *									      *
 ******************************************************************************/
/******************************************************************************
 *
 *
 * User	Commands					       initpcd(1)
 *
 *
 *
 * NAME
 *	initpcd	- Configure a PCD board	using an ascii configuration file.
 *
 * SYNOPSIS
 *	initpcd	[ -u unit# ] [ -v ] [ -p ] [ -n	] [ -N ] -f file.cfg
 *
 * DESCRIPTION
 *	The initpcd command  reads  configuration  commands  from
 *	the specified file.cfg and executes  them on the selected
 *	PCD board (default unit	0).  Currently supported commands
 *	are listed below.  All numberic	values are hex.
 *
 *	    bitfile:  bitfile_name [basedir]
 *
 *		When executed causes the named interface  bitfile
 *		to be loaded into the PCD.  bitfile_name  can  be
 *		a simple name or the path to a bitfile.  The optional
 *		basedir path specifies where to begin searching for bitfiles.
 *
 *	    intfc_reg:	hex_offset hex_value
 *
 *		Set the	8-bit interface	 register  at  offset  to
 *		hex_value.
 *
 *	    command_reg:  hex_value
 *
 *		Set the	8-bit PCD Command Register to hex_value.
 *
 *	    dpstat_reg:	 hex_value
 *
 *		Set the	8-bit PCD Data Path Status Register to hex_value.
 *
 *	    funct_reg:	hex_value
 *
 *		Set the	8-bit PCD Funct	Register to hex_value.
 *
 *	    stat_pol_reg:  hex_value
 *
 *		Set the	8-bit PCD Stat Solarity	Register to hex_value.
 *
 *	    config_reg:	 hex_value
 *
 *		Set the	8-bit PCD Interface Configuration  Register  to
 *		hex_value.
 *
 *	    direction_reg:  hex_value
 *
 *		Set the	16-bit PCD Direction Register to hex_value.
 *
 *	    byteswap:  hex_value
 *
 *		Enables	byteswap if 1, disables	byteswap if 0.
 *
 *	    shortswap:	hex_value
 *
 *		Enables	shortswap if 1,	disables shortswap if 0.
 *
 *	    byteswap_x86:  hex_value
 *
 *		If x86, Enables	byteswap if 1, disables	byteswap if 0.
 *
 *	    shortswap_x86:  hex_value
 *
 *		If x86, Enables	byteswap if 1, disables	byteswap if 0.
 * 
 *	    byteswap_sun:  hex_value
 *
 *		If sun, Enables	byteswap if 1, disables	byteswap if 0.
 *
 *	    shortswap_sun:  hex_value
 *
 *		If sun, Enables	byteswap if 1, disables	byteswap if 0.
 *
 *	    ssd_msb_first:  hex_value
 *
 *		Enables	most significant bit first if 1, disables if 0.
 *		Works on all SSD configurations	except the GPSSD16.
 *
 *	    ssd_chanbits:  hex_value
 *
 *		Sets the number	of bits	per serial channel.
 *		Works on all SSD configurations	except the GPSSD16.
 *
 *	    pll_clock_freq:  floating_value
 *
 *		Programs the PLL clock frequency to closely match
 *		the real value provided.
 *
 *	    flush_fifo:	 hex_value
 *
 *		Flush the PCD fifo after configuration is complete.
 *
 *	    run_program:  path_value
 *
 *		Run the	specified program and arguments	from a shell.
 *
 *	    run_command:  path_value
 *
 *		Run the	specified command and arguments from a shell,
 *		but with the argument "-u N" where N is the unit number
 *		initpcd is running on.
 *
 * OPTIONS
 *	-u unit#  Selects the PCD board	number to configure.  Default 0.
 *
 *	-v	  Print	all actions and	warnings.
 *
 *	-p	  Print	the table of commands  once  read  from	 the
 *		  .cfg file.
 *
 *	-n	  Don't	execute	any commands.  Useful to check	.cfg
 *		  file syntax or with the -p command.
 *
 *	-N	  Don't	load the Xilinx	bitfile, but do	everything else.
 *		  Useful for configuring after the Xilinx has been loaded.
 *
 *	-f file.cfg  Specifies the configuration  file	from  which
 *		     commands are read.
 *
 * ADDING NEW COMMANDS
 *
 *	New commands may be added by to	initpcd	by editing initpcd.c,
 *	adding a new command function to the func_cmd_*() declarations,
 *	a new line to the commands[] table, then adding	the definition
 *	for func_cmd_*().  Search for the string "ADD" in initpcd.c.
 *
 *	Other EDT boards may be	supported or new configuration programs
 *	created	by completely replacing	the set	of commands in initpcd.c.
 *
 * FILES
 *	/opt/EDTpcd/(*).cfg, /opt/EDTpcd/initedt.c, /opt/EDTpcd/initedt.c.
 *
 * NOTES
 *	Not all	commands and argument values work with all PCD configurations.
 *
 ******************************************************************************/

#include "edtinc.h"
#include "initedt.h"
#include "initpcd.h"
#include "edt_bitload.h"
#include <stdlib.h>
#include "edt_vco.h"


/*
 * ADD new command function declarations here.	Commands use and return	an int.
 */
int func_bitfile(cmd_operator *cmd_ptr)	;
int func_intfc_reg(cmd_operator *cmd_ptr) ;
int func_command_reg(cmd_operator *cmd_ptr) ;
int func_dpstat_reg(cmd_operator *cmd_ptr) ;
int func_funct_reg(cmd_operator *cmd_ptr) ;
int func_stat_pol_reg(cmd_operator *cmd_ptr) ;
int func_config_reg(cmd_operator *cmd_ptr) ;
int func_direction_reg(cmd_operator *cmd_ptr) ;
int func_ssd16_chen_reg(cmd_operator *cmd_ptr) ;
int func_ssd16_chdir_reg(cmd_operator *cmd_ptr)	;
int func_ssd16_chedge_reg(cmd_operator *cmd_ptr) ;
int func_byteswap(cmd_operator *cmd_ptr) ;
int func_byteswap86(cmd_operator *cmd_ptr) ;
int func_byteswapsun(cmd_operator *cmd_ptr) ;
int func_shortswap(cmd_operator *cmd_ptr) ;
int func_shortswap86(cmd_operator *cmd_ptr) ;
int func_shortswapsun(cmd_operator *cmd_ptr) ;
int func_ssd_msb_first(cmd_operator *cmd_ptr) ;
int func_ssd_chanbits(cmd_operator *cmd_ptr) ;
int func_pll_clock_freq(cmd_operator *cmd_ptr) ;
int func_flush_fifo(cmd_operator *cmd_ptr) ;
int func_run_program(cmd_operator *cmd_ptr) ;
int func_run_command(cmd_operator *cmd_ptr) ;


/*
 * Table of commands.  Each entry has a	unique name and	a type specified.
 *
 */
cmd_operator commands[] = 

/*
 * The cmd_type	determines how command arguments in the	.cfg file will be
 * processed:
 *
 *	1 = expect one hex integer
 *	2 = expect two hex integers
 *	3 = expect one floating	point number
 *	4 = expect one pathname
 *
 *    Command name:	     arg_type:	     cmd_func:
 */
{
	{ "bitfile",		   4,		func_bitfile		},

	{ "intfc_reg",		   2,		func_intfc_reg		},
	{ "command_reg",	   1,		func_command_reg	},
	{ "dpstat_reg",		   1,		func_dpstat_reg		},
	{ "funct_reg",		   1,		func_funct_reg		},
	{ "stat_pol_reg",	   1,		func_stat_pol_reg	},
	{ "config_reg",		   1,		func_config_reg		},
	{ "direction_reg",	   1,		func_direction_reg	},
	{ "ssd16_chen_reg",	   1,		func_ssd16_chen_reg	},
	{ "ssd16_chdir_reg",	   1,		func_ssd16_chdir_reg	},
	{ "ssd16_chedge_reg",	   1,		func_ssd16_chedge_reg	},

	{ "byteswap",		   1,		func_byteswap		},
	{ "shortswap",		   1,		func_shortswap		},
	{ "byteswap_x86",	   1,		func_byteswap86	},
	{ "shortswap_x86",	   1,		func_shortswap86	},
	{ "byteswap_sun",	   1,		func_byteswapsun	},
	{ "shortswap_sun",	   1,		func_shortswapsun	},
	{ "ssd_msb_first",	   1,		func_ssd_msb_first	},
	{ "ssd_chanbits",	   1,		func_ssd_chanbits	},
	{ "pll_clock_freq",	   3,		func_pll_clock_freq	},
	{ "flush_fifo",		   1,		func_flush_fifo		},
	{ "run_program",	   4,		func_run_program	},
	{ "run_command",	   4,		func_run_command	},

/*			ADD new	commands entries here.			    */
};


/******************************************************************************
 *									      *
 * Housekeeping	stuff.	Leave these alone; initedt.c uses them to access  *
 * the command table.							      *
 *									      *
 ******************************************************************************/

extern int initedt_verbose ;
extern int initedt_load_bitfile	;
extern int unit	;
extern EdtDev *initedt_p ;


cmd_operator
get_cmd_copy(int cmd_index)
{
    return commands[cmd_index] ;
}

char *
get_cmd_name(int cmd_index)
{
    return commands[cmd_index].cmd_name	;
}

int
cmd_table_entries()
{
    return (sizeof(commands) / sizeof(cmd_operator)) ;
}


/******************************************************************************
 *									      *
 * These functions carry out the work for each line found in the config	      *
 * file.  Add functions	for new	commands here.				      *
 *									      *
 ******************************************************************************/

int
func_bitfile(cmd_operator *cmd_ptr)
{
    char *p, *path = ".";
    int	ret = 0 ;

    if (initedt_verbose)
	printf("\tload_bitfile:	%s\n", (initedt_load_bitfile) ?	"Yes" :	"No") ;

    if (initedt_load_bitfile)
    {
	int level = 0 ;

	level |= EDTAPP_MSG_INFO_1;
	level |= EDTLIB_MSG_INFO_1;
	edt_msg_set_level(edt_msg_default_handle(), level);

	if ((p = strchr(cmd_ptr->cmd_pathval, ' '))
	 || (p = strchr(cmd_ptr->cmd_pathval, '\t')))
	{
	    *p++ = '\0' ;

	    while (*p == ' ' || *p == '\t')
		++p ;

	    if (*p != '\n' && *p != '\r')
		path = p ;
	}

	if (initedt_verbose)
	    printf("\tbitfile:  %s  path:  %s\n", cmd_ptr->cmd_pathval, path) ;
	
        if (ret = edt_bitload(initedt_p, path, cmd_ptr->cmd_pathval, 0, 0))
	    fputs("bitload: could not load bitfile.\n", stderr) ;
    }

    return ret ;
}

int
func_intfc_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf(
		"\tpcd set intfc reg:  offset 0x%x, val	0x%x\n",
					  cmd_ptr->cmd_intval1,
					  cmd_ptr->cmd_intval2) ;
    edt_intfc_write(initedt_p, PCD_CMD + cmd_ptr->cmd_intval1,
				 (u_char) cmd_ptr->cmd_intval2) ;

    if (initedt_verbose) printf("\treadback:  offset 0x%x val 0x%x\n\n",
					  cmd_ptr->cmd_intval1,
	  edt_intfc_read(initedt_p,	PCD_CMD	+ cmd_ptr->cmd_intval1));

	 return 0;
}


int
func_command_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd_set_cmd(0x%x)\n",
					  cmd_ptr->cmd_intval1) ;
    pcd_set_cmd(initedt_p, (u_char)	cmd_ptr->cmd_intval1) ;

    if (initedt_verbose) printf("\treadback:  0x%x\n\n", pcd_get_cmd(initedt_p)) ;

	 return 0;
}


int 
func_dpstat_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd set dpstat:  0x%x\n",
					  cmd_ptr->cmd_intval1) ;
    edt_intfc_write(initedt_p, PCD_DATA_PATH_STAT,
				 (u_char) cmd_ptr->cmd_intval1) ;
    if (initedt_verbose) printf("\treadback:  0x%x\n\n",
			edt_intfc_read(initedt_p, PCD_DATA_PATH_STAT)) ;
	 
	 return 0;
}


int
func_funct_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd_set_funct(0x%x)\n",
					  cmd_ptr->cmd_intval1) ;
    pcd_set_funct(initedt_p, (u_char) cmd_ptr->cmd_intval1) ;

    if (initedt_verbose) printf("\treadback:  0x%x\n\n",
					     pcd_get_funct(initedt_p)) ;
	 return 0;
}


int
func_stat_pol_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd_set_stat_polarity(0x%x)\n",
					  cmd_ptr->cmd_intval1) ;
    pcd_set_stat_polarity(initedt_p, (u_char) cmd_ptr->cmd_intval1) ;

    if (initedt_verbose) printf("\treadback:  0x%x\n\n",
				     pcd_get_stat_polarity(initedt_p)) ;
	 return 0;
}


int 
func_config_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd set config:  0x%x\n",
			    (u_char)	  cmd_ptr->cmd_intval1) ;
    edt_intfc_write(initedt_p, PCD_CONFIG,
			    (u_char)	  cmd_ptr->cmd_intval1) ;

    if (initedt_verbose) printf("\treadback:  0x%x\n\n",
				edt_intfc_read(initedt_p, PCD_CONFIG)) ;

	 return 0;
}


int 
func_direction_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose)
	printf("\tpcd_set_direction(0x%x)\n", cmd_ptr->cmd_intval1) ;

    edt_intfc_write(initedt_p, PCD_DIRA,
			(u_char) (cmd_ptr->cmd_intval1 & 0xff));
    edt_intfc_write(initedt_p, PCD_DIRB,
		(u_char) ((cmd_ptr->cmd_intval1 >> 8) & 0xff)) ;
    if (initedt_verbose)
	printf("\treadback: 0x%x\n\n", edt_get_direction(initedt_p)) ;

	 return 0;
}


int 
func_ssd16_chen_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose)
	printf("\tpcd set SSD16_CHEN 0x%x\n", cmd_ptr->cmd_intval1);

    edt_intfc_write(initedt_p, SSD16_CHENL,
			(u_char) (cmd_ptr->cmd_intval1 & 0xff));
    edt_intfc_write(initedt_p, SSD16_CHENH,
		(u_char)  ((cmd_ptr->cmd_intval1 >> 8) & 0xff));
    if (initedt_verbose)
	printf("\treadback: 0x%x\n\n",
	     (edt_intfc_read(initedt_p, SSD16_CHENH) << 8)
	    | edt_intfc_read(initedt_p, SSD16_CHENL)) ;

	 return 0;
}


int
func_ssd16_chdir_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose)
	printf("\tpcd set SSD16_CHDIR 0x%x\n", cmd_ptr->cmd_intval1);

    edt_intfc_write(initedt_p, SSD16_CHDIRL,
	    (u_char) (cmd_ptr->cmd_intval1 &	0xff));
    edt_intfc_write(initedt_p, SSD16_CHDIRH,
	    (u_char) ((cmd_ptr->cmd_intval1 >> 8) & 0xff));

    if (initedt_verbose)
	printf("\treadback: 0x%x\n\n",
		     (edt_intfc_read(initedt_p, SSD16_CHDIRH) << 8)
		    | edt_intfc_read(initedt_p, SSD16_CHDIRL)) ;

	 return 0;
}


int
func_ssd16_chedge_reg(cmd_operator *cmd_ptr)
{
    if (initedt_verbose) printf("\tpcd set SSD16_CHEDGE	0x%x\n",
					    cmd_ptr->cmd_intval1) ;
    edt_intfc_write(initedt_p, SSD16_CHEDGEL,
			  (u_char) (cmd_ptr->cmd_intval1 & 0xff)) ;
    edt_intfc_write(initedt_p, SSD16_CHEDGEH,
		   (u_char) ((cmd_ptr->cmd_intval1 >> 8) & 0xff)) ;
    if (initedt_verbose)
	printf("\treadback: 0x%x\n\n",
	     (edt_intfc_read(initedt_p, SSD16_CHEDGEH) << 8)
	    | edt_intfc_read(initedt_p, SSD16_CHEDGEL)) ;
	 return 0;
}


int
func_byteswap(cmd_operator *cmd_ptr)
{
    u_char  config;

    config = edt_intfc_read(initedt_p, PCD_CONFIG);

    if (initedt_verbose)
	printf("\tset byteswap:	 %d\n\n", cmd_ptr->cmd_intval1) ;

    if (cmd_ptr->cmd_intval1)
	config |= PCD_BYTESWAP;
    else
	config &= ~PCD_BYTESWAP;

    edt_intfc_write(initedt_p, PCD_CONFIG, config);

	 return 0;
}

int
func_byteswap86(cmd_operator *cmd_ptr)
{
    if (!edt_little_endian()) return 0 ;

    return (func_byteswap(cmd_ptr)) ;
}

int 
func_byteswapsun(cmd_operator *cmd_ptr)
{
    if (edt_little_endian()) return 0 ;

    return (func_byteswap(cmd_ptr)) ;
}


int 
func_shortswap(cmd_operator *cmd_ptr)
{
    u_char  config;

    config = edt_intfc_read(initedt_p, PCD_CONFIG);

    if (initedt_verbose) printf("\tset shortswap:  %d\n\n",
		 cmd_ptr->cmd_intval1) ;

    if (cmd_ptr->cmd_intval1)
	config |= PCD_SHORTSWAP;
    else
	config &= ~PCD_SHORTSWAP;

    edt_intfc_write(initedt_p, PCD_CONFIG, config);
	 
	 return 0;
}

int
func_shortswap86(cmd_operator *cmd_ptr)
{
    if (!edt_little_endian()) return 0 ;

    return (func_shortswap(cmd_ptr)) ;
}

int
func_shortswapsun(cmd_operator *cmd_ptr)
{
    if (edt_little_endian()) return 0 ;

    return (func_shortswap(cmd_ptr)) ;
}



int
func_ssd_msb_first(cmd_operator *cmd_ptr)
{
    u_char funct = pcd_get_funct(initedt_p);

    if (initedt_verbose) printf("\tset msb_first:  %d\n\n",
		 cmd_ptr->cmd_intval1) ;

    if (cmd_ptr->cmd_intval1)
	funct |=  0x04 ;
    else
	funct &= ~0x04 ;

    pcd_set_funct(initedt_p, funct);
	 return 0;
}



int
func_ssd_chanbits(cmd_operator *cmd_ptr)
{
    u_char funct = pcd_get_funct(initedt_p);
    int	bits = cmd_ptr->cmd_intval1 ;

    if (initedt_verbose) printf("\tset ssd_chanbits:  %d\n\n", bits) ;

    switch (bits)
    {
    case 1:
	{
	    funct &= ~0x03 ;
	    break;
	}
    case 2:
	{
	    funct &= ~0x03 ;
	    funct |=  0x01 ;
	    break;
	}
    case 4:
	{
	    funct |= 0x03 ;
	    break;
	}
    default:
	{
	    fprintf(stderr,
"\tssd_chanbits:  illegal value	%d.  Must be 1,	2, or 4.  Exiting.\n\n", bits) ;
	    return -1 ;
	    break;
	}
    }

    pcd_set_funct(initedt_p, funct);

    return 0 ;
}


int 
func_pll_clock_freq(cmd_operator *cmd_ptr)
{
    edt_pll clkset ;

    if (initedt_verbose)
	printf("\tpll set clock	freq: %f\n\n", cmd_ptr->cmd_realval);

    edt_find_vco_frequency(initedt_p, cmd_ptr->cmd_realval, 0,
						  &clkset, initedt_verbose) ;
    edt_set_out_clk(initedt_p, &clkset);

	 return 0;
}

int
func_flush_fifo(cmd_operator *cmd_ptr)
{
    if (cmd_ptr->cmd_intval1)
    {
	if (initedt_verbose) puts("\tcalling edt_flush_fifo()\n") ;
	edt_flush_fifo(initedt_p) ;
    }
	 return 0;
}

int
func_run_program(cmd_operator *cmd_ptr)
{
    if (initedt_verbose)
	printf("\trunning \"%s\"\n\n", cmd_ptr->cmd_pathval)	;

    return edt_system(cmd_ptr->cmd_pathval) ;
}

#include <string.h>

int
func_run_command(cmd_operator *cmd_ptr)
{
    static char cmd[512], tmp[512] ;
    char *p, unit_str[16] ;
    int len ;

    strncpy(tmp, cmd_ptr->cmd_pathval, 511) ;

    if ((p = strchr(tmp, ' ')) || (p = strchr(tmp, '\t')))
	*p++ = '\0' ;

    strncpy(cmd, tmp, 511) ;
    sprintf(unit_str, " -u %d ", unit) ;

    len = strlen(cmd) ;
    strncat(cmd, unit_str, 511 - len) ;

    if (p)
    {
	len = strlen(cmd) ;
	strncat(cmd, p, 511 - len) ;
    }

    if (initedt_verbose)
	printf("\trunning \"%s\"\n\n", cmd)	;

    return edt_system(cmd) ;
}

int
initpcd(EdtDev *edt_p, char *cfg_file, int no_execute, int no_load_bitfile, int verbose)
{
    extern int initedt_load_bitfile ;
    extern int initedt_verbose ;
    extern EdtDev * initedt_p ;

    initedt_load_bitfile = !no_load_bitfile ;
    initedt_p = edt_p ;

    if (input_config_file(cfg_file))
	return(-1) ;

    if (verbose) 
    {
	initedt_verbose = 1 ;
	print_cmds() ;
    }

    if (!no_execute)
	return process_cmds() ;
    else
	return 1 ;
}

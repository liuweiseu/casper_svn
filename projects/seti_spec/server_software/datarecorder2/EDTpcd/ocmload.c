/*
 * ocmload.c -- load the channel 0 and/or 1 xilinx on the OCM mezzanine board
 * detects and loads the ocm bitfile if required
 * 
 * If full path is specified (i.e. if there are any slashes in <file>) then it
 * will just look for the file. Otherwise it uses the following search path
 * (dir is . by default, or -d <dir> if specified):
 * 
 * dir/40xx/file if dv(k) dir/bitfiles/dv(k)/40xx/file
 * dir/40xx/file otherwise
 * 
 * if it finds but fails to load anywhere above the 40xx directories, then
 * it will quit trying. Otherwise it tries the next thing.
 * 
 * (C) 1997-2000 Engineering Design Team, Inc.
 */

#include "edtinc.h"
#include <stdlib.h>
#include "edt_bitload.h"

#ifdef _NT_
#define strcasecmp(a,b) stricmp((a),(b))
#else
#include <dirent.h>
#endif



static void
usage(char *progname)
{
    fprintf(stderr, "Usage: %s [-u unit] [-d dir] [-n] [-c <0 or 1>] [-e] [-v] [-V] [-q] [-S]\n", progname);
    fprintf(stderr, "-u unit     %s unit number (default 0)\n", EDT_INTERFACE);
    fprintf(stderr, "-d dir      base directory for search for bitfile (default ., then ./bitfiles\n");
    fprintf(stderr, "-n          channel 0 is configured for OC3/12 with no RAM instead of OC12/48 with RAM buffer\n");
    fprintf(stderr, "-c <0 or 1> loads only channel specified - default loads both channels\n");
    fprintf(stderr, "-e          use nofs (embedded) array not file (must be compiled with -DNO_FS)\n", EDT_INTERFACE);
    fprintf(stderr, "-v, -V      verbose, and more Verbose output\n" );
    fprintf(stderr, "-q          quiet (no output at all)\n" );
    fprintf(stderr, "-S          skips actual loading (for debug)\n" );
	fprintf(stderr, "-h          show this message\n" );
    fprintf(stderr, "-H          show this message\n" );
    fprintf(stderr, "-?          show this message\n" );
}

#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;
bitload(char *command_line)
#else
int
main(int argc, char **argv)
#endif
{
    int     unit = 0;
    int     ret, len;
    int     verbose=1;
	int		channel = -1;	/* default -1 loads both channels */
    int     nofs = 0;
    int     ovr = 1; /* always ignore size constraints based on SS or GS board */
    int     flags = 0;
    int     skip_load = 0;
    int     ch0_noram = 0;
    int     level;
    char    devname[128];
    char    basedir[256];
    char    bitname[256];
    char    errstr[128];
    u_char *ba;
    char   *unitstr = "0";
    char   *progname;
    EdtDev *edt_p;
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("bitload",command_line,&argc,&argv);
#endif

    /* check args */
    progname = argv[0];
    strcpy(basedir, ".");

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'q':
	    verbose = 0;
	    break;

	case 'n':
	    ch0_noram = 1;
	    break;

	case 'v':
	case 'V':
	    verbose = 2;
	    break;

	case 'h':
	case 'H':
	case '?':
	    usage(progname);
		exit(0);
	    break;


	case 'd':
	case 'b':		/* compat */
	    ++argv;
	    --argc;
	    strcpy(basedir, argv[0]);
	    break;

	case 'u':
	    ++argv;
	    --argc;
	    unitstr = argv[0];
	    break;

	case 'e':
#ifdef NO_FS
	    nofs = 1;
#else
	    printf("'e' option specified but not compiled with NO_FS\n");
	    exit(1);
#endif
	    break;


	case 'c':
	    ++argv;
	    --argc;
	    channel = atoi(argv[0]);	
		if ((channel < 0) || (channel > 1))
		{
	    	printf("channel number specified must be 0 or 1\n");
	     	usage(progname);
	    	exit(1);
		}
	    break;

	case 'S':
	    skip_load = 1;	/* debug use only */
	    break;

	default:
	    usage(progname);
	    exit(0);
	}
	argc--;
	argv++;
    }

    if (argc > 0)
    {
	printf("filename is ignored ocmload loads ocm48 and ocm12 only\n");
	usage(progname);
    }

    unit = edt_parse_unit(unitstr, devname, EDT_INTERFACE);

    if ((edt_p = edt_open(devname, unit)) == NULL)
    {
	sprintf(errstr, "edt_open(%s%d)", devname, unit);
	edt_perror(errstr);
	edt_msg(EDTAPP_MSG_FATAL, "couldn't load xilinx\n");
	exit(1);
    }

    if ((edt_p->devid != PSS4_ID) && (edt_p->devid != PGS4_ID))
	{
		printf("Selected board id is %s\n", edt_idstr(edt_p->devid));
		printf("OCM mezzanine board must be mounted on PCIGS or PCISS with 4 channel DMA configuration file\n");
		printf("\tPlease use \"pciload pciss4\" (for PCISS) to reprogram configuration\n");
		printf("\tPlease use \"pciload pcigs4\" (for PCIGS) to reprogram configuration\n");
		exit(1);
	}

    /*
     * output message stuff
     */
    level = edt_msg_default_level();
    if (verbose < 1)
	level = 0;
    else /* default is slightly verbose */
    {
	level |= EDTAPP_MSG_INFO_1;
	level |= EDTLIB_MSG_INFO_1;
    }
    if (verbose > 1)
    {
	level |= EDTAPP_MSG_INFO_1
	      | EDTAPP_MSG_INFO_2
	      | EDTLIB_MSG_INFO_1
	      | EDTLIB_MSG_INFO_2;
    }
    edt_msg_set_level(edt_msg_default_handle(), level);

    if (nofs)
	flags |= BITLOAD_FLAGS_NOFS;
	/* always don't check size */
	flags |= BITLOAD_FLAGS_OVR;

	/* first check if ocm config file is loaded by writting a 0 and reading a constant from a register */
	edt_reg_write(edt_p, OCM_X_CONST, 0x0);
	if ( edt_reg_read(edt_p, OCM_X_CONST) != OCM_CONSTANT)
	{
		printf("ocm.bit file not loaded.. loading now\n");
		ret = edt_bitload(edt_p, basedir, "ocm.bit", flags, skip_load);
		if (ret)
		{
			edt_msg(EDTAPP_MSG_FATAL, "ocm.bit bitload failed\n");
			if (verbose < 2)
				edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
			else printf("\n");
			exit(1);
		}
	}
	/* now load channel 0 and/or 1 */
	flags |= BITLOAD_FLAGS_OCM;
	if ( (channel==0) || (channel == -1))
	{ /* load channel 0 with ocm48.bit or ocm12.bit(for xcvp4) */
		if (ch0_noram == 0)
			ret = edt_bitload(edt_p, basedir, "ocm48.bit", flags, skip_load);
		else
			ret = edt_bitload(edt_p, basedir, "ocm12.bit", flags, skip_load);

		if (ret)
		{
			edt_msg(EDTAPP_MSG_FATAL, "channel 0 bitload failed\n");
			if (verbose < 2)
				edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
			else printf("\n");
			exit(1);
		}
	}
	flags |= BITLOAD_FLAGS_CH1;
	if ( (channel==1) || (channel == -1))
	{ /* load channel 1 with ocm12.bit (for xc3s200) */
		ret = edt_bitload(edt_p, basedir, "ocm12.bit", flags, skip_load);
		if (ret)
		{
			edt_msg(EDTAPP_MSG_FATAL, "ocm12.bit bitload failed\n");
			if (verbose < 2)
				edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
			else printf("\n");
			exit(1);
		}
	}

    edt_close(edt_p);

#ifdef NO_MAIN
    return(0) ;
#else
    exit(0);
#endif
}




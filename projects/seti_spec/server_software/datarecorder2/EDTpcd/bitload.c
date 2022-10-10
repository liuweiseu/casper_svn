/*
 * bitload.c -- load the EDT PCI interface Xilinx .bit file
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

#ifndef _NT_
#include <dirent.h>
#endif



static void
usage(char *progname)
{
    fprintf(stderr, "usage: %s [-u unit] [-d dir] [-v] [-V] bitfile\n", progname);
    fprintf(stderr, "-u unit     %s unit number (default 0)\n", EDT_INTERFACE);
    fprintf(stderr, "-d dir      base directory for search for bitfile (default ., then ./bitfiles\n");
    fprintf(stderr, "-e          use nofs (embedded) array not file (must be compiled with -DNO_FS)\n", EDT_INTERFACE);
    fprintf(stderr, "-v, -V      verbose, and more Verbose output\n" );
    fprintf(stderr, "-o          override ss/gs size check (debug)\n" );
    fprintf(stderr, "-q          quiet (no output at all)\n" );
    fprintf(stderr, "bitfile     bit file to load. If this argument is a path, will try to load\n");
    fprintf(stderr, "            it directly. If just a bitfile name, will search for the\n");
    fprintf(stderr, "            appropriate version of that file, starting in <dir>/bitfiles\n");
    fprintf(stderr, "            (PCI CD or GP), <dir>/dv/bitfiles (PCI DV), or <dir>/dvk/bitfiles\n");
    fprintf(stderr, "            (PCI DVK or PCI DV44) (see -d flag).\n");
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
    int     nofs = 0;
    int     ovr = 0;
    int     flags = 0;
    int     skip_load = 0;
    int     level;
    char    devname[128];
    char    basedir[256];
    char    bitname[256];
    char    errstr[128];
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

	case 'v':
	case 'V':
	    verbose = 2;
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


	case 'o':
	    ovr = 1;	/* debug use only */
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

    if (argc < 1)
    {
	usage(progname);
	exit(0);
    }

    len = strlen(argv[0]);

#ifdef NO_FS
    if (nofs) /* strip off .bit from name if nofs */
    {
	strcpy(bitname, argv[0]);
	if ((len >= 4) && (strcasecmp(&bitname[len-4], ".bit") == 0))
	    bitname[len-4] = '\0';
    }
    else
#endif
    if ((len < 4)
     || ((strcasecmp(&(argv[0][len-4]), ".bit") != 0)
      && (strcmp(&(argv[0][len-4]), ".BIT") != 0)))
	sprintf(bitname, "%s.bit", argv[0]);
    else strcpy(bitname, argv[0]);

    unit = edt_parse_unit(unitstr, devname, EDT_INTERFACE);

    if ((edt_p = edt_open(devname, unit)) == NULL)
    {
	sprintf(errstr, "edt_open(%s%d)", devname, unit);
	edt_perror(errstr);
	edt_msg(EDTAPP_MSG_FATAL, "couldn't load xilinx");
	exit(1);
    }

    if (edt_is_pdv(edt_p) &&
	!strcmp(basedir,"."))
	strcpy(basedir,"camera_config");

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
    if (ovr)
	flags |= BITLOAD_FLAGS_OVR;

    ret = edt_bitload(edt_p, basedir, bitname, flags, skip_load);

    edt_close(edt_p);

    if (ret)
    {
	edt_msg(EDTAPP_MSG_FATAL, "bitload failed");
	if (verbose < 2)
	    edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
	else printf("\n");
#ifdef NO_MAIN
	return(1) ;
#else
	exit(1);
#endif
    }

#ifdef NO_MAIN
    return(0) ;
#else
    exit(0);
#endif
}




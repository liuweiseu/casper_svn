/* #pragma ident "@(#)initpcd_main.c	1.6 10/06/04 EDT" */

#include "edtinc.h"
#include "initpcd.h"
#include <stdlib.h>

int
main(argc, argv)
int argc;
char **argv;
{
    extern int run_cmds, initedt_load_bitfile, initedt_verbose ;
    extern EdtDev *initedt_p ;
    extern char *initedt_config_file ;
    int ret ;
    int norun ;
    int noload ;

    if ( (ret = handle_args(argc, argv)) )
    	exit(ret) ;

    norun = !run_cmds ;
    noload = !initedt_load_bitfile ;

    ret = initpcd(initedt_p, initedt_config_file, norun,noload,initedt_verbose);

    if (ret)
    	perror(initedt_config_file);
     exit(ret);
}

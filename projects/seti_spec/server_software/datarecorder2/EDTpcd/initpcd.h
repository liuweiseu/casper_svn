/* #pragma ident "@(#)initpcd.h	1.4 05/20/02 EDT" */
#ifndef _INITPCD_H
#define _INITPCD_H

void EDTAPI print_cmds() ;
int EDTAPI process_cmds() ;
int EDTAPI input_config_file() ;
int EDTAPI handle_args(int argc, char **argv) ;
int EDTAPI initpcd(EdtDev *edt_p, char *cfg_file, int no_execute,
				int no_load_bitfile, int verbose) ;

#endif /* _INITPCD_H */

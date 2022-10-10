/* #pragma ident "@(#)edt_bitfile.h	1.2 12/24/01 EDT" */

#include "edtinc.h"
#include <sys/stat.h>

/**************************************************************************
 *
 * Include file definitions for edt_bitfile.c API subroutines.
 *
 **************************************************************************/
#define EDT_MAX_BOARDS 32
#define EDT_MAX_BITFILES 64

typedef struct {
	int unit_no ;
	char *intfc_bitfile ;
	char board_name[64] ;
	char pci_xilinx_type[64] ;
	char pci_flash_name[64] ;
} edt_board_desc ;

typedef struct {
    char *pci_bitfile_name ;
    char *pci_bitfile_comment ;
    int   intfc_bitfile_count ;
    char *intfc_bitfile_names[64] ;
    char *intfc_bitfile_comments[64] ;
} edt_bitfile_desc ; 

typedef edt_board_desc edt_board_desc_table[EDT_MAX_BOARDS] ; 
typedef edt_bitfile_desc edt_bitfile_desc_table[EDT_MAX_BITFILES] ; 

/*
 * The following subroutines are typically used in this order:
 */
int   edt_probe_boards(edt_board_desc_table edt_boards);
int   create_pcdload_script(edt_board_desc_table);
char *get_bitfile_list(edt_bitfile_desc_table edt_bitfiles);
char *get_board_list(edt_board_desc_table edt_boards);
void  app_perror(char *str);
int   edt_read_bitfile_config(
		char *cfg_file_name, edt_bitfile_desc_table edt_bitfiles);
edt_bitfile_desc *edt_get_bitfile_desc(
		char *flash_name, edt_bitfile_desc_table edt_bitfiles);

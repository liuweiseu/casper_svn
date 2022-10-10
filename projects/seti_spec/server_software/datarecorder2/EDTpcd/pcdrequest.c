/* #pragma ident "@(#)pcdrequest.c	1.9 02/21/03 EDT" */

#include "edt_bitfile.h"

/**************************************************************************
 *
 * Subroutine declarations
 *
 **************************************************************************/

char *interface_bitfile_menu(int, edt_board_desc_table, edt_bitfile_desc_table);
int   perform_menu_selection(edt_board_desc_table, edt_bitfile_desc_table);
void  print_bitfile_list(edt_bitfile_desc_table edt_bitfiles);
void  print_board_list(edt_board_desc_table edt_boards);


/**************************************************************************
 *
 * Local data
 *
 **************************************************************************/

char instr[128];

#define RUN_PCDLOAD_FILE ".run_pcdload"

#ifdef _NT_
#define PCDLOAD_FILE 	 "pcdload.bat"
#else
#define PCDLOAD_FILE 	 "pcdload"
#endif


/**************************************************************************
 *
 * Main
 *
 **************************************************************************/

main(argc, argv)
int argc;
char **argv;
{
    static edt_board_desc_table edt_boards ;
    static edt_bitfile_desc_table edt_bitfiles ;
    char *board_list, *bitfile_list ;
    int ret ;
    FILE *fp ;

#ifdef _NT_
    if (edt_system("runjava.bat PcdRequest"))
#else
    if (edt_system("./runjava PcdRequest"))
#endif
    {

	edt_perror("Java PcdRequest won't run; switching to text mode") ;
	puts("\n\n\n		Pcdrequest:  configure EDT PCI-CD boards.") ;

	ret = edt_probe_boards(edt_boards) ;
	if (ret) exit(1) ;

	ret = edt_read_bitfile_config("pcd_bitfiles", edt_bitfiles) ;
	if (ret) exit(1) ;

	board_list = get_board_list(edt_boards);
	bitfile_list = get_bitfile_list(edt_bitfiles);
	strcat(bitfile_list, board_list) ;
	puts(bitfile_list) ;

	ret = perform_menu_selection(edt_boards, edt_bitfiles) ;
	if (ret) 
	{
#ifdef _NT_
			fprintf(stderr, "Error detecting board(s). Make sure boards are installed properly");
			fprintf(stderr, " and drivers are loaded.\n");
#else
			fprintf(stderr, "Error detecting board(s). Try running 'make load'.\n");
#endif
			exit(1) ;
	}

	fputs("Press return to save the PCD configuration script: ", stdout) ;
	fflush(stdout) ;
	getchar() ;

	ret = create_pcdload_script(edt_boards) ;
	if (ret) exit(1) ;
    }

    if ((fp = fopen(PCDLOAD_FILE, "r")) != NULL)
    {
    	fclose(fp) ;

	if ((fp = fopen(RUN_PCDLOAD_FILE, "w")) != NULL)
	{
	    fputs("y\n", fp) ;
    	    fclose(fp) ;
	}
    }

    exit(0) ;
}

int
perform_menu_selection(edt_board_desc_table edt_boards, edt_bitfile_desc_table edt_bitfiles)
{
    int i ;
    int ret = 0 ;
    char *str ;

    for (i = 0; i < EDT_MAX_BOARDS; i++)
    {
	if (strncmp(edt_boards[i].board_name, "pcd", 3) == 0)
	{
	    str = edt_boards[i].intfc_bitfile = 
			    interface_bitfile_menu(i, edt_boards, edt_bitfiles);

	    if (str != NULL) ++ ret ;
	}
    }

    return (ret) ? 0 : -1 ;
}

char *
interface_bitfile_menu(int board_num, edt_board_desc_table edt_boards, edt_bitfile_desc_table edt_bitfiles)
{
    int i, selection = 0 ;
    edt_bitfile_desc *this = edt_get_bitfile_desc(
			    edt_boards[board_num].pci_flash_name, edt_bitfiles);

    printf("\n\n%s unit %d; PCI Xilinx \"%s\"; pci flash loaded \"%s\":\n", 
					      edt_boards[board_num].board_name,
						 edt_boards[board_num].unit_no,
					 edt_boards[board_num].pci_xilinx_type,
					 edt_boards[board_num].pci_flash_name);


    if (this && this->intfc_bitfile_count >= 1)
    {
	do
	{
	    puts("\n    Available interface bitfiles:\n") ;

	    printf("%5d. %-16s  -  %s\n", 0, "Do Not Configure",
		 "Applications will load the interface bitfile");

	    for (i = 0; i < this->intfc_bitfile_count; i++)
	    {
		printf("%5d. %-16s  -  %s\n", i+1,
					   this->intfc_bitfile_names[i],
					   this->intfc_bitfile_comments[i]);
	    }

	    puts("") ;
	    printf("	Enter Selection (0 - %d): ", this->intfc_bitfile_count);
	    fgets(instr, 127, stdin) ;
	    sscanf(instr, "%d", &selection) ;
	    puts("") ;

	    if (selection < 0 || selection > this->intfc_bitfile_count)
	    {
		puts("Illegal selection...\n") ;
		printf("\n%s unit %d; PCI Xilinx \"%s\"; pci flash loaded \"%s\":\n", 
				   edt_boards[board_num].board_name,
				      edt_boards[board_num].unit_no,
			      edt_boards[board_num].pci_xilinx_type,
			      edt_boards[board_num].pci_flash_name);
	    }

	} while (selection < 0 || selection > this->intfc_bitfile_count) ;


	printf("Selected %d: %s\n", selection,
				    (selection == 0) ? "Do Not Configure" :
				    this->intfc_bitfile_names[selection-1]);
	puts("") ;

	return (selection == 0) ?  NULL
				:  this->intfc_bitfile_names[selection-1] ;
    }
    else
    {
	puts("\n    No interface bitfiles are available for this flash.\n") ;
	fputs("Hit return to continue: ", stdout) ;
	fflush(stdout) ;
	fgets(instr, 127, stdin) ;
	return NULL ;
    }
}

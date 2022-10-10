/* #pragma ident "@(#)edt_bitfile.c	1.3 12/24/01 EDT" */

#include "edt_bitfile.h"

/**************************************************************************
 *
 * edt_bitfile API
 *
 *       Subroutines used to identify EDT boards and read a Xilinx
 *       configuration file with information about which interface
 *       bitfiles can be used with which PCI flash bitfiles. 
 *	 See /opt/EDTpcd/pcdrequest.c for a sample application.
 *
 * A typical application will start with:
 *
 *	#include "edt_bitfile.h"
 *
 *	static edt_board_desc_table edt_boards ;
 *	static edt_bitfile_desc_table edt_bitfiles ;
 *	
 *   edt_boards is a table of installed EDT boards, their type, unit number
 *   and which PCI bitfile is loaded in the flash prom.
 *
 *   edt_bitfiles is a table correlating PCI bitfiles with interface bitfiles,
 *   and each bitfile has a descriptive comment.  It is loaded from a text
 *   configuration file which lists each PCI bitfile and the interface
 *   bitfiles (and comments) that can be used with it.
 *
 * Make the following call to initialize edt_boards:
 *
 *	edt_probe_boards(edt_boards) ;
 *
 * Next initialize edt_bitfiles with:
 *
 *	edt_read_bitfile_config("cfg_file_name", edt_bitfiles) ;
 *
 * Finally, to search the edt_bitfiles table for a particular PCI bitfile,
 * call:
 *
 *	edt_bitfile_desc *ptr ;
 *
 *	ptr = edt_get_bitfile_desc(flash_name, edt_bitfiles) ;
 *
 * where flash_name is taken from edt_boards[i].pci_flash_name.
 * ptr now contains a pointer to the following struct:
 *
 * 	typedef struct {
 *	    char *pci_bitfile_name ;
 *	    int   intfc_bitfile_count ;
 *	    char *intfc_bitfile_names[64] ;
 *	    char *intfc_bitfile_comments[64] ;
 *	} edt_bitfile_desc ; 
 *
 * The intfc_bitfile_count variable contains the number of bitfile strings in
 * intfc_bitfile_names and intfc_bitfile_comments.
 *
 * See /opt/EDTpcd/pcdrequest for an example program.
 *
 *
 * The format of the bitfile config file is:
 *
 *	# Comments have a '#' in the leftmost column
 *	flash_file_name_1:
 *      flash_file_name_2:
 *            .
 *            .
 *      flash_file_name_N:
 *	< tab >bitfile_name_1	"Descriptive comment for bitfile_name_1"
 *	< tab >bitfile_name_2	"Descriptive comment for bitfile_name_1"
 *            	     .
 *            	     .
 *	< tab >bitfile_name_N	"Descriptive comment for bitfile_name_N"
 *
 *      flash_file_name_N+1:
 *	< tab >bitfile_name_N+1	"Descriptive comment for bitfile_name_N+1"
 *            .
 *            .
 *
 * Bitfile names must be preceeded by a tab character.  Empty lines and
 * lines beginning with '#' are ignored.  In the above example,
 * flash_file_name_1 through flash_file_name_N can use bitfile_name_1
 * through bit_file_name_N.  Here is an actual example for the PCD board:
 *
 *	pcd20.bit:
 *	pcd40.bit:
 *	pcd60.bit:
 *		pcd_src.bit	"16-bit parallel I/O synced to PCD source clock"
 *		pcd_looped.bit	"16-bit parallel I/O synced to external clock"
 *		ssd.bit		"One channel serial input"
 *		ssd2.bit	"Two channel serial input"
 *		ssd4.bit	"Four channel serial input"
 *		ssdio.bit	"Dual channel serial I/O"
 *		ssdout1.bit	"One channel serial output"
 *
 *	pcd16.bit:
 *		ssd16in.bit	"16 channel serial input"
 *		ssd16out.bit	"16 channel serial output"
 *
 **************************************************************************/

int
edt_probe_boards(edt_board_desc_table edt_boards)
{
    FILE *fp ;
    int need_unit_no = 1 ;
    int need_xilinx_type = 0 ;
    int curr_unit_no ;
    int list_entries = 0 ;
    int ret ;
    static char probe_str[512] ;
    static char board_name[128] ;
    static char curr_flash_name[128] ;

#ifdef _NT_
    if ((fp = _popen(".\\pciload -p", "r")) == NULL)
#else
    if ((fp = popen("./pciload -p", "r")) == NULL)
#endif
    {
	app_perror("./pciload -p") ;
	return -1 ;
    }

    memset(edt_boards, 0, sizeof(edt_board_desc_table)) ;

    while (fgets(probe_str, 511, fp))
    {
	/*
	 * Three states:  
	 *
	 *	1. need board name and unit number
	 *	2. need Xilinx type
	 *	3. need flash bitfile name
	 */
	if (need_unit_no)
	{
	    if ((ret = sscanf(probe_str, "%s unit %d", board_name,
						      &curr_unit_no)) == 2)
	    {
		edt_boards[list_entries].unit_no = curr_unit_no ;
		strncpy(edt_boards[list_entries].board_name, board_name, 127);
		need_unit_no = 0 ;
		need_xilinx_type = 1 ;
	    }
	}
	else if (need_xilinx_type)
	{
	    char *p = strrchr(probe_str, ' ') ;

	    if (p)
		*p = '\0' ;

	    if (p = strrchr(probe_str, ' '))
		strcpy(edt_boards[list_entries].pci_xilinx_type, p + 1) ;
	    else
		strcpy(edt_boards[list_entries].pci_xilinx_type, probe_str) ;

	    need_xilinx_type = 0 ;
	}
	else 		/* need flash bitfile name */
	{
	    int sector_num ;

	    ret = sscanf(probe_str, "  Sector %d PROM id: <%s", &sector_num,
						   &curr_flash_name) ;
	    if (ret == 1 && sector_num == 3)
	    {
		char *p = strchr(probe_str, '3') ;

		p += 2 ;
		if (strncmp(p, "No program id", 13) == 0)
		{
		    strcpy(edt_boards[list_entries].pci_flash_name,
						"No program id") ;
		}
		else
		{
		    strcpy(edt_boards[list_entries].pci_flash_name,
				      "Unrecognized program id") ;
		}
		need_unit_no = 1 ;
		++ list_entries ;
	    }
	    else if (ret == 2)
	    {
		/* strip off .ncd or .bit extension */
		char *p = strrchr(curr_flash_name, '.') ;

		if (p)
		    *p = '\0' ;

		p = strrchr(curr_flash_name, '_') ;

		/* strip off _3v or _5v substring */
		if (p && 
		      (( *(p+1) == '3' || *(p+1) == '5') && *(p+2) == 'v' ))
		    *p = '\0' ;
			
		strcpy(edt_boards[list_entries].pci_flash_name,
					    curr_flash_name) ;
		strcat(edt_boards[list_entries].pci_flash_name, ".bit") ;
		need_unit_no = 1 ;
		++ list_entries ;
	    }
	}
    }

    return 0 ;
}

edt_bitfile_desc *edt_get_bitfile_desc(char *flash_name, edt_bitfile_desc_table edt_bitfiles)
{
    int i ;

    for (i = 0; i < EDT_MAX_BITFILES; i++)
    {
	if (edt_bitfiles[i].pci_bitfile_name)
	{
	    if (strcmp(flash_name, edt_bitfiles[i].pci_bitfile_name) == 0)
		return &edt_bitfiles[i] ;
	}
    }
    return NULL ;
}

int
edt_read_bitfile_config(char *cfg_file_name,edt_bitfile_desc_table edt_bitfiles)
{
    FILE *cfg_fp ;
    struct stat sbuf ;
    char *cfg_buf, *cfg_ptr, *ptr ;
    int i, cfg_size, curr_pci, shared_pcis, curr_intfc,
	need_flash_names, need_intfc_names, line_no ;

    if ((cfg_fp = fopen(cfg_file_name, "rb")) == NULL)
    {
	app_perror(cfg_file_name) ;
	return -1 ;
    }

    memset(edt_bitfiles, 0, sizeof(edt_bitfile_desc_table)) ;

    fstat(fileno(cfg_fp), &sbuf) ;
    cfg_size = sbuf.st_size ;

    cfg_buf = malloc(cfg_size+1) ;
    fread(cfg_buf, 1, cfg_size, cfg_fp) ;
    cfg_buf[cfg_size] = '\0' ;
    fclose(cfg_fp) ;

    cfg_ptr = cfg_buf ;
    curr_pci = 0 ;
    shared_pcis = 0 ;
    curr_intfc = 0 ;
    need_flash_names = 1 ;
    need_intfc_names = 0 ;
    line_no = 1 ;
    while (cfg_ptr < cfg_buf + cfg_size)
    {
	if (need_flash_names)
	{
	    if (*cfg_ptr != '\n')
	    {
		if (*cfg_ptr == ' ' || *cfg_ptr == '\t')
		{
		    if (shared_pcis == 0)
		    {
			printf("%s:  <1>syntax error on line %d\n",
					   cfg_file_name, line_no);
			return -1 ;
		    }
		    need_flash_names = 0 ;
		    need_intfc_names = 1 ;
		}
		else
		{
		    if (*cfg_ptr == '#')
		    {
			++ line_no ;
			while (*cfg_ptr != '\n')
			    ++ cfg_ptr ;
		    }
		    else
		    {
			edt_bitfiles[curr_pci + shared_pcis].pci_bitfile_name
									= cfg_ptr;

			if (ptr = strchr(cfg_ptr, ':'))
			{
			    *ptr = '\0' ;
			    cfg_ptr = ptr + 1 ;
			}
			else
			{
			    printf("%s:  <2>syntax error on line %d\n",
			    		       cfg_file_name, line_no);
			    return -1 ;
			}

			if (ptr = strchr(cfg_ptr, '"'))
			{
			    cfg_ptr = ptr + 1 ;

			    edt_bitfiles[curr_pci + shared_pcis].
			           pci_bitfile_comment = cfg_ptr;

			    if (ptr = strchr(cfg_ptr, '"'))
			    {
				    *ptr = '\0' ;
				    cfg_ptr = ptr + 1 ;

				    while (*cfg_ptr != '\n')
				    {
					++ cfg_ptr ;
				    }

			    }
			    else
			    {
				    printf("%s:  <2.5>syntax error on line %d\n",
						       cfg_file_name, line_no);
				    return -1 ;
			    }
			}

			++ shared_pcis ;
		    }
		}
	    }
	    else
	    {
		++ line_no ;
		++ cfg_ptr ;
	    }
	}
	else if (need_intfc_names)
	{
	    if (*cfg_ptr != '\n')
	    {
		while (*cfg_ptr == ' ' || *cfg_ptr == '\t')
		    ++ cfg_ptr ;

		for (i = 0; i < shared_pcis; i++)
		    edt_bitfiles[curr_pci + i].
			     intfc_bitfile_names[curr_intfc] = cfg_ptr;

		while (*cfg_ptr != ' ' && *cfg_ptr != '\t')
		    ++ cfg_ptr ;

		*cfg_ptr++ = '\0' ;

		if (ptr = strchr(cfg_ptr, '"'))
		{
		    cfg_ptr = ptr + 1 ;

		    for (i = 0; i < shared_pcis; i++)
			edt_bitfiles[curr_pci + i].
			     intfc_bitfile_comments[curr_intfc] = cfg_ptr ;

		    if (ptr = strchr(cfg_ptr, '"'))
		    {
			*ptr = '\0' ;
			cfg_ptr = ptr + 1 ;

			while (*cfg_ptr != '\n')
			    ++ cfg_ptr ;
		    }
		    else
		    {
			printf("%s:  <3>syntax error on line %d\n",
					   cfg_file_name, line_no);
			return -1 ;
		    }
		}
		else
		{
		    printf("%s:  <4>syntax error on line %d\n", cfg_file_name,
								     line_no);
		    return -1 ;
		}

		++ curr_intfc ;
	    }
	    else
	    {
		for (i = 0; i < shared_pcis; i++)
		    edt_bitfiles[curr_pci + i].intfc_bitfile_count = curr_intfc;

		curr_intfc = 0 ;
		curr_pci += shared_pcis ;
		shared_pcis = 0 ;
		need_intfc_names = 0 ;
		need_flash_names = 1 ;
	    }
	}

	if (*cfg_ptr == '\n')
	{
	    ++ cfg_ptr ;
	    ++ line_no ;
	}
    }

    if (need_intfc_names)
    {
	for (i = 0; i < shared_pcis; i++)
	    edt_bitfiles[curr_pci + i].intfc_bitfile_count = curr_intfc;
    }

    return 0 ;
}

static char instr[128] ;

int
create_pcdload_script(edt_board_desc_table edt_boards)
{
    int i ;

#ifdef _NT_
	FILE *fp = fopen("\\edt\\pcd\\pcdload.bat", "w") ;
	puts("Creating \\edt\\pcd\\pcdload.bat") ;
#else
	FILE *fp = fopen("/opt/EDTpcd/pcdload", "w") ;
	puts("Creating /opt/EDTpcd/pcdload") ;
#endif

	if (fp == NULL)
	{
	    app_perror("/opt/EDTpcd/pcdload") ;
	    return -1 ;
	}

#ifdef _NT_
	fputs("REM\nREM See \\edt\\pcd\\pcd_bitfiles for flash and bitfile information.\nREM\n", fp) ;
#else
	fputs("#! /bin/csh -f\n\n", fp) ;
	fputs("#\n# See /opt/EDTpcd/pcd_bitfiles for flash and bitfile information.\n#\n\n", fp) ;
#endif

    for (i = 0; i < EDT_MAX_BOARDS; i++)
    {
	if (strncmp(edt_boards[i].board_name, "pcd", 3) == 0)
        {
#ifdef _NT_
    	    fprintf(fp, "REM\nREM PCD unit %d PCI Xilinx %s flash %s:\nREM\n",
#else
    	    fprintf(fp, "#\n# PCD unit %d PCI Xilinx %s flash %s:\n#\n",
#endif
    	        edt_boards[i].unit_no,
    	        edt_boards[i].pci_xilinx_type,
    	        edt_boards[i].pci_flash_name) ;
    
    	    if (edt_boards[i].intfc_bitfile)
    	    {
#ifdef _NT_
    	        fprintf(fp, ".\\bitload -u %d %s\n\n",
#else
    	        fprintf(fp, "./bitload -u %d %s\n\n",
#endif
    			       edt_boards[i].unit_no,
    		        edt_boards[i].intfc_bitfile);
    	    }
    	    else
    	    {
    	        fputs("\n", fp) ;
    	    }
        }
    }

    fclose(fp) ;
#ifndef _NT_
    system("chmod a+x /opt/EDTpcd/pcdload") ;
#endif

    return 0 ;
}

void
app_perror(char *str)
{
    perror(str) ;
}

char *
get_board_list(edt_board_desc_table edt_boards)
{
    int i ;
    char *str = calloc(4*1024, 1) ;

    strcpy(str, "                        Installed EDT boards:\n\n") ;
    for (i = 0; i < EDT_MAX_BOARDS; i++)
    {
	if (*edt_boards[i].board_name)
	{
	    sprintf(str + strlen(str), "\t%s unit %d: PCI Xilinx \"%s\" flash name \"%s\"\n", 
					    edt_boards[i].board_name,
					    edt_boards[i].unit_no,
					    edt_boards[i].pci_xilinx_type,
					    edt_boards[i].pci_flash_name);
	}
    }
    strcat(str, "\n") ;
    return str ;
}

char *
get_bitfile_list(edt_bitfile_desc_table edt_bitfiles)
{
    int i, j ;
    char *str = calloc(16*1024, 1) ;

    sprintf(str + strlen(str), "\n%50s\n\n\n", "Configured EDT bitfiles:") ;
    for (i = 0; i < EDT_MAX_BITFILES; i++)
    {
	if (edt_bitfiles[i].pci_bitfile_name)
	{
	    sprintf(str + strlen(str), "%-20s%s\n", edt_bitfiles[i].pci_bitfile_name,
				        edt_bitfiles[i].pci_bitfile_comment);

	    for (j = 0; j < edt_bitfiles[i].intfc_bitfile_count; j++)
		sprintf(str + strlen(str), "\t%-20s%s\n",
				     edt_bitfiles[i].intfc_bitfile_names[j],
				     edt_bitfiles[i].intfc_bitfile_comments[j]);
	    sprintf(str + strlen(str), "\n") ;
	}
    }
    sprintf(str + strlen(str), "\n") ;

    return str ;
}

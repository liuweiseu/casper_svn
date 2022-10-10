/* #pragma ident "@(#)initedt.c	1.12 10/06/04 EDT" */

#include "edtinc.h"
#include "initedt.h"
#include <stdlib.h>

/******************************************************************************
 *                                                                            * 
 * initedt:  Support routines for initpcd (or initXXX, XXX is an EDT product. *
 *                                                                            * 
 * Commands are defined and implemented in initpcd.c and initpcd.h	      *
 *                                                                            * 
 ******************************************************************************/

/*
 * Open file variables.
 */
int	unit = 0 ;
char	*initedt_config_file = NULL ;
EdtDev	*initedt_p ;


/*
 * Flow control and line parser loop variables.
 */
int	initedt_verbose = 0 ;
int	dump_cmds 	= 0 ;
int	run_cmds 	= 1 ;
int	initedt_load_bitfile 	= 1 ;
int	lineno 		= 0 ;
char	input_line[512] ;
char   *prog_name ;


/*
 * External declarations from C file defining a config interface.
 * Example:  see initpcd.c
 */
cmd_operator get_cmd_copy(int) ;
char *get_cmd_name(int cmd_index) ;
int cmd_table_entries() ;


/******************************************************************************
 *                                                                            * 
 * Functions are listed in reverse calling order starting with                *
 * main() at the bottom of this file.  Data declarations are listed first.    *
 *                                                                            * 
 ******************************************************************************/

cmd_operator *cmd_list = NULL ;

cmd_operator *
create_queued_command(int cmd_index)
{
    static cmd_operator **cmd_tail = &cmd_list ;
    cmd_operator *cmd_new ;

    cmd_new = calloc(1, sizeof(cmd_operator)) ;

    *cmd_new = get_cmd_copy(cmd_index) ;

    *cmd_tail = cmd_new ;

    cmd_tail = &(cmd_new->cmd_next) ;

    *cmd_tail = NULL ;

    return cmd_new ;
}

/******************************************************************************
 *                                                                            *
 * process_cmds() carries out the work for each line found in the config      *
 * file.  Add code for new commands here; see "MATCHES()" below.	      *
 *                                                                            *
 ******************************************************************************/
int
process_cmds()
{
    char *cmd_name ;
    int cmd_table_size = cmd_table_entries() ;
    int (*func)(cmd_operator *) ;
    cmd_operator *cmd_ptr ;

    if (initedt_verbose)
	puts("\nExecuting commands:\n") ;

    for (cmd_ptr = cmd_list; cmd_ptr; cmd_ptr = cmd_ptr->cmd_next)
    {
	cmd_name = cmd_ptr->cmd_name ;

	if (initedt_verbose)
	{
	    printf("\n%-16s arg1: %-4x arg2: 0x%-4x argf: %-10lf",
		cmd_ptr->cmd_name,
		cmd_ptr->cmd_intval1,
		cmd_ptr->cmd_intval2,
		cmd_ptr->cmd_realval) ;
	    if (cmd_ptr->cmd_pathval)
		printf(" pathval: %s", cmd_ptr->cmd_pathval) ;
	    puts("\n") ;
	}


	/*
	 * Execute the code for this function.
	 */
	func = cmd_ptr->cmd_func ;
	if ((*func)(cmd_ptr) != 0)
	{
	    if (initedt_verbose)
		fprintf(stderr, "\n\"%s\": command failed.  Aborting.\n\n",
								cmd_name) ;
	    return -1 ;
	}
    }
    return 0 ;
}

void
print_cmds()
{
    cmd_operator *cmd_ptr ;

    puts("") ;
    for (cmd_ptr = cmd_list; cmd_ptr; cmd_ptr = cmd_ptr->cmd_next)
    {
	printf("%-16s arg1: %-4x arg2: 0x%-4x argf: %-10lf",
	    cmd_ptr->cmd_name,
	    cmd_ptr->cmd_intval1,
	    cmd_ptr->cmd_intval2,
	    cmd_ptr->cmd_realval) ;
	if (cmd_ptr->cmd_pathval)
	    printf(" pathval: %s", cmd_ptr->cmd_pathval) ;
	puts("") ;
    }
    puts("") ;
}

int
get_cmd_index(char *name)
{
    int i ;
    int retval = -1 ;
    int cmd_table_size = cmd_table_entries() ;

    /*
     * The list is short, so a linear search will suffice.
     */
    for (i = 0; i < cmd_table_size; i++)
    {
	if (strcmp(get_cmd_name(i), name) == 0)
	    return i ;
    }
    return retval ;
}

int
get_pathval(char *cmd, char *s)
{
    /* Find the first ':' */
    while (*cmd != ':' && *cmd != '\0')
	++ cmd ;
    if (*cmd == '\0')
	return 1 ;
    ++ cmd ;

    /* Now move to the first non-space character */
    while ((*cmd == ' ' || *cmd == '\t') && *cmd != '\0')
	++ cmd ;
    if (*cmd == '\0')
	return 1 ;

    /* Copy from here to end of line; translate $u to unit number */
    while (*cmd != '\0')
    {
	if (*cmd == '$' && *(cmd+1) == 'u')
	{
	    *s++ = '0' + unit ;
	    cmd += 2 ;
	}
	else
	    *s++ = *cmd++;
    }
    
    /* Truncate any white space from end of line */
    while (*(s-1) == ' ' || *(s-1) == '\r' || *(s-1) == '\n' || *(s-1) == '\t')
	-- s ;
    *s = '\0' ;

    return 0 ;
}


int
cmd_line_parse(char *cmd_line)
{
    char	  cmd_name[128] ;
    char	  cmd_format[128] ;
    int		  cmd_index ;
    u_int	  arg1, arg2 ;
    double	  argf ;
    char         *path_ptr ;
    cmd_operator *cmd_ptr ;

    /*
     * First get the command name.
     */
    if (sscanf(cmd_line, "%[0-9a-zA-Z_]", cmd_name) != 1)
	return 1 ;
    
    /*
     * Next get the command table index for this command.
     */
    if ((cmd_index = get_cmd_index(cmd_name)) == -1)
    {
	fprintf(stderr,"\nSyntax error on line %d, no such command:\n", lineno);
	fputs("\n\t", stderr) ;
	fputs(input_line, stderr) ;
	fputs("\n", stderr) ;
	fputs("Fatal error.\n\n", stderr);
	exit(1);
    }

    /*
     * Allocate and queue a command operator.
     */
    cmd_ptr = create_queued_command(cmd_index) ;


    /*
     * Command argument switch:
     *     1 = get one hex integer argument,
     *	   2 = get two hex integer arguments,
     *	   3 = get one real argument,
     *	   4 = get one pathname argument.
     */
    switch (cmd_ptr->cmd_type)
    {
    case 1:

	/*
	 * Prepare format string and argument pointers for sscanf().
	 */
	sprintf(cmd_format, "%s: %%x", cmd_ptr->cmd_name) ;

	if (sscanf(cmd_line, cmd_format, &arg1) != 1)
	{
	    fputs("\nExpected one hex argument.", stderr) ;
	    return 1 ;
	}
	cmd_ptr->cmd_intval1 =  arg1 ;

	break ;

    case 2:

	/*
	 * Prepare format string and argument pointers for sscanf().
	 */
	sprintf(cmd_format, "%s: %%x %%x", cmd_ptr->cmd_name) ;

	if (sscanf(cmd_line, cmd_format, &arg1, &arg2) != 2)
	{
	    fputs( "\nExpected two hex integer arguments", stderr) ;
	    return 1 ;
	}

	cmd_ptr->cmd_intval1 =  arg1 ;
	cmd_ptr->cmd_intval2 =  arg2 ;

	break ;

    case 3:

	/*
	 * Prepare format string and argument pointers for sscanf().
	 */
	sprintf(cmd_format, "%s: %%lf", cmd_ptr->cmd_name) ;

	if (sscanf(cmd_line, cmd_format, &argf) != 1)
	{
	    fputs( "\nExpected one real argument", stderr) ;
	    return -1 ;
	}

	cmd_ptr->cmd_realval =  argf ;

	break ;

    case 4:
	/*
	 * Allocate memory to hold path string.
	 */
	path_ptr = malloc(strlen(cmd_line)) ;
	
	if (get_pathval(cmd_line, path_ptr))
	{
	    fputs( "\nExpected one pathname or string argument", stderr) ;
	    return -1 ;
	}

	cmd_ptr->cmd_pathval =  path_ptr ;

	break ;
    }

    return 0 ;
}

int
is_comment_or_empty(char *str)
{
    while (*str == ' ' || *str == '\t')
	++str ;

    if (*str == '#' || *str == '\n' || *str == '\r' || *str == '\0')
	return 1 ;

    return 0 ;
}

int
input_config_file(char *config_file)
{  
    int cmd_table_size = cmd_table_entries() ;
    FILE	*cfg_fp ;

    if ((cfg_fp = fopen(config_file, "r")) == NULL)
    {
	return -1 ;
    }

    while (fgets(input_line, 511, cfg_fp))
    {
	lineno++;

	if (initedt_verbose)
	    fputs(input_line, stdout);

	if (is_comment_or_empty(input_line))
	    continue;

	if (cmd_line_parse(input_line))
	{
	    fprintf(stderr, "\nSyntax error on line %d:\n", lineno) ;
	    fputs("\n\t", stderr) ;
	    fputs(input_line, stderr) ;
	    fputs("\n", stderr) ;
	    fputs("Fatal error.\n\n", stderr);
	    exit(1);
	}
    }

    return 0 ;
}

static void usage()
{
    printf( "\nUsage: %s [-u unit#] [-v] [-p] [-n] -f <file.cfg>\n", prog_name);
    printf("    -u unit#      -  %s unit select (default unit 0).\n",
								EDT_INTERFACE);
    puts("    -v	    -  Verbose - print lines from cfg file.");
    puts("    -p	    -  Print commands - print commands after parsing.");
    puts("    -n	    -  No execute - don't run the commands.");
    puts("    -f <file.cfg> -  Configuration file name - required.\n");
}

int
handle_args(int argc, char **argv)
{
    prog_name = argv[0] ;

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{

	case 'u':
	    {
		argv++;
		argc--;
		if (argc > 0)
		    unit = atoi(argv[0]);

		break;
	    }
	case 'v':
	    {
		initedt_verbose = 1 ;
		break;
	    }
	case 'p':
	    {
		dump_cmds = 1 ;
		break;
	    }
	case 'n':
	    {
		run_cmds = 0 ;
		break;
	    }
	case 'N':
	    {
		initedt_load_bitfile = 0 ;
		break;
	    }
	case 'f':
	    {
		argv++;
		argc--;
		if (argc > 0)
		    initedt_config_file = argv[0];
		break;
	    }
	default:
	    {
		usage() ;
		return -1 ;
		break ;
	    }
	}
	--argc;
	++argv;
    }

    if (initedt_config_file == NULL)
    {
	fputs("\n", stderr) ;
	usage() ;
	return -1 ;
    }

    if ((initedt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
	char str[64] ;
	sprintf(str, "\n/dev/%s%d", EDT_INTERFACE, unit) ;
	perror(str);
	usage() ;
	return -1 ;
    }

    return 0 ;
}

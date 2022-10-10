/* #pragma ident "@(#)initedt.h	1.1 05/20/02 EDT" */

#ifndef _INITEDT_H
#define _INITEDT_H

typedef struct cmdop {
    char         *cmd_name ;			/* Name	in cfg file	 */
    int		  cmd_type ;			/* Command argument type */
    int	        (*cmd_func)(struct cmdop *) ;	/* Function to call	 */
    struct cmdop *cmd_next ;			/* Pointer to next in list */
    u_int	  cmd_intval1 ;			/* First integer arg	 */
    u_int	  cmd_intval2 ;			/* Second integer arg	 */
    double	  cmd_realval ;			/* Floating arg		 */
    char         *cmd_pathval ;			/* Pathname or string	 */
} cmd_operator ;


#endif /* _INITEDT_H */

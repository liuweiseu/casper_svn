/* #pragma ident "@(#)pcdout.c	1.4 12/16/02 EDT" */

#include "edtinc.h"

void    usage();

main(argc, argv)
    int     argc;
    char  **argv;
{
    int     loops = 1000;
    int     interactive = 1;
    char    error_string[20];
    int     unit = 0;
    int     i;
    u_short **bufs;
    int     counter = 0;
    EdtDev *edt_p;
    int     bufsize = 1024 * 1024;



    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'u':
	    ++argv;
	    --argc;
	    unit = atoi(argv[0]);
	    break;

	case 'h':
	    usage();
	    return 0;
	    break;

	case 'l':
	    ++argv;
	    --argc;
	    loops = atoi(argv[0]);
	    break;

	case 'a':
	    interactive = 0;
	    break;


	    /*** for future use?
	    case 'v':
	        verbose = 1;
	        break;
	    ****/


	default:
	    usage();
	    exit(0);
	}
	argc--;
	argv++;
    }


    printf("edt_open unit %d\n", unit);
    if ((edt_p = edt_open("pcd", unit)) == NULL)
    {
	sprintf(error_string, "edt_open(%d)", unit);
	edt_perror(error_string);
	return 1;
    }
    edt_configure_ring_buffers(edt_p, bufsize, 2, EDT_WRITE, NULL);
    bufs = (u_short **) edt_buffer_addresses(edt_p);

    for (i = 0; i < bufsize / 2; i++)
    {
	bufs[0][i] = counter;
	bufs[1][i] = counter++;
    }

    if (interactive)
    {
	puts("Hit return to start");
	getchar();
    }

    edt_flush_fifo(edt_p);
    edt_start_buffers(edt_p, 0);

    for (i = 0; loops == 0 || i < loops; i++)
    {
	edt_wait_for_buffers(edt_p, 1);
	putchar('o');
	fflush(stdout);
    }
}


void
usage()
{
    puts("pcdout --  Basic test for high speed, 16 bit parallel");
    puts("usage: pcdout [-u unit-number] [-h]");
    puts("-u N	 sets unit number - default 0");
    puts("-h	 prints this usage message");
    puts("NOTE: run this test at after you've started pcdin.");
}

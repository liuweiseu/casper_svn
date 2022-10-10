/* #pragma ident "@(#)pcdin.c	1.4 12/16/02 EDT" */

#include "edtinc.h"

void    usage();

main(argc, argv)		/* pcdin.c */
    int     argc;
    char  **argv;
{
    int     i, counter = 0;
    int     interactive = 1;
    int     loops = 1000;
    int     unit = 0;
    char    error_string[20];
    u_short *buf, *b, *testbuf;
    u_int     bufsize = 1024 * 1024;
    u_int   offset;
    double  deltatime;
    EdtDev *edt_p;


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

	case 'l':
	    ++argv;
	    --argc;
	    loops = atoi(argv[0]);
	    break;

	case 'h':
	    usage();
	    return 0;
	    break;

	    /*** for future use?
	    case 'v':
	        verbose = 1;
	        break;
	    ****/


	case 'a':
	    interactive = 0;
	    break;


	default:
	    usage();
	    exit(0);
	}
	argc--;
	argv++;
    }


    printf("edt_open unit %d\n", unit);
    edt_p = edt_open("pcd", unit);
    if (edt_p == NULL)
    {
	sprintf(error_string, "edt_open(%d)", unit);
	edt_perror(error_string);
	return 1;
    }


    testbuf = (u_short *) edt_alloc(bufsize * 2);
    buf = (u_short *) edt_alloc(bufsize);
    for (i = 0; i < (int)(bufsize / 2) + 1; i++)
	testbuf[i] = i;
    for (i = 0; i < (int)(bufsize / 2) + 1; i++)
	testbuf[i + (bufsize / 2)] = i;


    edt_configure_ring_buffers(edt_p, bufsize, 4, EDT_READ, NULL);


    if (interactive)
    {
	puts("hit return to continue");
	getchar();
    }

    edt_flush_fifo(edt_p);
    edt_dtime();
    edt_start_buffers(edt_p, 0);

    for (i = 0; loops == 0 || i < loops; i++)
    {
	memcpy(buf, b = (u_short *) edt_wait_for_buffers(edt_p, 1), bufsize);
	offset = buf[0];

	if (offset < (bufsize / 2))
	{
	    if (memcmp(buf, &testbuf[offset], bufsize) == 0)
	    {
		putchar('.');
		fflush(stdout);
	    }
	    else
	    {
		FILE   *fp;

		edt_abort_dma(edt_p);
		fp = fopen("bad", "wb");
		fwrite(buf, 1, bufsize, fp);
		fclose(fp);
		fp = fopen("good", "wb");
		fwrite(&testbuf[offset], 1, bufsize, fp);
		fclose(fp);
		puts("output to \"good\" and \"bad\"");
		exit(1);
	    }
	}
	else
	{
	    printf(" %x:%x(%x)	", b[100000], offset, bufsize / 2);
	    fflush(stdout);
	}
    }
    deltatime = edt_dtime();
    printf("\n%d bytes in %lf seconds for %lf bytes/sec\n\n",
	   bufsize * i, deltatime, ((double) (bufsize * i)) / deltatime);
}


void
usage()
{
    puts("pcdin --  Basic test for high speed, 16 bit parallel");
    puts("usage: pcdin [-u unit-number] [-h]");
    puts("-u N	 sets unit number - default 0");
    puts("-h	 prints this usage message");

}

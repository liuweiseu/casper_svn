/* #pragma ident "@(#)sserot.c	1.3 07/17/01 EDT" */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    unsigned int bcnt=0, errcnt=0, b1=0, b2=0, v1=0, v2=0, rot=0, lasterr=0;
    FILE *fdin, *fdout;


    if ((argc<3) || (argc>4) || (sscanf(argv[1],"%x", &rot) != 1) || (rot>7)) {
	fprintf(stdout,"Usage: %s 0-7 infile [outfile] \n", argv[0]);
	exit(0);
    }

    fdin = fopen(argv[2], "rb");
    if (argc>3) fdout = fopen(argv[3], "wb");
    else        fdout = stdout;

    b1=getc(fdin);
    while ((b2=getc(fdin)) != EOF) {
	v2 = ((b2<<8 | b1) >> rot) & 0xff;
	if (((v1+1)&0xff) != v2 && bcnt > 0) {
	    lasterr = bcnt;
	    if (((errcnt++) < 100) && (argc>3))  {
		fprintf(stdout,
		    "At byte %x, expected %02x, got %02x (from %02x + %02x)\n", 
						       bcnt, v1+1, v2, b2, b1);
	    }
	}
	if (argc>3) putc(v2, fdout);
	v1=v2; b1=b2; bcnt++;
    }
    fprintf(stdout, "Rot:%d  Errors:%10d  Bytes tested:%10d  LastErr:%10d\n",
					      rot, errcnt, bcnt, lasterr);
	 exit(0);
}



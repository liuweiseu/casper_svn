

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "edtinc.h"

#include "libedt.h"
#include "edtreg.h"

#include "chkbuf.h"


#define NLOOPS 1

#define BUFSIZE 0x100000

int
main(int argc, char **argv)
{	int n;
	int fd ;
	int ret ;
	EdtDev *edt_p ;
	char *filename;
	EdtDev edt_tmp ;
	u_int tmp_p = (u_int) edt_alloc(BUFSIZE + 0x1000) ;
	u_short *testbuf ;
    int i ;
	edt_p = &edt_tmp ;
	system("bitload -u pdv0 xtest.bit"); 
	if (argc > 1)
		filename = argv[1];
	else
		filename = "/dev/pdv0";

	fd = open(filename, 2) ;
	edt_p->fd = fd ;
	tmp_p = (tmp_p + 0xfff) & ~(0xfff) ;
	printf("starting xtest data\n") ;

	for (n = 0; n < NLOOPS; n++) 
	{

	edt_intfc_write_short(edt_p, XTEST_CMD, XTEST_SWAPBYTES) ;
	edt_intfc_write_short(edt_p, XTEST_CMD, XTEST_UN_RESET_FIFO) ;
	edt_intfc_write_short(edt_p, XTEST_CMD, XTEST_SWAPBYTES | XTEST_UN_RESET_FIFO | XTEST_EN_DATA) ;
	testbuf = (u_short *)tmp_p ;
	for(i = 0 ; i < (BUFSIZE/2) ; i++) testbuf[i] = 0xaa55 ;
	printf("return to read %p",testbuf) ; getchar();
	ret = read(fd,testbuf,BUFSIZE) ;
	printf("read returned %x\n", ret);
	printf("return to read data") ; getchar();

	for (i=0;i<(BUFSIZE-4)/2;i += 0x200)
	{

	  printf("%04x: %04x %04x %04x %04x\n",
	   i,
		testbuf[0],
	   testbuf[1],
	   testbuf[2],
	   testbuf[3]) ;

		testbuf += 0x200;
	}

	}

	/* chkbuf_short(tmp_p, BUFSIZE/2, 0, 0); */

	printf("\nreturn to close") ; getchar();
	close(fd) ;
	exit(0) ;
}









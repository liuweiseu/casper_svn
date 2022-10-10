/* #pragma ident "@(#)ssepdb.c	1.5 05/28/02 EDT" */

/*
 * Debugger and utility for the SSE board.  Jerry Gaffke
 */

/*
Looks like the only stuff that touches edt_intfc_write(), edt_intfc_read() is
  pw8(), pw16, pr8, pr16   (used only by r, w, r16, w16 commands)
  spal() and wpp()

should remove the two edt_msleep(1)'s in spal(), use BUSY polling
does spal() really need to shut down AVSEL, or just remember to do "fm 50"?
*/


#include "edtinc.h"
#include "edt_vco.h"
#include <stdlib.h>
#include <ctype.h>
#ifndef _NT_
#include <sys/ioctl.h>
#endif

int   gspv=0;
u_short dbgflg =0;
EdtDev *edt_p ;

char *load_xilinx(FILE *infp, int xlf);
void  wpp(int val);
int   rpp();
int spal(int v);
void xosc(int val);
double  fosc(double fmhz);
void dly(int d1, int d2);


int
test_mac8100(EdtDev *fd, u_int add, int count)
{
    u_short readval, writeval;
    int loop;
    int errors = 0;
 
    while(count--)
    {
	writeval = 0x1;
	for (loop = 0; loop != 16; loop++)
	{
	    edt_write_mac8100(fd, add, writeval);
	    if ( (readval = edt_read_mac8100(fd, add)) != writeval)
	    {
		printf("mac8100 failed offset= %d was %04x s/b %04x\n",
			add, readval, writeval );
		errors++;
	    }
	    writeval <<= 1;
	} 
	writeval = 0xfffe;
	for (loop = 0; loop != 16; loop++)
	{
	    edt_write_mac8100(fd, add, writeval);
	    if ( (readval = edt_read_mac8100(fd, add)) != writeval)
	    {
		printf("mac8100 failed offset= %d was %04x s/b %04x\n",
			add, readval, writeval );
		errors++;
	    }
	    writeval = (writeval << 1) | 0x1;
	} 
    }
    return(errors);
}


void
dump_mac8100(void *fd)
{
    int i;

    for(i=0; i!=132; i++)
    {
	if (i%8 == 0)
		printf("\n%4x(%3d) ", i, i);
	printf(" %04x", edt_read_mac8100(fd, i));
    }
    printf("\n");
}



#ifndef _NT_

#ifdef __linux__

#include <asm/ioctls.h>

#else

#include <sys/filio.h>		/* for kbhitedt() */

#endif
int kbhitedt()
{
	 int numchars ;
    ioctl(0, FIONREAD, &numchars) ;
    /* for (n=0; n<numchars; n++)  ch=getchar() ;*/
    return(numchars);
}
#else
int _kbhit() ;

int kbhitedt()
{
    return(_kbhit()) ;
}
#endif /* _NT_ */

int wait_for_first = 0 ;
int runit = 0;
static char pdb_sendmsg[512] ;
static char msgbuf[512] ;
void do_connect(EdtDev *fd, char *infile)
{
    int retval ;
    int ret ;

	if (infile)
	{
		FILE *fp ;

		if (strcmp(infile, "-") == 0)
			fp = stdin ;
		else
			fp = fopen(infile, "rb") ;

		if (fp == NULL)
		{
			perror(infile) ;
			return ;
		}
		
		while(fgets(pdb_sendmsg, 511, fp))
		{
		    if (pdb_sendmsg[0] == 'Q') return ;
		    *(strrchr(pdb_sendmsg, '\n')) = '\0' ;
		    printf("sending %s\n",pdb_sendmsg) ;
		    ret = edt_send_msg(fd, runit, pdb_sendmsg, strlen(pdb_sendmsg));
		    if (ret)
		    {
		    	printf("bad send msg - dumping reg\n") ;
				dump_mac8100(fd) ;
				printf("return to continue\n") ;
			}
		    retval = 0 ;
		    while (retval == 0)
		    {
		    retval = edt_get_msg(fd, msgbuf, sizeof(msgbuf));
		    if (retval != 0)
			    printf("[%s]\n", msgbuf) ;
		    }
		}
	}
	else
	{
		int ii  ;
		char answer[10] ;
		puts("connect mode - Q to quit") ;
		if (wait_for_first)
		{
			printf("waiting for first message\n") ;
			retval = 0 ;
			while(retval == 0)
			{
				retval = edt_get_msg(fd, msgbuf, sizeof(msgbuf));
			}
			printf("got %d bytes\n",retval) ;
			wait_for_first = 0 ;
			for(ii = 0 ; ii < retval ; ii++)
			{
				printf("%02x ", msgbuf[ii]) ;
			}
			printf("\n") ;
			printf("continue? ") ; gets(answer) ;
			if (answer[0] != 'y')
			{
				return ;
			}
		}


		for(;;)
		{
			if (kbhitedt())
			{
				gets(pdb_sendmsg) ;
				if (pdb_sendmsg[0] == 'Q') return ;
				ret = edt_send_msg(fd, runit, pdb_sendmsg, strlen(pdb_sendmsg));
				if (ret)
				{
					printf("bad send msg - dumping reg\n") ;
					dump_mac8100(fd) ;
					printf("return to continue\n") ;
				}
			}
			retval = edt_get_msg(fd, msgbuf, sizeof(msgbuf));
			if (retval != 0)
	                {
			    printf("0x%02x%02x: [%s]\n", 
			    	msgbuf[0] & 0xff,  msgbuf[1] & 0xff, &msgbuf[2]);
			}
		}
	}
}




u_short
pr8(int addr)
{
	return(edt_intfc_read(edt_p, (u_int)addr)) ;
}


void
pw8(int addr, int value)
{
	edt_intfc_write(edt_p, (u_int)addr, (u_char)value) ;
}

u_short
pr16(int addr)
{
	if (edt_p->devid == PDVFOI_ID)
		return(edt_read_mac8100(edt_p, (u_int)addr)) ;
	else
		return(edt_intfc_read_short(edt_p, (u_int)addr)) ;
}


void
pw16(int addr, int value)
{
	if (edt_p->devid == PDVFOI_ID)
		edt_write_mac8100(edt_p, (u_int)addr, (u_short)value) ;
	else
		edt_intfc_write_short(edt_p, (u_int)addr, (u_short)value) ;
}

u_int
pr_cfg(int addr)
{
    int ret ;
    edt_buf buf ;
    buf.desc = addr ;
    if ((addr<0) || (addr>0x3c)) {
	printf("pr_cfg: addr out of range\n");
	return(0);
    } 
    ret = edt_ioctl(edt_p,EDTG_CONFIG,&buf) ;
    if (ret < 0)
    {
	perror("EDTG_CONFIG") ;
    }
    return(buf.value) ;
}


u_int
pr_cfg_copy(int addr)
{
    int ret ;
    edt_buf buf ;
    buf.desc = addr ;
    if ((addr<0) || (addr>0x3c)) {
	printf("pr_cfg_copy: addr out of range\n");
	return(0);
    } 
    ret = edt_ioctl(edt_p,EDTG_CONFIG_COPY,&buf) ;
    if (ret < 0)
    {
	perror("EDTG_CONFIG_COPY") ;
    }
    return(buf.value) ;
}


void
pw_cfg(int addr, int value)
{
    int ret ;
    edt_buf buf ;
    buf.desc = addr ;
    if ((addr<0) || (addr>0x3c)) {
	printf("pw_cfg: addr out of range\n");
	return;
    } 
    buf.value = value ;
    ret = edt_ioctl(edt_p,EDTS_CONFIG,&buf) ;
    if (ret < 0)
    {
	perror("EDTS_CONFIG") ;
    }
}
    
#define USAGE \
"usage: %s dev [cmdfile], reads file/stdin for access of dev\n\
     where dev can either be unit no or device name\n" 
	
#define BUFSIZE 256

int gettok(char *tbuf, char **pbufp) ;

int
main(argc,argv)
int argc;
char *argv[];
{
    FILE *xfd;			/* command file for reading */
    char *progname;
    int repeat=0; 		/* Counter for number of times to repeat */
    int repeat_flag=0;		/* Boolean, false if should get new command */
    char  buf[BUFSIZE];		/* Buffer for incomming command line */
    char  *bufp;		/* pointer into buf[] */
    char  tbuf[BUFSIZE];	/* Buffer for one token from buf[] */
    int unit = 0 ;
    int cmdfile_arg = 1 ;
    char devname[80] ;
    u_short *readbuf = 0 ;
    char errstr[64] ;
    int docheck = 0 ;
    int retval ;

    progname = argv[0];
    strcpy(devname, EDT_INTERFACE) ;

    /* Determine which unit number, using optional first argument */
    if ((argc>=2) && (argv[1][0] >= '0' && argv[1][0] <= '9'))
    {
	unit = atoi(argv[1]) ;
	cmdfile_arg=2;
    }
    /* unit number could be preceeded by "-u" */
    if ((argc>=3) && ((strcmp(argv[1], "-u")==0) && (argv[2][0] >='0' && argv[2][0] <='9')))
    {
	unit = atoi(argv[2]) ;
	cmdfile_arg=3;
    }


    /*  Open the command file to read from */
    if (argc == cmdfile_arg) xfd = stdin;
    else  if (argc == cmdfile_arg+1) {
        if ((xfd = fopen(argv[cmdfile_arg], "rb")) == NULL)
        {
	    fprintf(stderr,"couldn't open  command file %s\n", argv[1]);
	    fprintf(stderr,"aborting test - no command file\n");
	    exit(2);
        }
    }
    else {
	fprintf(stderr,USAGE,progname);
	exit(1);
    }

    if ((edt_p = edt_open(devname, unit)) == NULL)
    {
	    sprintf(errstr, "edt_open(%s%d)", EDT_INTERFACE, unit) ;
	    edt_perror(errstr) ;
	    puts("Hit return to exit") ;
	    getchar() ;
	    return(1) ;
    }
	

	if (edt_p->devid == PDVFOI_ID)
	{
		edt_reset_serial(edt_p) ;
		/* edt_check_foi(edt_p) ;*/
		runit = edt_p->foi_unit ;
		edt_msleep(100) ;

		/* check after init */
		dump_mac8100(edt_p);
		retval = edt_reg_read(edt_p, FOI_MSG) ;
		printf("foi read %08x\n", retval);

		/* reset message fifos */
		edt_reg_write(edt_p, FOI_WR_MSG_STAT,  FOI_FIFO_FLUSH);

	}


    while (1)  {
	if (kbhitedt())  { repeat=0;  repeat_flag=0; }

	/* note that fgets() returns a string that includes a final '\n' */
	if (!repeat_flag)  {
	    int n;
	    printf(": ");  fflush(stdout);
	    if (fgets(buf, BUFSIZE, xfd) == NULL)  exit(0);
	    for (n=0; n<BUFSIZE; n++) {		/* strip out comments */
		if (buf[n]=='#')  {		/* back up through whitespace*/
		    while ((n>0)&&(isspace(buf[n-1])))  n--;
		    buf[n]='\n';		/* add end of line */
		    buf[n+1]='\0';		/* add end of string */
		    break;
		}
		if (buf[n]=='\0') break;	/* normal end of string */
	    }
	}

        if (repeat!=0)  {
            if (repeat != -1) repeat--;         /* loop forever if -1 */
	    repeat_flag=repeat;
	} else repeat_flag=0;

	bufp = buf;
	if (gettok(tbuf, &bufp) == 0)  continue;    /* skip blank lines */


	/* Start of dispatch table */
	if (strcmp(tbuf, "h") == 0)  {
puts("Commands to access the pci space: \n\
  r addr	Read 8 bits from interface Xilinx \n\
  w addr data	Write 8 bits to interface Xilinx \n\
  r16 addr	Read 16 bits from interface Xilinx (or MAC8100) \n\
  w16 addr data	Write 16 bits to interface Xilinx (or MAC8100) \n\
  c             Connect to remote debugger\n\
  A             Do Auto Configure\n\
  R             Reinitialize Mac8100\n\
  U n           Change default unit addressing to \n\
  dmax w h n		Sending width height as dma\n\
  dmag w h n		Get as dma width/height\n\
  d             Dump regs\n\
  b  filename	Execute a batch file of debug commands  \n\
  sys  command	Execute a shell command \n\
  dbgflg b	b=0:normal, b=1:debug_msgs, b=2:no_print \n\
  h		Print this help message \n\
  L n		Loop next command n times, -1=forever  \n\
  cfgr/cfgw     Copy from driver or /tmp/p53dbg.cfg to PCI config space\n\
  spv  data     Print current state of 32 bit serial word sent to pal \n\
  spal data     Load 32 bit word serially out to PAL, print stuff from pal \n\
  xosc data	Load 14 bits out to MC12430 PLL, along with last spal data \n\
  fosc Mhz	Load MC12430 and/or AV9110 PLLs, freq in MHz (.000050 - 800.0)\n\
  dly  d1 d2    Load the MC100E195 delay lines for clock and data of channel 0 \n\
  xl file.bit	Load Xilinx from file.bit  \n\
  q		Quit the program \n");
}


	else if (strcmp(tbuf, "spv") == 0)
	{
	    printf("spv: %08x\n", gspv);
	}

	else if (strcmp(tbuf, "spal") == 0)
	{
	    long val32;
	    sscanf(bufp, "%lx", &val32);
	    gspv=val32;
	    spal(gspv);
	}

	else if (strcmp(tbuf, "xosc") == 0)
	{
	    int val;
	    sscanf(bufp, "%x", &val);
	    xosc(val);
	}

	else if (strcmp(tbuf, "fosc") == 0)
	{
	    float val;
	    sscanf(bufp, "%f", &val);
	    fosc(val);
	}

	else if (strcmp(tbuf, "dly") == 0)
	{
	    int n, d1, d2;
	    n = sscanf(bufp, "%d%d" , &d1, &d2);
	    if (n==2)    dly(d1, d2);
	    else   printf("bad command syntax\n");
	}

	else if ((strcmp(tbuf, "xl") == 0) || (strcmp(tbuf, "xlf") == 0))
	{
            char   *errmsg;
            FILE   *xfd;
	    int    xlf=0;

	    if (strcmp(tbuf, "xlf") == 0)    xlf=1;

	    if (!(gettok(tbuf, &bufp)))
	    {
		printf("xl: Need argument of xilinx file name\n");
		continue;
	    }

	    if ((xfd = fopen(tbuf, "rb")) == NULL)
	    {
		printf("Couldn't open  xilinx load file	%s, aborting\n", tbuf);
		continue;
	    }
    
            errmsg = load_xilinx(xfd, xlf);
            if (errmsg == NULL) puts("Xilinx loaded");
            else puts(errmsg);
            fclose(xfd);
        }

	else if (strcmp(tbuf, "looprw") == 0)	{
	    int data;
	    while (! kbhitedt()) {
	        data = pr8(0x00) ;
	        data = pr8(0x05) ;
	    	pw8(0x07,0x3c) ;
	    }
	    data=getchar();
        }

	else if (strcmp(tbuf, "dbgflg") == 0)	{
	    printf("dbgflg was: %x\n", dbgflg);
	    sscanf(bufp,"%x", &dbgflg);
	    printf("dbgflg is: %x\n", dbgflg);
	}
	else if (strcmp(tbuf, "r") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = pr8(addr) ;
	        if (!(dbgflg & 0x02))   printf("%02x ", data);
		addr += 1;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "w") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    pw8(addr,data) ;
		    addr += 1;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	else if (strcmp(tbuf, "r16") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = pr16(addr) ;
	        if (!(dbgflg & 0x02))   printf("%04x ", data);
		addr += 2;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "w16") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    pw16(addr,data) ;
		    addr += 2;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	/* Read current PCI config registers to /tmp/p53dbg.cfg */
	else if	(strcmp(tbuf, "cfgr") == 0)	{
	    int  addr, data;
	    FILE *fd2;
	    if ((fd2 = fopen("/tmp/p53dbg.cfg",	"wb")) == NULL)	{
		fprintf(stderr,"cfgr: Couldn't write to /tmp/p53dbg.cfg\n");
		continue;
	    }
	    for	(addr=0; addr<=0x3c; addr+=4) {
		data  = pr_cfg(addr);
		putc(data, fd2);
		putc(data>>8, fd2);
		putc(data>>16, fd2);
		putc(data>>24, fd2);
		printf("%02x:  %08x\n", addr, data);
	    }
	    fclose(fd2);
	    printf("Wrote config space state out to /tmp/p53dbg.cfg\n");
	}

	/* Write /tmp/p53dbg.cfg out to PCI config registers */
	else if	(strcmp(tbuf, "cfgw") == 0)	{
	    int addr, old, copy, new;
	    FILE *fd2;
	    if ((fd2 = fopen("/tmp/p53dbg.cfg",	"rb")) == NULL)	{
		fprintf(stderr,"cfgw: Couldn't read from /tmp/p53dbg.cfg\n");
		continue;
	    }
	    printf("Overwriting config space registers with /tmp/p53dbg.cfg\n");
	    printf("Hit any key to continue: "); fflush(stdout); 
	    getchar(); printf("\n");
	    
	    printf("	 old	   copy	     new\n");
	    for	(addr=0; addr<=0x3c; addr+=4) {
		old  = pr_cfg(addr);

		copy  =  getc(fd2)&0xff;
		copy |= (getc(fd2)&0xff)<<8;
		copy |= (getc(fd2)&0xff)<<16;
		copy |= (getc(fd2)&0xff)<<24;
		pw_cfg(addr, copy) ;

		new  = pr_cfg(addr);

		printf("%02x:  %08x  %08x  %08x	 ", addr, old, copy, new);
		if	(copy != new)	printf("ERROR\n");
		else if	(old  != new)	printf("changed\n");
		else			printf("\n");

	    }
	    fclose(fd2);
	}


	else if (strcmp(tbuf, "d") == 0)	{
	    dump_mac8100(edt_p);
	    retval = edt_reg_read(edt_p, FOI_MSG) ;
	    printf("foi read %08x\n", retval);
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "dmax") == 0)	{
	    char msgbuf[100] ;
	    int width ;
	    int sendword ;
	    int loops ;
	    int sent ;
	    int height ;
        int loop ;
        int loopcount ;
	    char response[80] ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &width);
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &height);
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &loopcount);
	    response[0] = 0 ;
	    
	    for(loop = 0 ; loop < loopcount ; loop++)
	    {
		if (response[0] != 'c')
		{
		    printf("enter/c to send %d X %d bytes (loop %d): ",
			width,height,loop) ;
		    gets(response) ;
		}
		if (response[1] == 'R')
	        {
		    printf("waiting for command from other end\n") ;
		    for(;;)
		    {
			retval = edt_get_msg( edt_p, msgbuf, sizeof(msgbuf));
			if (retval)
			{
			    printf("got message <%s>\n",msgbuf) ;
			    break ;
			}
		    }
		}
	        loops = ((width * height)/(14*4)) + 1;
                printf("loop %d\n",loop) ;
		sendword = loop << 4 ;
		sent = 0 ;
		while(loops--)
		{
		    edt_send_dma( edt_p, runit, sendword);
			sendword += 14 ;
		    /* change value at end of each line */
#if 0
		    if (sent >= (width * height)/256) 
		    {
			sendword += 14 ;
			sent = 0 ;
		    }
#endif
		    sent += 14*4 ;
		}
	    }

	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if	((strcmp(tbuf, "dmag") == 0)
		|| (strcmp(tbuf, "G") == 0))
	{
	    int	width ;
	  	int loop ;
	    int	height ;
	    int	loopcount ;
	    int	savefile = 0 ;
	    int	errcnt = 0 ;
	    char response[80] ;
	    if (strcmp(tbuf, "G") == 0)
		savefile = 1 ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &width);
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &height);
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%d", &loopcount);
	    if (!readbuf)
	    {
#ifdef _NT_
		readbuf	= (u_short *)
			((ULONG)malloc(width*height+0xfff)& ~0xfff) ;
#else
		readbuf	= (u_short *)valloc(width*height) ;
#endif
		printf("got buf at %p\n",readbuf) ;
	    }
	    response[0]	= 0 ;

	    for(loop = 0 ; loop	< loopcount ; loop++)
	    {
		int ret	;
		int idx	;
		u_short sb ;
		edt_reg_write(edt_p, FOI_WR_MSG_STAT,	FOI_FIFO_FLUSH);
		for(idx	= 0 ; idx < width*height/2 ; idx++) readbuf[idx] = 0xaa55 ;

		if (response[0]	!= 'c')
		{
		    printf("enter/c to read %d %d X %d bytes (loop %d):	",
			loopcount,width,height,loop) ;
		    gets(response) ;
		}
		printf("reading	loop %d	of %d\n",loop,width*height) ;
		if ((ret = edt_read(edt_p, (u_char *)readbuf, width*height))
				!= width*height) {
		    edt_perror("edt_read") ;
		    return 0 ;
		}
		printf("got %d\n",ret) ;
		for(idx	= 0 ; idx < 16 ; idx++)
		    printf("%04x ",readbuf[idx]) ;
		printf("\n") ;
		idx = 0	;
		errcnt = 0 ;
		if (docheck)
		{
			sb = readbuf[idx] ;
			if (sb == 0 && readbuf[1] != 0)	idx++ ;
			sb = readbuf[idx] ;
			for(; idx < width*height/2 ; idx += 2)
			{
				if (readbuf[idx] !=	sb || readbuf[idx+1] !=	0)
				{
				if (errcnt < 10)
				{
					printf("idx	%x %x s/b %x next word %x\n",
						idx,readbuf[idx],sb,readbuf[idx+1])	;
					printf("return") ; getchar();
				}
				errcnt++ ;
				}
				sb = readbuf[idx] +	1 ;
			}
			printf("\n") ;
			printf("%d errors\n",errcnt) ;
		}
		if (savefile)
		{
		    FILE *fsave	;
		    fsave = fopen("saveit","wb") ;
		    printf("saving image in saveit\n") ;
		    fwrite(readbuf, width*height, 1, fsave) ;
		    fclose(fsave) ;
		}
	    }

	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if (*tbuf == 'c')	{
		if (gettok(tbuf, &bufp))
			do_connect(edt_p, tbuf) ;
		else
			do_connect(edt_p, NULL) ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if (*tbuf == 'C')	{
		wait_for_first = 1 ;
		if (gettok(tbuf, &bufp))
			do_connect(edt_p, tbuf) ;
		else
			do_connect(edt_p, NULL) ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}
	
	else if (*tbuf == 'A')	{
		/*
		 * broadcast auto configure command
		 * set direction bit (0x10) so pci foi receives response
		 */
		edt_reset_serial(edt_p) ;
		edt_set_foicount(edt_p, 0) ; /* to force config */
		edt_check_foi(edt_p) ;
		printf("done with auto config\n") ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if (*tbuf == 'u')	{
		int foicount ;
		/*
		 * broadcast auto configure command
		 * set direction bit (0x10) so pci foi receives response
		 */
	    sscanf(bufp,"%d", &foicount);
		edt_reset_serial(edt_p) ;
		edt_set_foicount(edt_p, foicount) ;
		printf("force foicount to %d\n",foicount) ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if (*tbuf == 'U')	{
	    sscanf(bufp,"%x", &runit);
	    printf("runit 0x%02x\n",runit) ;
	    (void)edt_set_foiunit(edt_p, runit) ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}
	else if (*tbuf == 'R')	{
	    printf("Reinit Mac8100\n") ;
		edt_init_mac8100(edt_p) ;
		printf("done\n") ;
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "L") == 0)	{  /* loop on next command */
	    int n;
	    n=sscanf(bufp,"%d", &repeat);
	    if (n<=0)  repeat=0;
            if (n==0)  repeat=-1;
	    repeat_flag = 0;
	}

	else if (strcmp(tbuf, "b") == 0)  {
	    char sbuf[BUFSIZE];
	    int n;
	    n=gettok(tbuf, &bufp);
	    if (n==1)	{
	        sprintf(sbuf, "%s  %s", progname, tbuf);
	        system(sbuf);
	    }
	    else
	    {
		fprintf(stderr, "usage: b  batchfile\n");
	    }
	}

	else if (strcmp(tbuf, "sys") == 0)  {
	    system(bufp);
	}

	else if ((strcmp(tbuf, "q") == 0) || (strcmp(tbuf, "x") == 0)) {
	
	    if (edt_p) edt_close(edt_p) ;
	    exit(0);
	}

	else fprintf(stderr, "huh?\n");
    }
}

int gettok(char *tbuf, char **pbufp)		/* use strtok_r() instead? */
{
    int c;
    do   {  
		c = *(*pbufp)++;   
		if (c=='\0')  { *tbuf = 0;  return(0);  }
    } while (isspace(c));

    do  { 
		*tbuf++ = c;
		c = *(*pbufp)++;   
		if (c=='\0')  { *tbuf = 0;  return(1);  }
    }  while (!isspace(c));

    *tbuf=0;
    return(1);
}



#define DQ7 0x80
#define DQ5 0x20
#define DQ3 0x08

u_char
flipbits(u_char val)
{
    int             i;
    u_char          ret = 0;

    for (i = 0; i < 8; i++)
    {
	if (val & (1 << i))
	    ret |= 0x80 >> i;
    }
    return (ret);
}







#define	XC_X16 	(0x00)		/* PC Only: hi for SCLK=16MHz, not SCLK=CCLK */
#define	XC_AVSEL (0x80)		/* GP Only: hi for SCLK=AVCLK, not SCLK=16MHz*/

#define	XC_CCLK	(0x01)		/* These codes interpreted by wpp() */
#define	XC_DIN	(0x02)          /*  Xilinx config data and clock */
#define	XC_PROG	(0x04)		/*  inverted in wpp() when driving PROGL */
#define	XC_INITL (0x04)		/* Actual bit positions in status register */
#define	XC_DONE  (0x08)


void wpp(int val)	/* Write encoded data to parallel port data register */
{
    unsigned char bits;

    bits = edt_intfc_read(edt_p, PCD_FUNCT) & 0x08;	/* Preserve SWAPEND */
    if (val & XC_DIN)   bits |= 0x04;
    if (val & XC_PROG)  bits |= 0x01;
    if (val & XC_AVSEL) bits |= 0x80;
    edt_intfc_write(edt_p, PCD_FUNCT, bits);
    if (val & XC_CCLK) {		/* strobe the clock hi then low */
        edt_intfc_write(edt_p, PCD_FUNCT, (unsigned char) (bits | 0x02));
        edt_intfc_write(edt_p, PCD_FUNCT, bits);
    }
    if (dbgflg&2)  printf("wpp(%02x)  ", bits);
}

int rpp() 		/* Read parallel port status register */
{ return(edt_intfc_read(edt_p, PCD_STAT)); }


int spal(int v) {

/*
    unsigned char bits;
    bits = edt_intfc_read(edt_p, PCD_FUNCT);
    edt_intfc_write(edt_p, PCD_FUNCT, (unsigned char)(bits & ~XC_AVSEL));
*/

    edt_msleep(1);
    edt_intfc_write(edt_p, PCD_PAL0, (unsigned char) (v>> 0));
    edt_intfc_write(edt_p, PCD_PAL1, (unsigned char) (v>> 8));
    edt_intfc_write(edt_p, PCD_PAL2, (unsigned char) (v>>16));
/*
    edt_msleep(1);
    edt_intfc_write(edt_p, PCD_FUNCT, bits);
*/
    return(edt_intfc_read(edt_p, PCD_PAL3));
}


/* These bits are in hi word of long, must split into two to do AND's, OR's */
#define XPLLBYP (0x1<<17)
#define XPLLCLK (0x1<<18)
#define XPLLDAT (0x1<<19)
#define XPLLSTB (0x1<<20)
void xosc(int val) {
    int n, s;     /*First set up MC12430 lines, CLK=0, DAT=0, STB=0 */
    gspv = (gspv & ~XPLLBYP & ~XPLLCLK & ~XPLLDAT & ~XPLLSTB);
    wpp(0);  edt_msleep(1);		/* Shut down AVSEL */

    for (n=13; n>=0; n--) {		/* Send 14 bits of data to MC12430 */
	s=val&(0x1<<n);			/*  get the current data bit */
	if (s)  {
	    spal(gspv | XPLLDAT); 	/* Set up data, then clock it */
	    spal(gspv | XPLLDAT | XPLLCLK); 
	} else  {
	   spal(gspv);			/* Strobe clock only, no data */
	   spal(gspv | XPLLCLK);
	}
	if (dbgflg & 1) printf(" n:%d  s:%x\n", n, (s!=0));
    }
    spal(gspv | XPLLSTB);		/* Strobe serial_load */
    spal(gspv);				/* Clear the strobe */
    if ((val>>11)==6)  { 
	gspv |= XPLLBYP;  spal(gspv);
        wpp(XC_AVSEL );			/* On GP, drive SCLK from AV9110 */
    }  else wpp(0);  
    if (XC_X16)  wpp(XC_X16);		/* On PC, drive SCLK with 16 MHz ref */
}


double fosc(double fmhz) {	/* Set ECL clock to freq specified in MHz */
    int m, n, t, hex,  nn, noload, fx;
    edt_pll avp;   
	 double avf = -1.0; /* is there a better default? */
	 
    if (fmhz<0)  {fmhz=-fmhz; noload=1;}   else  noload=0;
    fx = (int)fmhz;

    if ((fmhz>800.0) || (fmhz<0.000050))  { 
	printf("Error, %f MHz requested.  Min of 0.000050, max of 800Mhz\n", fmhz);
	return(0);
    }
    else if (fx>400) { t=0;  n=3;  m=fx/2;  nn=1;}  /* Every 2 MHz */
    else if (fx>200) { t=0;  n=0;  m=fx;    nn=2;}  /* Every 1 MHz */
    else if (fx>100) { t=0;  n=1;  m=fx*2;  nn=4;}  /* Every 500 KHz */
    else if (fx>=50) { t=0;  n=2;  m=fx*4;  nn=8;}  /* Every 250 KHz */
    else  {                    
	avf = edt_find_vco_frequency(edt_p, fmhz*1E6, (double)30E6, &avp, 0);
	if ((avf!=0) && !noload) edt_set_out_clk(edt_p, &avp);  /* Load AV9110*/
	t=6;  n=3;  m=200;    nn=4;    /* Put MC12430 in bypass, use AV9110 */
    }

    hex = (t<<11) | (n<<9) | m;    
    if (!noload)  xosc(hex);			/* Load MC12430 hw */

    if (t!=6)  {
        printf("    %f MHz    MC12430: t:%d  n:%d  m:%d  hex:0x%04x\n", 
        2.0*m/nn, t, n, m, hex);
	return(2.0*m/nn);			/* Freq of MC12430 in MHz */
    }
    else  {     
	printf("    %f MHz    AV9110:  m:%d n:%d v:%d r:%d h:%d l:%d x:%d\n", 
	avf/1E6, avp.m, avp.n, avp.v, avp.r, avp.h, avp.l, avp.x);
	return(avf/1E6);			/* Freq of AV9110 in MHz */

    }
}


/* Load 7 bits of data into the MC100E195 delay lines for channel A input */
/* d1 and d2 are calibrated in 20ps, read MC100E195 data sheet about 2 LSB's */
void dly(int d1, int d2)
{
    gspv &= ~0x7F;              /* Zero the delay data bits XS6-0 */
    gspv |=  0x180;             /* Set the delay strobe bits XS8,XS7 */
    d1&=0x7C;   d2&=0x7C;
    spal(gspv | d1 );			/* set up the data */
    spal((gspv | d1) & ~(1<<8));	/* strobe into delay line for clock*/
    spal(gspv | d1 );			/* turn off the strobe */
    spal(gspv | d2 );			/* set up the data */
    spal((gspv | d2) & ~(1<<7));	/* strobe into delay line for data */
    spal(gspv | d2 );			/* turn off the strobe */
}




/* Configure Xilinx with file of binary data. */
/* Assume fast byte-at-a-time motherboard firmware if xlf==1  */
char *load_xilinx(FILE *infp, int xlf)
{
    int  h, d, b1, data;
    long  n;

    for (h=0; h<128; h++) {
	b1=getc(infp);
	if ((b1>=0x20) && (b1<=0x7E))  putchar(b1);   else putchar('.');
	if ((b1 & 0xff) == 0xAA)   break;	/* Config synch character */
    }
    putchar('\n');
    if (h==128)
	return ("Err, no sync pattern in first 128 bytes");
    printf("Header count:%d\n", h);


    wpp(XC_PROG);		/* Assert PROG (PROGL pin low) */
    edt_msleep(100);		/* for 0.1 seconds */
    if (rpp() & XC_INITL)  	/* INIT should be asserted (INITL low) */
	return("Err, INITL still hi when PROGL to Xilinx was asserted");

    wpp(0);			/* Unassert PROG (PROGL pin hi) */
    edt_msleep(100);		/* for 0.1 seconds */
    if (!(rpp() & XC_INITL))  /* INIT should be unasserted (INITL hi) */
	return ("Err, INITL still low after PROGL to Xilinx was deasserted");

    n=-4;			/* Send 32 bits of 1's before sync word */
    while (1)
    {				/* For each byte */
	if      (n<0)   data=0xFF;
        else if (n==0)  data=0xAA;
	else  {
	    data=getc(infp);
	    if (data==EOF) break;
	}

	/* If part too small, will smoke if we don't abort the load! */
        if ((data!=0x00) && (rpp() & XC_DONE)) {
	    wpp(XC_PROG);		/* Assert PROG (PROGL pin low) */
	    if (n<=0) return("Err, DONE was true before download was started");
	    else      return("Err, DONE went true early, part too small?");
	}

	if   (((n==1) && (data!=0x99)) || ((n==2) && (data!=0x55)) || 
              ((n==3) && (data!=0x66)))  {
	    return ("Err, missing sync byte 0x99 or 0x55 or 0x66");
	}
	n=n+1;

	if (xlf) pw8(23, data);
        else for (d=0; d<8; d++) {	/* For each bit in the byte */
	    if (data & 0x80)
		wpp(XC_CCLK | XC_DIN);	/* Strobe a 1 */
	    else
		wpp(XC_CCLK);		/* Strobe a 0 */
	    data <<= 1;
	}
    }
    printf("Config-data byte count:%ld\n", n);

    if (xlf) {pw8(23, 0);  pw8(23, 0); }
    else for (d=0; d<16; d++)   wpp(XC_CCLK);	/* Strobe in 16 zeros */
    if ((rpp() & XC_DONE) == 0)
	return ("Err, DONE was not true after download was completed");
    return (NULL);		/* Success */
}

#include "edtinc.h"


/*
Should consider making foicount=-1 at initialization, -2 on error.
The value of 0 is valid when a loopback cable is installed.

Make it possible to say "pdb 0" ?

Recent changes to pcifoi xilinx:
    made ROMD[] output mux flip on what register last written (in romdecode)
    cleaned up how reset strobe cleared that same register (in macio)
    disabled incomming broadcast command packets (in cmdin)
    hacked RXRBABC to ignore DIRIN

    currently have ignoreDIRIN, and ignoreUNIT plus DIRIN to be matched
    should be a way to ignore broadcast packets?  No

rp assumes incomming packet is printable ascii
need wpt for text out, assumes routing byte of 0x00

should remov 150 ohm resistor from SD on pcifoi,
must move resistor away from optical tranciever

##Old code to read boot strap and bus voltage
else if	(strcmp(tbuf, "rp") == 0) {    
    uint_t  rdval;
    u_char  stat;
    xw(EDT_FLASHROM_DATA, 0);
    xw(EDT_FLASHROM_DATA, BT_EN_READ | BT_READ);
    rdval = xr(EDT_FLASHROM_DATA);
    stat = (u_char) rdval;
    printf("pal:%02x\n", stat);
    xw(EDT_FLASHROM_DATA, BT_A0 | BT_A1);
}

*/


EdtDev *edt_p ;


/* for addr=0x040100C0     04:byte_count 01:interface_Xilinx, C0:PCI_address */
void
xw(u_int addr, u_int data)
{
    u_int	dmy;
    edt_reg_write(edt_p, addr, data) ;
    dmy = edt_reg_read(edt_p, 0x04000080) ;	/*flush chipset write buffers*/
}

u_int
xr(u_int addr)
{
    return(edt_reg_read(edt_p, addr)) ;
}


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

#elif defined(VXWORKS)

#include "ioLib.h"

#else

#include <sys/filio.h>		/* for kbhitedt() */

#endif
int kbhitedt()
{
    int numchars, n, ch ;
    ioctl(0, FIONREAD, (int)&numchars) ;

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
static char sendmsg[512] ;
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
			fp = fopen(infile, "r") ;

		if (fp == NULL)
		{
			perror(infile) ;
			return ;
		}
		
		while(fgets(sendmsg, 511, fp))
		{
		    if (sendmsg[0] == 'Q') return ;
		    *(strrchr(sendmsg, '\n')) = '\0' ;
		    printf("sending %s\n",sendmsg) ;
		    ret = edt_send_msg(fd, runit, sendmsg, strlen(sendmsg));
		    if (ret)
		    {
		    	printf("bad send msg - dumping reg\n") ;
				dump_mac8100(fd) ;
				printf("return to continue\n") ;
			}
		    retval = 0 ;
		    while (retval == 0)
		    {
		    retval = edt_get_msg_unit(fd, msgbuf, sizeof(msgbuf),runit);
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
			printf("continue? ") ; fgets(answer, 8, stdin) ;
			if (answer[0] != 'y')
			{
				return ;
			}
		}


		for(;;)
		{
			if (kbhitedt())
			{
				fgets(sendmsg, 511, stdin) ;
				if (sendmsg[0] == 'Q') return ;
				ret = edt_send_msg(fd, runit, sendmsg, strlen(sendmsg));
				if (ret)
				{
					printf("bad send msg - dumping reg\n") ;
					dump_mac8100(fd) ;
					printf("return to continue\n") ;
				}
			}
			retval = edt_get_msg_unit(fd, msgbuf, sizeof(msgbuf),runit);
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



/* The driver treats edt_intfc_write_short differently for PCIFOI,
/* setting it up as a command packet out to the RCI interface Xilinx.
/* Otherwise, edt_write_mac8100() same as edt_intfc_write_short()
*/

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

static volatile caddr_t mapaddr = 0 ;
static volatile caddr_t mapaddr1 = 0 ;

void
mmap_setup(EdtDev *edt_p)
{
    mapaddr = (volatile caddr_t)edt_mapmem(edt_p, 0, 0x10000) ;
    if ((int)mapaddr == 0)
    {
	printf("edt_mapmem failed\n") ;
	return ;
    }
    
	if (edt_get_mappable_size(edt_p) > 0)
		mapaddr1 = (volatile caddr_t)edt_mapmem(edt_p, 0x10000, edt_get_mappable_size(edt_p)) ;
	else
		mapaddr1 = 0;

}

int check_mmap(int space)
{
    if (mapaddr == 0) mmap_setup(edt_p) ;
    if (space == 0)
    {
	if (mapaddr == 0)
	{
	    fprintf(stderr,"bad mmap for space 0\n");
	    return(1) ;
	}
	else return(0) ;
    }
    else
    {
	if (mapaddr1 == 0)
	{
	    fprintf(stderr,"bad mmap for space 1\n");
	    return(1) ;
	}
	else return(0) ;
    }
}


u_short
mmap_read_16(EdtDev *edt_p, int addr, int space)
{
    if (mapaddr == 0) mmap_setup(edt_p) ;
    if (space == 0)
    {
	if (mapaddr == 0) return(0xdead) ;
	else return (*(u_short *)(mapaddr + addr)) ;
    }
    else
    {
		if (mapaddr1 == 0) return(0xbeef) ;
		else return (*(u_short *)(mapaddr1 + addr)) ;
    }
}
u_int
mmap_read_32(EdtDev *edt_p, int addr, int space)
{
    if (mapaddr == 0) mmap_setup(edt_p) ;
    if (space == 0)
    {
	if (mapaddr == 0) return(0xdeadbeef) ;
	else return (*(u_int *)(mapaddr + addr)) ;
    }
    else
    {
	if (mapaddr1 == 0) return(0xcafebeef) ;
	else return (*(u_int *)(mapaddr1 + addr)) ;
    }
}
void
mmap_write_16(EdtDev *edt_p, int addr, u_short data, int space)
{
    if (mapaddr == 0) mmap_setup(edt_p) ;
    if (space == 0)
    {
	if (mapaddr == 0) printf("mmap failed\n") ;
	else *((u_short *)(mapaddr + addr)) = data ;
    }
    else
    {
	if (mapaddr1 == 0) printf("mmap failed\n") ;
	else *((u_short *)(mapaddr1 + addr)) = data ;
    }
}
void
mmap_write_32(EdtDev *edt_p, int addr, u_int data, int space)
{
    if (mapaddr == 0) mmap_setup(edt_p) ;
    if (space == 0)
    {
	if (mapaddr == 0) printf("mmap failed\n") ;
	else *((u_int *)(mapaddr + addr)) = data ;
    }
    else
    {
	if (mapaddr1 == 0) printf("mmap failed\n") ;
	else *((u_int *)(mapaddr1 + addr)) = data ;
    }
}


#ifndef NO_MAIN
void
#endif
usage(char *s)
{
    printf("pdb -u unit [-c channel] [-f batchfile]\n");
    if (s && *s)  
    {
        puts(s);
	exit(1);
    }
    else
        exit(0);
}
    
	
#define PDBBUFSIZE 256

#define DMABUFSZ  (1024*1024*16)
unsigned char *dmabuf;

int gettok(char *tbuf, char **pbufp) ;

#ifndef _NT_
void
gotit(int dmy)
{
    printf("got sig\n") ;
    exit(0);
}
#endif


#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;

pdb(char *command_line)
#else
main(argc,argv)
int argc;
char *argv[];
#endif
{
    FILE *xfd;			/* command file for reading */
    char *progname;
    int repeat=0; 		/* Counter for number of times to repeat */
    int repeat_flag=0;		/* Boolean, false if should get new command */
    char  buf[PDBBUFSIZE];		/* Buffer for incomming command line */
    char  *bufp;		/* pointer into buf[] */
    char  tbuf[PDBBUFSIZE];	/* Buffer for one token from buf[] */
    u_short dbgflg =0;
    int errs = 0 ;
    u_short *readbuf = 0 ;
    int docheck = 0 ;
    int retval ;





/* ######################################################################## */
    int    i;
    char   devname[256] = "";
    char   errbuf[32];
    char   cmdfile[32] = "";
    int    channel=-1;
    char   *unitstr = "";
    int    unit = 0;
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("pdb",command_line,&argc,&argv);
#endif

    progname = argv[0];
#ifndef _NT_
#if defined(VXWORKS)
 /* ioctl(stdin,FIOSETOPTIONS,OPT_LINE|OPT_CRMOD|OPT_ECHO) ;*/
#else
    signal(SIGINT,gotit) ;
#endif
#endif

    /*
     * COMMAND LINE ARGUMENTS
     */

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] != '-')
	{
		/* Check for old style command line of "pdb unit [batfile]" */
		if ((i==1) && (argc<=3)) { 
	            unitstr = argv[1];
		    if (argc==3)  strcpy(cmdfile, argv[2]);
		    break;
		} else {
		    sprintf(errbuf, "Badly formed argument of %s", argv[i]);
		    usage(errbuf);
		}
	}
	else
	{
	    if (argv[i][1] == '-')
	    {
		if (strcmp(argv[i], "--help") == 0)
		    usage(NULL);
		else
		{
		    sprintf(errbuf, "unknown option \"%s\"", argv[i]);
		    usage(errbuf);
		}
	    }
	    if (argv[i][1] == 0 || argv[i][2] != 0)
		usage(NULL);
	    switch (argv[i][1])
	    {
	    case 'u':
		if (++i == argc)
		    usage("unfinished -u option");
		unitstr = argv[i];
		break;
	    case 'c':
		if (++i == argc)
		    usage("unfinished -c option");
		channel = atoi(argv[i]);
		break;
	    case 'f':
		if (++i == argc)
		    usage("unfinished -f option");
		strcpy(cmdfile, argv[i]);
		break;
/*
	    case 'V':
		verbose = verbose ? 0 : 1;
		break;
	    case 'q':
		quiet=1;
		break;
*/
	    case 'h':
		usage(NULL);
		break;
	    default:
		sprintf(errbuf, "\nunknown option \"-%c\"", argv[i][1]);
		usage(errbuf);
	    }
	}
    }


    if (*unitstr) 
	unit = edt_parse_unit(unitstr, devname, NULL);
    else
    {
	unit=0;
	strcpy(devname, EDT_INTERFACE) ;
    }



/* ######################################################################## */

    /*  Open the command file to read from */
    if (!*cmdfile)    xfd = stdin;
    else
    {
        if ((xfd = fopen(cmdfile, "r")) == NULL)
        {
	    fprintf(stderr,"couldn't open  command file %s\n", cmdfile);
	    fprintf(stderr,"aborting test - no command file\n");
	    exit(2);
        }
    }


    if (channel==-1) {
        printf("edt_open( devname:%s unit:%d)\n", devname, unit);
        edt_p = edt_open(devname, unit);
    } else {
        printf("edt_open_channel( devname:%s unit:%d channel:%d)\n", 
				  devname,   unit,   channel);
        edt_p = edt_open_channel(devname, unit, channel);
    }
    if (edt_p == NULL) {
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



    dmabuf = edt_alloc(DMABUFSZ);

    while (1)  {
	if (kbhitedt())   { repeat=0;  repeat_flag=0; }

	/* note that fgets() returns a string that includes a final '\n' */
	if (!repeat_flag)  {
	    int n;
	    printf(": ");  fflush(stdout);
	    if (fgets(buf, PDBBUFSIZE, xfd) == NULL)  exit(0);
	    for (n=0; n<PDBBUFSIZE; n++) {		/* strip out comments */
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
  r addr		Read 8 bits from interface Xilinx \n\
  w addr data		Write 8 bits to interface Xilinx \n\
  r16 addr		Read 16 bits from interface Xilinx \n\
  w16 addr data		Write 16 bits to interface Xilinx \n\
  mr16 addr		Read 16 bits from MAC8101 \n\
  mw16 addr data	Write 16 bits to MAC8101 \n\
  xr8/16/32 addr 	Read 8/16/32 bits from pci register \n\
  xw8/16/32 addr data	Write 8/16/32 bits to pci register \n\
  xwcfg addr data	Write 32 bits to config space\n\
  xrcfg addr		Read 32 bits from config space\n\
  rm016/032 addr	Read 16/32 bits from mmap space 0\n\
  wm016/032 addr data	Write 16/32 bits to mmap space 0\n\
  rm116/132 addr	Read 16/32 bits from mmap space 1\n\
  wm116/132 addr data	Write 16/32 bits to mmap space 1\n\
  rp		read incomming command packet from fiber \n\
  wp rb d d d	write command packet out, routing byte of 20 first \n\
  sniff		init pcifoi for use as sniffer (then use rdma cmd)\n\
  sniff16	same, but only sync word plus first 60 bytes of each packet \n\
  rdma fname bcount     dma read of bcount bytes to file fname \n\
  c             Connect to remote debugger\n\
  A             Do Auto Configure\n\
  R             Reinitialize Mac8101\n\
  U n           Change default unit addressing to \n\
  x w h n		Sending width height as dma\n\
  g w h n		Get as dma width/height\n\
  d             Dump regs\n\
  b  filename	Execute a batch file of debug commands  \n\
  sys  command	Execute a shell command \n\
  dbgflg b	b=0:normal, b=1:debug_msgs, b=2:no_print \n\
  h		Print this help message \n\
  L n		Loop next command n times, -1=forever  \n\
  cfgr/cfgw     Copy from driver or /tmp/p53dbg.cfg to PCI config space\n\
  reboot	cfgr, xw8 85 40, cfgw \n\
  q		Quit the program \n");
}



	else if (strcmp(tbuf, "dbgflg") == 0)	{
	    sscanf(bufp,"%hd", &dbgflg);
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
	        if (!(dbgflg & 0x02))   printf("%02x ", data&0xff);
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
	        if (!(dbgflg & 0x02))   printf("%04x ", data&0xffff);
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

	else if (strcmp(tbuf, "mr16") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	    	data = edt_read_mac8100(edt_p, addr);
	        if (!(dbgflg & 0x02))   printf("%04x ", data&0xffff);
		addr += 1;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "mw16") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    edt_write_mac8100(edt_p, addr, data);
		    addr += 1;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	else if (strcmp(tbuf, "xr8") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = xr(0x01000000+addr) ;
	        if (!(dbgflg & 0x02))   printf("%02x ", data);
		addr += 1;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "xw8") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    xw(0x01000000+addr, data) ;
		    addr += 1;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	else if (strcmp(tbuf, "xr16") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = xr(0x02000000+addr) ;
	        if (!(dbgflg & 0x02))   printf("%04x ", data);
		addr += 2;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "xw16") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    xw(0x02000000+addr, data) ;
		    addr += 2;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	else if (strcmp(tbuf, "xr32") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = xr(0x04000000+addr) ;
	        if (!(dbgflg & 0x02))   printf("%08x ", data);
		addr += 4;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "xw32") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    xw(0x04000000+addr, data) ;
		    addr += 4;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}

	else if (strcmp(tbuf, "xrcfg") == 0)	{
	    int addr, data, n, cnt;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	        data = pr_cfg(addr) ;
	        if (!(dbgflg & 0x02))   printf("%08x ", data);
		addr += 4;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "xwcfg") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    pw_cfg(addr, data) ;
		    addr += 4;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}
	else if (strcmp(tbuf, "rm016") == 0)	{
	    int addr, data, n, cnt;
	    if (check_mmap(0)) continue ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	    	data = mmap_read_16(edt_p, addr, 0);
	        if (!(dbgflg & 0x02))   printf("%04x ", data&0xffff);
		addr += 2;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "wm016") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    if (check_mmap(0)) continue ;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    mmap_write_16(edt_p, addr, data, 0);
		    addr += 2;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}
	else if (strcmp(tbuf, "rm032") == 0)	{
	    int addr, data, n, cnt;
	    if (check_mmap(0)) continue ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	    	data = mmap_read_32(edt_p, addr, 0);
	        if (!(dbgflg & 0x02))   printf("%08x ", data) ;
		addr += 4;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "wm032") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    if (check_mmap(0)) continue ;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    mmap_write_32(edt_p, addr, data, 0);
		    addr += 4;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}
	else if (strcmp(tbuf, "rm116") == 0)	{
	    int addr, data, n, cnt;
	    if (check_mmap(1)) continue ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	    	data = mmap_read_16(edt_p, addr, 1);
	        if (!(dbgflg & 0x02))   printf("%04x ", data&0xffff);
		addr += 2;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "wm116") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    if (check_mmap(1)) continue ;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    mmap_write_16(edt_p, addr, data, 1);
		    addr += 2;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}
	else if (strcmp(tbuf, "rm132") == 0)	{
	    int addr = 0, data, n, cnt;
	    if (check_mmap(1)) continue ;
	    gettok(tbuf, &bufp);
	    sscanf(tbuf,"%x", &addr);
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &cnt);
	    if (n!=1)  cnt=1;
	    for (n=0; n<cnt; n++) {
	    	data = mmap_read_32(edt_p, addr, 1);
	        if (!(dbgflg & 0x02))   printf("%08x ", data) ;
		addr += 4;
	    }
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "wm132") == 0)	{
	    int addr, data, n;
 	    int first=1;
	    if (check_mmap(1)) continue ;
	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &addr);
	    if (n==1) do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    mmap_write_32(edt_p, addr, data, 1);
		    addr += 4;
		    first=0;
		}
	    } while (n==1);
	    else printf("    bad address");
	    if (first==1)  printf("  nothing written\n");
	}



	else if (strcmp(tbuf, "rp") == 0)  {
	    int d, n;   int buf[16];
	    char c;
	    for (n=0; n<16; n++) {
		d=xr(0x020000C2);
		buf[n]=d;
		if (!(d & 0x8000))  break;
		printf("%04x ", d);
	    }
	    printf("%04x\n", d);

	    for (n=0; n<16; n++) {	/* Print text, skipping routing byte */
		if (!(buf[n] & 0x8000)) break;
		c = buf[n];
		if ((c<0x20) || (c>0x7e))  c='.';
		if ((n>1) || ((n==1) && ((buf[0]&0xff)==0x3F)))  putchar(c);
	    }
	    if (n>1)  putchar('\n');
	}

	else if (strcmp(tbuf, "wp") == 0)	{
	    int data, n;
 	    int first=1;

	    data=xr(0x010000C5);
	    if (data&0xa0) {
		printf("Clearing interrupts,  REG_C5:=02\n");
	    	xw(0x010000c5, 0x02);
	    }

	    do {
	        gettok(tbuf, &bufp);
	        n=sscanf(tbuf,"%x", &data);
	        if (n==1)   {
	    	    xw(0x020000C0, data & 0x00ff) ;
		    first=0;
		}
	    } while (n==1);
	    xw(0x020000C0, 0x0100) ;	/* Terminate packet */
	    
	    if (first==1)  printf("    empty packet sent");
	}

	else if (strcmp(tbuf, "sniff16") == 0)	{
	  edt_init_mac8100(edt_p) ;
	  edt_write_mac8100(edt_p, 7, 0x03B3);     /* rcv preamble and status */
	  /* edt_write_mac8100(edt_p, 8, 0x0880);  /* minimize discards */
          edt_reg_write(edt_p, 0x040000d4, 0x3f200000);   /* SNUFF+PASS on */
	}
	else if (strcmp(tbuf, "sniff") == 0)	{
	  edt_init_mac8100(edt_p) ;
	  edt_write_mac8100(edt_p, 7, 0x03B3);     /* rcv preamble and status */
	  /* edt_write_mac8100(edt_p, 8, 0x0880);  /* minimize discards */
          edt_reg_write(edt_p, 0x040000d4, 0x3D200000);   /* SNUFF+PASS on */
	}

	else if (strcmp(tbuf, "rdma") == 0)	{
	    int bcnt, gotcnt, n;
	    FILE *fd2;

	    n=gettok(tbuf, &bufp);
	    if (n!=1) { 
		fprintf(stderr, "Syntax error\n"); 
		continue; 
	    }
	    if ((fd2 = fopen(tbuf, "wb")) == NULL)	{
		fprintf(stderr, "Error opening file %s for output\n", tbuf);
		continue;
	    }

	    gettok(tbuf, &bufp);
	    n=sscanf(tbuf,"%x", &bcnt);
	    if (n!=1) { 
		fprintf(stderr, "Syntax error\n"); 
		continue; 
	    }
	    if (bcnt >DMABUFSZ) {
		fprintf(stderr, "Max bytecount of %x\n", DMABUFSZ);
		continue;
	    }

	    edt_set_rtimeout(edt_p, 0);
printf("edtread( edt_p:%08x, dmabuf:%08x, bcnt:%d \n", (int)edt_p, (int)dmabuf, bcnt);
	    gotcnt = edt_read(edt_p, dmabuf, bcnt);
gotcnt = edt_get_bytecount(edt_p) ;
printf("bytecount %d\n", gotcnt) ;

	    if (gotcnt != bcnt) {
	        fprintf(stderr, "asked for %x bytes, got %x bytes", bcnt, gotcnt);
	    }
	    n = fwrite(dmabuf, 1, gotcnt, fd2);
	    fclose(fd2);
	    if (n!=bcnt) fprintf(stderr, "failed on writing to output file\n");
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

	else if	(strcmp(tbuf, "reboot") == 0)	{
	    int  addr, data;
	    int  buf[0x40];
 	    int  old, copy, new;
	    FILE *fd2;
	    if ((fd2 = fopen("./pdb_reboot.cfg", "wb")) == NULL)	{
		fprintf(stderr,"cfgr: Couldn't write to ./pdb_reboot.cfg\n");
		continue;
	    }
	    for	(addr=0; addr<=0x3c; addr+=4) {
		data  = pr_cfg(addr);
		buf[addr] = data;
		putc(data, fd2);
		putc(data>>8, fd2);
		putc(data>>16, fd2);
		putc(data>>24, fd2);
		/* printf("%02x:  %08x\n", addr, data); */
	    }
	    fclose(fd2);
	    printf("Wrote config space state out to ./pdb_reboot.cfg\n");

	    edt_reg_write(edt_p, 0x01000085, 0x40) ;
	    edt_msleep(500) ;

	    printf("	 old	   copy	     new\n");
	    for	(addr=0; addr<=0x3c; addr+=4) {
		old  = pr_cfg(addr);
		copy  =  buf[addr];
		pw_cfg(addr, copy) ;
		new  = pr_cfg(addr);

		printf("%02x:  %08x  %08x  %08x	 ", addr, old, copy, new);
		if	(copy != new)	printf("ERROR\n");
		else if	(old  != new)	printf("changed\n");
		else			printf("\n");

	    }
	}

	



	else if (strcmp(tbuf, "d") == 0)	{
	    dump_mac8100(edt_p);
	    retval = edt_reg_read(edt_p, FOI_MSG) ;
	    printf("foi read %08x\n", retval);
	    if (!(dbgflg & 0x02))   printf("\n");
	}

	else if (strcmp(tbuf, "x") == 0)	{
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
		    fgets(response, 79, stdin) ;
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
	else if	((strcmp(tbuf, "g") == 0)
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
		printf("got buf	at %x\n",readbuf) ;
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
		    fgets(response, 79, stdin) ;
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
		printf("Autoconfig done, foicnt=%d\n", edt_get_foicount(edt_p));
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
	    edt_init_mac8100(edt_p) ;
	}

	else if (strcmp(tbuf, "L") == 0)	{  /* loop on next command */
	    int n;
	    n=sscanf(bufp,"%d", &repeat);
	    if (n<=0)  repeat=0;
            if (n==0)  repeat=-1;
	    repeat_flag = 0;
	}

	else if (strcmp(tbuf, "b") == 0)  {
	    char sbuf[PDBBUFSIZE];
	    int n;
	    n=gettok(tbuf, &bufp);
	    if (n==1)	{
	        sprintf(sbuf, "%s %d %s", progname, unit, tbuf);
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

	else if (strcmp(tbuf, "q") == 0)  {
	    if (edt_p) edt_close(edt_p) ;
#ifdef NO_MAIN
		return(0) ;
#else
		exit(0) ;
#endif
	}

	else fprintf(stderr, "huh?\n");
    }
#ifdef NO_MAIN
    return(0) ;
#else
    exit(0) ;
#endif
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



#if 0
#define DQ7 0x80
#define DQ5 0x20
#define DQ3 0x08

static u_char
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
#endif


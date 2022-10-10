
#include "edtinc.h"
usage()
{
    printf("Usage: testssmem\n") ;
    printf("-u unit		unit to test\n") ;
}

#define ALOOP	1    
#define RLOOP	2    
#define DLOOP	0xc    
#define LOSSTAT	4
#define AISSTAT 0x13
#define LXT0	0xc0
#define LXT1	0xe0
#define BITCNT	0x2c
#define COUNTS	0x30

#define E1CTL	0x2c
#define E1CNT	0x30

#define E3CTL	0x2d
#define E3CNT	0x40

#define ECLCTL	0x58
#define ECLCNT	0x50
#define ECLDIR	0x22	/* also pll debug select */

/* for CTLREGs */
#define TST_RESET	0x80 /* resets test pattern */
#define TST_ERRINS	0x40 /* inserts bit errors */
#define CNT_RESET	0x20 /* reset counters */
#define CNT_LATCH	0x10 /* latch counters */

#define MEMCTL0		0x60
#define MEMCTL1		0x61
/* for MEMCTL */
#define M_DATOUT	0x01	/* xilinx data out enable */
#define M_MEMOUT	0x02	/* mem data out enable */
#define M_MEMADV	0x04
#define M_MEMWRITE	0x08	/* write pulse */
#define M_BYTE0EN	0x10
#define M_BYTE1EN	0x20
#define M_BYTE2EN	0x40
#define M_BYTE3EN	0x80
#define M_WR		(M_DATOUT | 0xf0)
#define M_RD		(M_MEMOUT | 0xf0)

#define MEMTAGW		0x63 /* write tag/parity - 0 bottom 4 */
#define MEMADDR		0x64
#define MEMADDRL	0x64
#define MEMADDRM	0x65
#define MEMADDRH	0x66
#define MEMTAGR		0x67 /* read tag/parity - 0 bottom 4 */
#define MEMDATA		0x68
#define MEMDATA0	0x68
#define MEMDATA1	0x69
#define MEMDATA2	0x6a
#define MEMDATA3	0x6b
#define MEMREAD0	0x6c
#define MEMREAD1	0x6d
#define MEMREAD2	0x6e
#define MEMREAD3	0x6f
#define MEM1READ0	0x70
#define MEM1READ1	0x71
#define MEM1READ2	0x72
#define MEM1READ3	0x73

int dbgtag = 0 ;

typedef	volatile u_char v_char ;
v_char *mapaddr ;


u_char
mmap_intfc_read(EdtDev *edt_p, u_int desc)
{
	v_char *off_p ;
	v_char *data_p ;
	u_char val ;
	/* interface must set offset then read data */
	off_p = (v_char *)mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = (v_char *)mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	val = *off_p ;
	/* edt_msleep(10) ;*/
	val = *data_p ;
	return(val) ;
}

void
mmap_intfc_write(EdtDev *edt_p, u_int desc, u_char val)
{
	v_char *off_p ;
	v_char *data_p ;
	u_char tmp ;
	/* interface must set offset then read data */
	off_p = (v_char *)mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = (v_char *)mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	tmp = *off_p ;
	*data_p = val ;
}

ssmem_write0(EdtDev *edt_p, u_int addr, u_int data)
{
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL0, M_WR) ;
    /* set addr,data,tag, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;

    mmap_intfc_write(edt_p, MEMDATA0, data & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA1, (data >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA2, (data >> 16) & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA3, (data >> 24) & 0xff) ;

    mmap_intfc_write(edt_p, MEMCTL0, M_WR | M_MEMWRITE) ;
    mmap_intfc_write(edt_p, MEMCTL0, M_WR) ;
}

u_int
ssmem_read0(EdtDev *edt_p, u_int addr) 
{
    u_int val ;
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL0, M_RD) ;
    /* set addr, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;

    val = mmap_intfc_read(edt_p, MEMREAD0) ;
    val |= mmap_intfc_read(edt_p, MEMREAD1) << 8 ;
    val |= mmap_intfc_read(edt_p, MEMREAD2) << 16 ;
    val |= mmap_intfc_read(edt_p, MEMREAD3) << 24 ;
    return(val) ;
}
ssmem_write1(EdtDev *edt_p, u_int addr, u_int data)
{
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL1, M_WR) ;
    /* set addr,data,tag, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;

    mmap_intfc_write(edt_p, MEMDATA0, data & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA1, (data >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA2, (data >> 16) & 0xff) ;
    mmap_intfc_write(edt_p, MEMDATA3, (data >> 24) & 0xff) ;

    mmap_intfc_write(edt_p, MEMCTL1, M_WR | M_MEMWRITE) ;
    mmap_intfc_write(edt_p, MEMCTL1, M_WR) ;
}

u_int
ssmem_read1(EdtDev *edt_p, u_int addr) 
{
    u_int val ;
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL1, M_RD) ;
    /* set addr, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;

    val = mmap_intfc_read(edt_p, MEM1READ0) ;
    val |= mmap_intfc_read(edt_p, MEM1READ1) << 8 ;
    val |= mmap_intfc_read(edt_p, MEM1READ2) << 16 ;
    val |= mmap_intfc_read(edt_p, MEM1READ3) << 24 ;
    return(val) ;
}
ssmem_tagw0(EdtDev *edt_p, u_int addr, u_char tag)
{
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL0, M_WR) ;
    /* set addr,data,tag, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;
    mmap_intfc_write(edt_p, MEMTAGW, tag) ;
    mmap_intfc_write(edt_p, MEMCTL0, M_WR | M_MEMWRITE) ;
    mmap_intfc_write(edt_p, MEMCTL0, M_WR) ;
}
u_char
ssmem_tagr(EdtDev *edt_p, u_int addr) 
{
    u_char val ;
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL0, M_RD) ;
    mmap_intfc_write(edt_p, MEMCTL1, M_RD) ;
    /* set addr, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;

    val = mmap_intfc_read(edt_p, MEMTAGR) ;
    return(val) ;
}
ssmem_tagw1(EdtDev *edt_p, u_int addr, u_char tag)
{
    /* set xilinx data out, clr mem data out, all bytes enable  */
    mmap_intfc_write(edt_p, MEMCTL1, M_WR) ;
    /* set addr,data,tag, then strobe write */
    mmap_intfc_write(edt_p, MEMADDRL, addr & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRM, (addr >> 8) & 0xff) ;
    mmap_intfc_write(edt_p, MEMADDRH, (addr >> 16) & 0xff) ;
    mmap_intfc_write(edt_p, MEMTAGW, tag) ;
    mmap_intfc_write(edt_p, MEMCTL1, M_WR | M_MEMWRITE) ;
    mmap_intfc_write(edt_p, MEMCTL1, M_WR) ;
}


main(int argc,char **argv)
{
    EdtDev *edt_p ;
    int unit = 0 ;
    u_int addr ;
    u_int val ;
    u_int zeroval ;
    u_int tstval ;
    int have_b0 = 0 ;
    int have_b1 = 0 ;
    u_int bank0_size ;
    u_int bank1_size ;
    u_int max_add ;
    int i ;
    
    --argc;
    ++argv;
    while (argc	&& argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	  case 'u':
		++argv;
		--argc;
		unit = atoi(argv[0]);
		break;
	  default:
		usage();
		exit(0);
	}
	argc--;
	argv++;
    }
    
    if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
    {
	char str[64] ;
	sprintf(str, "%s%d", EDT_INTERFACE, unit) ;
	perror(str) ;
	exit(1) ;
    }
    mapaddr = (v_char *)edt_mapmem(edt_p, 0, 256) ;

    printf("checking bank 0\n") ;
    have_b0 = 1 ;
    addr = 0 ; 
    for (i = 0 ; i < 32 ; i++)
    {
	ssmem_write0(edt_p, i, 1 << i) ;
    }
    for (i = 0 ; i < 32 ; i++)
    {
	val = ssmem_read0(edt_p, i) ;
	if (val != (u_int)(1 << i))
	{
	    printf("bank 0 addr %x %08x s/b %08x\n",i,val,1 <<i) ;
	    printf("did you bitload sstest.bit?\n") ;
	    have_b0 = 0 ;
	    break ;
	}
    }
    if (have_b0)
    {
	printf("checking bank 0 size\n") ;
	bank0_size = 0 ;
	tstval = 0x1000 ;
	/* need to check for 0 changing to show wrap of address */
	/* and need to check memory not present also */
	while (!bank0_size)
	{
	    ssmem_write0(edt_p, 0, 0x12345678) ;
	    ssmem_write0(edt_p, tstval, tstval) ;
	    zeroval = ssmem_read0(edt_p, 0) ;
	    val = ssmem_read0(edt_p, tstval) ;
	    if (val != tstval || zeroval == tstval)
		bank0_size = tstval ;
	    printf("%08x\r",tstval) ;
	    fflush(stdout) ;
	    tstval += 0x1000 ;
	}
	printf("bank 0 %d (0x%x) words %d megs\n",bank0_size,bank0_size,
		(bank0_size * 4) / (1024*1024)) ;
		
		
	printf("testing bank 0\n") ;
	max_add = bank0_size ;
	for (addr = 0 ; addr < bank0_size ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("write %x\r",addr) ;
		fflush(stdout) ;
	    }
	    ssmem_write0(edt_p, addr, addr) ;
	}
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("read  %x\r",addr) ;
		fflush(stdout) ;
	    }
	    val = ssmem_read0(edt_p, addr) ;
	    if (val != addr)
	    {
		printf("%x s/b %x\n",val,addr) ;
		break ;
	    }
	}
	printf("testing bank 0 complement\n") ;
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("write %x\r",addr) ;
		fflush(stdout) ;
	    }
	    ssmem_write0(edt_p, addr, addr ^ 0xffffffff) ;
	}
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("read  %x\r",addr) ;
		fflush(stdout) ;
	    }
	    val = ssmem_read0(edt_p, addr) ;
	    if (val != (addr ^ 0xffffffff))
	    {
		printf("%x s/b %x\n",val,addr ^ 0xffffffff) ;
		break ;
	    }
	}
    }
    printf("checking bank 1\n") ;
    have_b1 = 1 ;
    for (i = 0 ; i < 32 ; i++)
    {
	ssmem_write1(edt_p, i, 1 << i) ;
    }
    for (i = 0 ; i < 32 ; i++)
    {
	val = ssmem_read1(edt_p, i) ;
	if (val != (u_int)(1 << i))
	{
	    printf("bank 1 addr %x %08x s/b %08x\n",i,val,1 <<i) ;
	    have_b1 = 0 ;
	    break ;
	}
    }
    if (have_b1)
    {
	printf("checking bank 1 size\n") ;
	/* need to check for 0 changing to show wrap of address */
	/* and need to check memory not present also */
	bank1_size = 0 ;
	tstval = 0x1000 ;
	while (!bank1_size)
	{
	    ssmem_write1(edt_p, 0, 0x12345678) ;
	    ssmem_write1(edt_p, tstval, tstval) ;
	    zeroval = ssmem_read1(edt_p, 0) ;
	    val = ssmem_read1(edt_p, tstval) ;
	    if (val != tstval || zeroval == tstval)
		bank1_size = tstval ;
	    printf("%08x\r",tstval) ;
	    fflush(stdout) ;
	    tstval += 0x1000 ;
	}
	printf("bank 1 %d (0x%x) words %d megs\n",bank1_size,bank1_size,
		(bank1_size * 4) / (1024*1024)) ;
	printf("testing bank 1\n") ;
	max_add = bank1_size ;
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("write %x\r",addr) ;
		fflush(stdout) ;
	    }
	    ssmem_write1(edt_p, addr, addr) ;
	}
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("read  %x\r",addr) ;
		fflush(stdout) ;
	    }
	    val = ssmem_read1(edt_p, addr) ;
	    if (val != addr)
	    {
		printf("%x s/b %x\n",val,addr) ;
		break ;
	    }
	}
	printf("testing bank 1 complement\n") ;
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("write %x\r",addr) ;
		fflush(stdout) ;
	    }
	    ssmem_write1(edt_p, addr, addr ^ 0xffffffff) ;
	}
	for (addr = 0 ; addr < max_add ; addr++)
	{
	    if ((addr % 0x10000) == 0) 
	    {
		printf("read  %x\r",addr) ;
		fflush(stdout) ;
	    }
	    val = ssmem_read1(edt_p, addr) ;
	    if (val != (addr ^ 0xffffffff))
	    {
		printf("%x s/b %x\n",val,addr ^ 0xffffffff) ;
		break ;
	    }
	}
    }
printf("testing tags\n") ;
/* first clear */
    for (addr = 0 ; addr < 16 ; addr++)
    {
	ssmem_tagw0(edt_p, addr, 0) ;
	ssmem_tagw1(edt_p, addr, 0) ;
    }
/* now try to write to both nibbles and make sure only see ours */
    if (have_b0)
    {
	printf("testing bank 0\n") ;
	for (addr = 0 ; addr < 16 ; addr++)
	{
	    ssmem_tagw0(edt_p, addr, 0x11*addr) ;
	}
	for (addr = 0 ; addr < 16 ; addr++)
	{
	    val = ssmem_tagr(edt_p, addr) & 0xff ; ;
	    if (!have_b1) val &= 0xf ;
	    if (val != addr)
	    {
	    printf ("%x s/b %x\n", val, addr);
	    }
	}
    }
/* clear again */
    for (addr = 0 ; addr < 16 ; addr++)
    {
	ssmem_tagw0(edt_p, addr, 0) ;
	ssmem_tagw1(edt_p, addr, 0) ;
    }
    if (have_b1)
    {
	printf("testing bank 1\n") ;
	for (addr = 0 ; addr < 16 ; addr++)
	{
	    ssmem_tagw1(edt_p, addr, 0x11*addr) ;
	}
	for (addr = 0 ; addr < 16 ; addr++)
	{
	    val = ssmem_tagr(edt_p, addr) & 0xff ; ;
	    if (!have_b0) val &= 0xf0 ;
	    if (val != addr * 0x10)
	    {
		printf("%x s/b %x\n",val,addr * 0x10) ;
	    }
	}
    }
    return(0) ;
}

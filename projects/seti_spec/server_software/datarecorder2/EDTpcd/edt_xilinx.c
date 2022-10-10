/*
 * xilinx header read and other xilinx utilities
 */
#include "edtinc.h"
#include "edt_bitload.h"
#include "pciload.h"
#include <stdlib.h>

typedef struct {
    u_char fi[8];
    u_char name[MAXPATH];
    u_char id[32];
    u_char date[16];
    u_char time[16];
    u_long dsize;
} XHEADER;
XHEADER Xh;

/* Xilinx "magic number" (first 13 bytes) is always the same ( so far... ) */
#define XMSIZE 13
u_char Xmh[XMSIZE] = {0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x00, 0x00, 0x01 };

/* shorthand debug level */
#define DEBUG1 EDTLIB_MSG_INFO_1
#define DEBUG2 EDTLIB_MSG_INFO_2

static int do_fast = 0;
static int force_slow = 0;
static int debug_fast = 0;
static int hubload = 0;
int sunswap = 0;

static void   bt_write(EdtDev *edt_p, uint_t desc, u_int addr, u_char val);
static u_char bt_read(EdtDev *edt_p, uint_t desc, u_int addr);
static int    bt_byte_program(EdtDev *edt_p, uint_t desc, u_int addr, u_char data);
static void   bt_reset(EdtDev *edt_p, uint_t desc);
static u_char bt_readstat(EdtDev *edt_p, uint_t desc);
static void   bt_print16(EdtDev *edt_p, uint_t desc, u_int addr);
static int    bt_sector_erase(EdtDev *edt_p, u_int sector, u_int sec_size, uint_t desc);
static void   byte_program(EdtDev * edt_p, u_int addr, u_char data, int isbt);
static void   xwrite(EdtDev *edt_p, u_int addr, u_char val);
static u_char xread(EdtDev *edt_p, u_int addr);
static int    xbyte_program(EdtDev * edt_p, u_int addr, u_char data);
static void   xreset(EdtDev *edt_p);
static u_char xreadstat(EdtDev *edt_p);
static void   xprint16(EdtDev *edt_p, u_int addr);
static int    xsector_erase(EdtDev * edt_p, u_int sector, u_int sec_size);
static void   tst_write(EdtDev * edt_p, uint_t desc, uint_t val);

int
edt_ltx_hub(EdtDev *edt_p)
{
    if (hubload)
	return edt_p->hubidx;
    return -1;
}

void
edt_set_ltx_hub(EdtDev *edt_p, int hub)
{
    hubload = 1;
    edt_p->hubidx = hub;
}

int
edt_get_debug_fast()
{
	return (debug_fast) ;
}
int
edt_set_debug_fast(int val)
{
	(debug_fast = val) ;
	return val;
}
int
edt_get_do_fast()
{
	return (do_fast) ;
}
void
edt_set_do_fast(int val)
{
	do_fast = val ;
}
int
edt_get_force_slow()
{
	return (force_slow) ;
}
void
edt_set_force_slow(int val)
{
	force_slow = val ;
}

/*
 * xilinx header decoder -- file (see below for array version)
 */
static int
xf_get_magic(FILE *fp)
{
    u_char buf[512];
    int i, j, n;

    if ((n = fread(buf, 1, XMSIZE, fp)) != XMSIZE)
    {
	edt_msg(EDT_MSG_FATAL, "unexpected EOF of xilinx file (%d bytes in)\n", n);
	return -1;
    }

    for (i=0; i<XMSIZE; i++)
    {
	if (buf[i] != Xmh[i])
	{
	    edt_msg(DEBUG1, "invalid xilinx magic bytes\n");
	    edt_msg(DEBUG1, "header: < ");
	    for (j=0; j<XMSIZE; j++)
		edt_msg(DEBUG1, "%x ", buf[j]);
	    edt_msg(DEBUG1, ">\nexpect: < ");
	    for (j=0; j<XMSIZE; j++)
		edt_msg(DEBUG1, "%x ", Xmh[j]);
	    edt_msg(DEBUG1, ">\n");
	    return -1;
	}
    }
    return 0 ;
}


static int
xf_get_field_id(FILE *fp, u_char *fi)
{
    int n;

    if ((n = fread(fi, 1, 1, fp)) != 1)
    {
	edt_msg(DEBUG1, "invalid xilinx field ID\n");
	return -1;
    }

    return 0;
}

static int
xf_get_string(FILE *fp, u_char *str, u_int maxcount)
{
    u_char tmpcount[2];
    u_short count;
    int n;

    if ((n = fread(&tmpcount, 1, 2, fp)) != 2)
    {
	edt_msg(DEBUG1, "missing field string count in xilinx header\n");
	return -1;
    }

    count = tmpcount[0] << 8 | tmpcount[1];


    if ((count > maxcount) || ((n = fread(str, sizeof(u_char), count, fp)) != count))
    {
	edt_msg(DEBUG1, "invalid xilinx header string (count %d)\n", count);
	return -1;
    }
    return 0;
}

static int
xf_get_long_size(FILE *fp, u_long *size)
{
    int n;
    u_char tmpsize[4];

    if ((n = fread(tmpsize, 1, 4, fp)) != 4)
    {
	edt_msg(DEBUG1, "invalid xilinx header size (%d s/b 4)\n", n);
	return -1;
    }
    *size =  (tmpsize[0] << 24) | (tmpsize[1] << 16) | (tmpsize[2] << 8) | (tmpsize[3]);
    /* printf("size %08x tmpsize %02x %02x %02x %02x\n",
        *size, tmpsize[0], tmpsize[2], tmpsize[1], tmpsize[0]); */

    return 0;
}

/*
 * get the xilinx header  (FILE version)
 *
 * open the file, get the header, and close, and return the # of bytes into
 * the file of the first byte of data.
 */
long
edt_get_x_file_header(char *fname, char *header, int *size)
{
    FILE *infd = fopen(fname, "rb");
    long start_of_data;
    int ret = 0;

    if (infd == NULL)
    {
	edt_msg_perror(EDT_MSG_FATAL, fname);
	return(-1);
    }

    if (ret = edt_get_x_header(infd, header, size) != 0)
	ret = -1;
    else start_of_data = ftell(infd);
    fclose(infd);

    if (ret)
	return ret;

    return start_of_data;
}

/*
 * get the xilinx header from a FILE
 *
 * takes as an argument an already open file pointer. for this reason, not to be
 * called directly by an application, ony from within the library. application 
 * programs should instead use edt_get_x_file_header.
 */
int
edt_get_x_header(FILE *fp, char *header, int *size)
{
    int     i;
    long start, end;

    edt_msg(DEBUG2, "edt_get_x_header:\n");

    *size = 0;

    if (xf_get_magic(fp) < 0)
    {
	edt_msg(EDT_MSG_FATAL, "invalid xilinx magic bytes\n");
	return  -1;
    }

    if ((xf_get_field_id(fp, &Xh.fi[0]) != 0)
     || (xf_get_string(fp, Xh.name, sizeof(Xh.name)) != 0)
     || (xf_get_field_id(fp, &Xh.fi[1]) != 0)
     || (xf_get_string(fp, Xh.id, sizeof(Xh.id)) != 0)
     || (xf_get_field_id(fp, &Xh.fi[2]) != 0)
     || (xf_get_string(fp, Xh.date, sizeof(Xh.date)) != 0)
     || (xf_get_field_id(fp, &Xh.fi[3]) != 0)
     || (xf_get_string(fp, Xh.time, sizeof(Xh.time)) != 0)
     || (xf_get_field_id(fp, &Xh.fi[4]) != 0)
     || (xf_get_long_size(fp, &Xh.dsize) != 0))
     {
	edt_msg(EDT_MSG_FATAL, "Error: bad xilinx header\n");
#if 0
    edt_msg(DEBUG2, "%c) '%s'  %c) '%s'  %c) '%s'  %c) '%s'  %c) %d\n",
	    Xh.fi[0], Xh.name,
	    Xh.fi[1], Xh.id,
	    Xh.fi[2], Xh.date,
	    Xh.fi[3], Xh.time,
	    Xh.fi[4], Xh.dsize);
	return  -1;
#endif
    }

    for (i=0; i < 5; i++)
    {
	if (Xh.fi[i] != 'a' + i)
	{
	    edt_msg(EDT_MSG_FATAL, "invalid xilinx field id: '%c' (0x%02x) s/b '%c' (0x%02x)\n", Xh.fi[i], Xh.fi[i], 'a'+i, 'a'+i);
	    return -1;
	}
    }

    edt_msg(DEBUG2, "%c) '%s'  %c) '%s'  %c) '%s'  %c) '%s'  %c) %d\n",
	    Xh.fi[0], Xh.name,
	    Xh.fi[1], Xh.id,
	    Xh.fi[2], Xh.date,
	    Xh.fi[3], Xh.time,
	    Xh.fi[4], Xh.dsize);

    /*
     * check remaining size of file against dsize, then rewind file
     * pointer back to start of data 
     */
    start = ftell(fp);
    fseek(fp, Xh.dsize, SEEK_CUR);
    end = ftell(fp);
    if (Xh.dsize < (unsigned long)(end-start))
	edt_msg(EDT_MSG_WARNING, "Warning! dsize/file size mismatch (dsize %u, EOF @ +%u (may work anyway)\n", Xh.dsize, end-start);
    else if (Xh.dsize > (unsigned long)(end-start))
	edt_msg(EDT_MSG_FATAL, "Error! dsize/file size mismatch -- dsize %u but EOF at +%u\n", Xh.dsize, end-start);
    fseek(fp, start, SEEK_SET);

    *size = Xh.dsize;

    sprintf(header, "%s %s %s %s", Xh.name, Xh.id, Xh.date, Xh.time );

    return 0;
}


/*
 * new COOL xilinx header decoder
 */
static int
xa_get_magic(u_char **ba)
{
    u_char buf[512];
    int i, j;

    if (*ba == NULL)
    {
	edt_msg(EDT_MSG_FATAL, "unexpected short xilinx array  pointer\n");
	return -1;
    }

    memcpy(buf, *ba, XMSIZE);
    *ba += XMSIZE;

    for (i=0; i<XMSIZE; i++)
    {
	if (buf[i] != Xmh[i])
	{
	    edt_msg(DEBUG1, "invalid xilinx magic bytes\n");
	    edt_msg(DEBUG1, "header: < ");
	    for (j=0; j<XMSIZE; j++)
		edt_msg(DEBUG1, "%x ", buf[j]);
	    edt_msg(DEBUG1, ">\nexpect: < ");
	    for (j=0; j<XMSIZE; j++)
		edt_msg(DEBUG1, "%x ", Xmh[j]);
	    edt_msg(DEBUG1, ">\n");
	    return -1;
	}
    }
    return 0 ;
}


static int
xa_get_field_id(u_char **ba, u_char *fi)
{
    memcpy(fi, *ba, 1);
    ++(*ba);
    return 0;
}

static int
xa_get_string(u_char **ba, u_char *str, u_int maxcount)
{
    u_char tmpcount[2];
    u_short count;
 
    memcpy(tmpcount, *ba, 2);
    *ba += 2;

    count = tmpcount[0] << 8 | tmpcount[1];
    if (count > maxcount)
    {
	edt_msg(DEBUG1, "invalid xilinx header string (count %d)\n", count);
	return -1;
    }
    memcpy(str, *ba, count);
    *ba += count;
    return 0;
}

static int
xa_get_long_size(u_char **ba, u_long *size)
{
    u_char tmpsize[4];

    memcpy(tmpsize, *ba, 4);
    *ba += 4;

    *size =  (tmpsize[0] << 24) | (tmpsize[1] << 16) | (tmpsize[2] << 8) | (tmpsize[3]);
    /* printf("size %08x tmpsize %02x %02x %02x %02x\n",
        *size, tmpsize[0], tmpsize[2], tmpsize[1], tmpsize[0]); */

    return 0;
}

/*
 * get the xilinx header (ARRAY version)
 *
 * open the array, get the header, and close, and return the # of bytes into
 * the file of the first byte of data.
 */
u_char *
edt_get_x_array_header(u_char *ba, char *header, int *size)
{
    u_char *start_of_data;

    if ((start_of_data = edt_get_xa_header(ba, header, size)) == NULL)
	start_of_data = ba;

    return start_of_data;
}

/*
 * get the xilinx header 
 *
 * takes as an argument an array pointer. for this reason, not to be
 * called directly by an application, ony from within the library. application 
 * programs should instead use edt_get_x_file_header.
 */
u_char *
edt_get_xa_header(u_char *ba, char *header, int *size)
{
    int     i;
    u_char *ta[1], *start, *end;

    edt_msg(DEBUG2, "edt_get_xa_header:\n");

    *size = 0;
    start = ba;
    *ta = ba;

    if (xa_get_magic(ta) < 0)
    {
	edt_msg(EDT_MSG_FATAL, "invalid xilinx magic bytes\n");
	return  NULL;
    }

    if ((xa_get_field_id(ta, &Xh.fi[0]) != 0)
     || (xa_get_string(ta, Xh.name, 128) != 0)
     || (xa_get_field_id(ta, &Xh.fi[1]) != 0)
     || (xa_get_string(ta, Xh.id, 32) != 0)
     || (xa_get_field_id(ta, &Xh.fi[2]) != 0)
     || (xa_get_string(ta, Xh.date, 16) != 0)
     || (xa_get_field_id(ta, &Xh.fi[3]) != 0)
     || (xa_get_string(ta, Xh.time, 16) != 0)
     || (xa_get_field_id(ta, &Xh.fi[4]) != 0)
     || (xa_get_long_size(ta, &Xh.dsize) != 0))
     {
	edt_msg(EDT_MSG_FATAL, "Error: bad xilinx header\n");
#if 0
    edt_msg(DEBUG2, "%c) '%s'  %c) '%s'  %c) '%s'  %c) '%s'  %c) %d\n",
	    Xh.fi[0], Xh.name,
	    Xh.fi[1], Xh.id,
	    Xh.fi[2], Xh.date,
	    Xh.fi[3], Xh.time,
	    Xh.fi[4], Xh.dsize);
	return  -1;
#endif
    }

    for (i=0; i < 5; i++)
    {
	if (Xh.fi[i] != 'a' + i)
	{
	    edt_msg(EDT_MSG_FATAL, "invalid xilinx field id: '%c' (0x%02x) s/b '%c' (0x%02x)\n", Xh.fi[i], Xh.fi[i], 'a'+i, 'a'+i);
	    return NULL;
	}
    }

    edt_msg(DEBUG2, "%c) '%s'  %c) '%s'  %c) '%s'  %c) '%s'  %c) %d\n",
	    Xh.fi[0], Xh.name,
	    Xh.fi[1], Xh.id,
	    Xh.fi[2], Xh.date,
	    Xh.fi[3], Xh.time,
	    Xh.fi[4], Xh.dsize);

    /*
     * check remaining size of file against dsize, then rewind file
     * pointer back to start of data 
     */
    start = *ta;
    *ta += Xh.dsize;
    end = *ta;
    if (Xh.dsize < (unsigned long)(end-start))
	edt_msg(EDT_MSG_WARNING, "Warning! dsize/file size mismatch (dsize %u, EOF @ +%u (may work anyway)\n", Xh.dsize, end-start);
    else if (Xh.dsize > (unsigned long)(end-start))
	edt_msg(EDT_MSG_FATAL, "Error! dsize/file size mismatch -- dsize %u but EOF at +%u\n", Xh.dsize, end-start);
    *ta = start;

    *size = Xh.dsize;

    sprintf(header, "%s %s %s %s", Xh.name, Xh.id, Xh.date, Xh.time );

    return start;
}

static char *device_name[NUM_DEVICE_TYPES] = {"pcd", "pdv", "p11w", "p16d"};
static void parse_id(char *idstr, u_int * date, u_int * time);
static void remove_char(char *str, char ch);
char *read_x_string(EdtDev *edt_p, u_int addr, char *buf, int size, int xtype);


/*
 * command bits for the 4028xla boot controller
 */
#define BT_EN_READ	0x8000

/*
 * bit to set in all prom addresses to enable the upper 2 bits of prom
 * address after we have read the status (jumper and 5/3.3v)
 */
#define	EN_HIGH_ADD	0x00800000

/* what are these? */
#define DQ7 0x80
#define DQ5 0x20
#define DQ3 0x08
static int maxloops = 0;

volatile caddr_t dmyaddr = 0;
volatile u_int *cfgaddr ;
volatile u_int *tstaddr ;
int needswap ;

u_char
mmap_intfc_read(caddr_t mapaddr, u_int desc)
{
	caddr_t off_p ;
	caddr_t data_p ;
	u_char val ;
	/* interface must set offset then read data */
	off_p = mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	val = *off_p ;
	val = *data_p ;
	return(val) ;
}

void
mmap_intfc_write(caddr_t mapaddr, u_int desc, u_char val)
{
	caddr_t off_p ;
	caddr_t data_p ;
	u_char tmp ;
	/* interface must set offset then read data */
	off_p = mapaddr + (EDT_REMOTE_OFFSET & 0xff) ;
	data_p = mapaddr + (EDT_REMOTE_DATA & 0xff) ;
	*off_p = desc & 0xff ;
	/* to flush */
	tmp = *off_p ;
	*data_p = val ;
}

int
ltx_tst_init(EdtDev * edt_p)
{

    if (((int)dmyaddr != 0) && ((int)edt_p->mapaddr != 0))
	return 0;

    dmyaddr = (volatile caddr_t)edt_mapmem(edt_p, 0, 0x10000);
    if ((int) dmyaddr == 0)
    {
	printf("edt_mapmem failed\n");
	return -1;
    }

    if (edt_get_mappable_size(edt_p) > 0)
	edt_p->mapaddr = (volatile caddr_t) edt_mapmem(edt_p, 0x10000, edt_get_mappable_size(edt_p));
    else edt_p->mapaddr = 0;

    (void)tst_write(edt_p, 0xf00004, 0xFF);
    return 0;
}


int 
ltx_check_mmap(EdtDev * edt_p, int space)
{
    if (ltx_tst_init(edt_p) < 0)
	return -1;
    if (space == 0)
    {
	if (dmyaddr == 0)
	{
	    fprintf(stderr, "bad mmap for space 0\n");
	    return (1);
	}
	else
	    return (0);
    }
    else
    {
	if (edt_p->mapaddr == 0)
	{
	    fprintf(stderr, "bad mmap for space 1\n");
	    return (1);
	}
	else
	    return (0);
    }
}

u_int
swap32(u_int val)
{
    /* already set hardware swap if LTX hub */
    if (hubload)
	return val;

    return (
	    ((val & 0x000000ff) << 24)
	    | ((val & 0x0000ff00) << 8)
	    | ((val & 0x00ff0000) >> 8)
	    | ((val & 0xff000000) >> 24));
}

#if 0 /* not used anywhere? */
void
prog_write(u_int val)
{
    if(needswap) *cfgaddr = swap32(val) ;
    else *cfgaddr = val ;
}

u_int
prog_read()
{
    if(needswap) return (swap32(*cfgaddr)) ;
    else return (*cfgaddr) ;
}
#endif

void tst_init(EdtDev *edt_p, int verbose)
{
    edt_p->mapaddr = (caddr_t)edt_mapmem(edt_p, 0, 256) ;
    cfgaddr = (volatile u_int *)(edt_p->mapaddr + (EDT_DMA_CFG & 0xff)) ;

    if (hubload)
    {
	printf("HUBLOAD INTERNAL ERROR: should never call tst_init\n");
	exit(1);
    }

    needswap = !edt_little_endian();
#ifdef sgi
    needswap = 0 ;
#endif

    if (verbose )
	printf("  mapaddr %p cfgaddr %p needswap %d\n",edt_p->mapaddr,cfgaddr,needswap) ;
}

void
tst_write(EdtDev * edt_p, uint_t desc, uint_t val)
{
    u_int dmy;

    if (hubload || do_fast)
    {
	if (debug_fast & 0x10)
	    edt_msleep(1);

	if (hubload)
	    tstaddr = (volatile u_int *)(edt_p->mapaddr + desc);
	else tstaddr = (volatile u_int *)(edt_p->mapaddr + (desc & 0xff)) ;


	if (needswap)
	{
	    *tstaddr = swap32(val) ;
/* printf("DEBUG tst_write desc %x mapaddr %x tstaddr %x val %x (SWAP)\n", desc, edt_p->mapaddr, tstaddr, val); */

	    /* debug: try writing a few more times */
	    if (debug_fast & 0x1)
	    {
		*tstaddr = swap32(val) ;
		*tstaddr = swap32(val) ;
		*tstaddr = swap32(val) ;
		*tstaddr = swap32(val) ;
	    }
	}
	else
	{
	    *tstaddr = val ;

/* printf("DEBUG tst_write desc %x mapaddr %x tstaddr %x val %x\n", desc, edt_p->mapaddr, tstaddr, val); */

	    /* debug: try writing a few more times */
	    if (debug_fast & 0x1)
	    {
		*tstaddr = val ;
		*tstaddr = val ;
		*tstaddr = val ;
		*tstaddr = val ;
	    }
	}

	dmy = *tstaddr;

	/* okay do a few more reads while things settle down if needed (-F 2) */
	if (debug_fast & 0x2)
	{
	    dmy = *tstaddr;
	    dmy = *tstaddr;
	    dmy = *tstaddr;
	    dmy = *tstaddr;
	    dmy = *tstaddr;
	}
    }
    else 
    {
	edt_reg_write(edt_p, desc, val) ;
        dmy = edt_reg_read(edt_p, desc) ;
    }
}

uint_t
tst_read(EdtDev * edt_p, uint_t desc)
{
    u_int	ret;

    if (do_fast || hubload)
    {
	if (debug_fast & 0x20)
	    edt_msleep(1);
	if (hubload)
	    tstaddr = (volatile u_int *)(edt_p->mapaddr + desc) ;
	else tstaddr = (volatile u_int *)(edt_p->mapaddr + (desc & 0xff)) ;

	if (needswap)
	{
	    ret = (swap32(*tstaddr)) ;
	}
	else ret =  (*tstaddr) ;


	/* debug: try reading a few more times */
	if (debug_fast & 0x4)
	{
	    ret = *tstaddr;
	    ret = *tstaddr;
	    ret = *tstaddr;
	    ret = *tstaddr;
	}

/* printf("DEBUG tst_read desc %x mapaddr %x tstaddr %x ret %x\n", desc, edt_p->mapaddr, tstaddr, ret); */

	return ret;
    }
    else return(edt_reg_read(edt_p, desc)) ;
}

void tst_pflash(EdtDev *edt_p, uint_t addr, uint_t val)
{
    if (hubload)
    {
	printf("ERROR -- tst_pflash with hubload N/A!\n");
	return;
    }

    tst_write(edt_p, EDT_FLASHROM_DATA, (u_char) 0xaa);
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_ADDR, (u_int) (0x5555 | EDT_WRITECMD));
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_DATA, (u_char) 0x55);
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_ADDR, (u_int) (0x2aaa | EDT_WRITECMD));
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_DATA, (u_char) 0xa0);
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_ADDR, (u_int) (0x5555 | EDT_WRITECMD));
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_DATA, (u_char) val);
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
    tst_write(edt_p, EDT_FLASHROM_ADDR, 
	    (u_int) (EN_HIGH_ADD | addr | EDT_WRITECMD));
    (void) tst_read(edt_p, EDT_FLASHROM_ADDR);
}

/*
 * check ltx hub. return -1 if not an SS4, 0 if is an SS4 but no hub found, 1 if ok
 */
int
edt_check_ltx_hub(EdtDev *edt_p)
{
    if ((!edt_p) || ((edt_p->devid != PSS4_ID) && (edt_p->devid != PCDFOX_ID)))
    	return -1;
    if (ltx_tst_init(edt_p) < 0)
	return -1;
    if (tst_read(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx)) == 0xdeaddead)
    	return -1;
    return 0;
}

struct
{
    u_int   mid;
    u_int   devid;
    u_int   prom;
}       devices[] =

{
    {
	0x1, 0x20, AMD_4013E
    },
    {
	0x1, 0x4f, AMD_4013XLA
    },
    {
	0x20, 0xe3, AMD_4013XLA
    },
    {
	0x00, 0x00, 0
    }
};



/* write a byte to the PCI interface prom (only 4013E and 4013XLA) */
void
xwrite(EdtDev * edt_p, u_int addr, u_char val)
{
    edt_buf buf;

    if (hubload)
    {
	printf("ERROR -- xwrite with hubload N/A!\n");
	return;
    }

    if (do_fast)
    {
	(void)tst_read(edt_p, EDT_FLASHROM_ADDR);
	tst_write(edt_p, EDT_FLASHROM_DATA, val) ;
	(void)tst_read(edt_p, EDT_FLASHROM_ADDR);
	tst_write(edt_p, EDT_FLASHROM_ADDR, EN_HIGH_ADD | addr | EDT_WRITECMD) ;
    }
    else
    {
	buf.desc = EN_HIGH_ADD | addr;
	buf.value = val;
	edt_ioctl(edt_p, EDTS_FLASH, &buf);
    }
}

/* read a byte from the PCI interface prom (only 4013E and 4013XLA) */
u_char
xread(EdtDev * edt_p, u_int addr)
{
    u_char  val;
    edt_buf buf;

    if (do_fast)
    {
	(void)tst_read(edt_p, EDT_FLASHROM_ADDR);
	tst_write(edt_p, EDT_FLASHROM_ADDR, EN_HIGH_ADD | addr) ;
	(void)tst_read(edt_p, EDT_FLASHROM_ADDR);
	return(tst_read(edt_p, EDT_FLASHROM_DATA));
    }
    else
    {
	buf.desc = EN_HIGH_ADD | addr;
	edt_ioctl(edt_p, EDTG_FLASH, &buf);
	val = (u_char) buf.value;
	return (val);
    }
}

/* ?reset? 4013e or 4013xla */
void 
xreset(EdtDev * edt_p)
{
    xwrite(edt_p, 0x5555, 0xaa);
    xwrite(edt_p, 0x2aaa, 0x55);
    xwrite(edt_p, 0x5555, 0xf0);
}




/*
 * write a byte to the PROM thru the boot controller on the 4028xla
 */
static void
bt_write(EdtDev * edt_p, uint_t desc, u_int addr, u_char val)
{
    /* write the address out in three chunks to the boot controller */
    /* first all commands are off */
    tst_write(edt_p, desc, 0);
    /* first byte of address */
    tst_write(edt_p, desc, BT_LD_LO | (addr & 0xff));
    tst_write(edt_p, desc, 0);
    /* second byte of address */
    tst_write(edt_p, desc, BT_LD_MID | ((addr >> 8) & 0xff));
    tst_write(edt_p, desc, 0);

    /* third byte of address */
    tst_write(edt_p, desc, BT_LD_HI | ((addr >> 16) & 0xff));
    tst_write(edt_p, desc, 0);

    tst_write(edt_p, desc, BT_LD_ROM | (val & 0xff));
    tst_write(edt_p, desc, 0);
}

/*
 * read a byte from the 4028xla prom through the boot controller.
 */
static u_char
bt_read(EdtDev * edt_p, uint_t desc, u_int addr)
{
    u_char  stat;

    /* write the address out in three chunks to the boot controller */
    /* first all commands are off */
    tst_write(edt_p, desc, 0);
    /* first byte of address */
    tst_write(edt_p, desc, BT_LD_LO | (addr & 0xff));
    /* clear BT_LD_LO */
    tst_write(edt_p, desc, 0);
    /* second byte of address */
    tst_write(edt_p, desc, BT_LD_MID | ((addr >> 8) & 0xff));
    /* clear BT_LD_MID */
    tst_write(edt_p, desc, 0);
    /* third byte of address */
    tst_write(edt_p, desc, BT_LD_HI | ((addr >> 16) & 0xff));
    /* clear BT_LD_HI , set rd command and disable output buffer */
    tst_write(edt_p, desc, BT_RD_ROM);

    /* read the byte */
    stat = (u_char) tst_read(edt_p, desc);
    /* all commands are off */
    tst_write(edt_p, desc, 0);
    return (stat);
}

/* ?reset? the 4028xla */
static void 
bt_reset(EdtDev * edt_p, uint_t desc)
{
    bt_write(edt_p, desc, 0x555, 0xaa);
    bt_write(edt_p, desc, 0x2aa, 0x55);
    bt_write(edt_p, desc, 0x555, 0xf0);
}

/*
 * read the jumper position and bus voltage from the boot controller
 */
static u_char
bt_readstat(EdtDev * edt_p, uint_t desc)
{
    u_char  stat;
    uint_t rdval;

    /*
     * read the boot controller status A0 and A1 low with read asserted -
     * enables boot controller on bus
     */
    tst_write(edt_p, desc, 0);
    tst_write(edt_p, desc, BT_EN_READ | BT_READ);
    rdval = tst_read(edt_p, desc);
    stat = (u_char) rdval;
    /* reset all boot command */
	tst_write(edt_p, desc, 0);
    return (stat);
}


u_char
xreadstat(EdtDev * edt_p)
{
    u_char  stat;
    uint_t  rdval;

    if (hubload)
    {
	printf("ERROR -- xreadstat with hubload N/A!\n");
	return 0;
    }


    /*
     * disable the high address drive by writing 0 to bit24 of FPROM address
     * register.
     */
    tst_write(edt_p, EDT_FLASHROM_ADDR, 0);
    rdval = tst_read(edt_p, EDT_FLASHROM_DATA);
    stat = (u_char) (rdval >> 8);
    return (stat);
}

u_char
edt_x_read(EdtDev * edt_p, u_int addr, int xtype)
{
    switch(xtype)
    {
	case XTYPE_BT:
	    return bt_read(edt_p, EDT_FLASHROM_DATA, addr);
	    break;
	case XTYPE_X:
	    return xread(edt_p, addr);
	    break;
	case XTYPE_LTX:
	    return bt_read(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx), addr);
	    break;
    }

	return 0;
}

void
edt_x_print16(EdtDev * edt_p, u_int addr, int xtype)
{
    switch(xtype)
    {
	case XTYPE_BT:
	    bt_print16(edt_p, EDT_FLASHROM_DATA, addr);
	    break;
	case XTYPE_X:
	    xprint16(edt_p, addr);
	    break;
	case XTYPE_LTX:
	    bt_print16(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx), addr);
	    break;
    }
}

/*
 * print 16 bytes at the address passed read from 4028xla boot controller
 */
void
bt_print16(EdtDev * edt_p, uint_t desc, u_int addr)
{
    int     i;

    printf("%08x ", addr);
    for (i = 1; i < 16; i++)
    {
	printf(" %02x", bt_read(edt_p, desc, addr));
	addr++;
    }
    printf("\n");
}

/*
 * print 16 bytes at the address passed
 */
void
xprint16(EdtDev * edt_p, u_int addr)
{
    int     i;

    printf("%08x ", addr);
    for (i = 1; i < 16; i++)
    {
	printf(" %02x", xread(edt_p, addr));
	addr++;
    }
    printf("\n");
}

int
edt_sector_erase(EdtDev * edt_p, u_int sector, u_int sec_size, int xtype)
{
    switch(xtype)
    {
	case XTYPE_BT:
	    bt_sector_erase(edt_p, sector, sec_size, EDT_FLASHROM_DATA);
	    break;
	case XTYPE_X:
	    xsector_erase(edt_p, sector, sec_size);
	    break;
	case XTYPE_LTX:
	    bt_sector_erase(edt_p, sector, sec_size, EDT_FLASHROM_LTXHUB(edt_p->hubidx));
	    break;
    }
    return 0;
}

/*
 * sector erase for 4028xla
 */
static int
bt_sector_erase(EdtDev * edt_p, u_int sector, u_int sec_size, uint_t desc)
{
    u_int   addr;
    int     done = 0;
    int     loops = 0;
    u_char  val;

    addr = sector * sec_size;
    bt_write(edt_p, desc, 0x5555, 0xaa);
    bt_write(edt_p, desc, 0x2aaa, 0x55);
    bt_write(edt_p, desc, 0x5555, 0x80);
    bt_write(edt_p, desc, 0x5555, 0xaa);
    bt_write(edt_p, desc, 0x2aaa, 0x55);
    bt_write(edt_p, desc, addr, 0x30);
    done = 0;
    while (!done)
    {
	val = bt_read(edt_p, desc, addr);
	loops++;
	if (val & DQ7)
	{
	    done = 1;
	}
    }
    return 0;
}

static int
xsector_erase(EdtDev * edt_p, u_int sector, u_int sec_size)
{
    u_int   addr;
    int     done = 0;
    int     loops = 0;
    u_char  val;

    addr = sector * sec_size;
    xwrite(edt_p, 0x5555, 0xaa);
    xwrite(edt_p, 0x2aaa, 0x55);
    xwrite(edt_p, 0x5555, 0x80);
    xwrite(edt_p, 0x5555, 0xaa);
    xwrite(edt_p, 0x2aaa, 0x55);
    xwrite(edt_p, addr, 0x30);
    done = 0;
    while (!done)
    {
	val = xread(edt_p, addr);
	loops++;
	if (val & DQ7)
	{
	    done = 1;
	    break;
	}
    }
    return 0;
}

/*
 * for 4028xla
 */
static int
bt_byte_program(EdtDev * edt_p, uint_t desc, u_int addr, u_char data)
{
    u_char  val;
    int     done = 0;
    int     loops = 0;

    bt_write(edt_p, desc, 0x5555, 0xaa);
    bt_write(edt_p, desc, 0x2aaa, 0x55);
    bt_write(edt_p, desc, 0x5555, 0xa0);
    bt_write(edt_p, desc, addr, data);
    tst_write(edt_p, desc, BT_RD_ROM);

    while (!done)
    {
    	val = (u_char) tst_read(edt_p, desc);

	if (val == data)
	    done = 1;
	if (loops > 1000)
	{
	    printf("failed on %x data %x val %x\n",
		   addr, data, val);
	    bt_reset(edt_p, desc);
	    bt_write(edt_p, desc, 0x555, 0xaa);
	    bt_write(edt_p, desc, 0x2aa, 0x55);
	    bt_write(edt_p, desc, 0x555, 0xa0);
	    bt_write(edt_p, desc, addr, data);
	    tst_write(edt_p, desc, BT_RD_ROM);
	    loops = 0;
	}
	loops++;
    }
    if (loops > maxloops)
    {
	maxloops = loops;
    }
    if (done)
	return (0);
    else
	return (addr);
}

void
edt_ltx_reboot(EdtDev *edt_p)
{
    tst_write(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx), 0x4000);
}

/*
 * for 4013e and others
 */

/* return 0 for success */
int
xbyte_program(EdtDev * edt_p, u_int addr, u_char data)
{
    u_char  val;
    edt_buf buf;
    int     done = 0;
    int     loops = 0;

    if (do_fast)
    {
	tst_pflash(edt_p, addr, data) ;
    }
    else
    {
	buf.desc = EN_HIGH_ADD | addr;
	buf.value = data;
	edt_ioctl(edt_p, EDT_PFLASH, &buf);
    }
    while (!done)
    {
	loops++;
	val = xread(edt_p, addr);
	if (val == data)
	    done = 1;
	if (loops > 100)
	{
	    printf("failed on %x data %x val %x\n",
		   addr, data, val);
	    loops = 0;
	}
    }
    if (loops > maxloops)
    {
	if (loops > 10)
	    printf("maxloops %x %d\n", addr, loops);
	maxloops = loops;
    }
    if (done)
	return (0);
    else
	return (addr);
}

int
edt_x_prom_detect(EdtDev * edt_p, u_char *stat, int verbose)
{
    u_int   desc = EDT_FLASHROM_DATA;
    u_int   flashrom_bits;
    u_char  mid, devid;
    int     prom;

    /* set up structures for direct peek/poke */
    tst_init(edt_p, verbose) ;

    if (edt_p->mapaddr == 0)
	force_slow = 1 ;
    /*
     * if the BT_A0 bit in the PROM_DATA can be written high
     * this must be a new board with a 4028xla or XC2S___
     */

    tst_write(edt_p, desc, BT_A0);
    flashrom_bits = tst_read(edt_p, desc) & BT_A0;

    if (flashrom_bits == BT_A0)
    {
	if (verbose) printf("  FLASHROM command BT_A0 can be set board has boot controller\n");

	*stat = bt_readstat(edt_p, desc);
	if (verbose) printf("\t\tRead code %02x id bits %x\n", *stat, (*stat & STAT_IDMASK)>> STAT_IDSHFT);
	
	if (hubload)
	{
	    switch ((*stat & STAT_IDMASK)>> STAT_IDSHFT)
	    {
		case STAT_LTX_XC2S300E:
		    prom = AMD_XC2S300E;
		    break;
		default:
		    prom = PROM_UNKN;
		    break;
	    }
	}
	else
	{
	    switch ((*stat & STAT_IDMASK)>> STAT_IDSHFT)
	    {
		case STAT_XC4028XLA:
		    prom = AMD_4028XLA;
		    break;
		case STAT_XC2S150:
		    prom = AMD_XC2S150;
		    break;
		case STAT_XC2S200_NP:
		    prom = AMD_XC2S200_4M;
		    break;
		case STAT_XC2S200:
		    prom = AMD_XC2S200_8M;
		    break;
		case STAT_XC2S100:
		    prom = AMD_XC2S100_8M;
		    break;
		default:
		    prom = PROM_UNKN;
		    break;
	    }
	}

    }
    else if (hubload)
    {
	if (verbose)
	    printf("  hubload TRUE but BT_A0 wrote, read back %02x; no hubs connected\n", flashrom_bits);
	return PROM_UNKN;
    }
    else
    {
	if (verbose) 
	    printf("  BT_A0 wrote, read back %02x, must not have boot controller\n", flashrom_bits);
	prom = PROM_UNKN;
    }
    /*
     * if prom code is not set yet test for other prom types by reading 
     * manufacturer code in flash prom.
     */
    if (prom == PROM_UNKN)  
    {
	int i;

	edt_x_reset(edt_p, 0);

	/* this code borrowed from xautoselect() */
	xwrite(edt_p, 0x5555, 0xaa);
	xwrite(edt_p, 0x2aaa, 0x55);
	xwrite(edt_p, 0x5555, 0x90);
	mid = xread(edt_p, 0x0);
	devid = xread(edt_p, 0x1);

	for (i = 0; devices[i].prom != 0; i++)
	{
	    if (mid == devices[i].mid && devid == devices[i].devid)
	    {
		prom = devices[i].prom;
		break;
	    }
	}

	if (verbose)
	    printf("  edt_prom_detect: autoselect manuf id %02x devid %02x code %d\n",
		   mid, devid, prom);
	edt_x_reset(edt_p, 0);

	*stat = xreadstat(edt_p);
    }
    return (prom);
}

int
edt_ltx_prom_detect(EdtDev * edt_p, u_char *stat, u_int *flashrom_bits, int verbose)
{
    u_int   desc = EDT_FLASHROM_DATA;
    int     prom;

   desc = EDT_FLASHROM_LTXHUB(edt_p->hubidx);

    /* set up structures for direct peek/poke */
    if (ltx_tst_init(edt_p) < 0)
	return PROM_UNKN;
    if (edt_p->mapaddr == 0)
	force_slow = 1 ;
    /*
     * if the BT_A0 bit in the PROM_DATA can be written high
     * this is probably the right kind of Xilinx
     */

    tst_write(edt_p, desc, BT_A0);
    *flashrom_bits = tst_read(edt_p, desc) & BT_A0;

    if (*flashrom_bits == BT_A0)
    {
	if (verbose) printf("  FLASHROM command BT_A0 can be set board has boot controller\n");

	*stat = bt_readstat(edt_p, desc);
	if (verbose) printf("\t\tRead code %02x id bits %x\n", *stat, (*stat & STAT_IDMASK)>> STAT_IDSHFT);
	
	switch ((*stat & STAT_IDMASK)>> STAT_IDSHFT)
	{
	    case STAT_LTX_XC2S300E:
		prom = AMD_XC2S300E;
		break;
	    default:
		prom = PROM_UNKN;
		break;
	}

    }
    else 
    {
	if (verbose)
	    printf("  hubload TRUE but BT_A0 wrote, read back %02x; no hubs connected\n", *flashrom_bits);
	return PROM_UNKN;
    }
    return (prom);
}


/*
 * get the onboard PROM device info from the string at the end of the segment
 */
void 
edt_readinfo(EdtDev *edt_p, int promcode, int segment, char *id, char *edtsn, char *oemsn)
{
    char    idbuf[ID_SIZE + 1];
    char    oemsnbuf[ID_SIZE + 1];
    char    edtsnbuf[ID_SIZE + 1];
    char    *ret;
    u_int   id_addr;
    u_int   edtsn_addr;
    u_int   oemsn_addr;
    int     xtype;

    id[0] = 0;
    edtsn[0] = 0;
    oemsn[0] = 0;

    id_addr = edt_get_id_addr(promcode, segment, &xtype);
    oemsn_addr = id_addr - OSN_SIZE;
    edtsn_addr = oemsn_addr - ESN_SIZE;
/* printf("\nDEBUG edt_readinfo oemsn addr %x edtsn addr %x\n", oemsn_addr, edtsn_addr); */

    if (ret = read_x_string(edt_p, id_addr, idbuf, ID_SIZE, xtype))
	strcpy(id, ret);
/* printf("\nDEBUG edt_readinfo %s\n", id); */

    if (ret = read_x_string(edt_p, oemsn_addr, oemsnbuf, OSN_SIZE, xtype))
	if (strncmp(ret, "OSN:", 4) == 0)
	{
	    strcpy(oemsn, &ret[4]);
	    if (oemsn[strlen(oemsn)-1] == ':')
		oemsn[strlen(oemsn)-1] = '\0';
	}

    if (ret = read_x_string(edt_p, edtsn_addr, edtsnbuf, ESN_SIZE, xtype))
	if (strncmp(ret, "ESN:", 4) == 0)
	    strcpy(edtsn, &ret[4]);
/* printf("\nDEBUG edt_readinfo edtsn %s\n", edtsn); */

    edt_x_reset(edt_p, xtype);
}


u_int
edt_get_id_addr(int promcode, int segment, int *xtype)
{
    u_int id_addr;

    switch(promcode)
    {
	case AMD_4013E:
	    id_addr = (8 * E_SECTOR_SIZE) - ID_SIZE;
	    *xtype = XTYPE_X;
	    break;

	case AMD_4013XLA:
	    id_addr = (segment * XLA_SECTOR_SIZE) + XLA_SECTOR_SIZE - MAX_STRING;
	    *xtype = XTYPE_X;
	    break;

	case AMD_4028XLA:
	    id_addr = ((segment + 1) * 2 * XLA_SECTOR_SIZE) - MAX_STRING;
	    *xtype = XTYPE_BT;
	    break;

	case AMD_XC2S150:
	    id_addr = ((segment + 1) * 2 * XLA_SECTOR_SIZE) - MAX_STRING;
	    *xtype = XTYPE_BT;
	    break;

	case AMD_XC2S200_4M:
	case AMD_XC2S200_8M:
	case AMD_XC2S100_8M:
	    id_addr = ((segment + 1) * 4 * XLA_SECTOR_SIZE) - MAX_STRING;
	    *xtype = XTYPE_BT;
	    break;

	case AMD_XC2S300E:
	    id_addr = ((segment + 1) * 4 * XLA_SECTOR_SIZE) - MAX_STRING;
	    *xtype = XTYPE_LTX;
	    break;

	default:
	    *xtype = XTYPE_X;
	    return 0;
	    break;

    }
    return id_addr;
}

/*
 * read a string out of the xilinx, of a given size, from a given address.
 * used for Xilinx ID id string and serial number strings
 *
 * ARGUMENTS
 *   edt_p     pointer to already opened edt device
 *   addr      address to start reading the string
 *   buf       buffer to read the string into
 *   size      number of bytes to read
 *
 * RETURNS
 *   pointer to magic first valid ASCII character in string (should be
 *   0th but may not be, esp. if blank)
 *
 */
char *
read_x_string(EdtDev *edt_p, u_int addr, char *buf, int size, int xtype)
{
    int  i;
    char *ret = 0;
    u_char  val;
/* printf("\nDEBUG read_x_string addr %x size %d xtype %d\n", addr, size, xtype); */

    /* printf("full string = "); */
    for (i = 0; i < size; i++)
    {
	switch(xtype)
	{
	    case XTYPE_BT:
		val = bt_read(edt_p, EDT_FLASHROM_DATA, (u_int)addr);
		break;
	    case XTYPE_X:
		val = xread(edt_p, (u_int)addr);
		break;
	    case XTYPE_LTX:
		val = bt_read(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx), (u_int)addr);
		break;
	}
	buf[i] = val;
	addr++;
	/* printf("%x ", val); */
	/* record start of string */
	if (!ret && val >= 0x20 && val <= 0x7e)
	    ret = &buf[i];
    }
    buf[i] = 0;
    return ret;
}


Edt_bdinfo *
edt_detect_boards(char *dev, int unit, int *nunits, int verbose)
{
    return(edt_detect_boards_id(dev, unit, 0xffff, nunits, verbose));
}

Edt_bdinfo *
edt_detect_boards_id(char *dev, int unit, u_int id, int *nunits, int verbose)
{
    int     board_count = 0;
    Edt_bdinfo buf[NUM_DEVICE_TYPES * MAX_BOARD_SEARCH];
    Edt_bdinfo *ret;
    int     all_ids = 0;
    int     d, i;
    EdtDev *temp;

    if (id == 0xffff)
    	all_ids = 1;

    /*
     * three possibilities: dev specified, unit specified, or neither
     */
    if (*dev != 0)
    {
	for (i = 0; i < MAX_BOARD_SEARCH; i++)
	{
	    temp = edt_open_quiet(dev, i);
	    if ((temp != NULL) && (all_ids || (id == temp->devid)))
	    {
		buf[board_count].bd_id = temp->devid;
		edt_close(temp);
		if (verbose)
		    printf("  Found a device %s:%d\n", dev, i);
		strcpy(buf[board_count].type, dev);
		buf[board_count].id = i;
		board_count++;
	    }
	}
    }
    else if (unit != -1)
    {
	for (d = 0; d < NUM_DEVICE_TYPES; d++)
	{
	    temp = edt_open_quiet(device_name[d], unit);
	    if ((temp != NULL) && (all_ids || (id == temp->devid)))
	    {
		buf[board_count].bd_id = temp->devid;
		edt_close(temp);
		if (verbose)
		    printf("  Found a device %s:%d\n", device_name[d], unit);
		strcpy(buf[board_count].type, device_name[d]);
		buf[board_count].id = unit;
		board_count++;
	    }
	}
    }
    else
    {
	for (d = 0; d < NUM_DEVICE_TYPES; d++)
	{
	    for (i = 0; i < MAX_BOARD_SEARCH; i++)
	    {
		temp = edt_open_quiet(device_name[d], i);
		if ((temp != NULL) && (all_ids || (id == temp->devid)))
		{
		    buf[board_count].bd_id = temp->devid;
		    edt_close(temp);
		    if (verbose)
			printf("  Found a device %s:%d\n", device_name[d], i);
		    strcpy(buf[board_count].type, device_name[d]);
		    buf[board_count].id = i;
		    board_count++;
		}

	    }
	}
    }
    buf[board_count].type[0] = 0;
    buf[board_count].id = -1;
    board_count++;
    ret = (Edt_bdinfo *)malloc(board_count * sizeof(Edt_bdinfo));
    for (i = 0; i < board_count; i++)
    {
	strcpy(ret[i].type, buf[i].type);
	ret[i].id = buf[i].id;
	ret[i].bd_id = buf[i].bd_id;
    }
    *nunits = board_count-1;
    return ret;
}

Edt_bdinfo *
edt_detect_boards_ltx(char *dev, int unit, int *nunits, int verbose)
{
    int     board_count = 0;
    Edt_bdinfo buf[NUM_DEVICE_TYPES * MAX_BOARD_SEARCH];
    Edt_bdinfo *ret;
    int     all_ids = 0;
    int     d, i;
    EdtDev *temp;

    /*
     * three possibilities: dev specified, unit specified, or neither
     */
    if (*dev != 0)
    {
	for (i = 0; i < MAX_BOARD_SEARCH; i++)
	{
	    temp = edt_open_quiet(dev, i);
	    if ((temp != NULL)
	      && ((temp->devid == PCDFOX_ID) || (temp->devid == PSS4_ID)))
	    {
		buf[board_count].bd_id = temp->devid;
		edt_close(temp);
		if (verbose)
		    printf("  Found a device %s:%d\n", dev, i);
		strcpy(buf[board_count].type, dev);
		buf[board_count].id = i;
		board_count++;
	    }
	}
    }
    else if (unit != -1)
    {
	for (d = 0; d < NUM_DEVICE_TYPES; d++)
	{
	    temp = edt_open_quiet(device_name[d], unit);
	    if ((temp != NULL)
	      && ((temp->devid == PCDFOX_ID) || (temp->devid == PSS4_ID)))
	    {
		buf[board_count].bd_id = temp->devid;
		edt_close(temp);
		if (verbose)
		    printf("  Found a device %s:%d\n", device_name[d], unit);
		strcpy(buf[board_count].type, device_name[d]);
		buf[board_count].id = unit;
		board_count++;
	    }
	}
    }
    else
    {
	for (d = 0; d < NUM_DEVICE_TYPES; d++)
	{
	    for (i = 0; i < MAX_BOARD_SEARCH; i++)
	    {
		temp = edt_open_quiet(device_name[d], i);
		if ((temp != NULL)
		  && ((temp->devid == PCDFOX_ID) || (temp->devid == PSS4_ID)))
		{
		    buf[board_count].bd_id = temp->devid;
		    edt_close(temp);
		    if (verbose)
			printf("  Found a device %s:%d\n", device_name[d], i);
		    strcpy(buf[board_count].type, device_name[d]);
		    buf[board_count].id = i;
		    board_count++;
		}

	    }
	}
    }
    buf[board_count].type[0] = 0;
    buf[board_count].id = -1;
    board_count++;
    ret = (Edt_bdinfo *)malloc(board_count * sizeof(Edt_bdinfo));
    for (i = 0; i < board_count; i++)
    {
	strcpy(ret[i].type, buf[i].type);
	ret[i].id = buf[i].id;
	ret[i].bd_id = buf[i].bd_id;
    }
    *nunits = board_count-1;
    return ret;
}

static void
remove_char(char *str, char ch)
{
    char    tmpstr[128];
    char   *p = tmpstr;
    char   *pp = str;

    strcpy(tmpstr, str);

    while (*p)
    {
	if (*p != ch)
	    *pp++ = *p;
	++p;
    }
    *pp = '\0';
}

static void
parse_id(char *idstr, u_int * date, u_int * time)
{
    int     ss;
    char    lcaname[128];
    char    id[32];
    char    datestr[32];
    char    timestr[32];

    ss = sscanf(idstr, "%s %s %s %s", lcaname, id, datestr, timestr);

    /*
     * DEBUG printf("read %d strings from <%s>:\n", ss, idstr);
     * printf("'%s'\n", lcaname); printf("'%s'\n", id); printf("'%s'\n",
     * datestr); printf("'%s'\n", timestr);
     */

    edt_fix_millennium(datestr, 90);
    remove_char(datestr, '/');
    *date = (u_int) atoi(datestr);
    remove_char(timestr, ':');
    *time = (u_int) atoi(timestr);
}

void
edt_x_byte_program(EdtDev *edt_p, u_int addr, u_char data, int xtype)
{
    switch(xtype)
    {
	case XTYPE_BT:
	    bt_byte_program(edt_p, EDT_FLASHROM_DATA, addr, data);
	    break;
	case XTYPE_X:
	    xbyte_program(edt_p, addr, data); 
	    break;
	case XTYPE_LTX:
	    bt_byte_program(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx), addr, data);
	    break;
    }
}

void
edt_x_reset(EdtDev *edt_p, int xtype)
{
    switch(xtype)
    {
	case XTYPE_BT:
	    bt_reset(edt_p, EDT_FLASHROM_DATA);
	    break;
	case XTYPE_X:
	    xreset(edt_p) ;
	    break;
	case XTYPE_LTX:
	    bt_reset(edt_p, EDT_FLASHROM_LTXHUB(edt_p->hubidx));
	    break;
    }
}

void
edt_get_esn(EdtDev *edt_p, char *esn)
{
    char dmy[128];
    edt_get_sns(edt_p, esn, dmy);
}

void
edt_get_osn(EdtDev *edt_p, char *osn)
{
    char dmy[128];
    edt_get_sns(edt_p, dmy, osn);
}

static int
isdigit_str(char *s)
{
    unsigned int i;

    for (i=0; i<strlen(s); i++)
	if (!isdigit(s[i]))
	    return 0;
    return 1;
}


int
edt_parse_esn(char *str, Edt_embinfo *ei)
{
    unsigned int i,j=0;
    int n;
    int nfields=0;
    char sn[128], pn[128], ifx[128], rev[128], clock[128], opt[128];
    char tmpstr[128];

    sn[0] = pn[0] = clock[0] = opt[0] = ifx[0] = '\0';

    for (i=0; i<strlen(str); i++)
    {
	tmpstr[j++] = str[i];
	if (str[i] == ':')
	{
	    ++nfields;
	    if (str[i+1] == ':') /* insert space if empty field */
	    	tmpstr[j++] = ' ';
	}
    }
    tmpstr[j] = '\0';

    if (nfields < 4)
	return -1;


    n = sscanf(tmpstr, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:", sn, pn, clock, opt, rev, ifx);

    /* first pass we had 10-digit p/ns; still may be some around that have that */
    if ((strlen(sn) > 10))
	sn[10] = '\0';

    if ((strlen(pn) > 10) || (strlen(opt) > 10) || !isdigit_str(clock))
	 return -1;

    if (n < 4)
	 return -1;

     if (n < 5)
	rev[0] = '\0';

     if (n < 6)
	ifx[0] = '\0';

    strcpy(ei->sn, sn);
    strcpy(ei->pn, pn);
    strcpy(ei->opt, opt);
    strcpy(ei->ifx, ifx);
    ei->clock = atoi(clock);
    ei->rev = atoi(rev);

    return 0;
}


void
edt_get_sns(EdtDev *edt_p, char *esn, char *osn)
{
    char dmy[128];
    int promcode;
    u_int frb;
    u_char stat;

    if (hubload)
	promcode = edt_ltx_prom_detect(edt_p, &stat, &frb, 0);
    else promcode = edt_x_prom_detect(edt_p, &stat, 0);

    switch (promcode)
    {
	case AMD_4013E:
	    edt_readinfo(edt_p, promcode, 0, dmy, esn, osn);
	    break;
	case AMD_4013XLA:
	    edt_readinfo(edt_p, promcode, 1, dmy, esn, osn);
	    break;
	case AMD_4028XLA:
	    edt_readinfo(edt_p, promcode, 2, dmy, esn, osn);
	    break;
	case AMD_XC2S150:
	    edt_readinfo(edt_p, promcode, 2, dmy, esn, osn);
	    break;
	case AMD_XC2S200_4M:
	    edt_readinfo(edt_p, promcode, 0, dmy, esn, osn);
	    break;
	case AMD_XC2S200_8M:
	case AMD_XC2S100_8M:
	case AMD_XC2S300E:
	    edt_readinfo(edt_p, promcode, 2, dmy, esn, osn);
	    break;
	case PROM_UNKN:
	default:
	    esn[0] = 0;
	    osn[0] = 0;
	    break;
    }
}

/*
 * given a date string in the format yy/mm/dd or yyy/mm/dd or yyyy/mm/dd,
 * correct for the millenium. Will only work for the next 100 years or
 * less, depending on rollover.
 * 
 * RETURNS: 0 on success, -1 if format error
 */
int
edt_fix_millennium(char *str, int rollover)
{
    char    tmpstr[16];
    int     century=20;
    int     yr;

    if (str[2] == '/')				/* yy/mm/dd */
    {
	yr = ((str[0] - '0') * 10) + (str[1] - '0');
	if (yr >= rollover)
	    century = 19;
    }
    else if (str[3] == '/')			/* yyy/mm/dd */
    {
	yr = ((str[1] - '0') * 10) + (str[2] - '0');
	sprintf(str, "%02d", yr);
    }
    else if (str[4] == '/')			/* yyyy/mm/dd */
	return 0;
    else return  -1;

    sprintf(tmpstr, "%d%s", century, str);
    strcpy(str, tmpstr);
    return 0;
}

/*
 * the following was ported from pdb to be used by pciload so's we wouldn't
 * have to power-down the system after loadning a xilinx. But before it ever
 * got into pciload, it was discovered that the reboot call will panic a
 * solaris system. So... do we solve that and put it in? Maybe someday...
 */
#if 0
static u_int
pr_cfg(EdtDev *edt_p, int addr)
{
    int ret ;
    edt_buf buf ;
    buf.desc = addr ;
    if ((addr<0) || (addr>0x3c)) {
	edt_msg(DEBUG1, "pr_cfg: addr out of range\n");
	return(0);
    } 
    ret = edt_ioctl(edt_p,EDTG_CONFIG,&buf) ;
    if (ret < 0)
    {
	edt_perror("EDTG_CONFIG") ;
    }
    return(buf.value) ;
}

static void
pw_cfg(EdtDev *edt_p, int addr, int value)
{
    int ret ;
    edt_buf buf ;
    buf.desc = addr ;
    if ((addr<0) || (addr>0x3c)) {
	edt_msg(DEBUG1, "pw_cfg: addr out of range\n");
	return;
    } 
    buf.value = value ;
    ret = edt_ioctl(edt_p,EDTS_CONFIG,&buf) ;
    if (ret < 0)
    {
	edt_perror("EDTS_CONFIG") ;
    }
}

int
edt_x_reboot(EdtDev *edt_p)
{

    int  addr, data;
    int  buf[0x40];
    int  old, copy, new;
    u_int dmy;
    FILE *fd2;
#ifdef _NT_
    char *tmpfile = ".\\tmp_pdb_reboot.cfg";
#else
    char *tmpfile = "tmp_pdb_reboot.cfg";
#endif

    if ((fd2 = fopen(tmpfile, "wb")) == NULL)
    {
	edt_msg(EDT_MSG_FATAL, "edt_x_reboot: Couldn't write to %s\n", tmpfile);
	return -1;
    }
    for	(addr=0; addr<=0x3c; addr+=4) {
	data  = pr_cfg(edt_p, addr);
	buf[addr] = data;
	putc(data, fd2);
	putc(data>>8, fd2);
	putc(data>>16, fd2);
	putc(data>>24, fd2);
	edt_msg(DEBUG1, "%02x:  %08x\n", addr, data);
    }
    fclose(fd2);
    edt_msg(DEBUG1, "Wrote config space state out to %d\n", tmpfile);

    edt_reg_write(edt_p, 0x01000085, 0x40) ;
    dmy = edt_reg_read(edt_p, 0x04000080) ; /*flush chipset write buffers*/

    edt_msleep(500) ;

    edt_msg(DEBUG1, "	 old	   copy	     new\n");
    for	(addr=0; addr<=0x3c; addr+=4) {
	old  = pr_cfg(edt_p, addr);
	copy  =  buf[addr];
	pw_cfg(edt_p, addr, copy) ;
	new  = pr_cfg(edt_p, addr);

	edt_msg(DEBUG1, "%02x:  %08x  %08x  %08x	 ", addr, old, copy, new);
	if	(copy != new)	edt_msg(EDT_MSG_FATAL, "ERROR\n");
	else if	(old  != new)	edt_msg(DEBUG1, "changed\n");
	else			edt_msg(DEBUG1, "\n");

    }
    return 0;
}

#endif


/* #pragma ident "@(#)edt_bitload.c	1.25 11/12/02 EDT" */

/*
 * edt_bitload.c -- functions to load the EDT PCI interface Xilinx .bit file
 * 
 * HISTORY
 *   1997 creation
 *   1998-1999 various mods, including incrementally add 4010, 4013, etc.
 *      capability
 *   2000 rewritten from standalone to library routine
 *   8/2001 rewrote xilinx header code to be smarter, now actually
 *       decodes header and finds data size and start of data reliably and
 *       independent of the xilinx size. Also rewrote to look at and get
 *       sizes of and create a list of files found, then sort by size and
 *       try in order smallest to largest, since can SMOKE a 600 part
 *       trying to load a larger file in it.
 * 
 * (C) 1997-2001 Engineering Design Team, Inc.
 */

#include "edtinc.h"
#include "edt_bitload.h"
#include <stdlib.h>

/* shorthand debug level */
#define DEBUG1 EDTLIB_MSG_INFO_1
#define DEBUG2 EDTLIB_MSG_INFO_2

typedef struct
{
    char    path[MAXPATH];
    int     size;
}       BFENTRY;

static int bitload_prog_xilinx(EdtDev * edt_p, FILE * xfd, char *header, int alt, int alt_param);
static int bitload_prog_xilinx_nofs(EdtDev * edt_p, u_char *xa, char *header);
static int bitload_rc_prog_xilinx(EdtDev * edt_p, FILE * xfd, char *header);
static int 
bitload_get_subdir_files(char *basedir, char *devdir, char *fname,
			 BFENTRY bitfiles[], int *nbfiles);
static int bitload_sort_bitfile_list(EdtDev *edt_p, BFENTRY bitfiles[], int nbfiles);
static void rc_prog_write(EdtDev * edt_p, u_char val);
static int valid_bit_dirname(char *name);
static int has_slashes(char *name);


#ifdef NO_FS
#include "nofs_bitfiles.h"
#endif

u_char *Prombuf;


#ifdef PCD

int
edt_program_ocm(EdtDev *edt_p, u_char *buf, int size, int channel)

{
	int loop;
	int inner_loop;
	int base;
	int xfer;
	int regrd;
	int count;
	int control;
	int release;
	int done = 0;
	int bad_program = 0;

	/* drive init and prog_l low */
	/* select channel for data */
	control = edt_reg_read(edt_p, OCM_X_CONT);
	if (channel == 1) 
	{
		control = control & ~(OCM_CONT_CH1_INIT | OCM_CONT_CH1_PROG );
		control |= OCM_CONT_CH1_PROG;
		release = control & ~(OCM_CONT_CH1_INIT | OCM_CONT_CH1_PROG);
		release |= (OCM_CONT_EN_FIFO | OCM_CONT_ENABLE | OCM_CONT_CH1_INIT | OCM_CONT_PRG_CH1);
	}
	else
	{
		control = control & ~(OCM_CONT_CH0_INIT | OCM_CONT_CH0_PROG | OCM_CONT_PRG_CH1);
		control |= OCM_CONT_CH0_PROG;
		release = control & ~(OCM_CONT_CH0_INIT | OCM_CONT_CH0_PROG);
		release |= (OCM_CONT_EN_FIFO | OCM_CONT_ENABLE | OCM_CONT_CH0_INIT);
	}
	edt_reg_write(edt_p, OCM_X_CONT, control);

    /* wait for done to be low */
    loop = 0;
    edt_msg(DEBUG2, "\nwaiting for DONE low\n");
    while (((regrd = edt_reg_read(edt_p, OCM_X_STAT)) & (OCM_STAT_CH0_DONE << (channel * 2))) != 0)
    {
		if (loop++ > 10000000)
		{
			/* edt_msg(DEBUG2, "reading %02x waiting for not DONE %02x\n",
					   regrd, OCM_STAT_CH0_DONE<< (channel *2)); */
			return (1);
		}
    }
    edt_msg(DEBUG2, "loop %d\n", loop);

    /* enbale serializer fifo, set prog_l high and release INIT */
	edt_reg_write(edt_p, OCM_X_CONT, release);

    /* wait for INIT to go high */
    edt_msg(DEBUG2, "waiting for INIT high\n");
    loop = 0;
    while (((regrd = edt_reg_read(edt_p, OCM_X_STAT)) & (OCM_STAT_CH0_INIT << (channel * 2))) == 0)
    {
		/* edt_msg(DEBUG2, "reading %02x waiting for not INIT %02x\n", regrd, OCM_STAT_CH0_INIT<< (channel *2)); */
		if (loop++ > 1000000)
		{
			/* edt_msg(DEBUG2, "reading %02x waiting for not INIT %02x\n", regrd, OCM_STAT_CH0_INIT<< (channel *2)); */
			return (1);
		}
    }
    edt_msg(DEBUG2, "loop %d stat %02x\n", loop, regrd);

	
	/* load the data */
	/* the channel has been selected above */
    bad_program = 0;
    base = 0;
    while ((base < size) && !bad_program && !done)
    {
    		regrd = edt_reg_read(edt_p, OCM_X_STAT);
    		if ((regrd & (OCM_STAT_CH0_DONE << (channel * 2))) != 0)
		{
			done = 1;
		}
    		else if ((regrd & (OCM_STAT_CH0_INIT << (channel * 2))) == 0)
		{
			bad_program = 1;
		}
		/* fifo holds 15 bytes amount to transfer is then 15 - the current count */
		count = (edt_reg_read(edt_p, OCM_X_STAT) & OCM_FCNT_MSK) >> OCM_FCNT_SHFT;
		xfer = 15 - count;

		for(inner_loop = 0; inner_loop < xfer; inner_loop++)
		{
			edt_reg_write(edt_p, OCM_X_DATA, buf[base+inner_loop]);
		}
		base += xfer;
    } 

    edt_msg(DEBUG2, "waiting for DONE high\n");

    loop = 0;

	/* output 1s until DONE is high or count exceeds reasonable amount */
    edt_msg(DEBUG2, "OCM Status %02x DONE %02x\n", edt_reg_read(edt_p, OCM_X_STAT),
						(OCM_STAT_CH0_DONE << (channel * 2)));

    while ((((regrd = edt_reg_read(edt_p, OCM_X_STAT)) & (OCM_STAT_CH0_DONE << (channel * 2))) == 0) &&
		!bad_program )
    {
		/* wait for data fifo to be empty */
		inner_loop = 0;
		while (((regrd = edt_reg_read(edt_p, OCM_X_STAT)) & OCM_FCNT_MSK) != 0)
		{
			if (inner_loop++ > 1000000)
			{
				edt_msg(DEBUG2, "timeout wait for fifo zero state register %02x\n", regrd);
				return (1);
			}
		}
		/* CLKSAFTER must be greater than 1 so we will output at least on batch of ones */
		if (loop > CLKSAFTER)
		{
			edt_msg(DEBUG2, "> %d writes without done - fail\n", CLKSAFTER);
			bad_program = 1;
		}
		/* output 15 bytes of ones if done not high yet */
		for(inner_loop = 0; inner_loop < 15; inner_loop++)
		{
			edt_reg_write(edt_p, OCM_X_DATA, 0xff);
		}
		loop++;
    }
    edt_msg(DEBUG2, "loop %d\n", loop);

    return (bad_program);
}
#endif 

int
bitload_read_header(char *bitpath, char *header, int *size);

/*
 * edt_bitload
 * 
 * DESCRIPTION searches for and loads a Xilinx gate array bit file into an EDT
 * PCI board.  Searches under <basedir>/bitfiles/xxx, or if a PCI DV, in the
 * appropriate sub-directory (<basedir>/bitfiles/dv/.../<file>.bit or
 * <basedir>/bitfiles/dvk/.../<file>.bit. '...' stands for all the subdirs
 * found under the base path.  Quits after the first successful load.
 * 
 * ARGUMENTS edt_p:	device handle returned from edt_open
 *           basedir:	base directory to start looking for the file
 *           fname:     name of the file to load
 *           flags:	misc flag bits -- see edt_bitload.h for #defines
 *                      (this var was formerly rcam which is obsolete)
 *           skip:	don't actually load, just find the files (debugging)
 */
int
edt_bitload(EdtDev *edt_p, char *basedir, char *fname, int flags, int skip)
{
    int     i, size;
    int     nofs = 0, ovr=0;
	int		alt_param = ALT_INTERFACE, prg_alt = 0;
    int     fullpath;
    int     nbfiles = 0;
    char    devdir[MAXPATH];
    char    header[256];
    BFENTRY bitfiles[64];
#ifdef NO_FS
    u_char *ba;
#endif

    if (flags & BITLOAD_FLAGS_NOFS)
	nofs = 1;
    if (flags & BITLOAD_FLAGS_OVR)
	ovr = 1;
#ifdef PCD
    if (flags & BITLOAD_FLAGS_OCM)
	prg_alt = ALT_OCM;
#endif
    if (flags & BITLOAD_FLAGS_CH1)
	alt_param = 1;

#ifdef NO_FS
    /* special case nofs  -- fname isn't really fname but instead pointer to data */
    if (nofs)
    {
	char nofsname[MAXPATH];

	/* ALERT: to be really complete, we'd detect (?) and prepend appropriate
	 * xilinx here, but should work to punt and just hardcode to 4013
	 * unless/until used with nofs pmc card that's other than 4013
	 */
	sprintf(nofsname, "4013_%s", fname);

	if (nofs) /* strip off .bit from name if nofs */
	{
	    int len = strlen(nofsname);
	    if ((len >= 4) && (strcasecmp(&nofsname[len-4], ".bit") == 0))
		nofsname[len-4] = '\0';
	}

	MAPBITARRAY(nofsname, ba);

	if (ba == NULL)
	{
	    printf("no match for nofs header array '%s' (bitload.c & nofs_bitfiles.h)\n", fname);
	    exit(1);
	}

	edt_msg(DEBUG2, "edt_bitload(basedir=%s array=%x nofs=%d skip=%d)\n",
	    basedir, fname, nofs, skip);
	return bitload_prog_xilinx_nofs(edt_p, ba, header);
    }
#endif

    edt_msg(DEBUG2, "edt_bitload(basedir=%s fname=%s nofs=%d skip=%d)\n",
	    basedir, fname, nofs, skip);

    edt_bitload_devid_to_bitdir(edt_p, devdir);

    fullpath = has_slashes(fname);

    if (fullpath)
    {
	if (!edt_access(fname, 0) == 0)
	{
	    edt_msg_perror(DEBUG2, fname);
	    return -1;
	}

	bitload_read_header(fname, header, &size);

	if (edt_bitload_loadit(edt_p, fname, size, skip, ovr, prg_alt, alt_param, 1) == 0)
	    return 0;
    }
    else
    {
	char    tmppath[MAXPATH];
	char    header[256];
	int     size;

	/* dir/file */

	if (strcmp(basedir,"."))
	{
		sprintf(tmppath, "./%s", fname);
		if ((edt_access(tmppath, 0) == 0)
		    && (bitload_read_header(tmppath, header, &size) == 0))
		{
		    bitfiles[nbfiles].size = size;
		    strcpy(bitfiles[nbfiles++].path, tmppath);
		}

	}

	sprintf(tmppath, "%s/%s", basedir, fname);
	if ((edt_access(tmppath, 0) == 0)
	    && (bitload_read_header(tmppath, header, &size) == 0))
	{
	    bitfiles[nbfiles].size = size;
	    strcpy(bitfiles[nbfiles++].path, tmppath);
	}

	/* dir[/devdir]/subdirs.../file */
	bitload_get_subdir_files(basedir, devdir, fname, bitfiles, &nbfiles);

	/* dir[/devdir]/bitfiles/subdirs.../file */
	sprintf(tmppath, "%s/bitfiles", basedir, fname);
	bitload_get_subdir_files(tmppath, devdir, fname, bitfiles, &nbfiles);

	bitload_sort_bitfile_list(edt_p, bitfiles, nbfiles);

	for (i = 0; i < nbfiles; i++)
	{
	    if (edt_bitload_loadit(edt_p, bitfiles[i].path, bitfiles[i].size, skip, ovr, prg_alt, alt_param, i+1) == 0)
		return 0;
	}
    }

    return -1;
}



/*
 * get files and sizes from all subdirs in a given dir
 * 
 * return 0 on success, -1 on failure
 */
int
bitload_get_subdir_files(char *basedir, char *devdir, char *fname,
			 BFENTRY bitfiles[], int *nbfiles)
{
    HANDLE  dirp = (HANDLE) 0;

    int     loaded = 0;
    char    d_name[MAXPATH];
    char    header[256];
    char    dirpath[MAXPATH];
    int     fcount = 0;

    sprintf(dirpath, "%s%s%s", basedir, (devdir && (*devdir)) ? "/" : "", devdir);

    if ((dirp = edt_opendir(dirpath)) == (HANDLE)0)
    {
	edt_msg_perror(DEBUG2, dirpath);
	edt_msg(DEBUG2, "could not open directory <%s>\n", dirpath);
	return -1;
    }

    while (edt_readdir(dirp, d_name))
    {
	/* look for numeric dir names, at least 4 chars long */
	if (valid_bit_dirname(d_name))
	{
	    static char    tmppath[MAXPATH];
	    static char    ztmppath[MAXPATH];
	    static char    cmd[MAXPATH];
	    int     size;
	    int     found_compressed = 0 ;
	    int     found_bitfile = 0 ;

	    /* ALERT: check if d_name is directory here! */
	    /* See stat (2) and stat (5) */
	    sprintf(tmppath, "%s/%s/%s", dirpath, d_name, fname);
	    edt_correct_slashes(tmppath);
	    if (edt_access(tmppath, 0) == 0)
		found_bitfile = 1 ;
	    else /* Check for compressed version of bitfile (.Z or .gz) */
	    {
		strcpy(ztmppath, tmppath);
		strcat(ztmppath, ".Z");
		if (edt_access(ztmppath, 0) == 0)
		{
		    sprintf(cmd, "uncompress %s", tmppath) ;
		    edt_msg(DEBUG1, "%s\n", cmd);
		    edt_system(cmd) ;
		    found_compressed = 1 ;
		}
		if (!found_compressed)
		{
		    strcpy(ztmppath, tmppath);
		    strcat(ztmppath, ".gz");
		    if (edt_access(ztmppath, 0) == 0)
		    {
			sprintf(cmd, "gunzip %s", tmppath) ;
			edt_msg(DEBUG1, "%s\n", cmd);
			edt_system(cmd) ;
			found_compressed = 1 ;
		    }
		}
		if (!found_compressed)
		{
		    strcpy(ztmppath, tmppath);
		    strcat(ztmppath, ".zip");
		    if (edt_access(ztmppath, 0) == 0)
		    {
			sprintf(cmd,"cd %s/%s ; unzip %s",
				  dirpath, d_name, fname);
			edt_correct_slashes(tmppath);
			edt_msg(DEBUG1, "%s\n", cmd);
			edt_system(cmd) ;
			printf("rm %s\n", ztmppath) ;
			unlink(ztmppath) ;
			found_compressed = 1 ;
		    }
		}
		if (found_compressed && edt_access(tmppath, 0) == 0)
		    found_bitfile = 1 ;
	    }
	    if (found_bitfile && bitload_read_header(tmppath,header,&size) == 0)
	    {
		bitfiles[*nbfiles].size = size;
		strcpy(bitfiles[*nbfiles].path, tmppath);
		++*nbfiles;
	    }
	}
    }

    edt_closedir(dirp);

	return 0;
}


int
bitload_read_header(char *bitpath, char *header, int *size)
{
    FILE   *xfd;
    int     ret;

    if ((xfd = fopen(bitpath, "rb")) == NULL)
    {
	edt_msg_perror(DEBUG2, bitpath);
	return -1;
    }
    ret = edt_get_x_header(xfd, header, size);
    fclose(xfd);
    return ret;
}



/*
 * extract xilinx directory token (2nd to last in path) from path string
 * Return 0 on failure, 1 on success (with xilinx dir in arg)
 */
bitload_xilinx_from_path(char *path, char *xilinx)
{
    char tmppath[MAXPATH];
    char *sp, *ep;

    xilinx[0] = '\0';
    strcpy(tmppath, path);
    edt_back_to_fwd(tmppath);
    if ((ep = strrchr(tmppath, '/')) == NULL)
	return 0;
    sp = ep-1;
    *ep = '\0';
    while (sp > tmppath)
    {
	if (*sp == '/')
	{
	    strcpy(xilinx, sp+1);
	    break;
	}
	else --sp;
    }
    if (*xilinx == '\0')
	return 0;
    return 1;
}


/*
 * bubble sort by size, in ascending order
 * if edt_parts.xpn xref file is present, read it and put
 * matching xilinx at the top of the sorted list
 */
int
bitload_sort_bitfile_list(EdtDev *edt_p, BFENTRY bitfiles[], int nbfiles)
{
    int     i, sorted = 0;
    int     tmpsize = 0;
    char    esn[128];
    char    bd_xilinx[32];
    Edt_embinfo ei;
    BFENTRY bftmp;

    if (!nbfiles)
    {
	edt_msg(DEBUG2, "bitfile not found -- check pathname, or specify a different basedir (see -d)\n");
	return 0;
    }

#if 0
    edt_msg(DEBUG2, "\nbitfile list before sorting:\n");
    for (i = 0; i < nbfiles; i++)
	edt_msg(DEBUG2, "%s %d\n", bitfiles[i].path, bitfiles[i].size);
    edt_msg(DEBUG2, "\n");
#endif

    edt_get_esn(edt_p, esn);
    if (edt_parse_esn(esn, &ei) == 0)
    {
	/* if xxx-xxxxx-00, replace last 2 digits with rev */
	if ((strcmp(&(ei.pn[8]), "00") == 0))
	    sprintf(&(ei.pn[8]), "%02d", ei.rev);
	if (*ei.ifx)
	    strcpy(bd_xilinx, ei.ifx);
	else if (edt_find_xpn(ei.pn, bd_xilinx))
	    edt_msg(DEBUG2, "matching xilinx found: pn <%s> xilinx <%s>\n", ei.pn, bd_xilinx);
	else edt_msg(DEBUG2, "no matching xilinx found: for pn <%s>\n", ei.pn);
    }
    else edt_msg(DEBUG2, "missing or invalid pn/xilinx, can't xref (esn <%s>)\n", esn);

    /*
     * if xref match exists, fake out the sort to make sure this one
     * ends up on top
     */
    if (bd_xilinx[0])
    {
	for (i=0; i<nbfiles; i++)
	{
	    char bf_xilinx[MAXPATH];
	    if (bitload_xilinx_from_path(bitfiles[i].path, bf_xilinx))
	    {
		if (strcasecmp(bf_xilinx, bd_xilinx) == 0)
		{
		    tmpsize = bitfiles[i].size;
		    bitfiles[i].size = 1;
		    break;
		}
	    }
	}
    }

    while (!sorted)
    {
	sorted = 1;
	for (i = 0; i < nbfiles - 1; i++)
	{
	    if (bitfiles[i].size > bitfiles[i + 1].size)
	    {
		sorted = 0;
		bftmp.size = bitfiles[i].size;
		strcpy(bftmp.path, bitfiles[i].path);

		bitfiles[i].size = bitfiles[i + 1].size;
		strcpy(bitfiles[i].path, bitfiles[i + 1].path);

		bitfiles[i + 1].size = bftmp.size;
		strcpy(bitfiles[i + 1].path, bftmp.path);
	    }
	}
    }

    /* if xref match, reset entry to correct size */
    if (tmpsize > 0)
	bitfiles[0].size = tmpsize;

    edt_msg(DEBUG2, "bitfile list after sorting:\n");
    for (i = 0; i < nbfiles; i++)
	edt_msg(DEBUG2, "%s %d\n", bitfiles[i].path, bitfiles[i].size);
    edt_msg(DEBUG2, "\n");

    return 0;
}

/*
 * edt_bitload_loadit
 * 
 * ARGUMENTS
 *     edt_p	same as it ever was
 *     bitpath  pathname to bitfile
 *     size     size of bitfile
 *     skip     skip load; test only
 *     ov       override GS/SS size test (mostly for eng/debug)
 * return 0 on success, -1 on failure
 */
int
edt_bitload_loadit(EdtDev * edt_p, char *bitpath, int size, int skip, int ov, int alt, int alt_param, int try)
{
    FILE   *xfd;		/* file for xilinx test load */
    int     ret = -1;
    char    header[128];

    if (skip)
	edt_msg(DEBUG2, "would try <%s>...\n", bitpath);
    else
	edt_msg(DEBUG2, "trying <%s>...\n", bitpath);

    if (!ov) /* or = override_gs/ss size test check */
    {
	/* check SS and GS min and min/max size respectively */
	if ((edt_p->devid == PGS4_ID) || (edt_p->devid == PGS16_ID))
	{
	    if (size < 2370000) /* approx. min xcvp250 size */
	    {
		edt_msg(DEBUG2, "size %d, out of range for GS; skipping (-o to override)\n", size);
		return -1;
	    }
	}


	if ((edt_p->devid == PSS4_ID) || (edt_p->devid == PSS16_ID))
	{
	    if ((size < 494000) || (size > 1280000))
	    {
		edt_msg(DEBUG2, "size %d out of range for SS; skipping (-o to override)\n", size);
		return -1;	/* approx. min/max xcv600e, xcv1000E, xcv2000e size */
	    }
	}
    }


    if ((xfd = fopen(bitpath, "rb")) == NULL)
    {
	edt_msg_perror(DEBUG2, bitpath);
	edt_msg(DEBUG2, "could not open file <%s>\n", bitpath);
    }
    else
    {
	if (!skip)
	{
	    if (bitload_prog_xilinx(edt_p, xfd, header, alt, alt_param) == 0)
	    {
		edt_msg(DEBUG1, "file <%s>\n", bitpath);
		edt_msg(DEBUG1, "id: \"%s\" loaded #%d\n", header, try);
		edt_set_bitpath(edt_p, bitpath) ;
		ret = 0;
	    }
	    else
	    {
		edt_msg(DEBUG2, "file <%s>\n", bitpath);
		edt_msg(DEBUG2, "id: \"%s\" -- file/device mismatch\n", header);
		edt_set_bitpath(edt_p, "") ;
	    }
	}
	fclose(xfd);
    }
    return ret;
}


/*
 * apparently this isn't used anymore? --doug static u_char flipbits(u_char
 * val) { int     i; u_char  ret = 0;
 * 
 * for (i = 0; i < 8; i++) { if (val & (1 << i)) ret |= 0x80 >> i; } return
 * (ret); }
 */

#define X_DATA		0x010000
#define X_CCLK		0x020000
#define X_PROG		0x040000
#define X_INIT		0x080000
#define X_DONE		0x100000
#define X_INITSTAT	0x200000


/*
 * program the xilinx part
 */
static int
bitload_prog_xilinx(EdtDev * edt_p, FILE * xfile, char *header, int alt, int alt_param)
{
    int     size, got;

    if (edt_get_x_header(xfile, header, &size) != 0)
	return -1;

    if (Prombuf)		/* already read/initialized */
    {
	free(Prombuf);
	Prombuf = NULL;
    }

    if ((Prombuf = (u_char *) malloc(size)) == NULL)
    {
	edt_msg(EDT_MSG_FATAL, "Error allocating memory for bitfile!\n");
	return -1;
    }

    if ((got = fread(Prombuf, 1, size, xfile)) < size)
    {
	edt_msg(EDT_MSG_FATAL, "Error -- xilinx file truncated!\n");
	return -1;
    }

	if (alt == ALT_INTERFACE)
		return (edt_program_xilinx(edt_p, Prombuf, size));
#ifdef PCD
	else if  (alt == ALT_OCM)
		return (edt_program_ocm(edt_p, Prombuf, size, alt_param));
#endif
	else
	{
	edt_msg(EDT_MSG_FATAL, "Error -- unknown alternate programming!\n");
	return -1;
	}
}

/*
 * program the xilinx part (nofs)
 */
static int
bitload_prog_xilinx_nofs(EdtDev * edt_p, u_char *xa, char *header)
{
    int     size;
    u_char *start_of_data;

    if ((start_of_data = edt_get_x_array_header(xa, header, &size)) == NULL)
	return -1;

    return (edt_program_xilinx(edt_p, start_of_data, size));
}

/*
 * program the remote camera through mode code bits in the PDV_MODE_CNTL
 * register
 */
#define RC_MASK		PDV_AIA_MC_MASK
#define	RC_X_DATA	PDV_AIA_MC2
#define RC_X_CCLK	PDV_AIA_MC1
#define RC_X_PROG	PDV_AIA_MC3
/*
 * program status readback is through the AIA CHAN bits of the status
 * register PDV_STAT
 */
#define RC_X_INITSTAT	PDV_CHAN_ID1
#define RC_X_DONE	PDV_CHAN_ID0

static void
rc_prog_write(EdtDev * edt_p, u_char val)
{
    u_char  regrd;

    regrd = edt_reg_read(edt_p, PDV_MODE_CNTL);
    regrd = (regrd & ~RC_MASK) | (val & RC_MASK);
    edt_reg_write(edt_p, PDV_MODE_CNTL, regrd);
}

static  u_char
rc_prog_read(EdtDev * edt_p)
{
    return (edt_reg_read(edt_p, PDV_STAT));
}

/*
 * program the xilinx part -- Remote Camera version
 */
static int
bitload_rc_prog_xilinx(EdtDev * edt_p, FILE * xfile, char *header)
{
    int     loop;
    int     ret;
    u_char  c;
    u_char  bit;
    u_char  regrd;
    u_char *tmpptr;
    u_char *endptr;
    int     got, size;
    int     cnt;
    int     i;

    if (edt_get_x_header(xfile, header, &size) != 0)
	return 1;

    if (Prombuf)		/* already read/initialized */
    {
	free(Prombuf);
	Prombuf = NULL;
    }

    if ((Prombuf = (u_char *) malloc(size)) == NULL)
    {
	edt_msg(EDT_MSG_FATAL, "Error allocating memory for bitfile!\n");
	return -1;
    }

    if ((got = fread(Prombuf, 1, size, xfile)) < size)
    {
	edt_msg(EDT_MSG_FATAL, "Error -- xilinx file truncated!\n");
	return -1;
    }

    /* set PROG  DIN CCLK all low */
    rc_prog_write(edt_p, 0);

    /* wait for done to be low */
    loop = 0;
    /* edt_msg(DEBUG2, "\nwaiting for DONE low\n"); */
    while ((regrd = rc_prog_read(edt_p)) & RC_X_DONE)
    {
	if (loop++ > 10000000)
	{
	    /*
	     * edt_msg(DEBUG2, "reading %02x waiting for not DONE %02x\n",
	     * regrd, RC_X_DONE);
	     */
	    return (1);
	}
    }
    edt_msg(DEBUG2, "loop %d\n", loop);
    /* set PROG high */
    rc_prog_write(edt_p, RC_X_PROG);

    /* wait for INITSTAT to go high */
    edt_msg(DEBUG2, "waiting for INIT high\n");
    loop = 0;
    while (((regrd = rc_prog_read(edt_p)) & RC_X_INITSTAT) == 0)
    {
	/*
	 * edt_msg(DEBUG2, "reading %02x waiting for not INIT %08x\n", regrd,
	 * RC_X_INITSTAT);
	 */
	if (loop++ > 1000000)
	{
	    /*
	     * edt_msg(DEBUG2, "reading %02x waiting for not INIT %08x\n",
	     * regrd, RC_X_INITSTAT);
	     */
	    return (1);
	}
    }
    edt_msg(DEBUG2, "loop %d\n", loop);

    tmpptr = Prombuf;
    endptr = &Prombuf[size];
    /* write all data bits */
    ret = 0;
    cnt = 0;
    while (tmpptr < endptr && (ret == 0))
    {
	c = *tmpptr++;
	for (i = 0; i < 8; i++)
	{
	    cnt++;
	    bit = c & 0x80;
	    c <<= 1;
	    if (bit)
	    {
		rc_prog_write(edt_p, RC_X_DATA | RC_X_CCLK | RC_X_PROG);
		rc_prog_write(edt_p, RC_X_DATA | RC_X_PROG);
	    }
	    else
	    {
		rc_prog_write(edt_p, RC_X_CCLK | RC_X_PROG);
		rc_prog_write(edt_p, RC_X_PROG);
	    }
	}
	if ((cnt % 0x100) == 0)
	    edt_msg(DEBUG2, "%08x\r", cnt);
    }
    /* write 1's until DONE is high */

    edt_msg(DEBUG2, "waiting for DONE high\n");
    loop = 0;
    ret = 0;
    edt_msg(DEBUG2, "prog_read %x DONE %x\n", rc_prog_read(edt_p), RC_X_DONE);
    while ((((regrd = rc_prog_read(edt_p)) & RC_X_DONE) == 0) && (ret == 0))
    {
	rc_prog_write(edt_p, RC_X_DATA | RC_X_CCLK | RC_X_PROG);
	rc_prog_write(edt_p, RC_X_DATA | RC_X_PROG);
	if (loop > CLKSAFTER)
	{
	    edt_msg(DEBUG2, "> %d writes without done - fail\n", CLKSAFTER);
	    ret = -1;
	}
	loop++;
    }

    return (ret);
}


/*
 * look for valid bitfile dir name -- should 4 or more chars and start with a
 * number from 4-9, or 'X'
 */
static int
valid_bit_dirname(char *name)
{
    if ((strlen(name) < 4)
	|| (((name[0] != 'X') && (name[0] != 'x'))
	    && ((name[0] < '4') || (name[0]) > '9')))
	return 0;
    return 1;
}


int
has_slashes(char *name)
{
    char   *p = name;

    while (*p)
    {
	if ((*p == '/') || (*p == '\\'))
	    return 1;
	++p;
    }
    return 0;
}

/*
 * fill in the devdir string based on the device ID
 */
void
edt_bitload_devid_to_bitdir(EdtDev * edt_p, char *devdir)
{
    switch(edt_p->devid)
    {
	case PDV44_ID:
	case PDVK_ID:
	    strcpy(devdir, "dvk");
	    break;

	case PDV_ID:
	case PGP_RGB_ID:
	    strcpy(devdir, "dv");
	    break;

	case PDVA_ID:
	case PDVA16_ID:
	    strcpy(devdir, "dva");
	    break;

	case PDVFOX_ID:
	    strcpy(devdir, "dvfox");
	    break;

	case PDVFCI_AIAG_ID:
	case PDVFCI_USPS_ID:
	    strcpy(devdir, "dvfci");
	    break;

	case PDVAERO_ID:
	    strcpy(devdir, "dvaero");
	    break;

	default:
	    devdir[0] = '\0';
    }
}


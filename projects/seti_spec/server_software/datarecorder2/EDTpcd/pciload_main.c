/* #pragma ident "@(#)pciload_main.c	1.48 01/30/02 EDT" */

/*
 * pciload_main.c
 *
 * main module for the EDT pciload program, the PCI Interface
 * FPGA code loader for EDT boards
 *
 * Copyright (C) 1997-2002 EDT, Inc.
 */

#include "edtinc.h"
#include "pciload.h"
#include <stdlib.h>
#include <ctype.h>

char   *argv0;
char    dev[256];
char	flash_dir[128];
int     force_fast = 0;
int     unit = -1;
int     sect;
int     proginfo = 0;
u_char  outbuf[MAX_BIT_SIZE];
Edt_embinfo embinfo;

int read_esn_file(u_int devid, char *esn);
int save_esn_file(u_int devid, char *esn, int vb);
void increment_sn(char *sn);

#ifdef NO_FS
#include "nofs_flash.h"
#endif


/* DEBUG */ int Option_hack = 0;

#ifdef NO_MAIN
#define exit(x) return(x)
#endif
/*
 * xilinx_dirnames -- indexes must match corresponding #defines
 */

char *xilinx_dirname[] = 
{
    ".",			/* PROM_UNKN      */
    "4013e",			/* AMD_4013E      */
    "4013xla",			/* AMD_4013XLA    */
    "4028xla",			/* AMD_4028XLA    */
    "xc2s150",			/* AMD_XC2S150    */
    "xc2s200",			/* AMD_XC2S200_4M */
    "xc2s200",			/* AMD_XC2S200_8M */
    "xc2s100",          /* AMD_XC2S100_8M */
    ".",
    ".",
    ".",
    ".",
    ".",
    ".",
    ".",
    ".",
    NULL
};

#define MAX_PROMCODE 7 /* MUST be same as max valid idx in xilinx_dirname! */

char *xilinx_promstr[] =
{
    "unknown [0]",
    "4013e PCI FPGA, AMD 29F010 FPROM",
    "4013xla PCI FPGA, AMD 29F010 FPROM",
    "4028xla PCI FPGA, AMD 29F010 FPROM",
    "XC2S150 PCI FPGA, AMD 29LV040B FPROM",
    "XC2S200 PCI FPGA, AMD 29LV040B 4MB FPROM",
    "XC2S200 PCI FPGA, AMD 29LV081B 8MB FPROM",
    "XC2S100 PCI FPGA, AMD 29LV081B 8MB FPROM",
    "unknown [8]",
    "unknown [9]",
    "unknown [10]",
    "unknown [11]",
    "unknown [12]",
    "unknown [13]",
    "unknown [14]",
    "unknown [15]",
    NULL
};

static void
strip_newline(char *s)
{
    int len = strlen(s);
    if (len && ((s[len-1] == '\n') || (s[len-1] == '\r')))
	s[len-1] = '\0';
}
static void
toupper_str(char *s)
{
    unsigned int i;

    for (i=0; i<strlen(s); i++)
	s[i] = toupper(s[i]);
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

char FnameRet[256];

char   *
fix_fname(char *fname, int promcode, int is_5_volt, int nofs, int vb)
{
    int     i;
    char    buf[256];
    char    buf2[256];
    char    extbuf[100];
    char   *promdir;

    /* if the file is simple, adjust it for 5 or 3.3 volt */
    char   *end;
    char   *ext = 0;
    int     haspath = 0;
    FILE   *f;

    if (strcmp(fname, "ERASE") == 0)
    {
	strcpy(FnameRet, fname);
	return FnameRet;
    }

    strcpy(buf, fname);
    promdir = xilinx_dirname[promcode];

    for (i = 0; buf[i] != 0; i++)
    {
	if (buf[i] == DIRECTORY_CHAR)
	    haspath = 1;
	else if (buf[i] == '.' && i != 0)
	    ext = &buf[i];
    }
    end = &buf[i];

    if (ext)
    {
	strcpy(extbuf, ext);
	*ext = 0;
	end = ext;
    }
    else
    {
	strcpy(extbuf, ".bit");
    }

    if (is_5_volt != -1)
    {
	if (strcmp(end - 3, "_3v") != 0 &&
	    strcmp(end - 3, "_5v") != 0)
	{
	    end[0] = '_';
	    end[1] = (is_5_volt == 1) ? '5' : '3';
	    end[2] = 'v';
	    end[3] = 0;
	}
	else
	{
	    if ((end[-2] == '3' && is_5_volt == 1) ||
		(end[-2] == '5' && is_5_volt == 0))
	    {
		printf("file contains a '_%cv' extension, while trying to program %svolt sect.\n",
		       end[-2], is_5_volt ? "5" : "3.3");
		printf("exiting...\n");
		exit(1);
	    }
	}
	/* printf("after voltage add, fname = '%s%s'\n", buf, extbuf); */
    }

    sprintf(buf2, "%s%s", buf, extbuf);

    if (nofs)
	sprintf(FnameRet, "%s/%s", promdir, buf, extbuf);
    else
    {
	if ((f = fopen(buf2, "r")) == NULL)
	{
	    if (!haspath)
	    {
		sprintf(buf2, "%s%c%s%c%s%s", flash_dir, DIRECTORY_CHAR, promdir, DIRECTORY_CHAR, buf, extbuf);
		if ((f = fopen(buf2, "r")) == NULL)
		{
		    sprintf(buf2, "%s%c%s%s", flash_dir, DIRECTORY_CHAR, buf, extbuf);
		    if ((f = fopen(buf2, "r")) == NULL)
		    {
			printf("Couldn't find the file in any of these locations:\n");
			printf("%s%s\n", buf, extbuf);
			printf("%s%c%s%c%s%s\n", flash_dir, DIRECTORY_CHAR, promdir, DIRECTORY_CHAR, buf, extbuf);
			printf("%s%c%s%s\n", flash_dir, DIRECTORY_CHAR, buf, extbuf);
			exit(1);
		    }
		}
	    }
	    else
	    {
		perror(buf2);
		exit(1);
	    }
	}
	fclose(f);

	strcpy(FnameRet, buf2);
    }

    if (vb)
	printf("Using bitfile %s\n", FnameRet);
    return FnameRet;
}


int
default_segment(promcode)
{
    switch (promcode)
    {
	case AMD_4013E:
	case AMD_XC2S200_4M:
	    return 0;

	case AMD_4013XLA:
	    return 1;

	case AMD_4028XLA:
	case AMD_XC2S150:
	case AMD_XC2S100_8M:
	case AMD_XC2S200_8M:
	    return 2;

	default: return 1;
    }
}
/*
 * extract the name from the device string read from the onboard PROM
 */
void
getname(EdtDev *edt_p, int promcode, char *name)
{
    char idstr[MAX_STRING+1], dmy1[MAX_STRING+1], dmy2[MAX_STRING+1];
    char *p;

    edt_readinfo(edt_p, promcode, default_segment(promcode), idstr, dmy1, dmy2);

    if ((p = strchr(idstr, '.')) != NULL)
	*p = '\0' ;

    if ((p = strrchr(idstr, '_')) != NULL)
    {
	if (( *(p + 1) == '3' || *(p + 1) == '5' )  && ( *(p + 2) == 'v' ))
	    *p = '\0' ;
    }
    strcpy(name, idstr);
}

void
load_bitfile(char *fname, BFS *bitfile, int promcode, int nofs, int vb)
{
    /* this code borrowed from main() */
    FILE   *infd;
    u_char *data_start;
    int     size = 0, got;

    /*
     * if ERASE don't do anything except copy the filename to the struct
     */
    if (strcmp(fname, "ERASE") != 0)
    {

	if (vb) printf("bitfile %s\n", fname);

	/*
	 * extract the prom type / name for the array if embedded bitfile
	 */
#ifdef NO_FS
	if (nofs)
	{
	    char *p;
	    u_char *ba, *bd;
	    char tmpname[128];
	    char bname[128];
	    char pname[128];
	    char hname[128];

	    strcpy(tmpname, fname);
	    if ((p = strrchr(tmpname, '/')) != NULL)
	    {
		strcpy(bname, p+1);
		*p = '\0';
		if ((p = strrchr(tmpname, '/')) != NULL)
		    strcpy(pname, p+1);
		else strcpy(pname, tmpname);
			
		sprintf(hname, "%s_%s", pname, bname);
		if ((p = strrchr(hname, '.')) != NULL)
		    *p = '\0';
	    }

	    MAPFLASHARRAY(hname, ba); /* see autogen header file nofs_bitfiles.h */

	    if (ba == NULL)
	    {
		fprintf(stderr, "error: no array referenced to %s in nofs_bitfiles.h\n", hname);
		exit(1);
	    }

	    /* get the header. opens and closes the file, and returns pointer to
	     * the start of data 
	     */
	    if ((bd = edt_get_x_array_header(ba, bitfile->prom, &size)) == NULL)
	    {
		printf("Error -- invalid header data in xilinx file\n");
		exit(2);
	    }

	    sscanf(bitfile->prom, "%s ", bitfile->id);
	    memcpy(bitfile->data, bd, size);

	    if (vb)
		printf("read input array length is %d bytes\n", size);
	    if (size < 16 * 1024)
	    {
		printf("not enough  data (%d bytes) in input array read\n", size);
		exit(2);
	    }
	}
	else
#endif
	{
	    long   offset;

	    /* get the header. opens and closes the file, and returns the
	     * offset to the start of data 
	     */
	    if ((offset = edt_get_x_file_header(fname, bitfile->prom, &size)) < 0)
	    {
		printf("Error -- invalid header data in xilinx file\n");
		exit(2);
	    }

	    sscanf(bitfile->prom, "%s ", bitfile->id);

	    /* open the bit file and seek to start of data as returned
	     * from edt_get_x_file_header
	     */
	    infd = fopen(fname, "rb");
	    if (infd == NULL)
	    {
		perror(fname);
		exit(1);
	    }
	    fseek(infd, offset, SEEK_SET);

	    if ((got = fread(bitfile->data, 1, size, infd)) < size)
	    {
		printf("ERROR -- xilinx file truncated\n");
		exit(2);
	    }

	    fclose(infd);

	    if (vb)
		printf("read input file length is %d bytes\n", size);
	    if (size < 16 * 1024)
	    {
		printf("not enough  data (%d bytes) in input file read\n", size);
		exit(2);
	    }
	}

	data_start = &bitfile->data[0];
	bitfile->data_index = 0;

	if (nofs)
	    printf("  Array id: <%s>\n", bitfile->prom);
	else printf("  File  id:  <%s>\n", bitfile->prom);

	/* end code borrow */
    }

    strcpy(bitfile->filename, fname);
    bitfile->filesize = size;
}


static void
usage(char *s)
{
    printf("\n");
    printf("pciload: update, verify or query FPGA firmware on EDT PCI/PMC/cPCI boards\n\n");
    if (s && (*s))
	printf("%s\n\n", s);

    printf("usage:\n");
    printf("  pciload [options] [keyword | filename]\n");
    printf("\n");
    printf("  If no arguments are given, pciload finds all EDT boards in the system\n");
    printf("  and prints the prom ID for each.\n");
    printf("\n");
    printf("options:\n");
    printf("  -D specifies the device type  (default %s)\n", EDT_INTERFACE);
    printf("  -u specifies the unit number (default 0). alternately you can specify\n");
    printf("     device type and board at the same time, e.g. -u %s1\n", EDT_INTERFACE);
    printf("  -i program new embedded info into board\n");
    printf("  -I program new embedded info into board; get defaults from file if present\n");
    printf("  -d specifies directory to look for files (default ./flash)\n");
    printf("  -s <0-3> sets the PROM sector number on boards using XLA parts\n");
    printf("     If no sector is specified (recommended), pciload will program all sectors\n");
    printf("     with the specidied file (or default if no file specified)\n");
    printf("  -v verify on-board firmware against file (will not burn new f/w)\n");
    printf("  -V sets verbose mode\n");
    printf("  -F <0-8> adds various rewrites/sleeps; works around problems with some systems'\n");
    printf("     chipsets. If programming fails, try -F 2, 4 or 3. See also -S\n");
    printf("  -S sets 'slow' mode -- necessary on some computers; try this if the load or\n"); 
    printf("     verify seems to hang or indicates corrupt firmware and -F doesn't fix it\n");
    printf("  -h this help/usage message ('pciload help' will also work)\n");
    printf("\n");
    printf("  The last argument can be one of the keywords 'update', 'verify', 'help' or\n");
    printf("  a full or partial filename. When updating, avoid using a filename unless\n");
    printf("  you know for sure what file you need; instead use the keyword 'update'.\n");
    printf("\n");
    printf("  If the keyword 'update' is present, pciload will attempt to find an up-\n");
    printf("  dated version of the board's current firmware under the flash/ directory\n");
    printf("  and update the PROM with the contents of that file.\n");
    printf("\n");
    printf("  If the keyword 'verify' is present, pciload will compare the firmware in\n");
    printf("  the PROM to the one in the file (optionally the '-v' flag can be used\n");
    printf("  with a filename to compare to a specific firmware file)\n");
    printf("\n");
    printf("  The keyword 'help' prints out this message (equivalent to '-h').\n");
    printf("\n");
    printf("  If a partial filename is specified (e.g. 'pcd',  or 'pci11w', pciload will\n");
    printf("  search for the appropriate version of the firmware file, and update to\n");
    printf("  that version (or compare if '-v' is specified). Complete/absolute pathnames\n");
    printf("  can also be used for filename (but with caution).\n");

    printf("\n");
    if (s == NULL)
	exit(0);
    exit(1);
}

char Tmppn[13];
char *fmt_pn(char *pn)
{
    if (strlen(pn) == 10)
	sprintf(Tmppn, "%c%c%c-%c%c%c%c%c-%c%c",
	  pn[0],pn[1],pn[2],pn[3],pn[4],pn[5],pn[6],pn[7],pn[8],pn[9]);
    else if (strlen(pn) < 10)
	strcpy(Tmppn, pn);
    else strcpy(Tmppn, "0");
    return Tmppn;
}

int isalnum_str(char *s)
{
    unsigned int i;

    for (i=0; i<strlen(s); i++)
	if (!isalnum(s[i]))
	    return 0;
    return 1;
}


/*
 * get the 5-10 digit serial number
 */
void
ask_sn(char *sn)
{
    int ok = 0;
    int tries = 0;
    char s[128];

    while (!ok)
    {
	printf("Enter board serial number (5-10 characters) %s%s%s > ",
						*sn?"[":"", sn, *sn?"]":"");
	fgets(s,127,stdin);
	strip_newline(s);

	if (*sn && !*s)
	    return;

	if ((strlen(s) < 5)
	 || (strlen(s) > 10)
	 || (!isalnum_str(s)))
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid serial number. Expecting 5-10 alphanumeric characters\n\n");
	}
	else
	{
	    strcpy(sn, s);
	    toupper_str(sn);
	    ok = 1;
	}
    }
    /* printf("\n"); */
}

/*
 * get the 10 digit option string. allow either XXX-XXXXX or
 * XXXXXXXX
 */
 void
ask_pn(char *pn)
{
    int i, j;
    int ok = 0;
    int tries = 0;
    char s[128];
    char tmpstr[11];

    while (!ok)
    {
	printf("Enter 10-digit part # (with or without dashes) %s%s%s > ",
						*pn?"[":"", fmt_pn(pn), *pn?"]":"");
	fgets(s,127,stdin);
	strip_newline(s);

	if (*pn && !*s)
	    return;

	if (strlen(s) == 12)		/* XXX-XXXXX-XX */
	{
	    /* reformat without the dashes */
	    ok = 1;
	    for (i=0, j=0; i<12; i++)
		if (s[i] != '-')
		    tmpstr[j++] = s[i];
	    tmpstr[10] = '\0';
	    strcpy(s, tmpstr);
	}
	else if (strlen(s) == 10)	/* XXXXXXXX */
	{
	    if (isdigit_str(s))
		ok = 1;
	}

	if (!ok)
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid part number. Format is XXX-XXXXX-XX, all\n");
	    printf("numeric (dashes optional)\n\n");
	}
	else
	{
	    strcpy(pn, s);
	    ok = 1;
	}
    }
    /* printf("\n"); */
}


int
match_ifx_dir(char *base, char *name)
{
    HANDLE  dirp = (HANDLE) 0;
    char    d_name[256];
    int     ret = 0;

    if ((dirp = edt_opendir(base)) == (HANDLE)0)
	return 0;

    while (edt_readdir(dirp, d_name))
    {
	if (strcasecmp(name, d_name) == 0)
	{
	    ret = 1;
	    break;
	}
    }
    edt_closedir(dirp);
    return ret;
}

/*
 * check IF xilinx name against dir names and return 1 if match, 0 if not
 * ADD new dv devices as they come along
 */
int
valid_ifxname(int devid, char *name)
{
    if ((devid == PDV_ID)
     || (devid == PDVA_ID)
     || (devid == PDVA16_ID)
     || (devid == PDVK_ID)
     || (devid == PDV44_ID)
     || (devid == PDVAERO_ID))
    {
	if (match_ifx_dir("./camera_config/bitfiles/dv", name)
	 || match_ifx_dir("./camera_config/bitfiles/dva", name)
	 || match_ifx_dir("./camera_config/bitfiles/dvaero", name)
	 || match_ifx_dir("./camera_config/bitfiles/dvk", name))
		return 1;
    }

    if (match_ifx_dir("./bitfiles", name))
	return 1;
    return 0;
}


void
ask_ifxilinx(int devid, char *pn, int rev, char *ifx)
{
    int ok = 0;
    int tries = 0;
    char s[128];
    char tmppn[32];
    int nifx = 0;

    /* if xxx-xxxxx-00, replace last 2 digits with rev */
    if ((strcmp(&(pn[8]), "00") == 0))
	sprintf(&(tmppn[8]), "%02d", rev);
    else strcpy(tmppn, pn);

    /*
     * ADD any devices that don't have an interface FPGA here 
     */
    if ((devid == P11W_ID)
     || (devid == P16D_ID)
     || (devid == PDVCL_ID)
     || (devid == PDVCL_ID)
     || (devid ==  PDVFCI_AIAG_ID)
     || (devid ==  PDVFCI_USPS_ID)
     || (devid ==  PCDFCI_SIM_ID)
     || (devid ==  PCDFCI_PCD_ID))
    {
	strcpy(ifx, "n/a");
	return;
    }

    if ((strlen(ifx) < 2) && (strlen(tmppn) > 7))
	edt_find_xpn(tmppn, ifx);

    while (!ok)
    {
	printf("Enter interface Xilinx FPGA, (0-10 characters) %s%s%s > ",
				*ifx?"[":"", ifx, *ifx?"]":"");
	fgets(s,127,stdin);
	strip_newline(s);

	if (*ifx && !*s)
	    return;

	if (strlen(s) > 10 || strchr(s, ':')) /* no colons allowed */
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid I/F Xilinx string -- must be 0-10 alphanumeric characters\n\n");
	}
	else
	{
	    strcpy(ifx, s);
	    ok = 1;
	}
    }
    /* printf("\n"); */
}

void
ask_clock(int *clock)
{
    int i = 0;
    int ok = 0;
    int tries = 0;
    int n;
    char s[128];

    while (!ok)
    {
	printf ("Enter clock speed (usually 10, 20, 30 or 40)");
	if (*clock)
	    printf(" [%d] > ", *clock);
	else printf(" > ");

	fgets(s,127,stdin);
	strip_newline(s);

	if (*clock && !*s)
	    return;

	n = atoi(s);

	if (n >= 10 && n <= 120)
	{
	    ok = 1;
	    *clock = n;
	}
	else
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid entry -- must be in the range 10-120\n");
	}
    }
    /* printf("\n"); */
}

void
ask_rev(int *rev)
{
    int i = 0;
    int ok = 0;
    int tries = 0;
    int n;
    char s[128];

    while (!ok)
    {
	printf ("Enter rev no.");

	if (*rev)
	    printf(" [%02d] > ", *rev);
	else printf(" > ");

	fgets(s,127,stdin);
	strip_newline(s);

	if (*rev && !*s)
	    return;

	n = atoi(s);

	if (n >= 0 && n <= 999)
	{
	    ok = 1;
	    *rev = n;
	}
	else
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid entry -- must be in the range 0-999\n");
	}
    }
    /* printf("\n"); */
}

/*
 * get the 10 digit option string
 */
void
ask_options(char *options)
{
    int ok = 0;
    int tries = 0;
    char s[128];

    while (!ok)
    {
	printf("Enter options, if any (0-10 characters) %s%s%s > ",
				*options?"[":"", options, *options?"]":"");
	fgets(s,127,stdin);
	strip_newline(s);

	if (*options && !*s)
	    return;

	if (strlen(s) > 10 || strchr(s, ':')) /* no colons allowed */
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid option string -- must be 0-10 alphanumeric characters\n\n");
	}
	else
	{
	    strcpy(options, s);
	    ok = 1;
	}
    }
    printf("\n");
}


int
ask_addinfo()
{
    char s[128];
    int ret = 0;

    printf("\n");
    printf("This board is either being programmed for the first time, or has a\n");
    printf("corrupted or erased XILINX PROM. Entering the embedded info (clock\n");
    printf("speed, board part number, serial number, and options) is rec-\n");
    printf("commended at this time. It is not required however, so if you\n");
    printf("don't have this information, simply skip this step by entering 'n'\n");

    *s = '\0';

    while (1)
    {
	printf("\nDo you wish to enter the embedded information? [y/n/q] > ");
	fgets(s,127,stdin);
	tolower(*s);

	if (*s == 'y')
	    return 1;
	else if (*s == 'n')
	    return 0;
	else if (*s == 'q')
	{
	    printf("exiting\n");
	    exit(0);
	}
	printf("\nInvalid entry\n");
    }
}

/*
 * print the serial numbers
 */
print_sns(char *esn, char *osn)
{
    Edt_embinfo ei;

    memset(&ei,0,sizeof(ei)) ;

    if (*esn)
    {
	if (edt_parse_esn(esn, &ei) == 0)
	{
	    printf("  s/n %s, p/n %s, i/f fpga %s, rev %d clock %d Mhz", ei.sn, fmt_pn(ei.pn), strlen(ei.ifx) > 2?ei.ifx:"(none)", ei.rev, ei.clock);
	    if (*ei.opt)
		printf(", opt %s", ei.opt);
	}
    }
    if (*osn)
	printf(", oem s/n %s", osn);

    if (*esn || *osn)
	printf("\n");
}



/*
 * prompt for info to put into the serial number, etc. part of
 * the xilinx PROM
 */
ask_edt_info(int devid, Edt_embinfo *si)
{
    char s[128];
    int ok = 0;
    int clock_alert = 0;

    while (!ok)
    {
	printf("\n");
	ask_sn(si->sn);
	ask_pn(si->pn);
	ask_rev(&(si->rev));
	ask_ifxilinx(devid, si->pn, si->rev, si->ifx);
	ask_clock(&(si->clock));
	ask_options(si->opt);

	if (si->clock != 10 && si->clock != 20
	 && si->clock != 30 && si->clock != 40)
		clock_alert = 1;

	printf("\nSerial Number: %s\n", si->sn);
	printf("Part Number:   %c%c%c-%c%c%c%c%c-%c%c\n",
	    si->pn[0], si->pn[1], si->pn[2], si->pn[3], si->pn[4],
	    si->pn[5], si->pn[6], si->pn[7], si->pn[8], si->pn[9]);
	printf("I/F FPGA:      %s %s", si->ifx, valid_ifxname(devid, si->ifx)?"\n":" (WARNING: no matching FPGA dir found)\n");
	printf("Rev:           %02d\n", si->rev);
	printf("Clock:         %d Mhz%s\n", si->clock, clock_alert?" (WARNING: non-standard clock speed)":"");
	printf("Options:       %s\n", si->opt[0]?si->opt:"<none>");

	*s = '\0';
	while (tolower(*s) != 'y' && (tolower(*s) != 'n'))
	{
	    printf("\nOkay? (y/n/q) [y]> ");
	    fgets(s,127,stdin);

	    if ((*s == '\n') || (*s == '\r') || (*s == '\0'))
	    *s = 'y'; /* default to 'yes' */

	    if (*s == 'y')
		ok = 1;
	    else if (*s == 'q')
	    {
		printf("exiting\n");
		exit(0);
	    }
	    else if (*s != 'n')
		printf("\nEnter 'y' if the above information is correct, 'n' to change, q to quit\n");
	}
    }
}

void
pad_sn(char *sn)
{
    int i=0;
    char *p;
    char tmpsn[128];
    int num;

    strcpy(tmpsn, sn);

    p = &tmpsn[strlen(sn)];

    while (p > tmpsn && (*(p-1) >= '0') && (*(p-1) <= '9'))
	--p;

    num = atoi(p);
    *p = '\0';
    if (Option_hack)
	sprintf(sn, "PDVF%04d", num);
    else sprintf(sn, "%s%04d", tmpsn, num);
}
int
check_ask_sn(EdtDev *edt_p, int promcode, int sect, char *fname, int vfonly, char *esn, char *new_esn, char *lcl_esn, char *pid, Edt_embinfo *embinfo, int *proginfo, int nofs, int vb)
{
    char osn[128];
    char tmp_esn[128];

    if (strcmp(fname, "ERASE") == 0)
	return 0;

    if (strcmp(new_esn, "erase") == 0)
    {
	strcpy(esn, "erase");
	return 0;
    }

    edt_readinfo(edt_p, promcode, sect, pid, esn, osn);
    edt_parse_esn(esn, embinfo);

    /* proginfo > 1 means use info in file for
     * defaults instead of whats already on the board
     */
    if ((!nofs) && (((*proginfo) > 1) || (!*lcl_esn)))
    {
	if (read_esn_file(edt_p->devid, tmp_esn) == 0)
	{
	    edt_parse_esn(tmp_esn, embinfo);
	    if (*embinfo->sn)
		increment_sn(embinfo->sn);
	}
    }

    if (*fname && !vfonly)
    {
	if (!*proginfo && (!*lcl_esn) && (!*pid))
	    *proginfo = ask_addinfo();

	if (*proginfo)
	{
	    ask_edt_info(edt_p->devid, embinfo);
	    memset(new_esn, 0, ESN_SIZE);
	    sprintf(new_esn, "%s:%s:%d:%s:%d:%s:", embinfo->sn, embinfo->pn,
				embinfo->clock, embinfo->opt, embinfo->rev, embinfo->ifx);
	    strcpy(esn, new_esn);
	}
	else if (Option_hack)
	{
	    strcpy(embinfo->opt, "lvds");
	    pad_sn(embinfo->sn);
	    sprintf(new_esn, "%s:%s:%d:%s:%d:%s:", embinfo->sn, embinfo->pn,
				embinfo->clock, embinfo->opt, embinfo->rev, embinfo->ifx);
	    strcpy(esn, new_esn);
	}
    }
    return *proginfo;
}


char *
fmt_promcode(int promcode)
{
    int ret = 0;

    if (promcode < 0 || promcode > MAX_PROMCODE)
	return "Unknown Flash ROM!";
    return xilinx_promstr[promcode];
}

void
print_prominfo(EdtDev *edt_p, int promcode, int vb)
{
    int ret = 0;
    char pid[MAX_STRING+1];
    char esn[ESN_SIZE];
    char osn[OSN_SIZE];

    switch (promcode)
    {
	case AMD_4013E:
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    break;
	case AMD_4013XLA:
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	    break;
	case AMD_4028XLA:
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	    break;
	case AMD_XC2S150:
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	    break;
	case AMD_XC2S200_4M:
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    break;
	case AMD_XC2S100_8M:
	case AMD_XC2S200_8M:
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	    break;
	default:
	    printf("Unknown Flash ROM - No information available\n");
	    ret = -1;
	    break;
    }
}

int     quiet = 0;
BFS    bitfile;

#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;
pciload(char *command_line)
#else
int
main(int argc, char *argv[])
#endif
{
    int     i;
    int     nofs = 0;
    int     reb = 0, ver = 0, vb = 0, vfonly = 0, reboot = 0;
    int     summary_only = 0;
    char    fname[MAX_STRING+1];
    int     promcode;
    int     nunits;
    int     autoupdate = 0;
    int     saveinfo = 0;
    u_char  stat;
    u_int   frdata;
    EdtDev *edt_p = NULL;
    char    errbuf[128];
    char   *newfname;
    char   *unitstr = "";
    char    pid[MAX_STRING+1];
    char    esn[ESN_SIZE];
    char    osn[OSN_SIZE];
    char    new_esn[ESN_SIZE];
    char    new_osn[OSN_SIZE];
    char    lcl_esn[ESN_SIZE];
    char    lcl_osn[ESN_SIZE];
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("pciload",command_line,&argc,&argv);
#endif

    errbuf[0] = 0;

    sprintf(flash_dir, ".%cflash", DIRECTORY_CHAR);
    sect = IS_DEFAULT_SECTOR;
    argv0 = argv[0];
    fname[0] = '\0';
    new_esn[0] = new_osn[0] = '\0';
    esn[0] = osn[0] = '\0';
    memset(&embinfo,0,sizeof(embinfo)) ;

    /*
     * COMMAND LINE ARGUMENTS
     */

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] != '-')
	{
	    if (i != argc - 1)
		usage("the fname must be the last argument");
	    strcpy(fname, argv[i]);
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
	    if ((argv[i][1] == 0 || argv[i][2] != 0)
	        && (strcmp(&argv[i][1], "Ossp") != 0))
		usage(NULL);
	    switch (argv[i][1])
	    {
	    case 'D':
		if (++i == argc)
		    usage("unfinished -D option");
		strcpy(dev, argv[i]);
		break;
	    case 'd':
		if (++i == argc)
		    usage("unfinished -d option");
		strcpy(flash_dir, argv[i]);
		break;
	    case 'u':
		if (++i == argc)
		    usage("unfinished -u option");
		unitstr = argv[i];
		break;
	    case 's':
		if (++i == argc)
		    usage("unfinished -s option");
		sect = atoi(argv[i]);
		break;
	    case 'e':
#ifdef NO_FS
		nofs = 1;
#else
		sprintf(errbuf, "-e specified but not compiled with embedded bitfiles.\nrecompile with -DNO_FS and try again\n");
		usage(errbuf);
		exit(1);
#endif
		break;
	    case 'v':
		vfonly = vfonly ? 0 : 1;
		break;
	    case 'r':
		reboot = 1;
		break;
	    case 'V':
		vb = vb ? 0 : 1;
		break;
	    case 'i':
		proginfo = 1;
		break;
	    case 'I':
		proginfo = 2;
		break;
	    case 'p':		/* print a summary of installed boards */
		summary_only=1;
		break;
	    case 'q':
		quiet=1;
		break;
	    case 'f':
		force_fast=1;
		edt_set_do_fast(1) ;
		break;

    /* DEBUG add delays, rewrites or rereads to debug fast mode */
            case 'F':
                if (++i == argc)
		{
                    printf("unfinished -F option. takes 1 int arg, set bits as follows:\n");
		    printf("0 (0x1): add extra writes in tst_write\n");
		    printf("1 (0x2): add extra dummy reads in tst_write\n");
		    printf("2 (0x4): add extra reads in tst_read\n"); 
		    printf("3 (0x8): unassigned\n");
		    printf("4 (0x10): msleep(1) before every tst_write\n");
		    printf("5 (0x20): msleep(1) before every tst_read\n");
		    exit(0);
		}
                edt_set_debug_fast(strtol(argv[i], NULL, 16));
                break;


#ifdef PSN
#include "pciload_sn.c"
#endif

	    case 'S':
		edt_set_force_slow(1) ;
		edt_set_do_fast(0) ;
		break;

	    case 'H':
		Option_hack= 1;
		break;

	    case 'h':
		usage(errbuf);
		break;
	    default:
		sprintf(errbuf, "\nunknown option \"-%c\"", argv[i][1]);
		usage(errbuf);
	    }
	}
    }
    if (*unitstr)
	unit = edt_parse_unit(unitstr, dev, NULL);

    /* check for valid values */
    if (sect != IS_DEFAULT_SECTOR &&
	(sect < 0 || sect > 3))
    {
	printf("Sector number %d should be between 0 and 3\n", sect);
	exit(1);
    }

    if (strcmp(fname, "update") == 0)
    {
	*fname = 0;
	autoupdate = 1;
    }

    if (strcmp(fname, "erase") == 0)
    {
	strcpy(fname, "ERASE");
	strcpy(new_osn, "erase");
	strcpy(new_esn, "erase");
    }

    else if (strcmp(fname, "verify") == 0)
    {
	*fname = 0;
	vfonly = 1;
    }

    else if (strcmp(fname, "help") == 0)
    {
	usage(errbuf);
	exit(0);
    }

    if ((autoupdate || vfonly) && !*fname)
    {
	if (unit == -1)
	    unit = 0 ;

	if (dev[0] == 0)
	    strcpy(dev, EDT_INTERFACE); 

	if ((edt_p = edt_open(dev, unit)) == NULL)
	{
	    char str[256];
	    sprintf(str, "%s unit %d", EDT_INTERFACE, unit) ;
	    edt_perror(str) ;
	    exit(1) ;
	}

	if ((promcode = edt_x_prom_detect(edt_p, &stat, vb)) == PROM_UNKN)
	{
	    printf("Unknown Flash ROM - No information available\n");
	    exit(1) ;
	}

	getname(edt_p, promcode, fname) ;
    }

    if (strlen(esn) > 26)
    {
	printf("invalid -E usage");
	printf("Contact EDT at 503-690-1234 or tech@edt.com for more information\n");
	exit(1);
    }
    if (strlen(osn) > 26)
    {
	printf("invalid -O usage");
	printf("Contact EDT at 503-690-1234 or tech@edt.com for more information\n");
	exit(1);
    }

    /* create critical defaults */
    if (dev[0] == 0 || unit == -1)
    {
	Edt_bdinfo *boards = edt_detect_boards(dev, unit, &nunits, vb);

	if (boards[0].id == -1)
	{
	    printf("No board detected\n");
	    exit(1);
	}

	/* PRINT OUT INFORMATION ON MUTLIPLE BOARDS */
	if (summary_only || boards[1].id != -1)
	{
	    if (boards[1].id != -1)
		printf("\nMultiple EDT PCI boards detected:\n\n");

	    if (!*fname)
	    {
		for (i = 0; boards[i].id != -1; i++)
		{
		    printf("%s unit %d (%s):\n", boards[i].type, boards[i].id,
						edt_idstr(boards[i].bd_id));
		    edt_p = edt_open(boards[i].type, boards[i].id);
		    promcode = edt_x_prom_detect(edt_p, &stat, vb);
		    edt_get_sns(edt_p, esn, osn);
		    printf("  %s\n", fmt_promcode(promcode));
		    print_sns(esn, osn);
		    print_prominfo(edt_p, promcode, vb);
		    frdata = edt_reg_read(edt_p,EDT_FLASHROM_DATA) ;
		    print_flashstatus(stat, sect, frdata, vb);
		    printf("\n");
		}
		exit(0);
	    }
	    else
	    {
#if 1 /* try this -- want to go ahead and use the default if no dev specified */
		strcpy(dev, EDT_INTERFACE); 
#else
		for (i = 0; boards[i].id != -1; i++)
		{
		    printf("%s unit %d\n", boards[i].type, boards[i].id);
		}
		printf("Can only %s one board at a time.\n", vfonly ? "verify" : "program");
		printf("Run pciload again and specify a board.\n");
		exit(1);
#endif
	    }
	}
	if (dev[0] == 0)
	    strcpy(dev, boards[0].type);

	unit = boards[0].id;
	free(boards);
    }
    else if (dev[0] == 0)
	strcpy(dev, EDT_INTERFACE); 

    /* DO SOMETHING FOR ONLY ONE BOARD */

    if ((edt_p = edt_open(dev, unit)) == NULL)
    {
	char    str[64];

	sprintf(str, "edt_open(%s%d)", dev, unit);
	edt_perror(str);
	return (2);
    }

    printf("\n%s unit %d (%s):\n", dev, unit, edt_idstr(edt_p->devid));

    /* determine prom type */

    edt_get_sns(edt_p, lcl_esn, lcl_osn);
    promcode = edt_x_prom_detect(edt_p, &stat, vb);
    printf("  %s\n", fmt_promcode(promcode));
    print_sns(lcl_esn, lcl_osn);
    /* print_prominfo(edt_p, promcode, vb); */
    frdata = edt_reg_read(edt_p,EDT_FLASHROM_DATA) ;
    print_flashstatus(stat, sect, frdata, vb);

    /* check for impossible programming */
    if (!vfonly && ((stat & EDT_ROM_JUMPER) == 0)
 		&& (promcode!=AMD_XC2S150) 
 		&& (promcode!=AMD_XC2S200_4M) 
 		&& (promcode!=AMD_XC2S100_8M) 
 		&& (promcode!=AMD_XC2S200_8M) 
		&& (promcode!=AMD_4028XLA))
    {
	printf("Protected ROM jumper is in the protected position pin 1-2\n");
	/* FIX jsc */
	printf("Verify only allowed\n");
	printf("To program, remove jumper and run pciload again\n");
	/* exit(1); */
	vfonly = 1;
    }

    if (*new_esn)
	strcpy(esn, new_esn);

    if (*new_osn)
	strcpy(osn, new_osn);

    /*
     * things to take care of in the next switch statement:
     * 
     * 1) if no fname, just get info.  otherwise, 2) program the sectors as
     * needed--- fix_fname's middle arg is 0 for 3.3volt file, 1 for 5volt
     * file, and -1 for non-XLA boards(no voltage) otherwise,
     * program_verify_PROMTYPE()
     * 
     * things like making sure values are within range should be taken care of
     * inside the getinfo and program_verify functions.
     */
    switch (promcode)
    {
	case AMD_XC2S100_8M:
	case AMD_XC2S200_8M:

	if (!*fname)
	{
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	}
	else
	{
	    /* dflt to fast with this xilinx */
	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;
	    if (sect == IS_DEFAULT_SECTOR)
	    {
		/* program sect 0 with 3.3volt file */
		newfname = fix_fname(fname, promcode, 0, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, 2, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, 2, vfonly, 1, vb);

		/* program sect 1 with 5volt file */
		newfname = fix_fname(fname, promcode, 1, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode,  nofs,vb);
		ver += program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, 3, vfonly, 0, vb);
	    }
	    else
	    {
		load_bitfile(fname, &bitfile, promcode,  nofs,vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, sect, vfonly, 1, vb);
	    }
	}

	break;

	case AMD_XC2S200_4M:

	if (!*fname)
	{
	    printf("  Sector 0 ");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1 ");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	}
	else
	{
	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;
	    if (sect == IS_DEFAULT_SECTOR)
	    {
		/* program sect 0 with 3.3volt file */
		newfname = fix_fname(fname, AMD_XC2S200_4M, 0, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode,  nofs,vb);
		saveinfo = check_ask_sn(edt_p, promcode, 0, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver += program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, 0, vfonly, 1, vb);


		/* program sect 1 with 5volt file */
		newfname = fix_fname(fname, AMD_XC2S200_4M, 1, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode,  nofs,vb);
		ver += program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, 1, vfonly, 0, vb);

	    }
	    else
	    {
		load_bitfile(fname, &bitfile, promcode,  nofs,vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_XC2S200(edt_p, &bitfile, dev, pid, esn, osn, sect, vfonly, 1, vb);
	    }
	}

	break;

	case AMD_XC2S150:

	if (!*fname)
	{
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	}
	else
	{
	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;
	    if (sect == IS_DEFAULT_SECTOR)
	    {

		/* program sect 2 with 3.3volt file */
		newfname = fix_fname(fname, AMD_XC2S150, 0, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode,  nofs,vb);
		saveinfo = check_ask_sn(edt_p, promcode, 2, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_XC2S150(edt_p, &bitfile, dev, pid, esn, osn, 2, vfonly, 1, vb);


		/* program sect 3 with 5volt file */
		newfname = fix_fname(fname, AMD_XC2S150, 1, nofs, vb);
		load_bitfile(newfname, &bitfile,promcode, nofs, vb);
		ver += program_verify_XC2S150(edt_p, &bitfile, dev, pid, esn, osn, 3, vfonly, 0, vb);

	    }
	    else
	    {
		load_bitfile(fname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_XC2S150(edt_p, &bitfile, dev, pid, esn, osn, sect, vfonly, 1, vb);
	    }
	}

	break;

	case AMD_4028XLA:
	if (!*fname)
	{
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	}
	else
	{

	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;
	    if (sect == IS_DEFAULT_SECTOR)
	    {

		/* program sect 2 with 3.3volt file */
		newfname = fix_fname(fname, AMD_4028XLA, 0, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, 2, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_4028XLA(edt_p, &bitfile, dev, pid, esn, osn, 2, vfonly, 1, vb);


		/* program sect 3 with 5volt file */
		newfname = fix_fname(fname, AMD_4028XLA, 1, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		ver += program_verify_4028XLA(edt_p, &bitfile, dev, pid, esn, osn, 3, vfonly, 0, vb);

	    }
	    else
	    {
		load_bitfile(fname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_4028XLA(edt_p, &bitfile, dev, pid, esn, osn, sect, vfonly, 1, vb);
	    }
	}

	break;

	case AMD_4013E:
	if (!*fname)
	{
	    getinfo(edt_p, promcode, 0, pid, esn, osn, vb);
	}
	else
	{
	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;
	    if (sect == IS_DEFAULT_SECTOR)
	    {
		newfname = fix_fname(fname, AMD_4013E, -1, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		program_verify_4013E(edt_p, &bitfile, dev, pid, esn, osn, vfonly, 1, vb);
	    }
	    else
	    {
		printf("No sect can be specified for the 4013e.  Run pciload again without the -s argument.\n");
		exit(1);
	    }

	}
	break;

	case AMD_4013XLA:
	if (!*fname)
	{
	    printf("  Sector 1");
	    getinfo(edt_p, promcode, 1, pid, esn, osn, vb);
	    printf("  Sector 3");
	    getinfo(edt_p, promcode, 3, pid, esn, osn, vb);
	}
	else
	{
	    if (edt_get_force_slow()) edt_set_do_fast(0) ;
	    else edt_set_do_fast(1) ;

	    if (sect == IS_DEFAULT_SECTOR)
	    {
		/* program sect 1 with 3.3volt file */
		newfname = fix_fname(fname, AMD_4013XLA, 0, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, 1, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_4013XLA(edt_p, &bitfile, dev, pid, esn, osn, 1, vfonly, 1, vb);


		/* program sect 3 with 5volt file */
		newfname = fix_fname(fname, AMD_4013XLA, 1, nofs, vb);
		load_bitfile(newfname, &bitfile, promcode, nofs, vb);
		ver += program_verify_4013XLA(edt_p, &bitfile, dev, pid, esn, osn, 3, vfonly, 0, vb);

	    }
	    else
	    {
		printf("B: %d\n", sect);
		load_bitfile(fname, &bitfile, promcode, nofs, vb);
		saveinfo = check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, &embinfo, &proginfo, nofs, vb);
		ver = program_verify_4013XLA(edt_p, &bitfile, dev, pid, esn, osn, sect, vfonly, 1, vb);
	    }
	}

	break;
    }

    if (nofs)
	saveinfo = 0;

    /* save the serial # info to a file if changed and programmed */
    if (!vfonly && (ver == 0) && (saveinfo))
	save_esn_file(edt_p->devid, esn, vb);

    edt_close(edt_p);

    if (*fname && !vfonly)
    {
	if (ver)
	{
	    printf("One or more sectors failed verification. Errors may be due to slow host\n");
	    printf("writes/reads. Try -F <level> or -S (pciload --help to see all options)\n");
	}
	else if (reb == 1)
	    printf("The xilinx has been rebooted with new firmware and is ready for use.\n");
	else if (strcmp(fname, "ERASE") == 0)
	{
	    printf("This board will be non-functional after the next power-down, and will remain \n");
	    printf("so until the appropriate EDT PCI FPGA firmware is loaded back into the\n");
	    printf("Board's EEPROM.\n\n");
	    printf("To reload the firmware, power the system down, move the protected mode jumper\n");
	    printf("on the board to the \"protected sector\" positon (pins 1-2). Power the system\n");
	    printf("up again, move the jumper back to the non-protected position (pins 2-3),\n");
	    printf("then load the appropriate firmware using the pciload program.\n");
	}
	else printf("The firmware upgrade will take effect after the next time you cycle power.\n");
    }
    return 0;
}


int
save_esn_file(u_int devid, char *esn, int vb)
{
    FILE *fp;
    char fname[32];

    sprintf(fname, "pciload_%02x.esn",  devid);

    if (edt_access(fname, 2) == 0)
    {
	/* if (vb) printf("overwriting existing ESN file '%s'\n", fname); */
    }
    else if ((edt_access(fname, 0) == 0) || (edt_access(".", 2) != 0))
    {
	if (vb) printf("can't write or create ESN file '%s'\n", fname);
	return -1;
    }
    /* else printf("DEBUG: creating file '%s'\n", fname); */

    if ((fp = fopen(fname, "w")) == NULL )
    {
	if (vb) edt_perror(fname);
	return -1;
    }

    fprintf(fp, "%s\n", esn);
    fclose(fp);
    return 0;
}

int read_esn_file(u_int devid, char *esn)
{
    FILE *fp;
    char s[128];
    int  ret = 0;
    char fname[32];

    sprintf(fname, "pciload_%02x.esn",  devid);

    if ((fp = fopen(fname, "r")) == NULL )
    {
	/* edt_perror(str) ; */
	return -1;
    }

    if (fgets(s, 127, fp) && (strlen(s) > 8) && (strlen(s) < 128))
	strcpy(esn, s);
    else ret = -1;

    fclose(fp);
    return ret;
}

void
increment_sn(char *sn)
{
    int i=0;
    char *p;
    char tmpsn[128];
    int num;

    strcpy(tmpsn, sn);
    p = &tmpsn[strlen(sn)];

    while (p > tmpsn && (*(p-1) >= '0') && (*(p-1) <= '9'))
	--p;

    num = atoi(p);
    *p = '\0';
    sprintf(sn, "%s%04d", tmpsn, ++num);
}

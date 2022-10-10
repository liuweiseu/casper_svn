

/*
 * hubload_main.c
 *
 * main module for the EDT hubload program, the PCI Interface
 * Xilinx code loader for EDT LTX boards
 *
 * Copyright (C) 2003 EDT, Inc.
 */

#include "edtinc.h"
#include "pciload.h"
#include <stdlib.h>
#include <ctype.h>

char   *argv0;
char    dev[256];
char	flash_dir[128];
int     unit = -1;
int     sect;
int     proginfo = 0;
BFS     bitfile;
u_char  outbuf[MAX_BIT_SIZE];
Edt_bdinfo *boards;

#define LTX_OK		0
#define LTX_DEADHUB	1
#define LTX_NOHUBS	2

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
    "xc2s100",                  /* AMD_XC2S100_8M */
    "xc2s300e",                 /* AMD_XC2S300E   */
    ".",
    ".",
    ".",
    ".",
    ".",
    ".",
    NULL
};

#define MAX_PROMCODE 8 /* MUST be same as max valid idx in xilinx_dirname! */

char *xilinx_promstr[] =
{
    "unknown [0]",
    "4013e Xilinx, AMD 29F010 FPROM",
    "4013xla Xilinx, AMD 29F010 FPROM",
    "4028xla Xilinx, AMD 29F010 FPROM",
    "XC2S150 Xilinx, AMD 29LV040B FPROM",
    "XC2S200 Xilinx, AMD 29LV040B 4MB FPROM",
    "XC2S200 Xilinx, AMD 29LV081B 8MB FPROM",
    "XC2S100 Xilinx, AMD 29LV081B 8MB FPROM",
    "XC2S300e Xilinx, AMD 29LV081B 8MB FPROM",
    "unknown [9]",
    "unknown [10]",
    "unknown [11]",
    "unknown [12]",
    "unknown [13]",
    "unknown [14]",
    "unknown [15]",
    NULL
};


void
toupper_str(char *s)
{
    unsigned int i;

    for (i=0; i<strlen(s); i++)
	s[i] = toupper(s[i]);
}

char FnameRet[256];

char   *
fix_fname(char *fname, int promcode, int verbose)
{
    int     i, l;
    char    buf[256];
    char    buf2[256];
    char    extbuf[100];
    char   *promdir;
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
	if ((buf[i] == '/') || (buf[i] == '\\'))
	    haspath = 1;
    }

    strcpy(buf, fname);
    l = strlen(buf);
    if ((l > 4) && (buf[l-4] == '.')
     && (tolower(buf[l-3]) == 'b')
     && (tolower(buf[l-2]) == 'i')
     && (tolower(buf[l-1]) == 't'))
    {
	sprintf(extbuf, &buf[l-4]);
	buf[l-4] = '\0';
    }
    else strcpy(extbuf, ".bit");

    sprintf(buf2, "%s%s", buf, extbuf);

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

    if (verbose)
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
	case AMD_XC2S300E:
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
load_bitfile(char *fname, BFS *bitfile, int promcode, int verbose)
{
    /* this code borrowed from main() */
    FILE   *infd;
    u_char *data_start;
    long   offset;
    int     size = 0, got;

    /*
     * if ERASE don't do anything except copy the filename to the struct
     */
    if (strcmp(fname, "ERASE") != 0)
    {

	if (verbose) printf("bitfile %s\n", fname);

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
	    edt_msg(EDT_MSG_FATAL, "Error -- xilinx file truncated\n");
	    exit(2);
	}

	fclose(infd);

	if (verbose)
	    printf("read input file length is %d bytes\n", size);
	if (size < 16 * 1024)
	{
	    printf("not enough  data (%d bytes) in input file read\n", size);
	    exit(2);
	}

	data_start = &bitfile->data[0];
	bitfile->data_index = 0;

	printf("  File id: <%s>\n", bitfile->prom);

	/* end code borrow */
    }

    strcpy(bitfile->filename, fname);
    bitfile->filesize = size;
}


static void
usage(char *s)
{
    printf("\n");
    printf("hubload: update, verify or query LTX hub firmware\n\n");
    if (s && (*s))
	printf("%s\n\n", s);

    printf("usage:\n");
    printf("  hubload [-d dir] [-u unit] [-s sect] [-v] [-V] [verify|update|erase] [<file>]\n");
    printf("\n");
    printf("options:\n");
    printf("  -u specifies the unit number (default 0)\n");
    printf("     If -u is unspecified, hubload will attempt to detect boards\n");
    printf("  -h <0-15> sets hub number. If not specified, will cycle through all hubs found\n");
    printf("  -d specifies directory to look for files (default ./flash)\n");
    printf("  -s <0-3> sets the PROM sector number on boards using XLA parts\n");
    printf("     Default is to program appropriate unprotected sector(s)\n");
    printf("     with the specified file (or default if no file specified)\n");
    printf("  -v <filename> verify on-board firmware against file (will not burn new f/w)\n");
    printf("  -V sets verbose mode\n");
    printf("  -h this help/usage message\n");
    printf("\n");
    printf("  The last argument can be one of the keywords 'update' or 'verify', or\n");
    printf("  a filename.\n");
    printf("\n");
    printf("  If the keyword 'update' is present, hubload will attempt to find an up-\n");
    printf("  dated version of the hub's current firmware under the flash/ directory\n");
    printf("  and update the PROM with the contents of that file\n");
    printf("\n");
    printf("  If the keyword 'verify' is present, hubload will compare the firmware in\n");
    printf("  the PROM to the one in the file (optionally the -v flag can be used with\n");
    printf("  a filename as the last arg to compare to a specific firmware file)\n");
    printf("\n");
    printf("  If a filename is specified, hubload will search the current directory, then the\n");
    printf("  flash/ directory the appropriate version of the firmware file, and update to that\n");
    printf("  version (or compare if the '-v' option is specified). Absolute pathnames can\n");
    printf("  also be used for filename (but with caution).\n");

    printf("\n");
    printf("  If none of fname, 'update' 'verify' 'erase' is specified, hubload will\n");
    printf("  print out the prom ID for the board specified via the '-u' flag\n");
    printf("\n");
    printf("  If no arguments are specified, hubload finds all EDT boards and hubs in the\n");
    printf("  system and prints the board, hub #, prom ID and serial # info for each.\n");
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

static int
isdigit_str(char *s)
{
    unsigned int i;

    for (i=0; i<strlen(s); i++)
	if (!isdigit(s[i]))
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
	int len;
	printf("Enter board serial number (5-10 characters) %s%s%s > ",
						*sn?"[":"", sn, *sn?"]":"");
	fgets(s,127,stdin);

	if (*sn && !*s)
	    return;

	len = strlen(s);
	if ((s[len-1] == '\n') || (s[len-1] == '\r'))
	    s[len-1] = '\0';

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
    printf("\n");
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
    int len;
    int tries = 0;
    char s[128];
    char tmpstr[11];

    while (!ok)
    {
	printf("Enter 10-digit part # (with or without dashes) %s%s%s > ",
						*pn?"[":"", fmt_pn(pn), *pn?"]":"");
	fgets(s,127,stdin);
	if (*pn && !*s)
	    return;

	len = strlen(s);
	if ((s[len-1] == '\n') || (s[len-1] == '\r'))
	    s[len-1] = '\0';

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
    printf("\n");
}


#if 0 /* not needed in info since we know this */
void
ask_xilinx(int *xilinx)
{
    int i = 0;
    int ok = 0;
    int tries = 0;
    int n;
    char s[128];

    printf("Choose the xilinx from the following list\n\n");
    for (i=0; i<MAX_PROMCODE+1; i++)
	printf("%d: %s\n", i, xilinx_promstr[i]);

    while (!ok)
    {
	printf ("\nChoose one (0-%d)", MAX_PROMCODE);
	if (*xilinx >= 0)
	    printf(" [%d] > ", *xilinx);
	else printf(" > ", *xilinx);
	fgets(s,127,stdin);

	if (*xilinx >= 0 && !*s)
	    return;

	n = atoi(s);
	if (n < 0 || n > MAX_PROMCODE)
	{
	    if (++tries > 2)
	    {
		printf("3 tries, giving up\n");
		exit(1);
	    }
	    printf("\nInvalid entry -- must be a number from 0 to %d\n\n", MAX_PROMCODE);
	}
	else
	{
	    ok = 1;
	    *xilinx = n;
	}
    }
    printf("\n");
}
#endif

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
	printf ("\nEnter clock speed (usually 66)");

	if (*clock)
	    printf(" [%d] > ", *clock);
	else printf(" > ");

	fgets(s,127,stdin);

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
    printf("\n");
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
	printf ("\nEnter rev no.");

	if (*rev)
	    printf(" [%02d] > ", *rev);
	else printf(" > ");

	fgets(s,127,stdin);

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
    printf("\n");
}

/*
 * get the 10 digit option string
 */
void
ask_options(char *options)
{
    int ok = 0;
    int tries = 0;
    int len;
    char s[128];

    while (!ok)
    {
	printf("\nEnter options, if any (0-10 characters) %s%s%s > ",
				*options?"[":"", options, *options?"]":"");
	fgets(s,127,stdin);

	len = strlen(s);
	if ((s[len-1] == '\n') || (s[len-1] == '\r'))
	    s[len-1] = '\0';

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
    printf("This board is being programmed for the first time, or has an erased\n");
    printf("or corrrupted XILINX PROM. Entering the embedded info (clock\n");
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
	    printf("aborted\n");
	    exit(0);
	}
	printf("\nInvalid entry\n");
    }
}

int
parse_esn(char *str, Edt_embinfo *emb)
{
    unsigned int i;
	int n;
    int nfields=0;
    char sn[128], pn[128], rev[128], clock[128], opt[128];

    sn[0] = pn[0] = clock[0] = opt[0] = '\0';

    for (i=0; i<strlen(str); i++)
	if (str[i] == ':')
	    ++nfields;

    if (nfields < 4)
	return -1;

    n = sscanf(str, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:", sn, pn, clock, opt, rev);

    /* first pass we had 10-digit p/ns; still may be some around that have that */
    if ((strlen(sn) > 10) || (strlen(pn) > 10) || (strlen(opt) > 10) || !isdigit_str(clock))
	 return -1;

    if (n < 4)
	 return -1;

     if (n < 5)
	rev[0] = '\0';

    strcpy(emb->sn, sn);
    strcpy(emb->pn, pn);
    strcpy(emb->opt, opt);
    emb->clock = atoi(clock);
    emb->rev = atoi(rev);

    return 0;
}
/*
 * print the serial numbers
 */
print_sns(char *esn, char *osn)
{
    Edt_embinfo emb;

    memset(&emb,0,sizeof(emb)) ;

    if (*esn)
    {
	if (parse_esn(esn, &emb) == 0)
	{
#if 0
	    printf("  Serial #:    %s\n", emb.sn);
	    printf("  Part #:      %s\n", emb.pn);
	    printf("  Rev #:       %02d\n", emb.rev);
	    printf("  Clock:       %d\n", emb.clock);
	    printf("  Options: %s\n", *emb.opt? emb.opt, "(none)");
#else
	    printf("  s/n %s, p/n %s, rev %d clock %d Mhz", emb.sn, fmt_pn(emb.pn), emb.rev, emb.clock);
	    if (*emb.opt)
		printf(", opt %s", emb.opt);
#endif
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
ask_edt_info(Edt_embinfo *si)
{
    char s[128];
    int ok = 0;
    int clock_alert = 0;

#if 1
    si->clock = 0;
    si->rev = 0;
    memset(si->pn, 0, sizeof(si->pn));
    memset(si->sn, 0, sizeof(si->sn));
    memset(si->opt, 0, sizeof(si->opt));
#endif

    while (!ok)
    {
	printf("\n");
	ask_sn(si->sn);
	ask_pn(si->pn);
	ask_rev(&(si->rev));
	ask_clock(&(si->clock));
	ask_options(si->opt);

	if (si->clock != 66)
		clock_alert = 1;

	printf("Serial Number: %s\n", si->sn);
	printf("Part Number:   %c%c%c-%c%c%c%c%c-%c%c\n",
	    si->pn[0], si->pn[1], si->pn[2], si->pn[3], si->pn[4],
	    si->pn[5], si->pn[6], si->pn[7], si->pn[8], si->pn[9]);
	printf("Rev:           %02d\n", si->rev);
	printf("Clock:         %d Mhz%s\n", si->clock, clock_alert?" (WARNING: non-standard clock speed)":"");
	printf("Options:       %s\n", si->opt[0]?si->opt:"<none>");

	*s = '\0';
	while (tolower(*s) != 'y' && (tolower(*s) != 'n'))
	{
	    printf("\nOkay? [y/n/q] > ");
	    fgets(s,127,stdin);

	    if (*s == 'y')
		ok = 1;
	    else if (*s == 'q')
	    {
		printf("aborted\n");
		exit(0);
	    }
	    else if (*s != 'n')
		printf("\nEnter 'y' if the above information is correct, 'n' to change, q to quit\n");
	}
    }
}

void
check_ask_sn(EdtDev *edt_p, int promcode, int sect, char *fname, int vfonly, char *esn, char *new_esn, char *lcl_esn, char *pid, Edt_embinfo *embinfo, int *proginfo, int verbose)
{
    char osn[128];

    if (strcmp(fname, "ERASE") == 0)
	return;

    edt_readinfo(edt_p, promcode, sect, pid, esn, osn);
    parse_esn(esn, embinfo);

    if (*fname && !vfonly)
    {
	if (!*proginfo && (!*lcl_esn) && (!*pid))
	    *proginfo = ask_addinfo();

	if (*proginfo)
	{
	    ask_edt_info(embinfo);
	    memset(new_esn, 0, ESN_SIZE);
	    sprintf(new_esn, "%s:%s:%d:%s:%d:", embinfo->sn, embinfo->pn,
				embinfo->clock, embinfo->opt, embinfo->rev);
	    strcpy(esn, new_esn);
	}
    }
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
print_prominfo(EdtDev *edt_p, int promcode, int verbose)
{
    int ret = 0;
    char pid[MAX_STRING+1];
    char esn[ESN_SIZE];
    char osn[OSN_SIZE];

    switch (promcode)
    {
	case AMD_XC2S300E: /* 3v only */
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, verbose);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, verbose);
	    break;
	default:
	    printf("Unknown LTX Flash ROM - No information available\n");
	    ret = -1;
	    break;
    }
}

int     quiet = 0;
BFS    bitfile;

int
hubload_checkhub(int u, int hub, int sect, int verbose)
{
    EdtDev *edt_p;
    u_char  stat;
    u_int   frb;
    int     promcode;

    if ((edt_p = edt_open("pcd", boards[u].id)) == NULL)
    {
	char str[256];
	sprintf(str, "%s unit %d LTX hub %d", "pcd", boards[u].id, edt_ltx_hub(edt_p)) ;
	edt_perror(str) ;
	return -1;
    }

    edt_set_ltx_hub(edt_p, hub);
    promcode = edt_ltx_prom_detect(edt_p, &stat, &frb, verbose);
    if ((promcode == PROM_UNKN) && (frb != BT_A0))
    {
	printf("%s unit %d (%s): no LTX hubs connected to this device\n",
		boards[u].type, boards[u].id, edt_idstr(boards[u].bd_id));
	edt_close(edt_p);
	return LTX_NOHUBS;
    }

    edt_close(edt_p);

    if (promcode == PROM_UNKN)
	return LTX_DEADHUB;
    return LTX_OK;
}


int
hubload_summarize(int u, int hub, int sect, int verbose)
{
    EdtDev *edt_p;
    u_char  stat;
    u_int   frdata, frb;
    int     ret, promcode;
    char    lcl_esn[ESN_SIZE];
    char    lcl_osn[OSN_SIZE];

    if (boards[u].id >= 0)
    {
	if ((edt_p = edt_open("pcd", boards[u].id)) == NULL)
	{
	    char str[256];
	    sprintf(str, "%s unit %d LTX hub %d", "pcd", boards[u].id, edt_ltx_hub(edt_p)) ;
	    edt_perror(str) ;
	    return -1;
	}

        edt_set_ltx_hub(edt_p, hub);
	if ((ret = edt_check_ltx_hub(edt_p) == 0))
	{
	    promcode = edt_ltx_prom_detect(edt_p, &stat, &frb, verbose);
	    printf("%s unit %d (%s) LTX hub %d:\n", boards[u].type, boards[u].id,
					edt_idstr(boards[u].bd_id), hub);
	    printf("  %s\n", fmt_promcode(promcode));
	    edt_get_sns(edt_p, lcl_esn, lcl_osn);
	    print_sns(lcl_esn, lcl_osn);
	    print_prominfo(edt_p, promcode, verbose);
	    frdata = edt_reg_read(edt_p,EDT_FLASHROM_DATA) ;
	    print_flashstatus(stat, sect, frdata, verbose);
	    printf("\n");
	}

	edt_close(edt_p);

	if (ret == -1)	
	    return ret;
    }

    return 0;
}

/* fwd decl */
int hubload_program_verify(BFS *bitfile, int u, int h, int sect, char *fname, Edt_embinfo *embinfo, int vfonly, int proginfo, int verbose);
void hubload_reset_pcd(int unit);


#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;
hubload(char *command_line)
#else
int
main(int argc, char *argv[])
#endif
{
    int     i;
    int     ret = 0, verbose = 0, vfonly = 0;
    int     u, h;
    int     unit = -1, hub = -1;
    int     nunits = 0;
    int     verify = 0;
    int     gothub;
    int     minunit, maxunit;
    int     minhub, maxhub;
    char    fname[MAX_STRING+1];
    int     autoupdate = 0;
    char    errbuf[32];
    char   *unitstr = "";
    char    esn[ESN_SIZE];
    char    osn[OSN_SIZE];
    char    new_esn[ESN_SIZE];
    char    new_osn[OSN_SIZE];
    EdtDev *edt_p = NULL;
    Edt_embinfo embinfo;
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("hubload",command_line,&argc,&argv);
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
	    case 'v':
		verify = verify ? 0 : 1;
		break;
	    case 'V':
		verbose = verbose ? 0 : 1;
		break;
	    case 'i':
		proginfo = 1;
		break;
	    case 'q':
		quiet=1;
		break;
	    case 'h':
		if (++i == argc)
		    usage("unfinished -h option");
		else hub = atoi(argv[i]);
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

	    default:
		sprintf(errbuf, "\nunknown option \"-%c\"", argv[i][1]);
		usage(errbuf);
	    }
	}
    }
    if (*unitstr)
    {
	unit = edt_parse_unit(unitstr, "pcd", NULL);
    }

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
	strcpy(new_osn, "ERASE");
	strcpy(new_esn, "ERASE");
    }

    else if (strcmp(fname, "verify") == 0)
    {
	*fname = 0;
	verify = 1;
    }

    else if (strcmp(fname, "help") == 0)
    {
	usage(errbuf);
	exit(0);
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

    printf("checking for EDT LTX boards...");
    fflush(stdout);
    boards = edt_detect_boards_ltx(dev, unit, &nunits, verbose);
    for (i=0; i<NUM_DEVICE_TYPES; i++)
	if (boards[i].bd_id <= 0)
	    break;


    if (nunits < 1)
    {
	printf(" none found\n");
	printf("\nIf you have an EDT board in the system, make sure it supports LTX\n"); 
	printf("(HSS, FOX or SS4), and that the appropriate bitfile is loaded (usually ssltx)\n\n");
	exit(0);
    }
    printf(" %d LTX (HSS, FOX or SS4) board%s found\n", nunits, nunits == 1?"":"s"); 

    /*
     * no unit, default is all units unless hub specified, then default to unit 0
     */
    if (unit <= 0)
    {
	minunit = 0;
	maxunit = nunits-1;
    }
    else minunit = maxunit = unit;

    if (hub >= 0)
	minhub = maxhub = hub;
    else
    {
	minhub = 0;
	maxhub = 15;
    }

    for (u = minunit; u < maxunit+1; u++)
    {
	hubload_reset_pcd(boards[u].id);

	gothub = 0;
	for (h = minhub; h < maxhub+1; h++)
	{
	    int hc;

	    /* printf("checking unit %d hub %d\r", u, h); fflush(stdout); */

	    if ((hc = hubload_checkhub(u, h, sect, verbose)) == LTX_NOHUBS)
	    {
		h = maxhub+1;
		continue;
	    }

	    if (hc == LTX_OK)
	    {
		gothub = 1;
		printf("\n");
		if (verify)
		{
		    hubload_summarize(u, h, sect, verbose);
		    ret = hubload_program_verify(&bitfile, u, h, sect, fname, &embinfo, 1, 0, verbose);
		}
		else if (*fname)
		{
		    hubload_summarize(u, h, sect, verbose);
		    ret = hubload_program_verify(&bitfile, u, h, sect, fname, &embinfo, 0, proginfo, verbose);
		}
		else
		{
		    hubload_summarize(u, h, sect, verbose);
		}
	    }
	}
	if (ret < 0)
	{
	    printf("\ngot a failure; exiting early\n");
	    exit(1);
	}
	if (!gothub)
	{
	    if (minhub == maxhub)
		printf(" hub %d not found on unit %d\n", minhub);
	    else printf("no hubs found on unit %d\n", u);
	}
    }

}

void
hubload_reset_pcd(int unit)
{
    EdtDev *edt_p;
    int     sunswap = !edt_little_endian();

    if ((edt_p = edt_open("pcd", unit)) == NULL)
    {
	char str[256];
	sprintf(str, "%s unit %d", "pcd", unit) ;
	edt_perror(str) ;
	return;
    }

    edt_reg_write(edt_p, PCD_CMD, 0x8);
    edt_msleep(500);		/* wait for the TLK to syn up */

    /* enable tlk output */
    edt_reg_write(edt_p, PCD_LTX_CMD, 0xf | (sunswap << 4));
    edt_close(edt_p);
}


int
hubload_program_verify(BFS *bitfile, int u, int h, int sect, char *fname, Edt_embinfo *embinfo, int vfonly, int proginfo, int verbose)
{
    u_char  stat;
    u_int   frb;
    int     ver;
    char    pid[MAX_STRING+1];
    char    esn[ESN_SIZE];
    char    osn[OSN_SIZE];
    char    lcl_esn[ESN_SIZE];
    char    new_esn[ESN_SIZE];
    char    *newfname;
    EdtDev *edt_p = edt_open(boards[u].type, boards[u].id);
    int promcode;

    if (edt_p == NULL)
    {
	char str[256];
	sprintf(str, "%s unit %d LTX hub %d", "pcd", boards[u].id, h) ;
	edt_perror(str) ;
	return -1;
    }

    edt_set_ltx_hub(edt_p, h);
    promcode = edt_ltx_prom_detect(edt_p, &stat, &frb, verbose);

    if ((promcode == PROM_UNKN) && (frb != BT_A0))
    {
	printf("%s unit %d (%s): no LTX hubs connected to this device\n",
		boards[u].type, boards[u].id, edt_idstr(boards[u].bd_id));
	edt_close(edt_p);
	return -1;
    }

    switch (promcode)
    {
	case AMD_XC2S300E:

	if (!*fname)
	{
	    printf("  Sector 0");
	    getinfo(edt_p, promcode, 0, pid, esn, osn, verbose);
	    printf("  Sector 2");
	    getinfo(edt_p, promcode, 2, pid, esn, osn, verbose);
	}
	else
	{
	    if (sect == IS_DEFAULT_SECTOR)
	    {
		/* program sect 2 with 3.3volt file */
		newfname = fix_fname(fname, promcode, verbose);
		load_bitfile(newfname, bitfile, promcode, verbose);
		check_ask_sn(edt_p, promcode, 2, fname, vfonly, esn, new_esn, lcl_esn, pid, embinfo, &proginfo, verbose);
		ver = program_verify_XC2S300E(edt_p, bitfile, "pcd", pid, esn, osn, 2, vfonly, 1, verbose);

	    }
	    else
	    {
		load_bitfile(fname, bitfile, promcode, verbose);
		check_ask_sn(edt_p, promcode, sect, fname, vfonly, esn, new_esn, lcl_esn, pid, embinfo, &proginfo, verbose);
		ver = program_verify_XC2S300E(edt_p, bitfile, "pcd", pid, esn, osn, sect, vfonly, 1, verbose);
	    }
		if ((ver == 0) & !vfonly)
		    edt_ltx_reboot(edt_p);
	}

	break;
    }

    edt_close(edt_p);
    return 0;
}


/*
 * pciload_fn.c
 *
 */
#include "edtinc.h"
#include "edt_bitload.h"
#include "pciload.h"
#include <stdlib.h>

static void parse_id(char *idstr, u_int * date, u_int * time);
static void remove_char(char *str, char ch);

void
getinfo(EdtDev *edt_p, int promcode, int segment, char *id, char *esn, char *osn, int verbose)
{
    edt_readinfo(edt_p, promcode, segment, id, esn, osn);

	if (*id)
	    printf("  PROM  id: <%s>", id);

#if 0
    if (verbose)
    {
	if (*esn)
	    printf(" sn <%s>", esn);

	if (*osn)
	    printf(" OEM sn <%s>", osn);
    }
#endif

    printf("\n");
}

void 
warnuser(char *dev, char *fname)
{
    char    s[32];
    char    *foa = "file";

#ifdef NO_FS
	if ((strlen(fname) < 4) || (strcmp(&fname[strlen(fname)-4], ".bit") != 0))
	    foa = "embedded array";
#endif

    if (strcmp(fname, "ERASE") == 0)
	return;

    printf("\n");
    printf("WARNING: pciload is preparing to re-burn the PCI firmware on the %s board\n", dev);
    printf("with the contents of the %s '%s'. This is\n", foa, fname);
    printf("NOT the same as running bitload, which only affects the external device\n");
    printf("interface firmware.\n");
    printf("\nIf you are sure you want to upgrade the board's PCI firmware,\n");
    printf("press <enter> now. Otherwise, press 'q', then <enter> -> ");

    fgets(s, 31, stdin);
    if ((tolower(s[0]) != 'y') && (s[1])) /* allow 'y' since its what a lot of people do automatically */
    {
	printf("exiting.\n");
	exit(0);
    }
}

void 
warnuser_ltx(char *dev, char *fname, int unit, int hub)
{
    char    s[32];

    if (strcmp(fname, "ERASE") == 0)
	return;

    printf("\n");
    printf("WARNING: hubload is preparing to re-burn the firmware on %s%d hub #%d\n", dev, unit, hub);
    printf("with the contents of the file '%s'. \n", fname);
    printf("\nIf you are sure you want to upgrade the hub's firmware,\n");
    printf("press <enter> now. Otherwise, press 'q', then <enter> -> ");

    fgets(s, 31, stdin);
    if ((tolower(s[0]) != 'y') && (s[1])) /* allow 'y' since its what a lot of people do automatically */
    {
	printf("exiting.\n");
	exit(0);
    }
}


void
program_sns(EdtDev *edt_p, int promcode, char *esn, char *osn, int sector, char *id, int verbose)
{
    int     i;
    int     xtype;
    int     id_addr, osn_addr, esn_addr;
    char    sn_str[ESN_SIZE];

    id_addr = edt_get_id_addr(promcode, sector, &xtype);
    osn_addr = id_addr - OSN_SIZE;
    esn_addr = osn_addr - ESN_SIZE;

    if (id_addr == 0)
    {
	printf("invalid device; no ID info\n");
	return;
    }

    if (verbose) printf("id addr %x\n", id_addr);

    if (*osn)
    {
	if (verbose) printf("program osn <%s> at %x\n", osn, osn_addr);

	edt_x_reset(edt_p, xtype);

	/* clear out the string */
	for (i=0; i<OSN_SIZE; i++)
	    sn_str[i] = '\0';

	/* special case "ERASE" just erases the s/n */
	if (strncasecmp(osn, "erase", 5) != 0)
	    sprintf(sn_str, "OSN:%s", osn);

	if (verbose) printf("oem sn %s size %d\n", osn, OSN_SIZE);
	for (i = 0; i < OSN_SIZE; i++)
	{
	    edt_x_byte_program(edt_p, osn_addr, sn_str[i], xtype);
	    osn_addr++;
	}
    }

    if (*esn)
    {
	if (verbose) printf("program esn <%s> at %x\n", esn, esn_addr);
	edt_x_reset(edt_p, xtype);

	/* clear out the string */
	for (i=0; i<ESN_SIZE; i++)
	    sn_str[i] = '\0';

	/* special case "ERASE" just erases the s/n */
	if (strncasecmp(esn, "erase", 5) != 0)
	    sprintf(sn_str, "ESN:%s", esn);

	if (verbose) printf("edt sn %s size %d\n", esn, ESN_SIZE);
	for (i = 0; i < ESN_SIZE; i++)
	{
	    edt_x_byte_program(edt_p, esn_addr, sn_str[i], xtype);
	    esn_addr++;
	}
    }
}

/*
 * check the bitfile ID against the PROM id.
 * If verify_only flag is set just print a message saying they differ.
 * If bitfile date is older than PROM date, print a warning.
 *
 * ARGUMENTS
 *    bitfile_id	id string from bitfile
 *    prom_id           id string from prom
 *    verify_only       flag whether we're verifying (print just "differ")
 *                      or burning (print warning if bitfile older than PROM
 *
 */
void 
check_id_times(char *bitfile_id, char *prom_id, int *verify_only, char *fname)
{
    u_int   fdate, ftime, pdate, ptime;
    char    s[MAX_STRING];

    if (*verify_only)
    {
	if (strncmp(bitfile_id, prom_id, MAX_STRING) != 0)
	{
	    printf("  (file ID and prom ID differ)\n");
	}
    }
    else
    {
	parse_id(bitfile_id, &fdate, &ftime);
	parse_id(prom_id, &pdate, &ptime);

	if (strcmp(fname, "ERASE") == 0)
	{
	    printf("\nPreparing to ERASE all EEprom info including board ID! To confirm,\n");
	    printf("press <enter> now.  Otherwise, press 'q', then <enter> -> ");
	    fflush(stdout);
	    fgets(s, MAX_STRING - 1, stdin);
	    if (s[1])
	    {
		printf("exiting\n");
		exit(0);
	    }
	}
	else if (fdate < pdate)
	{
	    printf("\nData in file is older than data in PROM! To program anyway,\n");
	    printf("press <enter> now.  Otherwise, press 'q', then <enter> -> ");
	    fflush(stdout);
	    fgets(s, MAX_STRING - 1, stdin);
	    if (s[1])
	    {
		printf("exiting\n");
		exit(0);
	    }
	}
	else if (fdate == pdate)
	{
	    if (ftime < ptime)
	    {
		printf("Data in file is older than data in PROM! To program anyway,\n");
		printf("press <enter> now.  Otherwise, press 'q', then <enter> ->");
		fflush(stdout);
		fgets(s, MAX_STRING - 1, stdin);
		if (s[1])
		{
		    printf("exiting.\n");
		    exit(0);
		}
	    }
	}
    }
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
    char    lcaname[32];
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
print_flashstatus(char stat, int sector, int frdata, int verbose)
{
    if (verbose)
	printf("  FLASHROM_DATA %08x\n", frdata);
    if (verbose)
    {
	if (sector != IS_DEFAULT_SECTOR)
	{
	    printf("  Using sector %d\n", sector);
	}
	printf("  Flash environment status = %02x:\n", stat & 0xff);

	if (stat & EDT_ROM_JUMPER)
	    printf("    - Protected boot jumper removed or on pin 2-3\n");
	else
	    printf("    - Protected boot jumper INSTALLED pins 1-2\n");

	if (stat & EDT_5_VOLT)
	    printf("    - Installed in 5 volt slot\n");
	else
	    printf("    - Installed in 3.3 volt slot\n");
    }
    else if (!(stat & EDT_ROM_JUMPER))
    {
	printf("  WARNING: Protected boot jumper installed pins 1-2! Protected\n");
	printf("           sectors may not contain the most recently burned FPGA.\n");
    }
}

/*
 * ask whether to reboot the xilinx, do so if yes
 *
 * RETURNS 1 on success, 0 on no, -1 on failure
 */
int
ask_reboot(EdtDev *edt_p)
{
    char s[128];

    printf("Done. ");
    while(1)
    {
	printf("Attempt to reboot the %s board now? (y/n/h)[y] > ", edt_idstr(edt_p->devid));
	fgets(s,127,stdin);
	if (tolower(s[0] == 'y') || s[0] == '\0')
	{
	    if (edt_pci_reboot(edt_p) != 0)
	    {
		printf("Couldn't reboot the xilinx; probably no s/w reboot support in this\nparticular bitfile.\n");
		return -1;
	    }
	    return 1;
	}
	else if (tolower(s[0] == 'h'))
	{
	    printf("\nAccepting this option will attempt to reboot the board, forcing it to reload from\n");
	    printf("the newly-updated PROM. This capability is only present in some xilinx versions; if\n");
	    printf("successful it precludes the necessity of cycling power after a firmware upgrade.\n");
	    printf("If you accept this option and it doesn't succeed, simply halt the system and cycle\n");
	    printf("power (previously the only way to do it).\n\n");
	}
	else if (tolower(s[0] == 'n'))
	    return 0;
    }
}

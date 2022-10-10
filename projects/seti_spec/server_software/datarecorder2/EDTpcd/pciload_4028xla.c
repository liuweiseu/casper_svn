#include "edtinc.h"

#include "pciload.h"
#include <stdlib.h>

void program(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *new_esn, char *new_osn, int segment, int sectors, int promcode, int xtype, int verbose);
int verify(EdtDev *edt_p, int promcode, BFS *bitfile, char *dev, char *pid, int segment, int sectors, int xtype, int verbose);
/*
 * XC2S200 uses the 29LV081B -- 8 sectors per bitfile
 * All others use the 29LV040B -- XC2S200_4M uses 4 sectors per bitfile,
 * 4028XLA and XC2S150 use 2
 */
int
program_verify_XC2S300E(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose)
{
    return program_verify(edt_p, bitfile, dev, pid, esn, osn, segment, 4, verify_only, warn, verbose, AMD_XC2S300E, XTYPE_LTX);
}
int
program_verify_XC2S200(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose)
{
    return program_verify(edt_p, bitfile, dev, pid, esn, osn, segment, 4, verify_only, warn, verbose, AMD_XC2S200_4M, XTYPE_BT);
}
int
program_verify_XC2S150(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose)
{
    return program_verify(edt_p, bitfile, dev, pid, esn, osn, segment, 2, verify_only, warn, verbose, AMD_XC2S150, XTYPE_BT);
}
int
program_verify_4028XLA(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose)
{
    return program_verify(edt_p, bitfile, dev, pid, esn, osn, segment, 2, verify_only, warn, verbose, AMD_4028XLA, XTYPE_BT);
}

/*
 * program the as follows:
 * 4085: 29LV040B 4Meg PROM, 8 64K sectors:
 * 0 & 1: 3.3 volt protected boot code sector
 * 2 & 3: 5 volt protected code sector
 * 4 & 5: 3.3 volt current code
 * 6 & 7: 5 volt current code
 *
 * XC2S200_4M: 29LV040B 4Meg PROM, 8 64K sectors:
 * 0-3: 3.3 volt current code
 * 4-7: 5 volt current code
 * (no protected mode sectors)
 *
 * XC2S200: 29LV081B 8 Meg PROM, 16 64K sectors:
 * with a 29LV081B, which has 16 64K sectors:
 * 0-3: 3.3 volt protected boot code sector
 * 4-7: 5 volt protected code sector
 * 8-11: 3.3 volt current code
 * 12-15: 5 volt current code
 *
 * The id string is stored in the last 128 bytes of the last sector the code is in.
 *
 * RETURNS: exits on failure, returns 0 on success or 1 if successfull and xilinx rebooted
 */
int 
program_verify(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *new_esn, char *new_osn, int segment, int sectors, int verify_only, int warn, int verbose, int promcode, int xtype)
{
    int     size = bitfile->filesize;
    char esn[128], osn[128];
    extern int quiet ;

    if ((strcmp(bitfile->filename, "ERASE") != 0) && (size < 80 * 1024))
    {
	printf("  bit file %s is too small(%d bytes) for 4028XLA\n",
	       bitfile->filename, bitfile->filesize);
	exit(2);
    }

    /*
     * printf("  %sing board with boot controller\n",
     * verify_only?"Verify":"Program");
     */

    if (verbose)
	printf("  preparing to %s logical sector %d, %d physical sectors starting at %d.\n",
	   verify_only ? "verify" : "re-burn", segment, sectors, segment * sectors);

    if (warn || verify_only)
    {
	getinfo(edt_p, promcode, segment, pid, esn, osn, verbose);
	check_id_times(bitfile->prom, pid, &verify_only, bitfile->filename);
    }

    if (!verify_only)
    {
	if (warn && !quiet)
	if (xtype == XTYPE_LTX)
	    warnuser_ltx(dev, bitfile->filename, edt_p->unit_no, edt_ltx_hub(edt_p));
	else warnuser(dev, bitfile->filename);

	program(edt_p, bitfile, dev, pid, new_esn, new_osn, segment, sectors, promcode, xtype, verbose);
	printf("\n");
    }

    if (strcmp(bitfile->filename, "ERASE") == 0)
	return 0;
    return verify(edt_p, promcode, bitfile, dev, pid, segment, sectors, xtype, verbose);
}


void
program(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *new_esn, char *new_osn, int segment, int sectors, int promcode, int xtype, int verbose)
{
    int     size = bitfile->filesize;
    char   *id = bitfile->prom;
    int     idsize = strlen(id);
    u_char *data_start = &bitfile->data[bitfile->data_index];
    int     i, s;
    u_char  val, flipval;
    u_char *inptr;
    char    esn[ESN_SIZE], osn[OSN_SIZE];
    int     addr;

    getinfo(edt_p, promcode, segment, pid, esn, osn, verbose);

    if (idsize > MAX_STRING)
    {
	printf("  prom ID size out of range (%d)\n", idsize);
	exit(2);
    }

    if (*new_esn)
	strcpy(esn, new_esn);

    if (*new_osn)
	strcpy(osn, new_osn);

    /* put id at the end of sector */
    fflush(stdout);
    printf("  erasing segment %d, sector", segment);
    for (s=0; s<sectors; s++)
    {
	printf(" %d", segment * sectors + s);
	fflush(stdout);
	edt_x_reset(edt_p, xtype);
	edt_sector_erase(edt_p, segment * sectors + s, XLA_SECTOR_SIZE, xtype);
    }
    printf("\n");

    program_sns(edt_p, promcode, esn, osn, segment, id, verbose);

    edt_x_reset(edt_p, xtype);

    if (strcmp(bitfile->filename, "ERASE") == 0)
    {
	if (verbose)
	    printf("  erased %08x - %08x\n", segment * sectors + s, segment * sectors + s + XLA_SECTOR_SIZE);
    }
    else
    {
	addr = ((segment + 1) * sectors * XLA_SECTOR_SIZE) - idsize - 1;
	if (verbose)
	    edt_x_print16(edt_p, segment * sectors * XLA_SECTOR_SIZE, xtype);
	if (verbose) 
	    printf("  id string %s size %d\n", id, idsize);
	printf("  programming segment %d (%d sectors): id", segment, sectors);
	fflush(stdout);
	for (i = 0; i < idsize; i++)
	{
	    if (!(i % 10))
	    {
		printf(".");
		fflush(stdout);
	    }

	    edt_x_byte_program(edt_p, addr, id[i], xtype);
	    addr++;
	}

	printf(" data");
	fflush(stdout);
	edt_x_byte_program(edt_p, addr, 0, xtype);
	edt_x_reset(edt_p, xtype);

	/* program data at start of sector */
	addr = segment * sectors * XLA_SECTOR_SIZE;

	/* skip the header */
	inptr = data_start;
	if (verbose)
	    printf("\n  programming to %x\n", segment * sectors * XLA_SECTOR_SIZE + size);
	for (i = 0; i < size; i++)
	{
	    val = *inptr;
	    flipval = edt_flipbits(val);

	    edt_x_byte_program(edt_p, addr, flipval, xtype);

	    if (!(i % 0x5000))
	    {
		printf(".");
		fflush(stdout);
	    }
	    else if (verbose && ((i % 0x100) == 0))
	    {
		printf("  %08x\r", addr);
		fflush(stdout);
	    }
	    addr++;
	    inptr++;
	}
	if (verbose)
	    printf("  last address programed %08x\n", addr);
    }
}

/*
 * verify the xilinx in the prom against the one in the file 
 *
 * RETURNS # of errors or 0 on success
 */
int
verify(EdtDev *edt_p, int promcode, BFS *bitfile, char *dev, char *pid, int segment, int sectors, int xtype, int verbose)
{
    int     size = bitfile->filesize;
    char   *id = bitfile->prom;
    int     idsize = strlen(id);
    u_char *data_start = &bitfile->data[bitfile->data_index];
    int     i, idx;
    int     errs = 0;
    u_char  val, flipval;
    u_char *inptr;
    int     addr;

    if (idsize > MAX_STRING)
    {
	printf("  prom ID size out of range (%d)\n", idsize);
	exit(2);
    }

    /* reset start */
    edt_x_reset(edt_p, xtype);

    addr = segment * sectors * XLA_SECTOR_SIZE;
    /* skip the header */
    inptr = data_start;
    if (verbose)
	printf("  verifying from %x to %x\n", addr,
	   segment * sectors * XLA_SECTOR_SIZE + size);
    else
    {
	printf("  verifying");
	fflush(stdout);
    }

    idx = 0;
    for (i = 0; i < size; i++)
    {
	u_char  tmpval;

	val = *inptr;
	flipval = edt_flipbits(val);
	tmpval = edt_x_read(edt_p, addr, xtype);
	if (tmpval != flipval)
	{
	    errs++;
	    if (verbose && errs < 16)
	    {
		printf("  error %d address %08x was %02x s/b %02x\n",
		       errs, addr, tmpval, flipval);
		tmpval = edt_x_read(edt_p, addr, xtype);
		printf("  second read %02x\n", tmpval);
	    }
	    if (verbose && errs == 16)
		printf("  printing errors halted at 16\n");

	}
	if (verbose)
	{
	    if (i < 32)
	    {
		printf("%02x ", tmpval);
		if ((++idx % 16) == 0)
		    printf("\n(0x%x)\n", addr);
	    }
	    if (i > size - 32)
	    {
		printf("%02x ", tmpval);
		if ((++idx % 16) == 0)
		    printf("\n(0x%x)\n", addr);
	    }
	}
	if (!(i % 20000))
	{
	    printf(".");
	    fflush(stdout);
	}
	addr++;
	inptr++;
    }
    printf(" %d errors\n\n", errs);
    edt_x_reset(edt_p, xtype);

    return (errs);
}

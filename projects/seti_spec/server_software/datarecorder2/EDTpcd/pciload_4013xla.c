#include "edtinc.h"
#include "pciload.h"
#include <stdlib.h>



/*
 * program the 4013xla board with a 29LV040B it has 8 64K sectors. sector 0
 * has the 3.3 volt protected boot code sector 1 has the 3.3 volt current
 * code sector 2 has the 5 volt protected code sector 3 has the 5 volt
 * current code sector 4-7 are not used and reserved, The current xilinx can
 * not address them. They might be used for a larger xilinx as an additional
 * 64k bytes on top of sectors 0-3. the id string is stored in the last 128
 * bytes of the sector the code is in.
 */
int 
program_verify_4013XLA(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *new_esn, char *new_osn, int sector, int verify_only, int warn, int verbose)
{

    u_char *data_start = &bitfile->data[bitfile->data_index];
    int     size = bitfile->filesize;
    char   *id = bitfile->prom;
    int     idsize = strlen(id);
    extern int quiet ;
    int     i, idx;
    char    esn[ESN_SIZE], osn[OSN_SIZE];
    int     errs = 0;
    u_char  val, flipval;
    u_char *inptr;
    int id_addr;

    /* code borrowed from main() */
    if ((strcmp(bitfile->filename, "ERASE") != 0) && (size < 32 * 1024))
    {
	printf("  bit file %s is to small(%d bytes) for 4013XLA\n",
	       bitfile->filename, bitfile->filesize);
	exit(2);
    }
    if (idsize > MAX_STRING)
    {
	printf("  prom ID size out of range (%d)\n", idsize);
	exit(2);
    }

    getinfo(edt_p, AMD_4013XLA, sector, pid, esn, osn, verbose);

    /*
     * serial numbers -- reprogram with new serial number(s) if specified.
     * if not specified but present in existing PROM, then check for
     * validity and reprogram with existing
     */
    if (*new_esn)
	strcpy(esn, new_esn);

    if (*new_osn)
	strcpy(osn, new_osn);

    if (verbose)
	printf("  preparing to %s (logical) sector %d (physical sectors %d and %d).\n",
    verify_only ? "verify" : "re-burn", sector, sector * 2, sector * 2 + 1);

    if (warn || verify_only)
	check_id_times(bitfile->prom, pid, &verify_only, bitfile->filename);

    /* code borrowed from prog_4013xla() */

    /*
     * program the xilinx
     */
    if (!verify_only)
    {
	if (warn && !quiet)
	    warnuser(dev, bitfile->filename);

	printf("\n  erasing/programming sector %d\n", sector);
	edt_sector_erase(edt_p, sector, XLA_SECTOR_SIZE, XTYPE_X);
	program_sns(edt_p, AMD_4013XLA, esn, osn, sector, id, verbose);
	edt_x_reset(edt_p, XTYPE_X);

	if (strcmp(bitfile->filename, "ERASE") != 0)
	{
	    if (verbose)
		edt_x_print16(edt_p, sector * XLA_SECTOR_SIZE, XTYPE_X);
	    printf("  id string %s size %d\n", id, idsize);
	    id_addr = (sector * XLA_SECTOR_SIZE) + XLA_SECTOR_SIZE - idsize - 1;
	    for (i = 0; i < idsize; i++)
	    {
		edt_x_byte_program(edt_p, id_addr, id[i], XTYPE_X);
		id_addr++;
	    }
	    edt_x_byte_program(edt_p, id_addr, 0, XTYPE_X);

	    /*
	     * finally, program the data (at start of sector)
	     */
	    edt_x_reset(edt_p, XTYPE_X);
	    id_addr = sector * XLA_SECTOR_SIZE;
	    /* skip the header */
	    inptr = data_start;
	    printf("  programming to %x\n", sector * XLA_SECTOR_SIZE + size);
	    for (i = 0; i < size; i++)
	    {
		val = *inptr;
		flipval = edt_flipbits(val);
		edt_x_byte_program(edt_p, id_addr, flipval, XTYPE_X);
		if (verbose && ((i % 0x100) == 0))
		{
		    printf("%08x\r", id_addr);
		    fflush(stdout);
		}
		id_addr++;
		inptr++;
	    }
	    if (verbose)
		printf("%08x\n", id_addr);
	}
    }
    edt_x_reset(edt_p, XTYPE_X);

    if (strcmp(bitfile->filename, "ERASE") != 0)
    {
	/* Verify */
	/* reset start */
	id_addr = sector * XLA_SECTOR_SIZE;
	/* skip the header */
	inptr = data_start;
	printf("  verifying to %x... ", sector * XLA_SECTOR_SIZE + size);
	idx = 0;
	for (i = 0; i < size; i++)
	{
	    u_char  tmpval;

	    val = *inptr;
	    flipval = edt_flipbits(val);
	    tmpval = edt_x_read(edt_p, id_addr, XTYPE_X);
	    if (tmpval != flipval)
	    {
		errs++;
	    }
	    if (verify_only && verbose)
	    {
		if (i < 32)
		{
		    printf("%02x ", tmpval);
		    if ((++idx % 16) == 0)
			printf("\n(0x%x)\n", id_addr);
		}
		if (i > size - 32)
		{
		    printf("%02x ", tmpval);
		    if ((++idx % 16) == 0)
			printf("\n(0x%x)\n", id_addr);
		}
	    }
	    id_addr++;
	    inptr++;
	}
	printf("%d errors\n\n", errs);
	if (verbose)
	    printf("%08x\n\n", id_addr);
	edt_x_reset(edt_p, XTYPE_X);
    }

    return errs;
}

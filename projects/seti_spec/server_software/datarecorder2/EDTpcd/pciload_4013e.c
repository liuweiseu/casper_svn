#include "edtinc.h"
#include "pciload.h"
#include <stdlib.h>


/*
 * program the 4013e board with a 29F010 it has 8 16K sectors. sector 0 and 1
 * have the protected code sectoe 4 and 5 are reprogrammed with the current
 * code sector 7 has the id string
 */
int 
program_verify_4013E(EdtDev * edt_p, BFS *bitfile, char *dev, char *pid, char *new_esn, char *new_osn, int verify_only, int warn, int verbose)
{

    u_char *data_start = &bitfile->data[bitfile->data_index];
    int     size = bitfile->filesize;
    char   *id = bitfile->prom;
    int     idsize = strlen(id);
    char    esn[ESN_SIZE], osn[OSN_SIZE];
    extern int quiet ;

    int     i, idx;
    int     errs = 0;
    u_char  val, flipval;
    u_char *inptr;
    int     id_addr = (8 * E_SECTOR_SIZE) - ID_SIZE;	

    /* check for ok values */

    if ((strcmp(bitfile->filename, "ERASE") != 0) && (bitfile->filesize > 32 * 1024))
    {
	printf("  bit file %s is too large(%d bytes) for 4013e\n",
	       bitfile->filename, bitfile->filesize);
	exit(2);
    }
    if (idsize > MAX_STRING)
    {
	printf("  prom ID size out of range (%d)\n", idsize);
	exit(2);
    }


    getinfo(edt_p, AMD_4013E, 0, pid, esn, osn, verbose);

    if (*new_esn)
	strcpy(esn, new_esn);

    if (*new_osn)
	strcpy(osn, new_osn);

    if (verbose) 
	printf("  preparing to %s (logical) sector 3 (physical sectors 4 and 5).\n",
	   verify_only ? "verify" : "re-burn");

    if (warn || verify_only)
	check_id_times(bitfile->prom, pid, &verify_only, bitfile->filename);

    if (!verify_only)
    {
	if (warn && !quiet)
	    warnuser(dev, bitfile->filename);

	/* put id at the end of sector 7 */
	id_addr = (8 * E_SECTOR_SIZE) - idsize;
	printf("  erasing/programming sector 7 id\n");
	edt_sector_erase(edt_p, 7, E_SECTOR_SIZE, XTYPE_X);
	program_sns(edt_p, AMD_4013E, esn, osn, sect, id, verbose);
	edt_x_reset(edt_p, XTYPE_X);
	for (i = 0; i < idsize; i++)
	{
	    edt_x_byte_program(edt_p, id_addr, id[i], XTYPE_X);
	    id_addr++;
	}
	edt_x_reset(edt_p, XTYPE_X);
	printf("  erasing sector 4\n");
	edt_sector_erase(edt_p, 4, E_SECTOR_SIZE, XTYPE_X);
	edt_x_reset(edt_p, XTYPE_X);
	printf("  erasing sector 5\n");
	edt_sector_erase(edt_p, 5, E_SECTOR_SIZE, XTYPE_X);
	edt_x_reset(edt_p, XTYPE_X);

	/* start data in sector 4 */
	id_addr = 4 * E_SECTOR_SIZE;
	if (strcmp(bitfile->filename, "ERASE") == 0)
	{
	    if (verbose)
		printf("  erased %08x - %08x\n", id_addr, id_addr + size);
	}
	else
	{
	    /* skip the header */
	    inptr = data_start;
	    printf("  programming to %x\n", 4 * E_SECTOR_SIZE + size);
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

    if (strcmp(bitfile->filename, "ERASE") != 0)
    {

	edt_x_reset(edt_p, XTYPE_X);

	/* Verify */
	/* start at  sector 4 */
	id_addr = 4 * 0x4000;
	/* skip the header */
	inptr = data_start;
	printf("  verifying to %x... ", 4 * 0x4000 + size);
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
	    printf("%08x\n", id_addr);

	edt_x_reset(edt_p, XTYPE_X);
    }

    return errs;
}

/* #pragma ident "@(#)setdebug.c	1.73 03/14/02 EDT" */


#include "edtinc.h"
#include <stdlib.h>

#define edt_swab32(x) \
(\
	((u_int)( \
		(((u_int)(x) & (u_int)0x000000ffUL) << 24) | \
		(((u_int)(x) & (u_int)0x0000ff00UL) <<  8) | \
		(((u_int)(x) & (u_int)0x00ff0000UL) >>  8) | \
		(((u_int)(x) & (u_int)0xff000000UL) >> 24) )) \
)

EdtDev *edt_p;

u_int   tracebuf[EDT_TRACESIZE];

int     curcount;

#include "edtregbits.h"

void
GetFlagsString(char *s, BitLabels * labels, long flags)

{

    int     i = 0;
    int     started = 0;

    s[0] = 0;

    for (i = 0; labels[i].label != NULL; i++)
    {
	if (flags & labels[i].bit)
	{
	    if (started)
		sprintf(s, " %s", labels[i].label);
	    else
		sprintf(s, "%s", labels[i].label);
	    started = 1;

	    s += strlen(s);

	}
    }
}

void
PrintRegister(EdtDev * edt_p, char *name, int offset, BitLabels * labels)

{
    char    flags[1024];
    char    format[80];
    int     value;

    int     size = (offset >> 24) & 3;

    if (!size)
	size = 4;

    size *= 2;

    sprintf(format, "%%-20s: %%0%dx", size);

    value = edt_reg_read(edt_p, offset);

    printf(format, name, value);

    if (labels)
    {
	GetFlagsString(flags, labels, value);
	printf(" %s", flags);
    }

    printf("\n");

}


u_int  *
bump_ptr(u_int * ptr, int count)
{
    u_int  *tmpp = ptr;

    curcount += count;
    if (count > 0)
    {
	while (count--)
	{
	    tmpp++;
	    if (tmpp == &tracebuf[EDT_TRACESIZE])
		tmpp = &tracebuf[2];
	}
    }
    else if (count < 0)
    {
	while (count++ < 0)
	{
	    tmpp--;
	    if (tmpp == &tracebuf[2])
		tmpp = &tracebuf[EDT_TRACESIZE - 1];
	}
    }
    return (tmpp);
}

#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;
setdebug(char *command_line)
#else
int
main(argc, argv)
    int     argc;
    char  **argv;
#endif
{
    int     i, unit = 0, report = 0, debug = -1, quiet = 0;
    int     channel = 0;
    char   *unitstr = "0";
    char    devname[100];
    int     do_channel = 0;
    int     do_foi = 0;
    int     test_timestamp = 0 ;
    int     get_trace = 0;
    int     wait_trace = 0;
    int     get_sglist = 0;
    int     set_sglist = 0;
    int     set_solaris_dma_mode = 0;
    int     solaris_dma_mode = 0;
    int     set_umem_lock_mode = 0;
    int     umem_lock_mode = 0;
    int     do_timeout = 0;
    int     clock_fifo = 0;
    int     do_loop = 0;
    int     fifocnt;
    u_int   cfg;
    int     mstimeout = -1;
    int     sgbuf = -1;
    double  svtime = 0;
    double  curtime;
    int	    foi_unit ;
    int	    version = 0;
    static long    isascii_str(u_char * buf, int n);
    void    print_hex(u_char *buf, int count);
    void    print_ascii(char *buf);

    int process_isr = -1;
    int do_process_isr = 0;

	int use_kernel_buffers = -1;
	int do_kernel_buffers = 0;

	int max_buffers = 0;
	int set_fv_done = -1;

	
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("setdebug",command_line,&argc,&argv);
#endif

#ifdef P53B
    int     soft_reset = 0;
    int     hard_reset = 0;
    int     set_coupled = 0;
    int     direct_coupled = 0;
    int     arg;

#endif

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'u':
	    ++argv;
	    --argc;
	    unitstr = argv[0];
	    break;

	case 'c':
	    ++argv;
	    --argc;
	    channel = atoi(argv[0]);
	    do_channel = 1;
	    break;

	case 'F':
	    ++argv;
	    --argc;
	    foi_unit = atoi(argv[0]);
	    do_foi = 1;
	    break;

	case 't':
	    ++argv;
	    --argc;
	    mstimeout = atoi(argv[0]);
	    do_timeout = 1;
	    break;

	case 'i':
	    ++argv;
	    --argc;
	    process_isr = atoi(argv[0]);
            do_process_isr = 1;
	    break;

	case 'k':

	    if (argc > 1)
		  {
			++argv;
			--argc;
			use_kernel_buffers = atoi(argv[0]);
          }
		do_kernel_buffers = 1;
	    break;

	  
	case 'r':
	    report = 1;
	    break;

	case 'v':
	    version = 1;
	    break;

	case 'T':
	    test_timestamp = 1;
	    break;

	case 'l':
	    do_loop = 1;
	    break;

	case 'V':
	    ++argv;
	    --argc;
	    set_fv_done = atoi(argv[0]);
	    break;
	    
	case 'd':
	    ++argv;
	    --argc;
	    debug = atoi(argv[0]);
	    break;
#ifdef P53B
	case 'D':
	    soft_reset = 1;
	    break;

	case 'C':
	    set_coupled = 1;
	    ++argv;
	    --argc;
	    direct_coupled = atoi(argv[0]);
	    break;

	case 'R':
	    hard_reset = 1;
	    break;
#else
	case 'f':
	    ++argv;
	    --argc;
	    clock_fifo = atoi(argv[0]);
	    break;

	case 'g':
	    get_trace = 1;
	    break;

	case 'G':
	    get_trace = 1;
	    wait_trace = 1;
	    break;

	case 's':
	    get_sglist = 1;
	    ++argv;
	    --argc;
	    sgbuf = atoi(argv[0]);
	    break;

	case 'S':
	    set_solaris_dma_mode = 1;
	    ++argv;
	    --argc;
	    solaris_dma_mode = atoi(argv[0]);
	    break;

	case 'U':
	    set_umem_lock_mode = 1;
	    ++argv;
	    --argc;
	    umem_lock_mode = atoi(argv[0]);
	    break;

	case 'b':
	    ++argv;
	    --argc;
	    max_buffers = atoi(argv[0]);
	    break;

	case 'q':
	    quiet = 1;
	    break;
	case 'x':
	    set_sglist = 1;
	    break;
#endif
	default:
#ifdef P53B
	    puts(
		 "Usage: setdebug [-u unit_no] [-c chan] [-R] [-D] [-r] [-d N] [-i N]");
#else
	    puts("Usage: setdebug [-u unit_no] [-c chan] [-r] [-d N] [-i N]");
#endif
	    exit(0);
	}
	argc--;
	argv++;
    }

    unit = edt_parse_unit(unitstr, devname, EDT_INTERFACE);

    if (do_channel)
    {
	if ((edt_p = edt_open_channel(devname, unit, channel)) == NULL)
	{
	    char    str[64];

	    sprintf(str, "%s%d_%d", devname, unit, channel);
	    perror(str);
	    exit(1);
	}
    }
    else if ((edt_p = edt_open(devname, unit)) == NULL)
    {
	char    str[64];

	sprintf(str, "%s%d", devname, unit);
	perror(str);
	exit(1);
    }

    if (do_foi)
    {
	edt_set_foiunit(edt_p, foi_unit);
    }
#ifdef P53B
    if (hard_reset)
    {
	arg = 1;
	if (edt_ioctl(edt_p, P53S_RESET, &arg) == -1)
	    perror("edt_ioctl(P53S_RESET)");
    }
    if (soft_reset)
    {
	arg = 0;
	if (edt_ioctl(edt_p, P53S_RESET, &arg) == -1)
	    perror("edt_ioctl(P53S_RESET)");
    }
    if (set_coupled)
    {
	if (direct_coupled)
	    printf("setting direct_coupled\n");
	else
	    printf("setting transformer_coupled\n");
	if (edt_ioctl(edt_p, P53S_COUPLED, &direct_coupled) == -1)
	    perror("edt_ioctl(P53S_DIRECT)");
    }
#endif				/* P53B */


    if (do_timeout)
    {
	printf("setting read timeout to %d ms\n", mstimeout);
	edt_set_rtimeout(edt_p, mstimeout);
	edt_set_wtimeout(edt_p, mstimeout);
    }

    if (do_process_isr)
    {
	edt_ioctl(edt_p, EDTS_PROCESS_ISR, &process_isr);
        printf("Process ISR state = %d\n", process_isr);

    }
	if (do_kernel_buffers)
	  {
		int old_state;

		old_state = edt_set_kernel_buffers(edt_p,use_kernel_buffers);
		printf("old_state = %d\n", old_state);

		printf("Use Kernel buffers is %s\n",((use_kernel_buffers == -1)?old_state: use_kernel_buffers)?"On":"Off");
 
	  }

    if (max_buffers > 0)
    {
	edt_set_max_buffers(edt_p,max_buffers);
	printf("Max Buffers = %d\n", edt_get_max_buffers(edt_p));

    }

    if (version)
    {

	edt_version_string version;
	edt_version_string build;

	edt_get_driver_version(edt_p,version,sizeof(version));
	edt_get_driver_buildid(edt_p,build,sizeof(version));

	printf("\n%s%d:  Driver version %s \n       Build %s\n", 
			devname, 
			unit, 
	        version,
			build);
    }
    if (report)
    {

		i = edt_get_debug(edt_p);
	
	printf("\n%s%d:  current debug %d type %d\n", devname, unit, i,
	       edt_get_drivertype(edt_p));
    }
    if (test_timestamp)
    {
	for(i = 0 ; i < 4 ; i++)
	{
		edt_ref_tmstamp(edt_p,i) ;
		edt_msleep (1000) ;
	}
    }

    if (set_fv_done != -1)
    {
	int i;
	i = (set_fv_done)?1:0;
	edt_ioctl(edt_p,EDTS_FVAL_DONE,&i);
	if (set_fv_done == 1)
		edt_reg_and(edt_p,PDV_UTIL3,~PDV_TRIGINT);
    } 

    if (debug != -1)
    {
	(void) edt_set_debug(edt_p, debug);
	if (report)
	    printf("%s%d:  new debug     %d\n", devname, unit, debug);

	if (!quiet)
	{
	    if (edt_p->devid != P53B_ID)
	    {
		PrintRegister(edt_p, "DMA_CUR_ADDR", EDT_DMA_CUR_ADDR, NULL);
		PrintRegister(edt_p, "DMA_NXT_ADDR", EDT_DMA_NXT_ADDR, NULL);
		PrintRegister(edt_p, "DMA_CUR_CNT", EDT_DMA_CUR_CNT, NULL);
		PrintRegister(edt_p, "DMA_NXT_CNT", EDT_DMA_NXT_CNT, NULL);
		PrintRegister(edt_p, "SG_NXT_CNT", EDT_SG_NXT_CNT, BL_SG_NXT_CNT);
		PrintRegister(edt_p, "SG_NXT_ADDR", EDT_SG_NXT_ADDR, NULL);
		PrintRegister(edt_p, "SG_CUR_CNT", EDT_SG_CUR_CNT, NULL);
		PrintRegister(edt_p, "SG_CUR_ADDR", EDT_SG_CUR_ADDR, NULL);
		PrintRegister(edt_p, "DMA_CFG", EDT_DMA_CFG, BL_DMA_CFG);
		PrintRegister(edt_p, "DMA_STATUS", EDT_DMA_STATUS, NULL);
	    }
	    switch (edt_p->devid)
	    {
#ifdef P53B
	    case P53B_ID:
		{
		    static char msgbuf[1024];
		    static u_int wordbuf[0x800];

		    edt_ioctl(edt_p, P53G_DRIVER_STATUS, msgbuf);
		    puts(msgbuf);
		    if (debug > 1)
		    {
			edt_ioctl(edt_p, P53G_WORDBUF1, wordbuf);
			edt_ioctl(edt_p, P53G_WORDBUF2, wordbuf + 0x400);
			for (i = 0; i < 0x800; i++)
			{

			    if (wordbuf[i] & 0x80000000)
				putchar('\n');
			    else
				putchar(' ');
			    printf("%x ", wordbuf[i] & 0x7fffffff);
			}
		    }
		    break;
		}
#endif
	    case PDVFOI_ID:
		PrintRegister(edt_p, "DMA_CFG", EDT_DMA_CFG, NULL);
		PrintRegister(edt_p, "DMA_STATUS", EDT_DMA_STATUS, NULL);
		PrintRegister(edt_p, "FOI_MSG", FOI_MSG, NULL);
		PrintRegister(edt_p, "PDV_REV", PDV_REV, NULL);
		PrintRegister(edt_p, "PDV_STAT", PDV_STAT, BL_PDV_STAT);
		PrintRegister(edt_p, "PDV_UTIL2", PDV_UTIL2, NULL);
		PrintRegister(edt_p, "PDV_SHIFT", PDV_SHIFT, NULL);
		PrintRegister(edt_p, "PDV_MASK", PDV_MASK, NULL);
		PrintRegister(edt_p, "PDV_DATA_PATH", PDV_DATA_PATH, NULL);
		PrintRegister(edt_p, "PDV_MODE_CNTL", PDV_MODE_CNTL, NULL);
		PrintRegister(edt_p, "PDV_ROICTL", PDV_ROICTL, NULL);

                if (edt_cameralink_foiunit(edt_p))
		{
		    PrintRegister(edt_p, "PDV_CL_DATA_PATH", PDV_CL_DATA_PATH, NULL);
		    PrintRegister(edt_p, "PDV_CL_CFG", PDV_CL_CFG, NULL);
		}
		break;
	    case PDVCL_ID:
	    case PDV_ID:
	    case PDVA_ID:
	    case PDVA16_ID:
	    case PDVFOX_ID:
	    case PDVFCI_AIAG_ID:
	    case PDVFCI_USPS_ID:
	    case PGP_RGB_ID:
	    case PDV44_ID:
	    case PDVK_ID:
		PrintRegister(edt_p, "DMA_CFG", EDT_DMA_CFG, NULL);
		cfg = edt_reg_read(edt_p, EDT_DMA_CFG);
		if (cfg & EDT_FIFO_FLG)
		{
		    fifocnt = ((cfg & EDT_FIFO_CNT) >> EDT_FIFO_SHIFT) + 1;
		    fifocnt *= 4;
		}
		else
		    fifocnt = 0;
		printf("fifocnt\t%d\n", fifocnt);
		PrintRegister(edt_p, "DMA_STATUS", EDT_DMA_STATUS, NULL);
		PrintRegister(edt_p, "PDV_STAT", PDV_STAT, BL_PDV_STAT);
		PrintRegister(edt_p, "PDV_CFG", PDV_CFG, BL_PDV_CFG);
		PrintRegister(edt_p, "PDV_SHUTTER_LEFT",
			      PDV_SHUTTER_LEFT, NULL);
		PrintRegister(edt_p, "PDV_UTIL3",
			      PDV_UTIL3, BL_PDV_UTIL3);
#ifdef PDV
		printf("Frame Period        : %d\n",
			pdv_get_frame_period(edt_p));
#endif
		PrintRegister(edt_p, "PDV_DATA_PATH",
			      PDV_DATA_PATH, BL_PDV_DATA_PATH);
		PrintRegister(edt_p, "PDV_MODE_CNTL",
			      PDV_MODE_CNTL, BL_PDV_MODE_CNTL);
		PrintRegister(edt_p, "PDV_DATA_MSB",
			      PDV_DATA_MSB, NULL);
		PrintRegister(edt_p, "PDV_DATA_LSB",
			      PDV_DATA_LSB, NULL);
		PrintRegister(edt_p, "PDV_SERIAL_DATA",
			      PDV_SERIAL_DATA, NULL);
		PrintRegister(edt_p, "PDV_SERIAL_DATA_STAT",
			      PDV_SERIAL_DATA_STAT, BL_PDV_SERIAL_DATA_STAT);
		PrintRegister(edt_p, "PDV_SERIAL_DATA_CNTL",
			      PDV_SERIAL_DATA_CNTL, BL_PDV_SERIAL_DATA_CNTL);

		PrintRegister(edt_p, "PDV_SERIAL_CNTL2", PDV_SERIAL_CNTL2, NULL);
		PrintRegister(edt_p, "PDV_BRATE", PDV_BRATE, NULL);
		PrintRegister(edt_p, "PDV_BYTESWAP", PDV_BYTESWAP, BL_PDV_BYTESWAP);
		edt_reg_write(edt_p, PDV_REV, 0);
		PrintRegister(edt_p, "PDV_REV", PDV_REV, NULL);
		if ((edt_p->devid == PDVA_ID) || (edt_p->devid == PDVFOX_ID))
		    PrintRegister(edt_p, "PDV_UTIL2", PDV_UTIL2, BL_PDVA_UTIL2);
		else PrintRegister(edt_p, "PDV_UTIL2", PDV_UTIL2, BL_PDV_UTIL2);
		PrintRegister(edt_p, "PDV_SHIFT", PDV_SHIFT, BL_PDV_SHIFT);
		PrintRegister(edt_p, "PDV_MASK", PDV_MASK, NULL);
		PrintRegister(edt_p, "PDV_ROICTL", PDV_ROICTL, BL_PDV_ROICTL);
		PrintRegister(edt_p, "PDV_REV", PDV_REV, NULL);
		PrintRegister(edt_p, "PDV_XOPT", PDV_XOPT, NULL);
		if (edt_p->devid != PGP_RGB_ID)
		{
		    PrintRegister(edt_p, "PDV_LHS_CONTROL", PDV_LHS_CONTROL, BL_PDV_LHS_CONTROL);
		    PrintRegister(edt_p, "PDV_LHS_DELAY", PDV_LHS_DELAY, NULL);
		    PrintRegister(edt_p, "PDV_LHS_PERIOD", PDV_LHS_PERIOD, NULL);
		    PrintRegister(edt_p, "PDV_LHS_COUNT_LOW", PDV_LHS_COUNT_LOW, NULL);
		    PrintRegister(edt_p, "PDV_LHS_COUNT_HI", PDV_LHS_COUNT_HI, NULL);
		}
		if (edt_p->devid == PDVCL_ID)
		{

		    PrintRegister(edt_p, "PDV_CL_DATA_PATH", PDV_CL_DATA_PATH, NULL);
		    PrintRegister(edt_p, "PDV_CL_CFG", PDV_CL_CFG, NULL);
		    PrintRegister(edt_p, "PDV_HSKIP", PDV_HSKIP, NULL);
		    PrintRegister(edt_p, "PDV_HACTV", PDV_HACTV, NULL);
		    PrintRegister(edt_p, "PDV_VSKIP", PDV_VSKIP, NULL);
		    PrintRegister(edt_p, "PDV_VACTV", PDV_VACTV, NULL);
		}

		{
		    edt_bitpath pathbuf ;
		    edt_get_bitpath(edt_p, pathbuf, sizeof(edt_bitpath)) ;
		    printf("Interface bitfile   : %s\n", pathbuf) ;
		}

		{
		int i = 0;
		edt_ioctl(edt_p,EDTG_FVAL_DONE,&i);
		if (i)
			printf("Fval Done enabled\n");
		}

		break;

	    case PCD20_ID:
	    case PCD40_ID:
	    case PCD60_ID:
	    case PCDA_ID:
	    case PCDA16_ID:
	    case PGP20_ID:
	    case PGP40_ID:
	    case PGP60_ID:
	    case PGP_THARAS_ID:
	    case PSS4_ID:	/* verify that this is right for PCI SS4 */
	    case PSS16_ID:	/* verify that this is right for PCI SS16 */
	    case PCD_16_ID:

		PrintRegister(edt_p, "DMA_CFG", EDT_DMA_CFG, NULL);
		cfg = edt_reg_read(edt_p, EDT_DMA_CFG);
		if (cfg & EDT_FIFO_FLG)
		{
		    fifocnt = ((cfg & EDT_FIFO_CNT) >> EDT_FIFO_SHIFT) + 1;
		    fifocnt *= 4;
		}
		else
		    fifocnt = 0;
		printf("fifocnt\t%d\n", fifocnt);
		PrintRegister(edt_p, "DMA_STATUS", EDT_DMA_STATUS, NULL);
		PrintRegister(edt_p, "PCD_CMD", PCD_CMD, NULL);
		PrintRegister(edt_p, "PCD_DATA_PATH_STAT",
			      PCD_DATA_PATH_STAT, NULL);
		PrintRegister(edt_p, "PCD_FUNCT", PCD_FUNCT, NULL);
		PrintRegister(edt_p, "PCD_STAT", PCD_STAT, NULL);
		PrintRegister(edt_p, "PCD_STAT_POLARITY",
			      PCD_STAT_POLARITY, NULL);
		PrintRegister(edt_p, "PCD_OPTION", PCD_OPTION, NULL);
		PrintRegister(edt_p, "PCD_DIRA", PCD_DIRA, NULL);
		PrintRegister(edt_p, "PCD_DIRB", PCD_DIRB, NULL);
		PrintRegister(edt_p, "PCD_CONFIG", PCD_CONFIG, NULL);
		PrintRegister(edt_p, "PCD_CYCLE_CNT", PCD_CYCLE_CNT, NULL);
		PrintRegister(edt_p, "EDT_REF_SCALE", EDT_REF_SCALE, NULL);
		PrintRegister(edt_p, "EDT_OUT_SCALE", EDT_OUT_SCALE, NULL);
	        if  (edt_p->devid == PCD_16_ID || edt_p->devid == PSS16_ID)
		{
		    PrintRegister(edt_p, "16_CHEN", SSD16_CHEN, NULL);
		    PrintRegister(edt_p, "16_CHDIR", SSD16_CHDIR, NULL);
		    PrintRegister(edt_p, "16_CHEDGE", SSD16_CHEDGE, NULL);
		    PrintRegister(edt_p, "16_LSB", SSD16_LSB, NULL);
		    PrintRegister(edt_p, "16_UNDER", SSD16_UNDER, NULL);
		    PrintRegister(edt_p, "16_OVER", SSD16_OVER, NULL);
		}
		{
		    edt_bitpath pathbuf ;
		    edt_get_bitpath(edt_p, pathbuf, sizeof(edt_bitpath)) ;
		    printf("Interface bitfile   : %s\n", pathbuf) ;
		}
		break;
	    case P16D_ID:
		PrintRegister(edt_p, "P16_CONFIG", P16_CONFIG, NULL);
		PrintRegister(edt_p, "P16_COMMAND", P16_COMMAND, NULL);
		PrintRegister(edt_p, "P16_STATUS", P16_STATUS, NULL);
		PrintRegister(edt_p, "P16_DATA", P16_DATA, NULL);
		break;
	    case P11W_ID:
		PrintRegister(edt_p, "P11_CONFIG", P11_CONFIG, NULL);
		PrintRegister(edt_p, "P11_COMMAND", P11_COMMAND, NULL);
		PrintRegister(edt_p, "P11_STATUS", P11_STATUS, NULL);
		PrintRegister(edt_p, "P11_DATA", P11_DATA, NULL);
		PrintRegister(edt_p, "P11_COUNT", P11_COUNT, NULL);
		break;
	    }
	    printf("xfer %d bytes\n", edt_get_bytecount(edt_p));

		{
			u_int mapsize;

			mapsize = edt_get_mappable_size(edt_p);

			if (mapsize != 0)
				{
					printf("Second memory space size = 0x%x bytes\n", mapsize);

				}
		}
	}
    }
    if (get_trace)
    {
	u_int   entrys;
	u_int   idx;
	u_int   last_entrys = 0;
	u_int   last_idx;
	u_int  *tmpp = NULL;
	u_int  *startp;
	int     count;
	int     firsttrace = 1;
	int     loop = 0;

	while (firsttrace || wait_trace)
	{
	    edt_get_tracebuf(edt_p, tracebuf);
	    entrys = tracebuf[0];
	    idx = tracebuf[1];
	    startp = &tracebuf[2];

	    if (firsttrace)
	    {
		printf("TraceBuf %d entrys idx %d\n", entrys, idx);
		if (entrys <= EDT_TRACESIZE - 2)
		{
		    startp = &tracebuf[2];
		    count = entrys;
		}
		else
		{
		    startp = &tracebuf[idx];
		    count = EDT_TRACESIZE - 2;
		}
		curcount = 0;
		tmpp = startp;
		firsttrace = 0;
	    }
	    else
	    {
		count = entrys - last_entrys;
		curcount = 0;
		/* leave tmpp alone */
		/*
		 * printf("Next start %x tmpp %x TraceBuf %d entrys idx %d %d
		 * new entrys\n", &tracebuf[2], tmpp, entrys, idx, count);
		 */
		if (count == 0)
		{
		    loop++;
		    if ((loop % 10) == 0)
		    {
			printf(".");
			fflush(stdout);
		    }
		}
		else
		{
		    loop = 0;
		    printf("\n");
		}

		if (count > EDT_TRACESIZE - 2)
		{
		    count = EDT_TRACESIZE - 2;
		    printf("\nlimit count to %d idx %x to %x\n", count, tmpp - tracebuf, idx);
		    tmpp = &tracebuf[idx];
		}

	    }

	    last_entrys = entrys;
	    last_idx = idx;;
	    startp = &tracebuf[idx];

	    while (curcount < count)
	    {
		printf("%08x ", *tmpp);
		tmpp = bump_ptr(tmpp, 1);
		switch (*tmpp)
		{
		case TR_CLEANUP:
		    printf("TR_CLEANUP\n");
		    break;
		case TR_READ_GRAB:
		    printf("TR_READ_GRAB\n");
		    break;
		case TR_ISR_START:
		    printf("TR_ISR_START\n");
		    break;
		case TR_ISR_END:
		    printf("TR_ISR_END\n");
		    break;
		case TR_DPC_START:
		    printf("TR_DPC_START\n");
		    break;
		case TR_DPC_END:
		    printf("TR_DPC_END\n");
		    break;
		case TR_START_BUF:
		    printf("TR_START_BUF\n");
		    break;
		case TR_DOGRAB:
		    printf("TR_DOGRAB\n");
		    break;
		case TR_WAIT_START:
		    printf("TR_WAIT_START\n");
		    break;
		case TR_NEED_WAIT:
		    printf("TR_NEED_WAIT\n");
		    break;
		case TR_WAIT_END:
		    printf("TR_WAIT_END\n");
		    break;
		case TR_LOADED:
		    printf("TR_LOADED\n");
		    break;
		case TR_DONE:
		    printf("TR_DONE\n");
		    break;
		case TR_SERIAL:
		    printf("TR_SERIAL\n");
		    break;
		case TR_TIMEOUT:
		    printf("TR_TIMEOUT\n");
		    break;
		case TR_SET:
		    printf("TR_SET\n");
		    break;

		case TR_SETBUF:
		    printf("TR_SETBUF\n");
		    break;
		case TR_EN_INTR:
		    printf("TR_EN_INTR\n");
		    break;
		case TR_SER_READ:
		    printf("TR_SER_READ\n");
		    {
			int     i;
			int     size;
			int     intsize;
			u_int   intbuf[8];
			unsigned char *sp;

			unsigned char tmpbuf[80];
			unsigned char *tp;
			char   *str_p;

			/* bump to next here */
			tmpp = bump_ptr(tmpp, 2);

			size = *(tmpp);
			tmpp = bump_ptr(tmpp, 1);

			printf("%d bytes: ", size);

			if (size > 15)
			    size = 15;

			tp = tmpbuf;
			intsize = (size + 3) >> 2;

			for (i = 0; i < intsize; i++)
			{
			    tmpp = bump_ptr(tmpp, 1);
			    intbuf[i] = *tmpp;
			    tmpp = bump_ptr(tmpp, 1);
			}

			sp = (unsigned char *) intbuf;

			for (i = 0; i < size; i++)
			{
			    if (i == 1 && edt_p->devid == PDVFOI_ID)
			    {
				char    hexbuf[20];
				int     j;

				sprintf(hexbuf, "<%2x>", (unsigned char) sp[i]);
				for (j = 0; j < 4; j++)
				    *tp++ = hexbuf[j];
			    }
			    else if (sp[i] == 0)
			    {
				*tp++ = '[';
				*tp++ = '0';
				*tp++ = ']';
			    }
			    else
				*tp++ = (unsigned char) sp[i];

			    *tp = 0;
			}
			*tp = 0;
			str_p = (char *) tmpbuf;
			if (isascii_str((u_char *)str_p, strlen(str_p)))
			    print_ascii((char *)str_p);
			else print_hex((u_char *)str_p, strlen(str_p));
			/* get bumped below so back up one */
			tmpp = bump_ptr(tmpp, -1);
		    }
		    break;
		case TR_DEVINT:
		    printf("TR_DEVINT\n");
		    break;
		case TR_SER_WRITE:
		    printf("TR_SER_WRITE\n");
		    {
			int     i;
			u_int   tmpbuf[4];
			char   *str_p;
			int	len ;

			/* bump to next here */
			tmpp = bump_ptr(tmpp, 1);

			tmpp = bump_ptr(tmpp, 1);
			len = *tmpp ;
			tmpp = bump_ptr(tmpp, 1);

			for (i = 0; i < 4; i++)
			{
			    tmpp = bump_ptr(tmpp, 1);
			    /* printf("%08x %08x\n",*tmpp,*(tmpp+1)) ;*/
			    tmpbuf[i] = *tmpp;
			    tmpp = bump_ptr(tmpp, 1);
			}
			str_p = (char *) tmpbuf;
			if (isascii_str((u_char *)str_p, len))
			    print_ascii((char *)str_p);
			else print_hex((u_char *)str_p, len);
			/* get bumped below so back up one */
			tmpp = bump_ptr(tmpp, -1);
		    }
		    break;
		case TR_SNDMSG:
		    printf("TR_SNDMSG\n");
		    break;
		case TR_TMSTAMP:
		    printf("TR_TMSTAMP: ");
		    tmpp = bump_ptr(tmpp, 2);
		    curtime = (double) (*tmpp) * 1000000000L;
		    printf("%d.", *tmpp);
		    tmpp = bump_ptr(tmpp, 2);
		    curtime += (double) (*tmpp);
		    curtime /= 1000000000.0;
		    printf("%09d (%3.9f)\n", *tmpp, curtime - svtime);
		    svtime = curtime;
		    break;
		case TR_REFTMSTAMP:
		    printf("TR_REFTMSTAMP: ");
		    tmpp = bump_ptr(tmpp, 2);
		    printf("%08x ", *tmpp);
		    tmpp = bump_ptr(tmpp, 4);
		    curtime = (double) (*tmpp) * 1000000000L;
		    printf("%d.", *tmpp);
		    tmpp = bump_ptr(tmpp, 2);
		    curtime += (double) (*tmpp);
		    curtime /= 1000000000.0;
		    printf("%09d (%3.9f)\n", *tmpp, curtime - svtime);
		    svtime = curtime;
		    break;
		case TR_SER_READ_END:
		    printf("TR_SER_READ_END\n");
		    break;
		case TR_SER_WRITE_END:
		    printf("TR_SER_WRITE_END\n");
		    break;
		case TR_ISRGRAB:
		    printf("TR_ISRGRAB\n");
		    break;
		case TR_P11W_ATTNINT:
		    printf("TR_P11W_ATTNINT\n");
		    break;
		case TR_P11W_CNTINT:
		    printf("TR_P11W_CNTINT\n");
		    break;
		case TR_ENDCLR:
		    printf("TR_ENDCLR\n");
		    break;
		case TR_STARTCLR:
		    printf("TR_STARTCLR\n");
		    break;
		case TR_FLUSH:
		    printf("TR_FLUSH\n");
		    break;
		case TR_SETEVENT:
		    printf("TR_SETEVENT\n");
		    break;
		case TR_EVENTSLP:
		    printf("TR_EVENTSLP\n");
		    break;
		case TR_EVENTWK:
		    printf("TR_EVENTWK\n");
		    break;
		case TR_EVENTWARN:
		    printf("TR_EVENTWARN\n");
		    break;
		case TR_EN_INT:
		    printf("TR_EN_INT\n");
		    break;
		case TR_SERWAIT:
		    printf("TR_SERWAIT\n");
		    break;
		case TR_SEREND:
		    printf("TR_SEREND\n");
		    break;
		case TR_CONTSTATE:
		    printf("TR_CONTSTATE\n");
		    break;
		case TR_ABORT:
		    printf("TR_ABORT\n");
		    break;
		default:
		    printf("%08x\n", *tmpp);
		}
		tmpp = bump_ptr(tmpp, 1);
	    }
	    if (wait_trace && count == 0)
		edt_msleep(10);
	}
    }
    if (get_sglist)
    {
	edt_buf sg_args;
	u_int   todo_size;
	u_int   sg_size;
	u_int  *todo_list;
	u_int  *sg_list;
	u_int  *log_list;
	u_int   sg_virtual;
	u_int   sg_physical;
	u_int   todo_virtual;
	u_int  *ptr;
	u_int   ii;

	sg_args.desc = EDT_SGLIST_SIZE;
	sg_args.value = sgbuf;
	edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
	sg_size = sg_args.value;

	sg_args.desc = EDT_SGLIST_VIRTUAL;
	sg_args.value = sgbuf;
	edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
	sg_virtual = sg_args.value;

	sg_args.desc = EDT_SGLIST_PHYSICAL;
	sg_args.value = sgbuf;
	edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
	sg_physical = sg_args.value;

	sg_args.desc = EDT_SGTODO_SIZE;
	sg_args.value = sgbuf;
	edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
	todo_size = sg_args.value;

	sg_args.desc = EDT_SGTODO_VIRTUAL;
	sg_args.value = sgbuf;
	edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
	todo_virtual = sg_args.value;

	printf("todo size %x addr  %x\n", todo_size, todo_virtual);
	printf("sg size %x viraddr %x physaddr %x\n",
	       sg_size, sg_virtual, sg_physical);


	todo_list = (u_int *) malloc(todo_size);
	sg_list = (u_int *) malloc(sg_size);

	if (todo_size)
	{
	    printf("return to read todo list: ");
	    getchar();
	    printf("todo list %p\n", todo_list);
	    edt_ioctl(edt_p, EDTG_SGTODO, todo_list);
	    ptr = todo_list;
	    printf("todo_list:\n");
	    printf("todo size %x\n", todo_size);
	    printf("return to continue");
	    getchar();
	    for (ii = 0; ii < todo_size / 8; ii++)
	    {
		printf("%08x %08x\n", *ptr, *(ptr + 1));
		ptr += 2;
	    }
	}
	if (sg_size)
	{
	    buf_args sg_getargs ;
	    sg_getargs.addr = (unsigned int) sg_list ;
	    sg_getargs.size = sg_size ;
	    sg_getargs.index = 0 ;
	    printf("return to read sg list: %x at %x",sg_size,sg_list);
	    getchar();
	    edt_ioctl(edt_p, EDTG_SGLIST, &sg_getargs);
	    ptr = sg_list;
	    printf("sg list:\n");
	    if (edt_little_endian())
		for (ii = 0; ii < sg_size / 8; ii++)
		{
		    printf("%08x %08x\n", *ptr, *(ptr + 1));
		    ptr += 2;
		}
	    else
		for (ii = 0; ii < sg_size / 8; ii++)
		{
		    printf("%08x %08x\n", edt_swab32(*ptr), edt_swab32(*(ptr + 1)));
		    ptr += 2;
		}
	}
	if (set_sglist)
	{

	    printf("return to test sg list: ");
	    getchar();
	    log_list = (u_int *) malloc(1024 * 8);
	    for (i = 7; i >= 0; i--)
	    {
		log_list[i * 2] = 1024 * (1024 / 8) * i ;
		log_list[i * 2 + 1] = 1024 * (1024 / 8) ;
	    }
	    edt_set_sglist(edt_p, 0, log_list, 8) ;
	}
    }
    if (clock_fifo)
    {
	/* tmp test code for ssdio(1) and pcd(2) */
	for (;;)
	{
	    if (clock_fifo == 1)
	    {
		u_int   stat;
		u_int   tmp;
		u_int   bytenum;

		stat = edt_reg_read(edt_p, PCD_STAT);
		bytenum = (stat & (SSDIO_BYTECNT_MSK)) >> SSDIO_BYTECNT_SHFT;
		printf("stat %02x next strobe ", stat);
		if (stat & SSDIO_LAST_BIT)
		    printf("will fill byte to start filling %d\n", bytenum);
		else
		    printf("filling byte %d\n", bytenum);
		tmp = edt_reg_read(edt_p, PCD_FUNCT);
		printf("return to strobe: ");
		getchar();
		edt_reg_write(edt_p, PCD_FUNCT, tmp & ~SSDIO_STROBE);
		edt_reg_write(edt_p, PCD_FUNCT, tmp | SSDIO_STROBE);
	    }
	    else
	    {
		u_int   cfg;
		u_int   cnt;

		cnt = edt_get_bytecount(edt_p);
		cfg = edt_reg_read(edt_p, EDT_DMA_CFG);
		printf("0x%x tranferred  fifo %d cfg %08x\n",
		       cnt, (cfg & EDT_FIFO_CNT) >> EDT_FIFO_SHIFT, cfg);
		printf("return to clock fifo: ");
		getchar();
		edt_reg_write(edt_p, EDT_DMA_INTCFG, cfg | EDT_WRITE_STROBE);
	    }
	}
    }
    if (do_loop)
    {
	u_int   bcount;

	for (;;)
	{
	    edt_msleep(1000);
	    bcount = edt_get_bytecount(edt_p);
	    printf("xfer %d (0x%x) bytes\n", bcount, bcount);
	}
    }
    if (set_solaris_dma_mode)
    {
	edt_ioctl(edt_p, EDTS_SOLARIS_DMA_MODE, &solaris_dma_mode);
    }
    if (set_umem_lock_mode)
    {
	edt_ioctl(edt_p, EDTS_UMEM_LOCK, &umem_lock_mode);
    }


#ifndef NO_MAIN
    exit(0);
#endif
    return (0);
}

static long
isascii_str(u_char * buf, int n)
{
    int     i;

    for (i = 0; i < n; i++)
	if ((buf[i] < ' ' || buf[i] > '~')
	    && (buf[i] != '\t')
	    && (buf[i] != '\n')
	    && (buf[i] != '\r'))
	    return 0;
    return 1;
}

void
print_hex(u_char *buf, int count)
{
    int i;

    printf("<");
    for (i=0; i<count; i++)
	printf("%02x%s", buf[i], i == count-1?">":" ");
    printf("\n");
}

void
print_ascii(char *buf)
{
    unsigned int i;

    printf("<");
    for (i=0; i<strlen(buf); i++)
    {
	if (buf[i] >= ' ' && buf[i] <= '~')
		printf("%c", buf[i]);
	    else if (buf[i] == '\t')
		printf("\\t");
	    else if (buf[i] == '\n')
		printf("\\n");
	    else if (buf[i] == '\r')
		printf("\\r");
    }
    printf(">\n");
}

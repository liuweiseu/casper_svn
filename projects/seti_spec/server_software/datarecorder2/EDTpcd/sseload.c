
/* This sseload program uses some of Jerry's codes in "ssepdb.c" to
 * 
 * load configuration files for PCISS/GS SSE
 * program the MPC9230 PLL
 */

#include "edtinc.h"
#include "edt_bitload.h"
#include <stdlib.h>
#include <fcntl.h>

#define PROGL 0x08
#define INITL_OUTPUT_OFF 0x04
#define CCLK_ON 0x10
#define PROG_ENABLE 0x20
#define BUF_ENABLE 0x40

#define INITL 0x01
#define DONE  0x02

#define DATA_REG 0x40	/* data register */
#define PROG_REG 0x41	/* control register for loading xilinx */
#define STAT_REG 0x42	/* status of programming phase */

/**
 * PLL programming
 */
#define PLL_REG 0x80

#define SLOAD 0x01
#define SDATA 0x02
#define SCLK  0x04

#define SSE_CONST_REG 	0x43
#define SSE_CONST	0x0d	
#define RS_CONST	0x0e

int load_sse(EdtDev *, FILE *, int);		/* load bitfile for Virtex 2 Pro on daughter board */
unsigned short fosc(double fmhz);	/* Set PLL clock to freq specified in MHz */

EdtDev *edt_p;

static void
usage(char *progname)
{
    fprintf(stderr, "Usage: %s [-u unit] [-F fmhz] [-b bitfile]\n", progname);
    fprintf(stderr, "-u unit     %s unit number (default 0)\n", EDT_INTERFACE);
    fprintf(stderr, "-F fmhz     set PLL to fmhz MHz, range: 50 - 800\n" );
    fprintf(stderr, "-b bitfile	 load mezzanie board with this bitfile\n");
    fprintf(stderr, "-R 	 load Reed Soloman decoder configuration files\n");
    fprintf(stderr, "-?          show this message\n" );
    fprintf(stderr, "This program either loads the bitfile or program the PLL\n");
}

int main(int argc, char **argv)
{
  int i,unit=0,len,rs=0;
  FILE *stream;
  char file[50], rs_file[50];
  char bitname[50];
  char temp[50];
  char *unitstr = "0";
  char *progname;
  char devname[128];
  int path=0;
  int pll=0;
  int given=0;
  double fmhz;
  unsigned short program_word;

  sprintf(file,"%s","./bitfiles/XC2VP2/sseio.bit");
  sprintf(rs_file,"%s","./bitfiles/XC2VP2/sseio_asm.bit");

    /* check args */
    progname = argv[0];

    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1]) {
	    case '?':
	    	usage(progname);
		exit(0);
	    	break;

	    case 'u':
	    	++argv;
	    	--argc;
	    	if(argc)
	    	    unitstr = argv[0];
	   	else {
		    usage(progname);
		    exit(1);
		}
	    	break;

	    case 'F':
		++argv;
		--argc;
		if(argc) {
		    pll = 1;
		    fmhz = atof(argv[0]);
		} else {
		    usage(progname);
		    exit(1);
		}
		break;

	    case 'b':
		++argv;
		--argc;
		if(argc) {
		    given = 1;
		    strcpy(bitname, argv[0]);
		} else {
		    usage(progname);	
		    exit(1);
		}
		break;
			
	    case 'R':
		rs = 1;
		break;

	    default:
	    	usage(progname);
	    	exit(0);
	}
	argc--;
	argv++;
    }

  unit = edt_parse_unit(unitstr, devname, EDT_INTERFACE);

  edt_p = edt_open(devname, unit);			/* open interface */
  if (!edt_p)
  {
      edt_perror("pcd");
      exit(1);	
  }
  if ((edt_p->devid != PSS4_ID) && (edt_p->devid != PGS4_ID)){
	printf("Selected board id is %s\n", edt_idstr(edt_p->devid));
	printf("SSE mezzanine board must be mounted on PCIGS or PCISS with 4 channel DMA configuration file\n");
	printf("\tPlease use \"pciload pciss4\" (for PCISS) to reprogram configuration\n");
	printf("\tPlease use \"pciload pcigs4\" (for PCIGS) to reprogram configuration\n");
	exit(1);
  }

  if(!pll) {		/* load bitfile */
     if(given) {
	strcpy(temp, bitname);
     	strtok(bitname,"/");
     	if(!(strtok(NULL,"/"))) {		/* path not specified */
            len = strlen(bitname);
            if(strcmp(bitname+(len-4),".bit")==0 || strcmp(bitname+(len-4),".BIT")==0)
	   	sprintf(file,"%s%s","./bitfiles/XC2VP2/",bitname);
            else
	   	sprintf(file,"%s%s.bit","./bitfiles/XC2VP2/",bitname);
	  	
            stream = fopen(file, "rb");
        } else {				/* path specified */
	    path = 1;
            stream = fopen(temp, "rb");
     	}
     } else {
	if(!rs)
		stream = fopen(file, "rb");
	else
		stream = fopen(rs_file, "rb");
     }

     if (!stream){
	if(path)
	    printf("%s not found\n", temp);
	else
            printf("%s not found\n", file);
        exit(1);
     } else printf("\n");

  
     load_sse(edt_p, stream, rs);
  
     printf("\n");
     fclose(stream);

  		/* by default, PLL is set to 100 MHz */
     fmhz = 100;
  }


  program_word = fosc(fmhz);	/* get the corresponding 14-bit value */
  program_word <<=2;		/* load 14 bits	*/
   
  edt_intfc_write(edt_p, (u_int)PLL_REG, SLOAD); /* assert sload */
  for (i=0; i<14; i++) {
     if(program_word & 0x8000) {
    	edt_intfc_write(edt_p, (u_int)PLL_REG, SDATA | SLOAD);
 	edt_intfc_write(edt_p, (u_int)PLL_REG, SCLK | SDATA | SLOAD);	/* strobe a '1' */
     } else {
 	edt_intfc_write(edt_p, (u_int)PLL_REG, SLOAD);
	edt_intfc_write(edt_p, (u_int)PLL_REG, SCLK | SLOAD);	/* strobe a '0' */
     }

     edt_intfc_write(edt_p, (u_int)PLL_REG, SLOAD);	/* bring sclk low */

     program_word <<= 1;
  }

  edt_intfc_write(edt_p, (u_int)PLL_REG, 0); /* deassert sload */
  edt_intfc_write(edt_p, (u_int)PLL_REG, SLOAD); /* assert sload */
  edt_intfc_write(edt_p, (u_int)PLL_REG, 0); /* deassert sload */

  edt_close(edt_p);

  return 0;
  
}

/**
 * Program the Virtex 2P on the ECL_WITH_OPT daughter board for SSE 
 * functionality through the Interface Xilinx registers
 */
int load_sse(EdtDev *edt_p, FILE *bitfile, int rs)
{
unsigned char b1,data;
int count=0,i,n,left=0;
char basedir[256];
int skip_load=0;
int flags=0;
int ret=0,verbose=1;
edt_pll pll;
double xtal = 10368100.0;
double target=80000000.0;
double actual;
int clock_channel=0;

  strcpy(basedir , ".");
  /* first check if sse config file is loaded by writting 
   * a 0 and reading a constant from a register 
   */
  edt_intfc_write(edt_p, SSE_CONST_REG, 0x0);
  if(!rs) {
     if ( edt_intfc_read(edt_p, (u_int)SSE_CONST_REG) != SSE_CONST) {
	printf("eclopt_sse.bit file for base board not loaded.. loading now\n");
	ret = edt_bitload(edt_p, basedir, "eclopt_sse.bit", flags, skip_load);
	if (ret) {
	    edt_msg(EDTAPP_MSG_FATAL, "eclopt_sse.bit bitload failed");
	    if (verbose < 2)
		edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
	    else printf("\n");
			exit(1);
	}
     }
  } else {
     if ( edt_intfc_read(edt_p, (u_int)SSE_CONST_REG) != RS_CONST) {
	printf("sse_rs_decoder.bit file for base board not loaded.. loading now\n");
	ret = edt_bitload(edt_p, basedir, "sse_rs_decoder.bit", flags, skip_load);
	if (ret) {
	    edt_msg(EDTAPP_MSG_FATAL, "sse_rs_decoder.bit bitload failed");
	    if (verbose < 2)
		edt_msg(EDTAPP_MSG_FATAL, " (run with -v to look for the cause of the failure)\n");
	    else printf("\n");
			exit(1);
	}
     }
  }

  for (i=0; i<128; i++) {
	b1=fgetc(bitfile);
	if ((b1>=0x20) && (b1<=0x7E))  putchar(b1);  else putchar('.');
	if ((b1 & 0xff) == 0xAA)   break;	/* Config synch character */
  }
  count = i;
  putchar('\n');
  if(i==128) {
	printf("No SYNC pattern in the first 128 bytes.\n");
	return 1;
  }
					/* stop driving INITL */
  edt_intfc_write(edt_p, (u_int)PROG_REG, INITL_OUTPUT_OFF);
  edt_intfc_write(edt_p, (u_int)PROG_REG, PROGL);	/* drive progl low */

  edt_msleep(100);		/* for 0.1 seconds */
  if (edt_intfc_read(edt_p,(u_int)STAT_REG) != 0) { 	/* INIT should be asserted (INIT low) */
	printf("Err, INITL still hi when PROGL to Xilinx was asserted");
	return 1;
  }

  edt_intfc_write(edt_p, (u_int)PROG_REG, INITL_OUTPUT_OFF);	/* drive progl hi */

  edt_msleep(100);		/* for 0.1 seconds */
  if (!(edt_intfc_read(edt_p,(u_int)STAT_REG) & INITL)){  /* INIT should be unasserted (INIT hi) */
  	printf("INITL still low after PROGL to Xilinx was deasserted");
	return 1;
  }

  					/* turn on programming state machine */
  edt_intfc_write(edt_p, (u_int)PROG_REG, INITL_OUTPUT_OFF | PROG_ENABLE);
  					/* turn on data buffer and enable CCLK */
  edt_intfc_write(edt_p, (u_int)PROG_REG, INITL_OUTPUT_OFF | PROG_ENABLE | BUF_ENABLE | CCLK_ON);

  n=-4;			/* Send 32 bits of 1's before sync word */
  while (1) {				/* For each byte */
	if      (n<0)   data=0xFF;
        else if (n==0)  data=0xAA;
	else  {
	    data=fgetc(bitfile);
	    if (feof(bitfile)) break;
	}

	/* If part too small, will smoke if we don't abort the load! */
        if ((data!=0x00) && (edt_intfc_read(edt_p, (u_int)STAT_REG) & DONE)) {
	    if (n<=0) {
		printf("Err, DONE was true before download was started");
		return 1;
	    } else
		break;
	}

	if   (((n==1) && (data!=0x99)) || ((n==2) && (data!=0x55)) || 
              ((n==3) && (data!=0x66)))  {
	    printf("Err, missing sync byte 0x99 or 0x55 or 0x66");
	    return 1;
	}
	n=n+1;


	while ((edt_intfc_read(edt_p, (u_int)STAT_REG))>>2 & 0xf); /* wait if fifo full */

	edt_intfc_write(edt_p, (u_int)DATA_REG, data);	/* send data bytes */
	count++;
  }

  if (!(edt_intfc_read(edt_p, (u_int)STAT_REG) & DONE)){
	printf("Err, DONE was not true after download was completed");
	return 1;
  }

  printf("Configuration file is loaded.\n");

  fflush(stdout);

  return 0;
}

unsigned short fosc(double fmhz) {	/* Set ECL clock to freq specified in MHz */
    int m, n, t, hex,  nn, fx;
	 
    fx = (int) fmhz;

    if ((fmhz>800.0) || (fmhz<50))  { 
	printf("Error, %f MHz requested.  Min of 50, max of 800Mhz\n", fmhz);
	exit(1);
    } 
    else if (fx>400) { t=0;  n=3;  m=fx/2;  nn=1;}  /* Every 2 MHz */
    else if (fx>200) { t=0;  n=0;  m=fx;    nn=2;}  /* Every 1 MHz */
    else if (fx>100) { t=0;  n=1;  m=fx*2;  nn=4;}  /* Every 500 KHz */
    else if (fx>=50) { t=0;  n=2;  m=fx*4;  nn=8;}  /* Every 250 KHz */
    else  {                    
	printf("The frequency needs to be at least 50 MHz.\n");
	exit(1);
    }

    hex = (t<<11) | (n<<9) | m;    

    printf("    %f MHz    MPC9230: t:%d  n:%d  m:%d  hex:0x%04x\n", 2.0*m/nn, t, n, m, hex);

    return ((unsigned short) hex);
}


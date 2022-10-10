/* #pragma ident "@(#)pciload.h	1.9 07/11/01 EDT" */

#ifdef _NT_
 #define DIRECTORY_CHAR '\\'
#else
 #define DIRECTORY_CHAR '/'
#endif

/* for vxworks */
#ifdef NO_MAIN
#define exit return
#endif

#define	MAX_BIT_SIZE	256 * 1024
#define	MAX_STRING	128
#define	ID_SIZE		MAX_STRING
#define	OSN_SIZE         32
#define	ESN_SIZE         64
#define XLA_SECTOR_SIZE	0x10000
#define	E_SECTOR_SIZE	0x04000

#define	EDT_ROM_JUMPER	0x01
#define	EDT_5_VOLT	0x02

/*
 * legal proms
 */
#define PROM_UNKN	0
#define	AMD_4013E	1
#define	AMD_4013XLA	2
#define	AMD_4028XLA	3
#define	AMD_XC2S150	4
#define	AMD_XC2S200_4M	5
#define	AMD_XC2S200_8M	6
#define	AMD_XC2S100_8M	7
#define	AMD_XC2S300E	8

#define XTYPE_X		0
#define XTYPE_BT	1
#define XTYPE_LTX	2

/*
 * status bits
 * bits 2-7 are Xilinx ID for 4028 and above.
 * shift down 2 and we get a number
 * 0 - 4028XLA boot controller
 * 1 - XC2S150 boot controller 
 * 2 - XC2S200 boot controller with out protect 29LV040B FPROM 
 * 3 - XC2S200 boot controller with protect restored (future 8Mbit FPROM)
 * 4 - XC2S100 boot controller
 */
#define STAT_PROTECTED	0x01
#define STAT_5V		0x02
/* #define STAT_IDMASK	0xfc */
#define STAT_IDMASK	0x7c /* change jsc 12/6/02 to include xc2s100 */
#define STAT_IDSHFT	2
#define STAT_XC4028XLA	0x00
#define STAT_LTX_XC2S300E 0x00
#define STAT_XC2S150	0x01
#define STAT_XC2S200_NP	0x02
#define STAT_XC2S200	0x03
#define STAT_XC2S100    0x04


/*
 * command bits for the 4028xla
 * boot controller
 */
#define	BT_READ		0x0100
#define	BT_WRITE	0x0200
#define BT_INC_ADD	0x0400
#define	BT_A0		0x0800
#define	BT_A1		0x1000
#define	BT_RSVD 	0x2000
#define	BT_REINIT	0x4000
#define BT_EN_READ	0x8000
#define	BT_LD_LO	BT_WRITE
#define	BT_LD_MID	BT_WRITE | BT_A0
#define	BT_LD_HI	BT_WRITE | BT_A1
#define	BT_LD_ROM	BT_WRITE | BT_A0 | BT_A1
#define BT_RD_ROM	BT_READ | BT_A0 | BT_A1 | BT_EN_READ
#define	BT_MASK		0xff00

#define IS_DEFAULT_SECTOR -6167


extern int sect;
extern u_char outbuf[MAX_BIT_SIZE];

#define MAX_BOARD_SEARCH 8
#define NUM_DEVICE_TYPES 4

struct board_info {
  char type[8];
  int id;
  int bd_id;
};

typedef struct {
  char filename[256];
  int filesize;
  char id[MAX_STRING];
  char prom[MAX_STRING];
  u_char data[MAX_BIT_SIZE];
  int data_index;
} BFS;

struct board_info *detect_boards(char *dev, int unit, int verbose);

void warnuser(char *dev, char *fname);
void warnuser_ltx(char *dev, char *fname, int unit, int hub);

void check_id_times(char *bid, char *pid, int *verify_only, char *fname);
void getinfo(EdtDev *edt_p, int promcode, int segment, char *pid, char *esn, char *osn, int verbose);
void readinfo(EdtDev *edt_p, int promcode, int segment, char *pid, char *esn, char *osn);
void getname(EdtDev *edt_p, int promcode, char *name);

int program_verify_4013E(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int verify_only, int warn, int verbose);
int program_verify_4013XLA(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int sector, int verify_only, int warn, int verbose ) ;
int program_verify(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int sectors, int verify_only, int warn, int verbose, int promcode, int xtype) ;
int program_verify_4028XLA(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose) ;
int program_verify_XC2S150(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose) ;
int program_verify_XC2S200(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose) ;
int program_verify_XC2S300E(EdtDev *edt_p, BFS *bitfile, char *dev, char *pid, char *esn, char *osn, int segment, int verify_only, int warn, int verbose) ;
void program_sns(EdtDev *edt_p, int promtype, char *new_esn, char *new_osn, int sector, char *id, int verbose);
void print_flashstatus(char stat, int sector, int frdata, int verbose);
void edt_get_sns(EdtDev *edt_p, char *esn, char *osn);
void edt_get_osn(EdtDev *edt_p, char *osn);
void edt_get_esn(EdtDev *edt_p, char *esn);
int ask_reboot(EdtDev *edt_p);

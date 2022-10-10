#ifndef _C2M_H_
#define _C2M_H_

//standard junk
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <signal.h>
#include <time.h>
#include <math.h>

//miriad uv file control interfaces
#include "mir/miriad.h"
#include "mir/hio.h"
#include "mir/io.h"

//libastro header
#include "libastro/astro.h"

// Some miriad macros...
#define PREAMBLE_SIZE 5
#define MAXVAR 256
#define MAXANT 256
#define MAX_COEFF 16
#define MAX_LINE_LEN 80
#define GETI(bl) (((int) bl >> 8) - 1)
#define GETJ(bl) (((int) bl & 255) - 1)
#define MKBL(i,j) ((float) (((i+1)<<8) | (j+1)))
#define ITEM_HDR_SIZE		4

// statics for MJD and LST calculations
#define MJD0  2415020.0
#define J2000 (2451545.0 - MJD0)      /* yes, 2000 January 1 at 12h */
#define	SIDRATE		.9972695677		  /* ratio of from synodic (solar) to sidereal (stellar) rate */

/* conversions among hours (of ra), degrees and radians. */
#define	degrad(x)	((x)*PI/180.)
#define	raddeg(x)	((x)*180./PI)
#define	hrdeg(x)	((x)*15.)
#define	deghr(x)	((x)/15.)
#define	hrrad(x)	degrad(hrdeg(x))
#define	radhr(x)	deghr(raddeg(x))

#ifndef PI
#define	PI		3.141592653589793
#endif

#define VERSION_ID "c2m (C version) 02.17.09"
#define VERSION "02.17.09"

#define check(iostat) if(iostat)bugno_c('f',iostat)

/* maximum number of antennas in the correlator,
 * this is because I am a poor c programmer,
 * i'm sure i'll figure out a better way to do this later */
#define MAX_ANT 512

/* antenna location in DD:MM:SS.ss,DD:MM:SS.ss */
typedef struct loc
{
	double latitude;
	double longitude;
} loc;

/* antenna position offsets from location in wavelengths */
typedef struct pos
{
	float x;
	float y;
	float z;
} pos;

/* structure to hold the contents of a correlator config file */
typedef struct CorrConf
{	
	/*borphserver*/
	char *servers[16]; /* bee fpga servers*/
	int n_servers;
	char *time_server;
	char *bitstream;

	/*correlator*/
	int n_chans;
	int n_ants;
	char *fft_shift;
	int acc_len;
	int xeng_acc_len;
	float adc_clk;
	int n_stokes;
	int x_per_fpga;
	int clk_per_sync;
	float ddc_mix_freq;
	float ddc_decimation;
	int feng_bits;
	int feng_fix_pnt_pos;
	int ten_gbe_pkt_len;
	int ten_gbe_port;
	int ten_gbe_ip;
	int x_eng_clk;

	/*receiver*/
	int t_per_file;
	char *db_file;
	int rx_buffer_size;
	int max_payload_len;
	int udp_port;

	/*equalisation*/
	float tolerance;
	int auto_eq;
	int eq_poly[MAX_ANT][MAX_COEFF];

	/*antennas*/
	int order[MAX_ANT];
	struct loc location;
	pos position[MAX_ANT];

	int verbose_mode;
	int single_capture;
	int n_bls;
	int int_size;
	int n_int_buffer;
	float timestamp_scale_factor;
	int timestamp_rounding;
	int read_ahead_allow;
	time_t start_t;
	int quit;
	char *stokes[4];
	float int_time;

} CorrConf;

typedef struct
{
	//int nants;
	//double uu[MAXANT], vv[MAXANT], ww[MAXANT];
	double uu;
	double vv;
	double ww;
} UVW;


typedef struct PREAMBLE
{
	UVW uvw;
	double t;
	int i;
	int j;
} PREAMBLE;

/* Wrapper Function */
int FileLoader(char *argv);
CorrConf initCorrConf(); //Create a default CorrConf struct
char * cat_str(char * arg1, const char * arg2);

// functions to initialize corr from conf file
void parseCommandLine(int argc, char **argv);
void uv_init();
void uv_open(const char *name);
int file_exists(const char *filename);
void printCorrConfig(CorrConf arg);
void uv_close();
void conv_time(struct tm* t, double *current_mjd_real, double *current_mjd);
void calc_mjd(int mn, double dy, int yr, double *mjp);
void calculateCorrelatorParameters(char *config_file);

void uv_write_int(double data[], uint64_t timestamp);
//void apply_data(int xeng, double data[]);
void file_data(int timestamp, double data[]) ;
void gen_preamble(int t, int i, int j, PREAMBLE *p);
void record(PREAMBLE p, int pol, double data[]);

void bl2ij(int bl, int *ij);
void il2bl(int i, int j, int *bl);
void get_bl_order(int n_ants, int *bl_order);
int str2pol(char * pol);

//functions for calculating the uvw
void eq2top_m(double ha, double dec, double *m);
//void generate_zbls(double *z);
double *generate_zbls(double z[][3]);

/*
 * Global Variables
 */
// miriad file pointers
int tno, iostat;
int history[MAXOPEN];
CorrConf corr_config;

#endif // _C2M_H_

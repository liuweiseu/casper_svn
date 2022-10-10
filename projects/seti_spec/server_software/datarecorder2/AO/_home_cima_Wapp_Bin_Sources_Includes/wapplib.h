/*****************************************************************************
 *
 * FILE: wapplib.h
 *
 *   Structures and procedures used by 'wapplib'.
 *
 *   This file should be included in any program that uses 'wapplib'.
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * lerner        6 Feb 2007    Original coding
 * lerner       15 Feb 2007    Added AO_MIN_ZA, removed AOHORIZ and added new
 *                             argument to 'compute_horizon'
 * lerner        2 Nov 2007    Extended struct 'RFCALC' from 4 to 8
 *                             frequencies, added 'gr327_filt', 'gr430_filt',
 *                             'alfa_filt', 'compute_rise' and 'error_greatc',
 *                             removed 'lbn_filt' and added 'AZ_SLEW' and
 *                             'ZA_SLEW'
 *
 *****************************************************************************/


#include <math.h>
#include <time.h>
#include <netinet/in.h>
#include <tk.h>



/*****************************************************************************
 *
 *   Set up some useful constants
 *
 *****************************************************************************/

/* perrilat uses: #define VLIGHT 299792.4562 */

#define VLIGHT 299792.458             /* from NIST web page */
#define LIGHT_SPEED_MPS 299792458.0

#define UT_TO_SID 1.0027379093

#define AOLAT 18.353806               /* degrees north 18d21'13.7" */
#define AOLONG 66.753056              /* degrees west 66d45'11.0" */

#define AO_MIN_ZA 1.06                /* min ZA */
#define AO_MAX_ZA 19.69               /* max ZA */

#define SLEW_AZ       0.4     /* azimuth slew rate in degrees/sec */
#define SLEW_ZA       0.04    /* zenith distance slew rate in degrees/sec */

#define TUT (4.0/24.0)                /* convert AST to UT */

/*  Constants for 'socklib.c'  */

#define MAXPORT  10 
#define MAXDEST 100
#define MAXNAME  30

/*  Constants for 'rfcalc.c'  */

#define IFNUM300    1 
#define IFNUM750    2 
#define IFNUM1500   3 
#define IFNUM1500UP 4 
#define IFNUMNONE   5 
#define IFNUMALFA   6 

#define INPFRQ1500  3
#define INPFRQ750   2
#define INPFRQ300   1
#define INPFRQ750NB 0

#define RFONLY  1
#define SUBBAND 2

/*  Constants for 'vanvleck.c'  */

#define VV3LEV 1
#define VV9LEV 2



/*****************************************************************************
 *
 *   Set up some useful macros
 *
 *****************************************************************************/

#define DEG_TO_RAD(x)       ((x)*M_PI/180.0)
#define RAD_TO_DEG(x)       ((x)*180.0/M_PI)
#define HOUR_TO_RAD(x)      ((x)*M_PI/12.0)
#define RAD_TO_HOUR(x)      ((x)*12.0/M_PI)

#define MAX(x, y)           ((x) > (y) ? (x) : (y))
#define MIN(x, y)           ((x) < (y) ? (x) : (y))



/*****************************************************************************
 *
 *   Define the structures we need
 *
 *****************************************************************************/


/*  Structures for 'socklib.c'  */

struct DEST {
  int dport;
  char host[MAXNAME];
  char dname[MAXNAME];
};

struct SOCK {
  struct SOCK *next;
  struct DEST *dest;
  int fd;
  struct sockaddr_in sin;
};

struct BOUND {
  int bind_fd;
  struct DEST *dest;
  struct SOCK *head; /* head of list */
  struct sockaddr_in sin;
  unsigned int bufct;
  int interrupt;
  int isxview; /* true if xview */
  int isaio;   /* true if aioread is turned on (Solaris2) */
  int (*open)(struct SOCK *, int);
  int (*close)(int);
  void (*error)(char *, char *);
};

/*  Structures for 'rfcalc.c'  */

struct RFCALC {

  int calctype; /* RFONLY - move the center freq and shift all freqs
                   based on the center freq
                   SUBBAND - set the downstairs LOs to move
                */
  double vel;
  char veltype[24];
  char velframe[24];
  double cent_freq;
  double restfreq[8];

  double ant_vel;
  double ant_time; 
  
  double cent_new; 
  double sky_new[8];

/* IF system state: */

                    /* inputs  frequencies in MHz */
  int rfnum;        /* rx switch number */
  int ifnum;        /* IF1 path */
  int inpfrq;       /* IF2 path */
  int filt[8];      /* IF2 filters (if 1500 selected) */
  double dest[8];   /* destination center frequency of backend */
  double up;        /* upstairs synthesizer */
  double bw[8];     /* bandwidth at dest */
  double syn[8];    /* downstairs synthesizer freq */ 

/* variables for computing ranges for up to 3 mixing stages */

  int nstages;          /* number of mixing stages */
  double low;           /* min freq of rx bw */
  double hi;            /* max freq of rx bw */
  double stage0[8][2];  /* dest +- half bw (len of 2) */
  double stage1[8][4];  /* after first mixer ( 2 ranges ) */
  double stage2[8][8];  /* after second mixer ( 4 ranges ) */
  double stage3[8][16]; /* after third mixer ( 8 ranges ) */

                      /* results */

  int flip[8];        /* is band flipped by iflo */
  double cent[8];     /* computed center sky freq (that I guess) */ 
  double sky[8][2];   /* sky freq band passed (that I guess) */
  int err[8];         /* true if I think there is a problem  */
  int verbose;

  char err_msg[512];
};

/*  Structures for 'vanvleck.c'  */

struct VANVLECK {
  double m1; 
  double m2;
  double v1;
  double v2;
  int fitpts;
  int level;
};



/*****************************************************************************
 *
 *   All the procedures in 'wapplib'
 *
 *****************************************************************************/

/*  Procedures in 'alfaoff.c'  */

double find_alfaoff(int beam, int isza);
void find_alfarot(double angle, int beam, double *paz, double *pza);
void alfa_position(double ra, double dec, double lst, double epoch,
                   double angle, double off1, double off2, int beam,
                   double *pra, double *pdec);

/*  Procedures in 'azza.c'  */

void compute_azza(double ra, double dec, double lst, double *paz,
                  double *pza);
void compute_zaha(double az, double dec, double *pza, double *pha);
void off_hadec(double daz, double dza, double az, double za, double ha,
               double dec, double *pdha, double *pddec);

/*  Procedures in 'flip.c'  */

/* Ignored to avoid to include lots of special include files */

/*
void flip_agc(SCRM_B_AGC *agc);
void flip_tt(SCRM_B_TT *tt);
void flip_pnt(SCRM_B_PNT *pnt);
void flip_tie(SCRM_B_TIE *tie);
void flip_if1(SCRM_B_IF1 *if1);
void flip_if2(SCRM_B_IF2 *if2);
void flip_exec(struct EXECSHM *exec);
void flip_wapp(struct WAPPSHM *wapp);
void flip_alfa(struct ALFASHM *alfa);
void flip_data(void *v, int n);
*/

/*  Procedures in 'general.c'  */

double cal2mjd(int iy, int im, int id);
double current_mjd();
double epoch_to_mjd(time_t secs);
int makefactor2(int in);
int near(double a, double b);
double sign(double a, double b);

/*  Procedures in 'inverse_cerf.c'  */

double inverse_erf(double x);
double inverse_cerf(double x);

/*  Procedures in 'message.c'  */

int check_if_file_exists(char *filename);
int get_hostname(char *hostname, int uppercase);
void set_up_log_file(char *filename, int send_back_to_caller);
void write_to_log(char *format, ...);
void log_socket_error(char *format, char *message);
void redirect_socklib_errors();
void send_to_cima(char *format, ...);
void to_cima_log(char *type, char *source, char *format, ...);

/*  Procedures in 'prec.c'  */

double precess(double epoch_ra, double epoch_dec, double *ret_ra,
               double *ret_dec, double curtime, int src);
void sla_preces(char *system, double ep0, double ep1, double *ra, double *dc);
double current_time();
double para(double tlst, double ra, double dec, double alat);

/*  Procedures in 'radec.c'  */

void compute_radec(double az, double za, double lst, double *pra,
                   double *pdec);
double compute_horizon(double dec, double za_hor);
int compute_rise(double ra, double dec, double lst, double *prise);
void rise_set(double lst_rise, double lst_set, double lst, double *rise,
              double *set);
void jeq2gal(double jra, double jdec, double *l, double *b);
void gal2jeq(double l, double b, double *jra, double *jdec);

/*  Procedures in 'receiver.c'  */

char *find_rxname(double turpos);
int find_rxnum(double turpos);
int find_rxrange(char *name, double *min, double *max);
int find_rxrange_num(int num, double *min, double *max);
int find_rxnum_abbr(char *abbr);
char *gr327_filt(int filt);
char *gr430_filt(int filt);
char *lbw_filt(int filt);
char *sbw_filt(int filt);
char *alfa_filt(int filt);

/*  Procedures in 'rfcalc.c'  */

void compute_rf(struct RFCALC *rf);
int compute_synf(struct RFCALC *v);
void velcalc(struct RFCALC *v);
double compute_skycenter(int ifnum, int inpfrq, double syn, double hi,
                         double lo, double dest);

/*  Procedures in 'scram.c'  */

struct SCRAMNET *init_scramread(struct SCRAMNET *init);
int fd_scram(struct SCRAMNET *scram);
int read_scram(struct SCRAMNET *scram);
void scramlock(struct SCRAMNET *scram, int l);
double error_greatc(struct SCRAMNET *scram);

/*  Procedures in 'socklib.c'  */

int sock_bind(char *mbname);
struct SOCK *sock_connect(char *mbname);
int read_sock(struct SOCK *s, char *msg, int *l);
int sock_close(char *name);
struct SOCK *last_msg();
char *sock_name(struct SOCK *handle);
int sock_fd(struct SOCK *handle);
int sock_bufct(int bufct);
int sock_find(char *mbname);
int sock_findfd(int fd);
void sock_intr(int flag);
int sock_sel(char *message, int *l, int *p, int n, int tim, int rd_in);
int sock_ssel(struct SOCK *ss[], int ns, char *message, int *l, int *p, int n,
              int tim, int rd_in);
int sock_fastsel(char *message, int *l, int *p, int n, struct timeval *tim,
                 int rd_in);
int sock_only(struct SOCK *handle, char *message, int tim);
int sock_poll(struct SOCK *handle, char *message, int *plen);
int accept_sock();
int sock_send(struct SOCK *s, char *message);
int sock_write(struct SOCK *s, char *message, int l);
struct DEST *find_dest(char *name);
struct DEST *find_port(int p);
void bind_any(int fd);
#ifdef VXWORKS
static char *strtok(char *s, char *delim);
#endif
void sock_openclose(int (*open)(struct SOCK *, int), int (*close)(int));
void sock_seterror(void (*err)(char *, char *));
void standard_error(char *a, char *b);

/*  Procedures in 'socktk.c'  */

int init_sockettk(Tcl_Interp *i);
int sock_msg(unsigned char **pp);

/*  Procedures in 'swapdata.c'  */

double swapd(double *d);
long swapl(long *l);
int swapi(int *l);
unsigned short swaps(unsigned short *s);
void in_swapus(unsigned short *s);
void in_swapf(float *f);
void in_swaps(short *s);
void in_swapd(double *d);
void in_swapl(long *ll);
void in_swapi(int *ii);
void set_swapd(double *p, double d);
void set_swapi(int *p, int l);

/*  Procedures in 'vanvleck.c'  */

void vanvleck_z(double z1, double z2, int fitpts, int level,
                struct VANVLECK *vm);
void vanvleck(double *acf, int n, struct VANVLECK *vm);
void vanvleckf(float *acf, int n, struct VANVLECK *vm);
void vanvleck_pol(double *ccfx, double *ccfy, int n, struct VANVLECK *vm);
void vanvleckf_pol(float *ccfx, float *ccfy, int n, struct VANVLECK *vm);
double pow3lev(double zho);
double pow9lev(double zho);
double attndb3lev(double zho);
double attndb9lev(double zho);
double hamming(int index, int npts);

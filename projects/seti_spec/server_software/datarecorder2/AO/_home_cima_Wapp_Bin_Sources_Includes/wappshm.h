/*****************************************************************************
 *
 * FILE: wappshm.h
 *
 *   ...
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * jeffh        16 Jun 2004    Original coding
 * lerner        1 Nov 2007    Renamed 'channelafirst' to 'firstchannel' and
 *                             corrected comment for new usage of 'mode'
 *
 *****************************************************************************/


/* wapp state broadcasted shared memory */

struct WAPPSHM {
  char project_id[8];   /* project-ID from CIMA used to choose file */
  char wapp[8];         /* machine name */
  char config[24];      /* configuration string */
  float bw;             /* bandwidth in MHz */
  float power[4];       /* the 4 last computed power values */
  int mode;             /* bit 0:  pulsar (1) or spectral line (0)
			   bit 1:  SPECTRA_TOTALPOWER (1) or not (0)
			   bit 2:  dual board (1) or not (0)
			   bit 3:  ALFA (1) or not (0) */
  int bits;             /* 16 or 32 bits */
  int firstchannel;     /* single channel is pol A (0) or pol B (1) */
  int bins;             /* number of bins in folding mode */
  float dump_ival;      /* in seconds */
  int running;          /* true if running */
  float samp_time;      /* lowest level integration time in microseconds */
  int num_lags;         /* number of lags this mode */
  int attena[2];        /* two atten values for dual-board mode pol-A */
  int attenb[2];        /* two atten values for dual-board mode pol-B */
  int countdown;        /* seconds */
  int total;            /* seconds */
  char file[128];       /* current filename */
};

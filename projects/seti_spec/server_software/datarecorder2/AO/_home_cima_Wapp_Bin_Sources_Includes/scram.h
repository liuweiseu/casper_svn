/*****************************************************************************
 *
 * FILE: scram.h
 *
 *   ...
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * jeffh         5 Apr 2004    Original coding
 * lerner        6 Feb 2007    Removed function prototypes
 *
 *****************************************************************************/


/* shared memory mystery structures */

struct BIGENOUGH {
  char magic[8];
  union {
    SCRM_B_AGC agcData;
    SCRM_B_TT ttData;
    SCRM_B_PNT pntData;
    SCRM_B_TIE tieData;
    SCRM_B_IF1 if1Data;
    SCRM_B_IF2 if2Data;
    struct EXECSHM exec;
    struct WAPPSHM wapp;
    struct ALFASHM alfa;
  } u;
};

struct SCRAMNET {
  int sock;                 /* multicast socket */
  int lock;                 /* true if update blocked for reading */
  struct sockaddr_in from;  /* read address */
  struct BIGENOUGH in;      /* input buffer */
  SCRM_B_AGC agcData;       /* data */
  SCRM_B_TT ttData;
  SCRM_B_PNT pntData;
  SCRM_B_TIE tieData;
  SCRM_B_IF1 if1Data;
  SCRM_B_IF2 if2Data;
  struct EXECSHM exec;
  struct WAPPSHM wapp[4];
  struct ALFASHM alfa;
};

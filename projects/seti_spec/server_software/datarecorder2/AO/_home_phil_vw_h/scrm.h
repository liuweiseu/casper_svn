#ifndef scrmhInc
/* 
 * include file for ao  scramnet routines
 *modification history:
 *09nov98, changed header. removed buflen,initialized from header.
 *			put in scrmAdAr
*/
#define scrmhInc        1

#define SCRM_OBJ_INITIALIZED 12345

/*
 *  note numbers/names . no longer used. file ~phil/vw/etc/scrm
 *  has the id's and they are accessed via scrmNodeId in scrmLoad.c
*/
#define SCRM_NODE_OBSERVER  1
#define SCRM_NODE_CONTROL   2
#define SCRM_NODE_DA        3
#define SCRM_NODE_COR       4
#define SCRM_NODE_PNT       5 
/*
 * scrm block indices  into scrmLoad.h. array static SCRM_ADR_INFO scrmAdrAr[]= 
*/
#define SCRM_PNT_IND        0
#define SCRM_AGC_IND        1
#define SCRM_TT_IND         2
#define SCRM_TIE_IND        3

/*  devices     */

#define SCRM_IF2_IND        4
#define SCRM_IF1_IND        5
#define SCRM_DOP_IND        6
#define SCRM_PROC_IND       7
#define SCRM_PLRDR_IND      8
#define SCRM_SB723_IND      9
/* no implemented yet */
#define SCRM_TER_IND        10
#define SCRM_ALFM_IND       11
#if FALSE
#define SCRM_SPS_IND        10
#define SCRM_PNC_IND        11
#endif
#define SCRM_COR_IND        12
#define SCRM_DCD_IND        13
#define SCRM_DDS_IND        14

#define SCRM_WEATHER_IND    15
#define SCRM_TEMP_IND       16
#define SCRM_DBG_IND        17
#define SCRM_LAST_IND       17

/*	typedefs	*/
/*
 *	1.scrm memory is made up of multiple blocks. 
 *  2.Each block belongs to a particular program: agc,pnt,tie, tt, etc..
 *  3.Each block contains a blk header  followed by 2 data buffers. 
 *  4.The writer of a block will alternate between these two buffers when
 *	  making updates. Each buffers has a complete copy of the data.
 *  5.readers will read from the last updated buffer.
 *  6.The read index ptr will determine which buf to read and/or write from.
*/
/*
 *	The header at the beginning of each srcmBlk;
*/
typedef struct {
	int     rdInd;		/* 0 or 1*/
	int		filler;	
	int		modCnt[2];	/* 1 for each buffer*/
	} SCRM_BLK_HDR;

/*
 * BLK_INFO .. used by the user when accessing a scrm blk.
*/
typedef struct {
	short        initialized; /* 12345*/
    short        blkInd; /* which scrmblock 0..15 user requested*/
    int          blkOff; /* offset scrmBloclk from start of scrm memory*/
    volatile SCRM_BLK_HDR *pblkHdr;/* ptr to start of header in scrmBlk*/
    int          bufInd; /* which buffer in block we are using*/

    int          modCnt; /* for buffer we pnt to */
	char		*pbuf;   /* ptr to start of data*/
    } SCRM_BLK_INFO;
	

#endif

/*****************************************************************************
 *
 * FILE: scram.c
 *
 *   ...
 *
 * HISTORY
 *
 * who          when           what
 * ---------    -----------    ----------------------------------------------
 * jeffh         5 Apr 2004    Original coding
 * lerner       30 Jun 2005    Adapted for move to /home/cima
 * lerner        6 Feb 2007    Added 'wapplib.h' and cleaned up for '-Wall'
 * lerner        2 Nov 2007    Added 'error_greatc' from 'spectra.c'
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mshmLib.h"
#include "execshm.h"
#include "wappshm.h"
#include "alfashm.h"
#include "scram.h"
#include "wapplib.h"


struct SCRAMNET *scram;


void flip_agc(SCRM_B_AGC *agc);
void flip_tt(SCRM_B_TT *tt);
void flip_pnt(SCRM_B_PNT *pnt);
void flip_tie(SCRM_B_TIE *tie);
void flip_if1(SCRM_B_IF1 *if1);
void flip_if2(SCRM_B_IF2 *if2);
void flip_exec(struct EXECSHM *exec);
void flip_wapp(struct WAPPSHM *wapp);
void flip_alfa(struct ALFASHM *alfa);

struct SCRAMNET *init_scramread(struct SCRAMNET *init);
int fd_scram(struct SCRAMNET *scram);
int read_scram(struct SCRAMNET *scram);
void scramlock(struct SCRAMNET *scram, int l);
double error_greatc(struct SCRAMNET *scram);



struct SCRAMNET *init_scramread(struct SCRAMNET *init) {

  const int ttl = 64;
  int itmp;
  struct ip_mreq multiaddr;

  if ( ! init )
    scram = (struct SCRAMNET *) malloc(sizeof(struct SCRAMNET));
  else
    scram = init;

  memset(scram, 0, sizeof(struct SCRAMNET));

  scram->sock = socket(AF_INET, SOCK_DGRAM, 0);

  /* avoid EADDRINUSE error on bind() */

  itmp = TRUE;
  if ( setsockopt(scram->sock, SOL_SOCKET, SO_REUSEADDR, &itmp,
		  sizeof(int)) < 0 )
    perror("setsockopt SO_REUSEADDR");

  /* Join a multicast group 224.1.1.1 on all interfaces */
  multiaddr.imr_multiaddr.s_addr = htonl(0xe0010104); /* 224.1.1.1 */
  multiaddr.imr_interface.s_addr = htonl(INADDR_ANY);
  if ( setsockopt(scram->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		  &multiaddr, sizeof(multiaddr)) < 0 )
    perror("sockopt");

#ifndef SOLARIS
  /* solaris doesnt use this, no se */
  if ( setsockopt(scram->sock, IPPROTO_IP, IP_MULTICAST_TTL,
		  &ttl, sizeof(ttl)) < 0 )
    perror("sockopt");
#endif

  /* setup the destination address of 224.1.1.1 and UDP port 2007 */
  scram->from.sin_family = AF_INET;
  scram->from.sin_addr.s_addr = htonl(0xe0010104);
  scram->from.sin_port = htons(2007);

  if ( bind(scram->sock, (struct sockaddr *) &scram->from,
	    sizeof(struct sockaddr_in) ) < 0 )
    perror("bind");

  return(scram);
}



int fd_scram(struct SCRAMNET *scram) {

  if ( scram )
    return(scram->sock);
  else
    return(-1);
}



int read_scram(struct SCRAMNET *scram) {

  struct BIGENOUGH *b;
  int ret;
  socklen_t fromlen;

  if ( ! scram )
    return(0);

  fromlen = sizeof(struct sockaddr_in);

  if ( recvfrom(scram->sock,&scram->in, sizeof(struct BIGENOUGH),
		MSG_WAITALL, (struct sockaddr*)&scram->from, &fromlen) < 0 ) {
    perror("recvfrom");
    return(-1);
  }

  if ( scram->lock ) /* if blocked drop read */
    return(0);

  /* printf("recvfrom %d bytes, %s\n", nbytes, scram->in.magic); */
  b = &scram->in;

  if ( strncmp("AGC", b->magic, 3) == 0 ) {
    memcpy(&scram->agcData, &b->u.agcData, sizeof( SCRM_B_AGC));
#ifndef SOLARIS
    flip_agc(&scram->agcData);
#endif
    ret = sizeof(SCRM_B_AGC);
  } else if ( strncmp(b->magic, "TT", 2) == 0 ) {
    memcpy(&scram->ttData, &b->u.ttData, sizeof(SCRM_B_TT));
#ifndef SOLARIS
    flip_tt(&scram->ttData);
#endif
    ret = sizeof(SCRM_B_TT);
  } else if ( strncmp(b->magic, "PNT", 3) == 0 ) {
    memcpy(&scram->pntData, &b->u.pntData, sizeof(SCRM_B_PNT));
#ifndef SOLARIS
    flip_pnt(&scram->pntData);
#endif
    ret = sizeof(SCRM_B_PNT);
  } else if ( strncmp(b->magic, "TIE", 3) == 0 ) {
    memcpy(&scram->tieData, &b->u.tieData, sizeof(SCRM_B_TIE));
#ifndef SOLARIS
    flip_tie(&scram->tieData);
#endif
    ret = sizeof(SCRM_B_TIE);
  } else if ( strncmp(b->magic, "IF1", 3) == 0 ) {
    memcpy(&scram->if1Data, &b->u.if1Data, sizeof(SCRM_B_IF1));
#ifndef SOLARIS
    flip_if1(&scram->if1Data);
#endif
    ret = sizeof(SCRM_B_IF1);
  } else if ( strncmp(b->magic, "IF2", 3) == 0 ) {
    memcpy(&scram->if2Data, &b->u.if2Data, sizeof(SCRM_B_IF2));
#ifndef SOLARIS
    flip_if2(&scram->if2Data);
#endif
    ret = sizeof(SCRM_B_IF2);
  } else if ( strncmp(b->magic, "EXECSHM", 7) == 0 ) {
    memcpy(&scram->exec, &b->u.exec, sizeof(struct EXECSHM));
#ifndef SOLARIS
    flip_exec(&scram->exec);
#endif
    ret = sizeof(struct EXECSHM);
  } else if ( strncmp(b->magic, "WAPP", 4) == 0 ) {
    int ix;

    if ( b->magic[4] == 0 )
      ix = 0;
    else
      ix = b->magic[4] - '0' -1;

    if ( ix < 0 || ix > 3 )
      ix = 0;

    memcpy(&scram->wapp[ix], &b->u.wapp, sizeof(struct WAPPSHM));

#ifdef SOLARIS
    flip_wapp(&scram->wapp[ix]);
#endif
    ret = sizeof(struct WAPPSHM);
  } else if ( strncmp(b->magic, "ALFASHM", 7) == 0 ) {
    memcpy(&scram->alfa, &b->u.alfa, sizeof(struct ALFASHM));
#ifdef SOLARIS
    flip_alfa(&scram->alfa);
#endif
    ret = sizeof(struct ALFASHM);
  } else
    ret = 0;
  return(ret);
}



void scramlock(struct SCRAMNET *scram, int l) {

  if ( scram )
    scram->lock = l != 0;
}



/*  compute greate circle error       */
/*  this code stolen from pntMonGet.c */

double error_greatc(struct SCRAMNET *scram) {

  double azDif, zaDif, azPos, zaPos, dtemp1, dtemp2, gcDif;

  azPos = scram->agcData.st.cblkMCur.dat.posAz * 1.0e-4;

  if ( scram->pntData.st.x.pl.req.master == MASTER_CH )
    zaPos = scram->agcData.st.cblkMCur.dat.posCh * 1.0e-4;
  else
    zaPos = scram->agcData.st.cblkMCur.dat.posGr * 1.0e-4;

  azDif = RAD_TO_DEG(scram->pntData.st.x.pl.curP.azRd) - azPos;
  zaDif = RAD_TO_DEG(scram->pntData.st.x.pl.curP.zaRd) - zaPos;

  if ( scram->pntData.st.x.pl.req.reqState == PNTX_REQ_TRACK ) {
    if ( scram->pntData.st.x.pl.req.master == MASTER_CH )
      dtemp1 = RAD_TO_DEG(scram->agcData.st.posErr.reqPosDifRd[2]);
    else
      dtemp1 = RAD_TO_DEG(scram->agcData.st.posErr.reqPosDifRd[1]);
    dtemp2 = RAD_TO_DEG(scram->agcData.st.posErr.reqPosDifRd[0]) *
             sin(DEG_TO_RAD(zaPos));
  } else {
    dtemp1 = zaDif;
    dtemp2 = azDif * sin(DEG_TO_RAD(zaPos));
  }
  gcDif = sqrt(dtemp1 * dtemp1 + dtemp2 * dtemp2)*3600.0; /* to arcseconds*/
  if ( gcDif > 999999.0 )
    gcDif = 999999.0;

  return(gcDif);
}

/*
 * file: caspern_packet.h
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-20
 */

#ifndef _CASPERN_PACKET_H_
#define _CASPERN_PACKET_H_

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Define FPGA-to-host and host-to-FPGA byte order functions.
 *
 * The FPGA fabric spits out data in anti-network-byte-order,
 * so the normal network-endian byte-functions will not work.
 * The following functions should replace them appropriately.
 */
# ifndef BYTE_ORDER
#   error BYTE_ORDER not defined. Software needs to know the byte ordering of the processor.
# elif BYTE_ORDER == LITTLE_ENDIAN
#   define ftohll(x)  (x)
#   define ftohl(x)   (x)
#   define ftohs(x)   (x)
#   define htofll(x)  (x)
#   define htofl(x)   (x)
#   define htofs(x)   (x)
# elif BYTE_ORDER == BIG_ENDIAN
#   define ftohll(x)  __bswap_64 (x)
#   define ftohl(x)   __bswap_32 (x)
#   define ftohs(x)   __bswap_16 (x)
#   define htofll(x)  __bswap_64 (x)
#   define htofl(x)   __bswap_32 (x)
#   define htofs(x)   __bswap_16 (x)
# else
#   error BYTE_ORDER is defined but not recognized. What kind of crazy architecture is this?
# endif

/*
 * Structure Definitions
 */

typedef struct caspern_header
{
	uint64_t time;
	uint32_t x_engine;
	uint32_t data_offset;
	uint32_t flags;
	uint32_t data_length;
} CASPERN_HEADER;

typedef struct caspern_packet
{
	struct caspern_header header;
	uint8_t *data;
} CASPERN_PACKET;

/*
 * Function Declarations
 */

CASPERN_PACKET *caspern_packet_cast(uint8_t *pkt_data, size_t pkt_size);

#endif // _CASPERN_PACKET_H_

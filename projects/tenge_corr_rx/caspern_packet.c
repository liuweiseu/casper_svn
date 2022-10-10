/*
 * file: caspern_packet.c
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-20
 */

#include "caspern_packet.h"

/*
 * Function Definitions
 */

CASPERN_PACKET *caspern_packet_cast(uint8_t *pkt_data, size_t pkt_size)
{
	CASPERN_PACKET *pkt = (CASPERN_PACKET *)(pkt_data);
	CASPERN_HEADER *hdr = &pkt->header;

	hdr->time = ftohll(hdr->time);
	hdr->x_engine = ftohl(hdr->x_engine);
	hdr->data_offset = ftohl(hdr->data_offset);
	hdr->flags = ftohl(hdr->flags);
	hdr->data_length = ftohl(hdr->data_length);

	pkt->data = (uint8_t *)(pkt_data + sizeof(CASPERN_HEADER));

	if (hdr->data_length + sizeof(CASPERN_HEADER) != pkt_size)
	{
		fprintf(stderr, "WARNING: Packet length mismatch.\n");
		// TODO: error accordingly.
	}

	return pkt;
}

/*
 * isi_push.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 08-Apr-2010
 */

#include "libfpga.h"

typedef struct isi_corr_pkt
{
	uint8_t board_id;
	uint8_t group_id;
	uint16_t unused;
	uint32_t pkt_id;
	uint8_t data[8064];
	uint8_t zero[120];
} PKT;

uint32_t get_board_id (int pid);
void init_packets (int pid, PKT *A, PKT *B, PKT *C);

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s [host] [port]\n", argv[0]);
		exit(1);
	}

	char *host = argv[1];
	char *port = argv[2];

	int pid = find_bof_pid();

	struct isi_corr_pkt pkt_A;
	struct isi_corr_pkt pkt_B;
	struct isi_corr_pkt pkt_C;

	init_packets(pid, &pkt_A, &pkt_B, &pkt_C);

	int addr = open_fpga_device_ro(pid, "vacc_A_addr");
	int bram_A = open_fpga_device_ro(pid, "vacc_A_bram");
	int bram_B = open_fpga_device_ro(pid, "vacc_B_bram");
	int bram_C = open_fpga_device_ro(pid, "vacc_C_bram");
	int sock = open_udp_send_socket(host, port);

	init_signals();

	uint32_t addr0, addr1;
	uint32_t pkt_id0, pkt_id1;
	uint16_t offset;

	pkt_id1 = -1;
	while (NOT_KILLED)
	{
		read_fpga_register(addr, &addr0);
		pkt_id0 = addr0 >> 12;
		offset = addr0 & 0x0800;

		if (pkt_id0 == pkt_id1)
		{
			usleep(POLL_INTERVAL);
			continue;
		}

		read_fpga_bram(bram_A, &pkt_A.data, sizeof(pkt_A.data), offset);
		read_fpga_bram(bram_B, &pkt_B.data, sizeof(pkt_B.data), offset);
		read_fpga_bram(bram_C, &pkt_C.data, sizeof(pkt_C.data), offset);

		read_fpga_register(addr, &addr1);
		pkt_id1 = addr1 >> 12;

		if (pkt_id0 != pkt_id1)
		{
			printf("Dropped a packet!\n");
			continue;
		}

		pkt_A.pkt_id = pkt_id1;
		pkt_B.pkt_id = pkt_id1;
		pkt_C.pkt_id = pkt_id1;

		Send(sock, &pkt_A, sizeof(PKT), 0);
		Send(sock, &pkt_B, sizeof(PKT), 0);
		Send(sock, &pkt_C, sizeof(PKT), 0);
	}

	close_file(addr);
	close_file(bram_A);
	close_file(bram_B);
	close_file(bram_C);
	close_file(sock);

	return 0;
}

uint32_t get_board_id (int pid)
{
	uint32_t board_id;

	int board_id_reg = open_fpga_device_ro(pid, "corr_id");
	read_fpga_register(board_id_reg, &board_id);
	close_file(board_id_reg);

	return board_id;
}

void init_packets (int pid, PKT *A, PKT *B, PKT *C)
{
	memset(A, 0, sizeof(PKT));
	memset(B, 0, sizeof(PKT));
	memset(C, 0, sizeof(PKT));

	uint32_t id = get_board_id(pid);

	A->board_id = id;
	B->board_id = id;
	C->board_id = id;

	A->group_id = 3 * id + 0;
	B->group_id = 3 * id + 1;
	C->group_id = 3 * id + 2;
}

/*
 * fifo_rx.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 13-Feb-2008
 */

#include "libfpga.h"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s [port] [file]\n", argv[0]);
		exit(1);
	}

	char *port = argv[1];
	char *path = argv[2];

	int sock = open_udp_recv_socket(port);
	void *data = calloc(FIFO_WIDTH, sizeof(uint32_t));
	int file = open_file_wo(path);

	init_signals();

	while (NOT_KILLED)
	{
		Recv(sock, data, FIFO_WIDTH, 0);
		Write(file, data, FIFO_WIDTH);
	}

	close_file(sock);
	free(data);
	close_file(file);

	return 0;
}

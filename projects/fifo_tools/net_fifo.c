/*
 * net_fifo.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 18-Feb-2008
 */

#include "libfpga.h"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s [port] [path]\n", argv[0]);
		exit(1);
	}

	char *port = argv[1];
	char *path = argv[2];

	int sock = open_udp_recv_socket(port);
	void *data = calloc(FIFO_WIDTH, sizeof(uint32_t));
	int file = open_unix_fifo_wo(path);

	init_signals();

	ssize_t num_bytes = 0;
	while (NOT_KILLED)
	{
		num_bytes = Recv(sock, data, FIFO_WIDTH, 0);
		Write(file, data, num_bytes);
	}

	close_file(sock);
	free(data);
	close_file(file);

	return 0;
}

/*
 * fifo_tx.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 13-Feb-2008
 */

#include "libfpga.h"

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		printf("Usage: %s [dev] [host] [port]\n", argv[0]);
		exit(1);
	}

	char *dev = argv[1];
	char *host = argv[2];
	char *port = argv[3];

	int pid = find_bof_pid();

	int fifo = open_fpga_device_ro(pid, dev);
	void *data = calloc(FIFO_WIDTH, sizeof(uint32_t));
	int sock = open_udp_send_socket(host, port);

	init_signals();

	ssize_t bytes_read;
	while (NOT_KILLED)
	{
		bytes_read = Read(fifo, data, FIFO_WIDTH);
		if (bytes_read > 0)
		{
			Send(sock, data, FIFO_WIDTH, 0);
		}
		else
		{
			usleep(POLL_INTERVAL);
		}
	}

	close_file(fifo);
	free(data);
	close_file(sock);

	return 0;
}

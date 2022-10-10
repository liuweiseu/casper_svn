/*
 * fifo_cat.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 17-Feb-2008
 */

#include "libfpga.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s [dev]\n", argv[0]);
		exit(1);
	}

	char *dev = argv[1];

	int pid = find_bof_pid();

	int fifo = open_fpga_device_ro(pid, dev);
	void *data = calloc(FIFO_WIDTH, sizeof(uint32_t));

	init_signals();

	ssize_t bytes_read;
	while (NOT_KILLED)
	{
		bytes_read = Read(fifo, data, FIFO_WIDTH);
		if (bytes_read > 0)
		{
			hexdump(data, FIFO_WIDTH);
		}
		else
		{
			usleep(POLL_INTERVAL);
		}
	}

	close_file(fifo);
	free(data);

	return 0;
}

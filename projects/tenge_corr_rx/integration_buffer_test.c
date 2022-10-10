/*
 * file: integration_buffer_test.c
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-20
 */

#include "integration_buffer.h"

int main (int argc, char **argv)
{
	size_t n_byte = 294912; // bytes per integration per X engine
	size_t n_xeng = 8;      // number of X engines sending data
	size_t n_ints = 4;      // number of integrations to buffer
	size_t buf_size = n_byte * n_xeng * n_ints;

	printf("Creating a %ld byte table buffer "
			"(%ld ints X %ld xeng X %ld B/xeng/int).\n",
			buf_size, n_ints, n_xeng, n_byte);
	INTEGRATION_BUFFER *ib = integration_buffer_create(n_byte, n_xeng, n_ints);
	printf("Success!\n");

	printf("Deleting table buffer.\n");
	integration_buffer_delete(ib);
	printf("Success!\n");

	return 0;
}

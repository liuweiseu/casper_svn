/*
 * file: mem2mir.c
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-11-15
 */

#include "mem2mir.h"

/*
 * Main()
 */
int main(int argc, char **argv)
{
	parseCommandLine(argc, argv);

	const char path[] = "/dev/shm/integration-buffer";
	INTEGRATION_BUFFER *ib = integration_buffer_import(path);
	INT_BUF_METADATA *metadata = &ib->metadata;

	int matrix_size = metadata->n_xengines * metadata->n_bytes_per_xeng_per_int;
	double *matrix = (double *)calloc(matrix_size, sizeof(double));
	uint64_t timestamp = 0;

	// open a new miriad file
	uv_init();

	// start listening for Ctrl-C
	signal(SIGINT, cleanup);

	while(1)
	{
		INTEGRATION_ITEM *integration = NULL;

		integration = integration_buffer_get_next_full_int(ib);
		corner_turn_and_cast(metadata, integration, matrix);
		timestamp = integration->timestamp;
		integration_buffer_remove_item(ib, integration);

		uv_write_int(matrix, timestamp);
	}

	free(matrix);

	return 0;
}

void corner_turn_and_cast(INT_BUF_METADATA *metadata, INTEGRATION_ITEM *integration, double *matrix)
{
	uint8_t *src = integration->xeng_list[0].data;
	double *dst = matrix;

	int ydim = metadata->n_xengines;
	int xdim = metadata->n_bytes_per_xeng_per_int;

	int i, j;
	for (j=0; j<ydim; j++)
	{
		for (i=0; i<xdim; i++)
		{
			dst[j + i * ydim] = (double)src[i + j * xdim];
		}
	}
}

void cleanup(int signal)
{
	printf("Ctrl-C caught! Cleaning up.\n");

	// close the miriad file
	uv_close();

	// TODO: free matrix.

	printf("Cleanup complete. Goodbye!\n");

	exit(0);
}

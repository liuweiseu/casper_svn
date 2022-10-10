/*
 * file: net2mem.c
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-28
 */

#include "net2mem.h"

/*
 * Main()
 */
int main(int argc, char **argv)
{
	parseCommandLine(argc, argv);
	calculate_integration_parameters();

	size_t list_length = corr.rx_buffer_size / corr.max_payload_len;
	size_t buffer_size = corr.rx_buffer_size;
	RING_BUFFER *caspern_pkt_buffer = ring_buffer_create(list_length, buffer_size);

	size_t n_byte = corr.vec_size * corr.vecs_per_xeng; // bytes per integration per X engine
	size_t n_xeng = corr.n_xeng;                        // number of X engines sending data
	size_t n_ints = corr.int_buffer_size;               // number of integrations to buffer
	INTEGRATION_BUFFER *integration_buffer = integration_buffer_create(n_byte, n_xeng, n_ints);

	NET_THREAD_ARGS network_thread_args;
	network_thread_args.pkt_buffer = caspern_pkt_buffer;

	ASM_THREAD_ARGS assemble_thread_args;
	assemble_thread_args.pkt_buffer = caspern_pkt_buffer;
	assemble_thread_args.int_buffer = integration_buffer;

	// start listening for Ctrl-C
	signal(SIGINT, cleanup);

	setbuf(stdout, NULL);

	pthread_t network_thread, assemble_thread;
	pthread_create(&network_thread, NULL, network_thread_function, &network_thread_args);
	pthread_create(&assemble_thread, NULL, assemble_thread_function, &assemble_thread_args);

	pthread_join(network_thread, NULL);
	pthread_join(assemble_thread, NULL);

	return 0;
}

/*
 * Read data from the network.
 * Write data to ring buffer.
 */
void *network_thread_function(void *arg)
{
	NET_THREAD_ARGS *args = (NET_THREAD_ARGS *)arg;
	RING_BUFFER *ring_buf = args->pkt_buffer;

	RING_ITEM *this_slot = ring_buf->write_ptr;
	RING_ITEM *next_slot = NULL;

	socket_t sock = setup_network_listener();
	void *buffer = NULL;
	size_t length = corr.max_payload_len;
	int flags = 0;
	SA_in addr; // packet source's address
	socklen_t addr_len = sizeof(addr);
	ssize_t num_bytes = 0;

	debug_printf("Entering network thread loop.\n");

	/*
	 * loop forever:
	 *   update relevant local pointers,
	 *   wait for next free buffer slot,
	 *   grab current buffer slot write_mutex,
	 *   read data from network into the slot,
	 *   release the buffer slot read_mutex,
	 *   validate received data based on length,
	 *   advance write pointer to next buffer slot.
	 */
	while (1)
	{
		next_slot = this_slot->next;
		buffer = this_slot->data;

		sem_wait(&this_slot->write_mutex);
		num_bytes = recvfrom(sock, buffer, length, flags, (SA *)&addr, &addr_len);
		this_slot->size = num_bytes;
		sem_post(&this_slot->read_mutex);

		if (num_bytes == -1)
		{
			perror("Unable to receive packet.\n");
			exit(1);
		}
		else
		{
			//debug_printf("[net thread] Received %ld bytes.\n", num_bytes);
		}

		this_slot = next_slot;
	} // end while
}

/*
 * Bind to a port and listen for incoming data.
 */
int setup_network_listener()
{
	int sock = -1;
	struct sockaddr_in my_addr; // server's address information
	int ret = 0;

	// create a new UDP socket descriptor
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("Unable to set socket descriptor.\n");
		exit(1);
	}

	// initialize local address struct
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons(corr.port); // short, network byte order
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen on all interfaces
	memset(my_addr.sin_zero, 0, sizeof(my_addr.sin_zero));

	// bind socket to local address
	ret = bind(sock, (SA *)&my_addr, sizeof(my_addr));
	if (ret == -1)
	{
		perror("Unable to bind to socket.\n");
		exit(1);
	}

	// prevent "address already in use" errors
	const int on = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
	if (ret == -1)
	{
		perror("setsockopt");
		exit(1);
	}

	debug_printf("Listening on IP address %s on port %i\n", inet_ntoa(my_addr.sin_addr), corr.port);

	return sock;
}

/*
 * Map the ring buffer of variable-length raw packets
 * into another buffer of fixed-length integrations.
 */
void *assemble_thread_function(void *arg)
{
	ASM_THREAD_ARGS *args = (ASM_THREAD_ARGS *)arg;
	RING_BUFFER *pkt_buf = args->pkt_buffer;
	INTEGRATION_BUFFER *int_buf = args->int_buffer;

	RING_ITEM *this_slot = pkt_buf->write_ptr;
	RING_ITEM *next_slot = NULL;

	debug_printf("[asm thread] Entering assemble thread loop.\n");

	/*
	 * loop forever:
	 *   update relevant local pointers,
	 *   wait for next full buffer slot,
	 *   grab current buffer slot read_mutex,
	 *   process packet in current buffer slot,
	 *   release the buffer slot write_mutex,
	 *   advance write pointer to next buffer slot.
	 */
	while (1)
	{
		next_slot = this_slot->next;

		sem_wait(&this_slot->read_mutex);
		assemble_integration(this_slot, int_buf);
		sem_post(&this_slot->write_mutex);

		this_slot = next_slot;
	} // end while
}

void assemble_integration(RING_ITEM *cur_pkt, INTEGRATION_BUFFER *int_buf)
{
	CASPERN_PACKET *src_pkt = NULL;
	INTEGRATION_ITEM *dst_int = NULL;

	src_pkt = caspern_packet_cast(cur_pkt->data, cur_pkt->size);
	src_pkt->header.time >>= TIMESTAMP_TRUNC;
	dst_int = integration_buffer_insert_pkt(int_buf, src_pkt);
	integration_transcribe(dst_int, src_pkt);

	// check if X engine is full.  if so,
	// then it should be flushed to disk.
	if (dst_int->unfinished_count == 0)
	{
		debug_printf("[asm thread] Integration time %ld: Fully reassembled.\n", dst_int->timestamp);

		sem_wait(&dst_int->metadata.mutex);
		dst_int->metadata.data_ready = true;
		sem_post(&dst_int->metadata.mutex);

		sem_post(&int_buf->ready_count);
	}
}

/*
 * Calculate useful correlator parameters.
 */
void calculate_integration_parameters()
{
	corr.n_ants = corr_config.n_ants;
	corr.n_xeng = corr_config.n_servers * corr_config.x_per_fpga;
	corr.n_chans = corr_config.n_chans;
	corr.n_stokes = corr_config.n_stokes;
	corr.n_bls = (corr.n_ants * (corr.n_ants + 1)) / 2;

	corr.vecs_per_int = corr.n_stokes * corr.n_bls * corr.n_chans;
	corr.vecs_per_xeng = corr.vecs_per_int / corr.n_xeng;
	corr.vec_size = sizeof(uint64_t);

	corr.chans_per_xeng = corr.n_chans / corr.n_xeng;
	corr.vecs_per_chan = corr.n_bls * corr.n_stokes;
	corr.table_offset = corr.chans_per_xeng / 2;

	corr.rx_buffer_size = corr_config.rx_buffer_size;   // in bytes
	corr.int_buffer_size = corr_config.n_int_buffer;    // in integrations
	corr.max_payload_len = corr_config.max_payload_len; // in bytes

	corr.port = corr_config.udp_port;

	debug_printf("Expecting data from %d antennas.\n", corr.n_ants);
	debug_printf("Expecting data from %d baselines.\n", corr.n_bls);
}

void cleanup(int signal)
{
	printf("Ctrl-C caught! Cleaning up.\n");

	const char filename[] = "/dev/shm/integration-buffer";
	remove(filename);
	printf("Removing file: %s\n", filename);

	printf("Cleanup complete. Goodbye!\n");

	exit(0);
}

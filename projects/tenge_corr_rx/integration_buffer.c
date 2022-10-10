/*
 * file: integration_buffer.c
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-20
 */

#include "integration_buffer.h"

/*****************************************
 *
 * BUFFER MANAGEMENT: PUBLIC METHODS
 *
 *****************************************/

/*
 * Construct and initialize an INTEGRATION_BUFFER.
 */
INTEGRATION_BUFFER *integration_buffer_create(size_t n_byte, size_t n_xeng, size_t n_ints)
{
	char ib_path[] = "/dev/shm/integration-buffer";
	size_t ib_datastructure_size = sizeof(INTEGRATION_BUFFER);
	size_t integration_list_size = n_ints * sizeof(INTEGRATION_ITEM);
	size_t one_xengine_list_size = n_xeng * sizeof(XENGINE_ITEM);
	size_t all_xengine_list_size = n_ints * one_xengine_list_size;
	size_t integration_data_size = n_byte * n_xeng * n_ints;

	size_t ib_size = 0;
	ib_size += ib_datastructure_size;
	ib_size += integration_list_size;
	ib_size += all_xengine_list_size;
	ib_size += integration_data_size;

	void *ib_datastructure_ptr = NULL;
	void *integration_list_ptr = NULL;
	void *all_xengine_list_ptr = NULL;
	void *integration_data_ptr = NULL;

	int ib_fd = ib_create_shared_file(ib_path, ib_size);
	void *ib_memory = ib_map_memory(ib_fd, ib_size);
	memset(ib_memory, 0, ib_size);

	ib_datastructure_ptr = ib_memory;
	integration_list_ptr = ib_datastructure_ptr + ib_datastructure_size;
	all_xengine_list_ptr = integration_list_ptr + integration_list_size;
	integration_data_ptr = all_xengine_list_ptr + all_xengine_list_size;

	// create integration table
	INTEGRATION_ITEM *integration_list = (INTEGRATION_ITEM *)integration_list_ptr;
	int i;
	for (i=0; i<n_ints; i++)
	{
		// calculate list pointers
		INTEGRATION_ITEM *this_integration = &integration_list[i];
		INTEGRATION_ITEM *next_integration = NULL;
		if (i < n_ints - 1)
		{
			next_integration = &integration_list[i + 1];
		}

		// create X engine table
		uint8_t *this_integration_data = integration_data_ptr + i * n_xeng * n_byte;
		XENGINE_ITEM *this_xeng_list = (XENGINE_ITEM *)(all_xengine_list_ptr + i * one_xengine_list_size);
		int j;
		for (j=0; j<n_xeng; j++)
		{
			XENGINE_ITEM *this_xeng = &this_xeng_list[j];
			this_xeng->data = this_integration_data + j * n_byte;
			this_xeng->length = n_byte;
		}

		sem_init(&this_integration->metadata.mutex, 1, 1);
		this_integration->next = next_integration;
		this_integration->xeng_list = this_xeng_list;
		this_integration->xeng_count = n_xeng;
		this_integration->unfinished_count = n_xeng;
	}

	// create integration list
	INTEGRATION_BUFFER *ib = (INTEGRATION_BUFFER *)ib_datastructure_ptr;
	ib->metadata.memory_addr = ib;
	ib->metadata.memory_size = ib_size;
	ib->metadata.n_bytes_per_xeng_per_int = n_byte;
	ib->metadata.n_xengines = n_xeng;
	ib->metadata.n_integrations = n_ints;
	ib->data_addr = integration_data_ptr;
	ib->data_size = integration_data_size;
	ib->list_ptr = integration_list;
	ib->list_length = n_ints;
	sem_init(&ib->ready_count, 1, 0);
	sem_init(&ib->pointer_lock, 1, 1);
	ib->first = NULL;
	ib->last = NULL;
	ib->free = ib->list_ptr;
	ib->current = NULL;

	ib_dump_state(ib);
	return ib;
}

/*
 * Open and parse an INTEGRATION_BUFFER.
 */
INTEGRATION_BUFFER *integration_buffer_import(const char *path)
{
	INTEGRATION_BUFFER *ib = NULL;

	INT_BUF_METADATA metadata;
	memset(&metadata, 0, sizeof(INT_BUF_METADATA));

	ib_read_metadata(path, &metadata);
	ib = ib_import(path, &metadata);

	ib_dump_state(ib);
	return ib;
}

/*
 * Destroy an INTEGRATION_BUFFER.
 */
void integration_buffer_delete(INTEGRATION_BUFFER *ib)
{
	int status = 0;

	// delete mmap'd buffer
	void *buffer = ib->data_addr;
	size_t buf_size = ib->data_size;
	memset(buffer, 0, buf_size);
	status = munmap(buffer, buf_size);
	if (status)
	{
		perror("munmap");
	}
	buffer = NULL;

	// delete integration table
	INTEGRATION_ITEM *integration_list = ib->list_ptr;
	size_t n_ints = ib->list_length;
	int i;
	for (i=0; i<n_ints; i++)
	{
		INTEGRATION_ITEM *this_item = &integration_list[i];

		// delete X engine table
		memset(this_item->xeng_list, 0, this_item->xeng_count * sizeof(XENGINE_ITEM));
		free(this_item->xeng_list);
		this_item->xeng_list = NULL;

		sem_destroy(&this_item->metadata.mutex);
		memset(this_item, 0, sizeof(INTEGRATION_ITEM));
	}
	free(integration_list);
	integration_list = NULL;

	// delete ring buffer
	sem_destroy(&ib->ready_count);
	sem_destroy(&ib->pointer_lock);
	memset(ib, 0, sizeof(INTEGRATION_BUFFER));
	free(ib);
	ib = NULL;
}

INTEGRATION_ITEM *integration_buffer_insert_pkt(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	INTEGRATION_ITEM *item = NULL;

	sem_wait(&ib->pointer_lock);
	item = ib_insert_pkt(ib, pkt);
	sem_post(&ib->pointer_lock);

	return item;
}

INTEGRATION_ITEM *integration_buffer_get_next_full_int(INTEGRATION_BUFFER *ib)
{
	INTEGRATION_ITEM *item = NULL;

	do
	{
		sem_wait(&ib->ready_count);

		sem_wait(&ib->pointer_lock);
		item = ib_find_oldest_ready_item(ib);
		sem_post(&ib->pointer_lock);
	}
	while (item == NULL);

	sem_wait(&item->metadata.mutex);
	item->metadata.busy_reading = true;
	sem_post(&item->metadata.mutex);

	debug_printf("Next full int: %lld\n", item->timestamp);

	return item;
}

void integration_buffer_remove_item(INTEGRATION_BUFFER *ib, INTEGRATION_ITEM *item)
{
	sem_wait(&item->metadata.mutex);
	item->metadata.done_reading = true;
	sem_post(&item->metadata.mutex);

	sem_wait(&ib->pointer_lock);
	ib_remove_item(ib, item);
	sem_post(&ib->pointer_lock);

	ib_dump_state(ib);
}

/*****************************************
 *
 * BUFFER MANAGEMENT: PRIVATE METHODS
 *
 *****************************************/

int ib_create_shared_file(const char *path, size_t size)
{
	int fd = -1;
	int status = 0;

	fd = open(path, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	if (fd < 0)
	{
		perror("open");
	}

	// truncate file to exact buffer size
	status = ftruncate(fd, size);
	if (status)
	{
		perror("ftruncate");
	}

	return fd;
}

int ib_open_shared_file(const char *path)
{
	int fd = -1;

	fd = open(path, O_RDWR);
	if (fd < 0)
	{
		perror("open");
	}

	return fd;
}

void ib_close_shared_file(int fd)
{
	int status = 0;

	status = close(fd);
	if (status)
	{
		perror("close");
	}
}

void *ib_map_memory(int file, size_t size)
{
	void *memory = NULL;

	void *addr = NULL;
	size_t len = size;
	int prot = PROT_READ|PROT_WRITE;
	int flags = MAP_SHARED;
	int fd = file;
	off_t off = 0;

	memory = mmap(addr, len, prot, flags, fd, off);
	if (memory == MAP_FAILED)
	{
		perror("mmap");
	}

	return memory;
}

void ib_unmap_memory(void *addr, size_t size)
{
	int status;

	status = munmap(addr, size);
	if (status)
	{
		perror("munmap");
	}
}

void ib_read_metadata(const char *path, INT_BUF_METADATA *metadata)
{
	void *src = NULL;
	void *dst = NULL;
	int file = 0;
	size_t size = 0;

	dst = (void *)metadata;
	size = sizeof(INT_BUF_METADATA);

	file = ib_open_shared_file(path);
	src = ib_map_memory(file, size);
	ib_close_shared_file(file);
	memcpy(dst, src, size);
	ib_unmap_memory(src, size);
}

INTEGRATION_BUFFER *ib_import(const char *path, INT_BUF_METADATA *metadata)
{
	void *memory = NULL;

	void *addr = metadata->memory_addr;
	size_t length = metadata->memory_size;
	int prot = PROT_READ|PROT_WRITE;
	int flags = MAP_SHARED;
	int fd = ib_open_shared_file(path);
	off_t off = 0;

	memory = mmap(addr, length, prot, flags, fd, off);
	if (memory == MAP_FAILED)
	{
		perror("mmap");
	}

	return (INTEGRATION_BUFFER *)memory;
}

INTEGRATION_ITEM *ib_insert_pkt(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	if (ib->first == NULL)
	{
		return ib_add_first_item(ib, pkt);
	}

	if (pkt->header.time == ib->current->timestamp)
	{
		return ib->current;
	}

	if (pkt->header.time < ib->first->timestamp)
	{
		return ib_prepend_item(ib, pkt);
	}

	if (pkt->header.time > ib->last->timestamp)
	{
		return ib_append_item(ib, pkt);
	}

	return ib_inject_item(ib, pkt);
}

void ib_remove_item(INTEGRATION_BUFFER *ib, INTEGRATION_ITEM *item)
{
	debug_printf("[int buffer] remove_item(%ld)\n", item->timestamp);

	if (item == ib->first)
	{
		INTEGRATION_ITEM *old_first;

		old_first = ib->first;
		ib->first = ib->first->next;

		old_first->next = ib->free;
		ib->free = old_first;
		integration_reset(old_first);

		if (item == ib->last)
		{
			ib->last = ib->first;
		}

		if (item == ib->current)
		{
			ib->current = ib->first;
		}
	}
	else
	{
		INTEGRATION_ITEM *prev = NULL;
		INTEGRATION_ITEM *iter;

		iter = ib->first;
		while (iter->next != NULL)
		{
			if (item == iter->next)
			{
				prev = iter;
				break;
			}

			iter = iter->next;
		}

		prev->next = item->next;
		item->next = ib->free;
		ib->free = item;
		integration_reset(item);

		if (item == ib->last)
		{
			ib->last = prev;
		}

		if (item == ib->current)
		{
			ib->current = prev;
		}
	}
}

INTEGRATION_ITEM *ib_add_first_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	debug_printf("[int buffer] add_first_item(%ld)\n", pkt->header.time);

	ib->first = ib->free;
	ib->free = ib->free->next;
	ib->first->next = NULL;
	integration_init(ib->first, pkt);

	ib->last = ib->first;
	ib->current = ib->first;

	ib_dump_state(ib);
	return ib->current;
}

INTEGRATION_ITEM *ib_prepend_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	debug_printf("[int buffer] prepend_item(%ld)\n", pkt->header.time);

	if (ib->free == NULL)
	{
		// no space for old data.
		ib_dump_state(ib);
		return NULL;
	}

	INTEGRATION_ITEM *new_first;
	new_first = ib->free;
	ib->free = ib->free->next;

	new_first->next = ib->first;
	integration_init(new_first, pkt);

	ib->first = new_first;
	ib->current = ib->first;

	ib_dump_state(ib);
	return ib->current;
}

INTEGRATION_ITEM *ib_append_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	debug_printf("[int buffer] append_item(%ld)\n", pkt->header.time);

	if (ib->free == NULL)
	{
		ib_free_oldest_item(ib);
	}

	INTEGRATION_ITEM *new_last;

	new_last = ib->free;
	ib->free = ib->free->next;

	ib->last->next = new_last;
	new_last->next = NULL;
	ib->last = new_last;
	integration_init(new_last, pkt);

	ib->current = ib->last;

	ib_dump_state(ib);
	return ib->current;
}

INTEGRATION_ITEM *ib_inject_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt)
{
	debug_printf("[int buffer] inject_item(%ld)\n", pkt->header.time);

	INTEGRATION_ITEM *iter = ib->first;

	if (pkt->header.time == iter->timestamp)
	{
		ib->current = iter;
		ib_dump_state(ib);
		return ib->current;
	}

	while (iter->next != NULL)
	{
		if (pkt->header.time < iter->next->timestamp)
		{
			if (ib->free == NULL)
			{
				ib_free_oldest_item(ib);
			}

			INTEGRATION_ITEM *new_item;
			new_item = ib->free;
			ib->free = ib->free->next;

			new_item->next = iter->next;
			iter->next = new_item;
			integration_init(new_item, pkt);

			ib->current = new_item;
			ib_dump_state(ib);
			return ib->current;
		}

		if (pkt->header.time == iter->next->timestamp)
		{
			ib->current = iter->next;
			ib_dump_state(ib);
			return ib->current;
		}

		iter = iter->next;
	}

	ib_dump_state(ib);
	return NULL;
}

INTEGRATION_ITEM *ib_find_oldest_ready_item(INTEGRATION_BUFFER *ib)
{
	INTEGRATION_ITEM *iter;

	iter = ib->first;
	do
	{
		sem_wait(&iter->metadata.mutex);

		if (iter->metadata.data_ready)
		{
			sem_post(&iter->metadata.mutex);
			break;
		}
		else
		{
			sem_post(&iter->metadata.mutex);
			iter = iter->next;
		}
	}
	while (iter != NULL);

	return iter;
}

INTEGRATION_ITEM *ib_find_oldest_unbusy_item(INTEGRATION_BUFFER *ib)
{
	INTEGRATION_ITEM *iter;

	iter = ib->first;
	do
	{
		sem_wait(&iter->metadata.mutex);

		if (!iter->metadata.busy_reading)
		{
			sem_post(&iter->metadata.mutex);
			break;
		}
		else
		{
			sem_post(&iter->metadata.mutex);
			iter = iter->next;
		}
	}
	while (iter != NULL);

	return iter;
}

void ib_free_oldest_item(INTEGRATION_BUFFER *ib)
{
	INTEGRATION_ITEM *item = NULL;

	item = ib_find_oldest_unbusy_item(ib);
	ib_remove_item(ib, item);

	ib_dump_state(ib);
}

void ib_dump_state(const INTEGRATION_BUFFER *ib)
{
	INTEGRATION_ITEM *iter;

	iter = ib->first;
	if (iter == NULL)
	{
		debug_printf("list:\n");
		debug_printf("  [empty]\n");
	}
	else
	{
		debug_printf("list:\n");

		while (iter != NULL)
		{
			debug_printf("  %#lx (%ld)\n", (unsigned long int)iter, iter->timestamp);

			iter = iter->next;
		}
	}

	iter = ib->free;
	if (iter == NULL)
	{
		debug_printf("free:\n");
		debug_printf("  [empty]\n");
	}
	else
	{
		debug_printf("free:\n");

		while (iter != NULL)
		{
			debug_printf("  %#lx (%ld)\n", (unsigned long int)iter, iter->timestamp);

			iter = iter->next;
		}
	}

	if (ib->first == NULL)
	{
		debug_printf("first: NULL\n");
	}
	else
	{
		debug_printf("first: %#lx (%ld)\n", (unsigned long int)ib->first, ib->first->timestamp);
	}

	if (ib->last == NULL)
	{
		debug_printf("last:  NULL\n");
	}
	else
	{
		debug_printf("last:  %#lx (%ld)\n", (unsigned long int)ib->last, ib->last->timestamp);
	}
}

/*****************************************
 *
 * INTEGRATION MANAGEMENT: PUBLIC METHODS
 *
 *****************************************/

/*
 * Initialize an INTEGRATION_ITEM based on a CASPERN_PACKET.
 */
void integration_init(INTEGRATION_ITEM *integration, CASPERN_PACKET *pkt)
{
	integration->timestamp = pkt->header.time;
}

/*
 * Transcribe a CASPERN_PACKET into an INTEGRATION_ITEM.
 */
void integration_transcribe(INTEGRATION_ITEM *dst_int, CASPERN_PACKET *src_pkt)
{
	CASPERN_HEADER *pkt_hdr;
	uint32_t xeng_id;
	uint32_t offset;
	XENGINE_ITEM *xeng;

	void *dst;
	const void *src;
	size_t len;

	// check if packet should be dropped.
	if (dst_int == NULL)
	{
		debug_printf("[asm thread] Dropped an out-of-date packet.\n");
		return;
	}

	// check if X engine is already full.
	if (dst_int->unfinished_count == 0)
	{
		debug_printf("[asm thread] WARNING: Unfinished X engine counter already zero!\n");
	}

	// gather useful information
	pkt_hdr = &src_pkt->header;
	xeng_id = pkt_hdr->x_engine;
	offset = pkt_hdr->data_offset;
	xeng = &dst_int->xeng_list[xeng_id];

	// prepare to copy packet
	dst = xeng->data + offset;
	src = src_pkt->data;
	len = pkt_hdr->data_length;

	// check for buffer overflow
	if (offset + len > xeng->length)
	{
		debug_printf("[asm thread] WARNING: Packet longer than integration buffer. Truncating.");
		len = xeng->length - offset;
	}

	// copy packet into integration
	memcpy(dst, src, len);

	// update integration data counter
	uint32_t old_rx_count = xeng->rx_count;
	xeng->rx_count += len;

	// check for too much counted data
	if (xeng->rx_count > xeng->length)
	{
		debug_printf("[asm thread] WARNING: X engine #%d received more data than expected!\n", xeng_id);
	}

	// check if X engine became full.
	if (xeng->rx_count >= xeng->length
		&& old_rx_count < xeng->length)
	{
		dst_int->unfinished_count--;

		debug_printf("[asm thread] Integration time %ld: X engine #%d is now full.\n", dst_int->timestamp, xeng_id);
	}
}

/*
 * Reset an INTEGRATION_ITEM.
 */
void integration_reset(INTEGRATION_ITEM *integration)
{
	size_t n_xeng = integration->xeng_count;

	integration->timestamp = 0;
	sem_wait(&integration->metadata.mutex);
	integration->metadata.data_ready = false;
	integration->metadata.busy_reading = false;
	integration->metadata.done_reading = false;
	sem_post(&integration->metadata.mutex);

	int i;
	for (i=0; i<n_xeng; i++)
	{
		xengine_reset(&integration->xeng_list[i]);
	}

	integration->unfinished_count = n_xeng;
}

/*
 * Reset an XENGINE_ITEM.
 */
void xengine_reset(XENGINE_ITEM *xengine)
{
	memset(xengine->data, 0, xengine->length * sizeof(uint8_t));
	xengine->rx_count = 0;
}

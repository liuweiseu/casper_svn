/*
 * file: integration_buffer.h
 * auth: William Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-10-20
 */

#ifndef _INTEGRATION_BUFFER_H_
#define _INTEGRATION_BUFFER_H_

#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "caspern_packet.h"
#include "debug_macros.h"

/*
 * Structure Definitions
 */

typedef struct xengine_item {
	uint8_t *data;
	size_t length;
	uint32_t rx_count;
} XENGINE_ITEM;

typedef struct int_metadata {
	sem_t mutex;
	bool data_ready;
	bool busy_reading;
	bool done_reading;
} INT_METADATA;

typedef struct integration_item {
	struct int_metadata metadata;

	struct integration_item *next;
	uint64_t timestamp;

	struct xengine_item *xeng_list;
	size_t xeng_count;
	uint32_t unfinished_count;
} INTEGRATION_ITEM;

typedef struct int_buf_metadata {
	struct integration_buffer *memory_addr;
	size_t memory_size;
	size_t n_bytes_per_xeng_per_int;
	size_t n_xengines;
	size_t n_integrations;
} INT_BUF_METADATA;

typedef struct integration_buffer {
	struct int_buf_metadata metadata;

	void *data_addr;
	size_t data_size;

	struct integration_item *list_ptr;
	size_t list_length;

	sem_t ready_count;
	sem_t pointer_lock;

	struct integration_item *first;
	struct integration_item *last;
	struct integration_item *free;
	struct integration_item *current;
} INTEGRATION_BUFFER;

/*
 * Function Declarations
 */

INTEGRATION_BUFFER *integration_buffer_create(size_t n_byte, size_t n_xeng, size_t n_ints);
INTEGRATION_BUFFER *integration_buffer_import(const char *path);
void integration_buffer_delete(INTEGRATION_BUFFER *ib);

INTEGRATION_ITEM *integration_buffer_insert_pkt(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
INTEGRATION_ITEM *integration_buffer_get_next_full_int(INTEGRATION_BUFFER *ib);
void integration_buffer_remove_item(INTEGRATION_BUFFER *ib, INTEGRATION_ITEM *item);

int ib_create_shared_file(const char *path, size_t size);
int ib_open_shared_file(const char *path);
void ib_close_shared_file(int fd);
void *ib_map_memory(int file, size_t size);
void ib_unmap_memory(void *addr, size_t size);
void ib_read_metadata(const char *path, INT_BUF_METADATA *metadata);
INTEGRATION_BUFFER *ib_import(const char *path, INT_BUF_METADATA *metadata);
INTEGRATION_ITEM *ib_insert_pkt(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
void ib_remove_item(INTEGRATION_BUFFER *ib, INTEGRATION_ITEM *item);
INTEGRATION_ITEM *ib_add_first_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
INTEGRATION_ITEM *ib_prepend_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
INTEGRATION_ITEM *ib_append_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
INTEGRATION_ITEM *ib_inject_item(INTEGRATION_BUFFER *ib, CASPERN_PACKET *pkt);
INTEGRATION_ITEM *ib_find_oldest_ready_item(INTEGRATION_BUFFER *ib);
INTEGRATION_ITEM *ib_find_oldest_unbusy_item(INTEGRATION_BUFFER *ib);
void ib_free_oldest_item(INTEGRATION_BUFFER *ib);
void ib_dump_state(const INTEGRATION_BUFFER *ib);

void integration_init(INTEGRATION_ITEM *integration, CASPERN_PACKET *pkt);
void integration_transcribe(INTEGRATION_ITEM *dst_int, CASPERN_PACKET *src_pkt);
void integration_reset(INTEGRATION_ITEM *integration);
void xengine_reset(XENGINE_ITEM *xengine);

#endif // _INTEGRATION_BUFFER_H_

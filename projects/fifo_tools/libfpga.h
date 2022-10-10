/*
 * libfpga.h
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 13-Feb-2008
 */

#ifndef _LIBFPGA_H_
#define _LIBFPGA_H_

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "libwrap.h"

extern unsigned char NOT_KILLED;
extern unsigned int PATH_LENGTH;
extern unsigned int FIFO_WIDTH;
extern unsigned int POLL_INTERVAL;

int open_udp_send_socket(char *dst_host, char *dst_port);
int open_udp_recv_socket(char *dst_port);
uint32_t parse_host(char *host_name);
uint16_t parse_port(char *port_str);

int open_file_ro(char *path);
int open_file_wo(char *path);
void close_file(int fd);
void seek_to_posn(int fd, off_t posn);

int open_fpga_device_ro(int pid, char *dev);
int open_unix_fifo_wo(char *path);
void read_fpga_register(int fd, uint32_t *word);
void read_fpga_bram(int fd, void *buf, size_t count, off_t start);

int find_bof_pid();
int find_bof_filter(const struct dirent *file);

void hexdump(void *data, int size);
void init_signals();
void handle_eintr(int signal);

#endif /* _LIBFPGA_H_ */

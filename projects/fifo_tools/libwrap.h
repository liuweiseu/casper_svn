/*
 * libwrap.h
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 18-Feb-2008
 */

#ifndef _LIBWRAP_H_
#define _LIBWRAP_H_

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

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t Send(int socket, const void *buffer, size_t length, int flags);
ssize_t Recv(int socket, void *buffer, size_t length, int flags);
ssize_t Write(int fildes, const void *buf, size_t nbyte);
ssize_t Read(int fildes, void *buf, size_t nbyte);
int Open(const char *pathname, int flags, mode_t mode);
int Close(int fd);
int Mkfifo(const char *pathname, mode_t mode);

#endif /* _LIBWRAP_H_ */

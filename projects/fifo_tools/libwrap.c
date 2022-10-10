/*
 * libwrap.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 18-Feb-2008
 */

#include "libwrap.h"

int Socket(int domain, int type, int protocol)
{
	int sock;

	sock = socket(domain, type, protocol);
	if (sock == -1)
	{
		perror("socket");
		exit(1);
	}

	return sock;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int status;

	status = bind(sockfd, addr, addrlen);
	if (status == -1)
	{
		perror("bind");
		exit(1);
	}

	return status;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int status;

	status = connect(sockfd, addr, addrlen);
	if (status == -1)
	{
		perror("connect");
		exit(1);
	}

	return status;
}

ssize_t Send(int socket, const void *buffer, size_t length, int flags)
{
	ssize_t bytes_sent;

	bytes_sent = send(socket, buffer, length, flags);
	if (bytes_sent == -1)
	{
		perror("send");
		if (errno != EINTR)
		{
			exit(1);
		}
	}

	return bytes_sent;
}

ssize_t Recv(int socket, void *buffer, size_t length, int flags)
{
	ssize_t bytes_received;

	bytes_received = recv(socket, buffer, length, flags);
	if (bytes_received == -1)
	{
		perror("recv");
		if (errno != EINTR)
		{
			exit(1);
		}
	}

	return bytes_received;
}

ssize_t Write(int fildes, const void *buf, size_t nbyte)
{
	ssize_t bytes_written;

	bytes_written = write(fildes, buf, nbyte);
	if (bytes_written == -1)
	{
		perror("write");
		if (errno != EINTR)
		{
			exit(1);
		}
	}

	return bytes_written;
}

ssize_t Read(int fildes, void *buf, size_t nbyte)
{
	ssize_t bytes_read;

	bytes_read = read(fildes, buf, nbyte);
	if (bytes_read == -1)
	{
		perror("read");
		if (errno != EINTR)
		{
			exit(1);
		}
	}

	return bytes_read;
}

int Open(const char *pathname, int flags, mode_t mode)
{
	int fd;

	fd = open(pathname, flags, mode);
	if (fd == -1)
	{
		printf("Cannot open %s\n.", pathname);
		perror("open");
		exit(1);
	}

	return fd;
}

int Close(int fd)
{
	int status;

	status = close(fd);
	if (status == -1)
	{
		perror("close");
		exit(1);
	}

	return status;
}

int Mkfifo(const char *pathname, mode_t mode)
{
	int status;

	status = mkfifo(pathname, mode);
	if (status == -1)
	{
		if (errno == EEXIST)
		{
			fprintf(stderr, "Warning: FIFO %s already exists.\n", pathname);
		}
		else
		{
			perror("mkfifo");
			exit(1);
		}
	}

	return status;
}

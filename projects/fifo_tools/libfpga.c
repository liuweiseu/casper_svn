/*
 * libfpga.c
 *
 * auth: Billy Mallard
 * mail: wjm@llard.net
 * date: 13-Feb-2008
 */

#include "libfpga.h"

unsigned char NOT_KILLED = 1;
unsigned int PATH_LENGTH = 128;
unsigned int FIFO_WIDTH = 32;
unsigned int POLL_INTERVAL = 10000;

/*
 * Open a UDP connection to the specified host.
 */
int open_udp_send_socket(char *dst_host, char *dst_port)
{
	int sock = 0;

	struct sockaddr_in dst_addr;
	memset(&dst_addr, 0, sizeof(struct sockaddr_in));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_addr.s_addr = parse_host(dst_host);
	dst_addr.sin_port = parse_port(dst_port);

	sock = Socket(AF_INET, SOCK_DGRAM, 0);

	Connect(sock, (struct sockaddr *)&dst_addr, sizeof(dst_addr));

	return sock;
}

/*
 * Open a UDP connection on the specified port.
 */
int open_udp_recv_socket(char *dst_port)
{
	struct sockaddr_in src_addr;
	memset(&src_addr, 0, sizeof(struct sockaddr_in));
	src_addr.sin_family = AF_INET;
	src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	src_addr.sin_port = parse_port(dst_port);

	int sock = Socket(AF_INET, SOCK_DGRAM, 0);

	Bind(sock, (struct sockaddr *)&src_addr, sizeof(src_addr));

	return sock;
}

/*
 * Parse an address string into an address structure.
 */
uint32_t parse_host(char *host_str)
{
	struct in_addr host;
	struct hostent *he;

	he = gethostbyname(host_str);
	if (he == NULL)
	{
		printf("Cannot find host %s.\n", host_str);
		perror("gethostbyname");
		exit(1);
	}
	host = *((struct in_addr *)he->h_addr);

	return host.s_addr;
}

/*
 * Parse a port string into a port number.
 */
uint16_t parse_port(char *port_str)
{
	uint16_t port = 0;
	sscanf(port_str, "%hu", &port);
	return htons(port);
}

/*
 * Open a file for reading.
 */
int open_file_ro(char *path)
{
	int flags = O_RDONLY;
	mode_t mode = 0;

	return Open(path, flags, mode);
}

/*
 * Open a file for writing.
 */
int open_file_wo(char *path)
{
	int flags = O_WRONLY|O_CREAT|O_TRUNC;
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	return Open(path, flags, mode);
}

/*
 * Close a file.
 */
void close_file(int fd)
{
	Close(fd);
}

/*
 * Seek to a given position in a file.
 */
void seek_to_posn(int fd, off_t posn)
{
	off_t cur_posn = -1;

	cur_posn = lseek(fd, posn, SEEK_SET);
	if (cur_posn != posn)
	{
		perror("lseek");
		exit(1);
	}
}

/*
 * Open an FPGA device for reading.
 */
int open_fpga_device_ro(int pid, char *dev)
{
	char path[PATH_LENGTH];
	memset(&path, 0, PATH_LENGTH);
	snprintf(path, PATH_LENGTH, "/proc/%d/hw/ioreg/%s", pid, dev);

	return open_file_ro(path);
}

/*
 * Open a UNIX fifo for writing.
 */
int open_unix_fifo_wo(char *path)
{
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	Mkfifo(path, mode);

	return open_file_wo(path);
}

/*
 * Read an FPGA register.
 */
void read_fpga_register(int fd, uint32_t *word)
{
	seek_to_posn(fd, 0);
	Read(fd, word, 4);
}

/*
 * Read an FPGA bram.
 */
void read_fpga_bram(int fd, void *buf, size_t count, off_t start)
{
	seek_to_posn(fd, start);
	Read(fd, buf, count);
}

/*
 * Find the pid of the currently running bof file.
 */
int find_bof_pid ()
{
	int pid = 0;
	struct dirent **filelist = {0};
	int i;

	int filecount = scandir("/proc", &filelist, find_bof_filter, alphasort);

	if (filecount < 0)
	{
		perror("Cannot open /proc");
		exit(1);
	}

	if (filecount == 0)
	{
		printf("Cannot find any running bof files.\n");
		exit(1);
	}

	if (filecount > 1)
	{
		printf("WARN: Multiple bof files appear to be running.\n");
		printf("      Choosing the most recently executed one.\n");
	}

	for (i=0; i<filecount; i++)
	{
		sscanf(filelist[i]->d_name, "%d", &pid);
		free(filelist[i]);
	}
	free(filelist);

	return pid;
}

/*
 * Helper function for find_bof_pid(). Filters dirent structs.
 */
int find_bof_filter (const struct dirent *file)
{
	char *result = NULL;

	/*
	 * cat /proc/sys/kernel/pid_max
	 *
	 * Should be smaller than 2**32,
	 * so 10 bytes should be enough.
	 *
	 * "/proc/xxxxxxxxxx/exe" is 21,
	 * so 32 bytes should be enough.
	 */
	const int PATH_LEN = 32;

	char linkpath[PATH_LEN];
	char linkdest[PATH_LEN];
	memset((void *)linkdest, 0, PATH_LEN);

	int pid, status;
	status = sscanf(file->d_name, "%d", &pid);
	if (status == 1) // it is a pid.
	{
		snprintf(linkpath, PATH_LEN, "/proc/%s/exe", file->d_name);
		readlink(linkpath, linkdest, PATH_LEN);
		result = strstr(linkdest, ".bof");
	}

	return (result ? 1 : 0);
}

/*
 * Print the contents of a raw memory chunk.
 */
void hexdump(void *data, int size)
{
	printf("data:");

	int i;
	for (i=0; i<size; i++)
	{
		if (i%4 == 0)
		{
			printf(" ");
		}
		printf("%02X", *(unsigned char *)(data + i));
	}

	printf("\n");
}

/*
 * Setup Ctrl+C signal handler.
 */
void init_signals()
{
	int status = -1;

	signal(SIGINT, handle_eintr);

	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));

	status = sigaction(SIGINT, NULL, &action);
	if (status == -1)
	{
		perror("sigaction");
		exit(1);
	}

	action.sa_flags &= !SA_RESTART;

	status = sigaction(SIGINT, &action, NULL);
	if (status == -1)
	{
		perror("sigaction");
		exit(1);
	}
}

/*
 * Handle a Ctrl+C event.
 */
void handle_eintr(int signal)
{
	printf("\nCtrl-C caught! Quitting.\n");

	NOT_KILLED = 0;
}

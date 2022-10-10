/* file: gen_data.c
 * auth: Billy Mallard
 * mail: wjm@berkeley.edu
 * date: 2008-04-11
 * desc: Generates packets for testing corr_rx.c.
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEST_PORT 8888
#define PACKET_SIZE 8192 // in bytes
#define DATA_RATE 10000 // in megabits per second
#define PACKETS_PER_DUMP 65536
#define BURST_PERIOD 1 // in seconds

typedef int socket_t;
typedef struct sockaddr_in SA_in;
typedef struct sockaddr SA;
typedef unsigned char U8;
typedef unsigned int U32;

struct in_addr parseAddress(char *);
U32 calculateSleepTime();

int main(int argc, char **argv)
{
  /* count arguments */
  if (argc < 2)
  {
    printf("Usage: %s HOSTNAME\n", argv[0]);
    exit(1);
  }

  /* initialize variables */
  int i = 0;
  socket_t sock = -1;
  char *dest_host = argv[1];
  size_t msg_length = PACKET_SIZE;
  ssize_t bytes_sent = 0;
  long long int bytes_per_burst = PACKET_SIZE * PACKETS_PER_DUMP;
  int seconds_per_burst = BURST_PERIOD;
  long long int avg_data_rate = (double) bytes_per_burst / seconds_per_burst;
  long long int sleep_time = calculateSleepTime();

  /* construct source address */
  SA_in src_addr;
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  src_addr.sin_port = 0;

  /* construct destination address */
  SA_in dst_addr;
  memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.sin_family = AF_INET;
  dst_addr.sin_addr = parseAddress(dest_host);
  dst_addr.sin_port = htons(DEST_PORT);

  /* construct message */
  U8 msg[PACKET_SIZE];
  memset(msg, 0, PACKET_SIZE);
  for(i=0; i<PACKET_SIZE/sizeof(U32); i++)
  {
    U32 *target = ((U32 *)msg) + i;
    *target = htonl(0xDEADBEEF);
  }

  /* setup networking */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  bind(sock, (SA *)&src_addr, sizeof(src_addr));

  /* print useful info */
  printf("Packet generator for correlator simulation.\n");
  printf("\n");
  printf("Target host: %s:%u\n", dest_host, DEST_PORT);
  printf("Bytes per packet: %d\n", PACKET_SIZE);
  printf("Packets per dump: %d\n", PACKETS_PER_DUMP);
  printf("\n");
  printf("Bytes per dump: %lldMB\n", bytes_per_burst>>20);
  printf("Seconds per dump: %ds\n", seconds_per_burst);
  printf("Average data rate: %lldMB/s.\n", avg_data_rate>>20);
  printf("Burst data rate: %dMB/s.\n", DATA_RATE);
  printf("\n");
  printf("Dump data period: %lldus\n", 1000000-sleep_time);
  printf("Dump rest period: %lldus\n", sleep_time);
  printf("\n");
  printf("About to start ...\n");
  printf("(Ctrl+C to kill.)\n");
  sleep(2);
  printf("\n");
  printf("Now spewing tons of data.\n");

  /* spew packets! */
  while(1)
  {
    for(i=0; i<PACKETS_PER_DUMP; i++)
    {
      bytes_sent = sendto(sock, msg, msg_length, 0, (SA *)&dst_addr, sizeof(dst_addr));
    }

    usleep(sleep_time);
  }
}

struct in_addr parseAddress(char *hostname)
{
  struct in_addr hostaddr;
  struct hostent *he;

  he = gethostbyname(hostname);
  if (he == NULL)
  {
    perror("gethostbyname");
    exit(1);
  }
  hostaddr = *((struct in_addr *)he->h_addr);

  return hostaddr;
}

U32 calculateSleepTime()
{
	double ms_per_packet = (double) 8 * PACKET_SIZE / DATA_RATE;
	U32 packet_rx_time = ms_per_packet * 1000;
	U32 us_per_burst = BURST_PERIOD * 1000000;
	U32 us_to_wait = us_per_burst - packet_rx_time;
	return us_to_wait;
}

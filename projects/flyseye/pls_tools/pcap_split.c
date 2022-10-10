/*
 * file: pcap_split.c
 * auth: Billy Mallard
 * mail: wjm@berkeley.edu
 * date: 06-Jul-2008
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 32

extern char *strptime(const char *a, const char *b, struct tm *c);

void open_input_file(pcap_t **infile, const char *filename);
void close_input_file(pcap_t **infile);

void open_output_files(pcap_dumper_t ***outfiles, pcap_t *infile, time_t start_time, time_t chunk_size, int numfiles);
void split_input_file(pcap_dumper_t ***outfiles, pcap_t *infile, time_t start_time, time_t chunk_size);
void close_output_files(pcap_dumper_t ***outfiles, int numfiles);

void get_start_stop_time(pcap_t *infile, time_t *start_gm, time_t *stop_gm);
time_t get_chunk_size(const char *chunk_string);
void generate_outfile_name(char *filename, time_t chunk_time);
double calculate_mjd(time_t time);
int calculate_index(time_t pkt_time, time_t start_time, time_t chunk_size);

int main(int argc, char **argv)
{
	/* validate arguments */
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s [infile] [timechunk]\n", argv[0]);
		fprintf(stderr, "Specify timechunk as HH:MM:SS.\n");
		exit(1);
	}

	/* initialization */
	pcap_t *infile = NULL;
	pcap_dumper_t **outfiles = NULL;
	time_t start_time = 0;
	time_t stop_time = 0;
	time_t chunk_size = 0;
	int numfiles = 0;

	printf("%s: A splitter for libpcap capture files.\n", argv[0]);
	printf("\n");

	printf("First pass: Scanning to determine start/end times.\n");

	/* first pass */
	open_input_file(&infile, argv[1]);
	get_start_stop_time(infile, &start_time, &stop_time);
	close_input_file(&infile);

	printf("First pass: Completed.\n");
	printf("\n");

	/* calculations */
	chunk_size = get_chunk_size(argv[2]);
	numfiles = ((double)(stop_time - start_time) / chunk_size) + 1; 

	printf("Start time: %s", ctime(&start_time));
	printf(" Stop time: %s", ctime(&stop_time));
	printf("Chunk size: %ds\n", (int)chunk_size);
	printf(" Files out: %d\n", numfiles);
	printf("\n");

	printf("Second pass: Splitting input file accordingly.\n");

	/* second pass */
	open_input_file(&infile, argv[1]);
	open_output_files(&outfiles, infile, start_time, chunk_size, numfiles);
	split_input_file(&outfiles, infile, start_time, chunk_size);
	close_output_files(&outfiles, numfiles);
	close_input_file(&infile);

	printf("Second pass: Completed.\n");

	return 0;
}

/*
 * Open the input pcap file specified on the command line.
 */
void open_input_file(pcap_t **infile, const char *filename)
{
	char errbuf[PCAP_ERRBUF_SIZE];

	*infile = pcap_open_offline(filename, errbuf);
	if (*infile == NULL)
	{
		fprintf(stderr, "Error: %s\n", errbuf);
		exit(1);
	}
}

/*
 * Close the input pcap file specified on the command line.
 */
void close_input_file(pcap_t **infile)
{
	pcap_close(*infile);
	*infile = NULL;
}

/*
 * Open the necessary number of output pcap files
 * to split the input pcap file across.
 */
void open_output_files(pcap_dumper_t ***outfiles, pcap_t *infile, time_t start_time, time_t chunk_size, int numfiles)
{
	/* Allocate a zeroed list of pcap_dumper_t pointers. */
	*outfiles = calloc(numfiles, sizeof(pcap_dumper_t *));

	/* Allocate a filename buffer. */
	char filename[MAX_FILENAME_LENGTH];
	memset(filename, 0, MAX_FILENAME_LENGTH);

	int i;
	for (i=0; i<numfiles; i++)
	{
		/* Generate the current filename. */
		time_t chunk_time = start_time + i * chunk_size;
		generate_outfile_name(filename, chunk_time);

		/* Open that file in current slot. */
		pcap_dumper_t **outfile = *outfiles + i;
		*outfile = pcap_dump_open(infile, filename);
		if (*outfile == NULL)
		{
			char *err = pcap_geterr(infile);
			fprintf(stderr, "Error: %s\n", err);
			exit(1);
		}
	}
}

/*
 * Close all output pcap files.
 */
void close_output_files(pcap_dumper_t ***outfiles, int numfiles)
{
	int i;
	for (i=0; i<numfiles; i++)
	{
		pcap_dumper_t **outfile = *outfiles + i;
		pcap_dump_close(*outfile);
		*outfile = NULL;
	}
	free(*outfiles);
	*outfiles = NULL;
}

/*
 * Split the input pcap file across
 * the appropriate output pcap files.
 */
void split_input_file(pcap_dumper_t ***outfiles, pcap_t *infile, time_t start_time, time_t chunk_size)
{
	const u_char *packet = NULL;
	struct pcap_pkthdr header;

	/* Read all available packets from input file. */
	packet = pcap_next(infile, &header);
	while (packet != NULL)
	{
		/* Extract the packets timestamp. */
		time_t pkt_time = header.ts.tv_sec;

		/* Find the appropriate output file. */
		int index = calculate_index(pkt_time, start_time, chunk_size);

		/* Write the packet to that file. */
		pcap_dumper_t **outfile = *outfiles + index;
		pcap_dump((u_char *)*outfile, &header, packet);

		/* Read the next available packet. */
		packet = pcap_next(infile, &header);
	}
}

/*
 * Scan through the entire input pcap file
 * to determine the start and end times.
 */
void get_start_stop_time(pcap_t *infile, time_t *start_time, time_t *stop_time)
{
	const u_char *packet = NULL;
	struct pcap_pkthdr header;

	/* Initialize timestamp bounds. */
	packet = pcap_next(infile, &header);
	if (packet == NULL)
	{
		return;
	}
	else
	{
		*start_time = header.ts.tv_sec;
		*stop_time = header.ts.tv_sec;
	}

	/* Expand bounds using remaining packets. */
	while (packet != NULL)
	{
		if (header.ts.tv_sec < *start_time)
		{
			*start_time = header.ts.tv_sec;
		}
		if (header.ts.tv_sec > *stop_time)
		{
			*stop_time = header.ts.tv_sec;
		}
		packet = pcap_next(infile, &header);
	}
}

/*
 * Parse the timechunk command line argument
 * into a time_t value.
 */
time_t get_chunk_size(const char *chunk_string)
{
	time_t chunk_time = 0;
	struct tm chunk_tm;
	memset(&chunk_tm, 0, sizeof(struct tm));

	/* Set expected timechunk format. */
	char format[] = "%H:%M:%S";

	/* Parse timechunk string. */
	char *ret = strptime(chunk_string, format, &chunk_tm);
	if (ret == NULL || *ret != 0)
	{
		fprintf(stderr, "Error: Cannot parse format string \"%s\"\n", chunk_string);
		exit(1);
	}

	/* Manually convert the "struct tm" into a "time_t". */
	chunk_time = chunk_tm.tm_hour * 60 * 60 + chunk_tm.tm_min * 60 + chunk_tm.tm_sec;

	return chunk_time;
}

/*
 * Generate the output pcap file's filename
 * based on the start time of the timechunk. 
 */
void generate_outfile_name(char *filename, time_t chunk_time)
{
	double mjd = calculate_mjd(chunk_time);
	sprintf(filename, "data_%017.11f.log", mjd);
}

/*
 * Map a time_t value into MJD.
 */
double calculate_mjd(time_t time)
{
	return time / 86400. + 40587;
}

/*
 * Map a packet's timestamp to the appropriate
 * output pcap file's array index.
 */
int calculate_index(time_t pkt_time, time_t start_time, time_t chunk_size)
{
	return (pkt_time - start_time) / chunk_size;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <gsl/gsl_histogram.h>
#include <grace_np.h>
#define NUM 100000000
#define U32 unsigned int 
#define BINS 134217728.0

#define FILENAME "largefile0"
#define BUFFER_SIZE 1024
#define HEADER_STRING "HEADER\n"
#define HEADER_SIZE_STRING "HEADER_SIZE "
#define DATA_SIZE_STRING "DATA_SIZE "
#define DR_HEAER_SIZE 1024*4
#define MAX_DATA_SIZE 10240*1024
#define HEADER_SIZE 8
#define ERROR_BUF_SIZE 1024
#define SETI_HEADER_SIZE_IN_BYTES 512+8+8

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#  define EXIT_FAILURE -1
#endif

/* structure for spectral data */
struct spectral_data{
        int coarse_spectra[4096];
        int hits[409600][2];
 		int flags[409600][3];
 		int numhits;
};

struct data_vals{
    unsigned int raw_data;
    unsigned int overflow_cntr;
};

void my_error_function(const char *msg);
unsigned int slice(unsigned int value,unsigned int width,unsigned int offset);

int grace_init();

/* this function plots the data in spectra on the graph specified by beam_number */
int plot_beam(spectral_data * spectra, int beam_number);
int print_fine(spectral_data * spectra, int beam_number, double start_freq);
int print_fine_normalized(spectral_data * spectra, int beam_number, int counter, double start_freq);
int print_fine_normalized_file(FILE *fp, spectral_data * spectra, int beam_number, int counter, double start_freq);
int print_errors(spectral_data * spectra, int beam_number, int counter);
int increment_histograms(gsl_histogram *sigma_histogram, gsl_histogram *freq_histogram, spectral_data * spectra, int beam_number, int counter, double start_freq);
int print_coarse(spectral_data * spectra, int beam_number, double start_freq);

int find_first_header(int fd);
int read_header(char * header);
int64_t read_data(char * data, int datasize);
int read_beam(char * data, int datasize);

void grace_open_deux(int bufsize);
void grace_init_deux(int maxbin,float maxvalue,int xmax);


/*
 * singleGpuSpectrometer
 * 
 * Version 2.0, April 12 2010
 *
 * This program was written by Hirofumi Kondo at the Supercomputing Engineering Laboratory,
 * Graduate School of Information Science and Technology, Osaka University, Japan.
 *
 * Copyright 2010 Supercomputing Engineering Laboratory, Graduate School of Information
 * Science and Technology, Osaka University, Japan
 *
 *
 * Compile : 
 *   nvcc -o singleGpuSpectrometer singleGpuSpectrometer.cu -I /usr/local/cuda/NVIDIA_GPU_Computing_SDK/common/inc
 *                                                             /usr/local/cuda/NVIDIA_GPU_Computing_SDK/C/lib/libcutil.a
 *                                                          -L /usr/local/cuda/lib -l cufft
 *
 * Usage : ./singleGpuSpectrometer [options]
 *   -length           : signal length of this spectrometer handle in M-points 
 *   -boxcar           : length of boxcar for smoothing
 *   -threshold        : value of threshold
 *   -max_detect_point : value of maximum detected points over threshold in each boxcar
 *   -output_file      : filename of output file
 *
 * Output file format :
 *   The file format is binary format.
 *   The output file records all spikes whose power exceed (boxcar_mean) * (threashold).
 *   The file contains 3 data
 *     1) index of signal
 *     2) the power of signal
 *     3) mean power of boxcar which the signal is in
 * 
 * Special Instruction
 *   1) Memory capacity
 *     The memory capacity that this GPU spectrometer requires is changed by the signal length.
 *     If you want to analyze 128M-points signal, GPU has to have 4GB VRAM.
 *     The maximum length that 1GB VRAM GPU can handle is 32M-points.
 *
 *   2) CUDA
 *     We recommend that you use CUDA 2.3 and CUFFT 2.3.
 *     This is not necessary condition.
 *     But the execution time is wrong if you use CUDA 2.2 and CUFFT 2.2.
 */


// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

// includes, project
#include <cutil_inline.h>
#include "pasp.h"
#include "pasp.c"
#include "pasp_config.h"
#include "pasp_process.h"

/*
 * typedef
 */
typedef struct {
	int           index;
	float         power;
	float         mean;
} outputStruct;



/*
 * define constant value
 */
#define FILENAME_BUFSIZE 200
#define SUM_MAX_THREAD   256
#define SUB_MAX_THREAD   256
#define MAX_THREAD       256
#define MAX_GRID         32*1024
#define LOOP_NUM         1



/*
 * global variable
 */
// default length of singal data that this program analyze
int  signalLength = 1024 * 1024 * 16;

// default value of boxcar, this can be change by -boxcar argument
int  boxcar = 8192;

// default value of threshold, this can be changed by -threshold argument
int  threshold = 20;

// default filename of output file, this can be changed by -output_file argument
char outputFileName[FILENAME_BUFSIZE] = "report.txt";

// the memory size and pointer for generated signal data
unsigned int hostSignalDataMemSize = 0;
char         *hostSignalData = NULL;

// the memory size and pointer for output data
unsigned int hostOutputDataMemSize = 0;
outputStruct *hostOutputData = NULL;

// we limit the detected signal points in each boxcar.
// 'maximumDetectPointInBoxcar' specify its value, this can be changed by -max_detect_point argument
int maximumDetectPointInBoxcar = 16;

// output file 
int   outputCounter = 0;
int   outputFclosePeriod = 10;
FILE *outputFilePointer = NULL;


/*
 * include other source
 */
#include "kernelExec.cu"
#include "fourStepFFT.cu"
#include "output.c"


/* 
 * Prototype declaration
 */
void parse_args(int, char**);
void init_host();
void terminate_host();
void generate_signal();
void do_analyze_on_gpu();



/*
 * Program main
 */
int main(int argc, char** argv){
 
	// parse the arguments
	parse_args(argc, argv);

	// initialize host memory
	init_host();

	// generate signal
	generate_signal();

	// Analyze signal on GPU
	do_analyze_on_gpu();

	// free host memory
	terminate_host();

	// exit program
	cutilExit(argc, argv);
}



/*
 * init_host();
 */
void init_host(){
	hostSignalDataMemSize = sizeof(char) * signalLength * 2;
	hostOutputDataMemSize = sizeof(outputStruct) * maximumDetectPointInBoxcar * (signalLength / boxcar);

	cutilSafeCall( cudaMallocHost( (void**)&hostSignalData, hostSignalDataMemSize));
	if(hostSignalData==NULL){
		fprintf(stderr,"Error : cudaMallocHost failed\n");
		exit(-1);
	}

	cutilSafeCall( cudaMallocHost( (void**)&hostOutputData, hostOutputDataMemSize));
	if(hostOutputData==NULL){
		fprintf(stderr,"Error : cudaMallocHost failed\n");
		exit(-1);
	}

	return;
}



/*
 * parse_args
 */
void parse_args(int argc, char** argv) {
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i], "-length")) {
            signalLength = atoi(argv[++i]) * 1024 * 1024;
        } else if (!strcmp(argv[i], "-boxcar")){
			boxcar = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-threshold")){
			threshold = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-max_detect_point")){
			maximumDetectPointInBoxcar = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-output_file")){
			strncpy(outputFileName, argv[++i], FILENAME_BUFSIZE);
			if(outputFileName[FILENAME_BUFSIZE-1]!='\0'){
				fprintf(stderr,"Error : Too long output file name. maximum length = %d\n", FILENAME_BUFSIZE-1);
				exit(-1);
			}
		} else {
			fprintf(stderr,"Error : wrong argument\n");
        }
    }

	return;
}



/*
 * terminate_host()
 */
void terminate_host(){
	
	// Free signal data memory
	cudaFreeHost(hostSignalData);
	hostSignalData = NULL;

	// Free output data memory
	cudaFreeHost(hostOutputData);
	hostOutputData = NULL;

	return;
}



/*
 * generate_signal()
 */
void generate_signal(){



//	srand((int) time(NULL));

//	for(int i=0; i<signalLength*2; i++){
//		hostSignalData[i] = ((rand() % 16) - 8);
//	}


/*    
	FILE *bin_file;
	bin_file=fopen("file.bin","rb");
	int buffer,bytesread;
    
	for(int i=0; i<signalLength*2; i++){

		if(bytesread != 0){
	    
		    fread(&buffer,4,1,bin_file);
		    hostSignalData[i] = ((buffer % 16) - 8);
		}
	}
	
	fclose(bin_file);
*/


    for(int i=0; i<signalLength; i++){
	hostSignalData[2*i] = (int) 127*sinf(4* i * 8 * 2*3.14159265/(float)signalLength);
	hostSignalData[2*i+1] = (int) 127*cosf(4* i * 8 * 2*3.14159265/(float)signalLength);
    }



//=== Get FIFO Dada ===//


printer();
/*

    int fifo;
    int ret;
    struct sigaction newact;

    //create the fifo
    debug_fprintf(stderr, "Creating fifo %s\n", RAW_UDP_FILE_NAME);
    ret = mkfifo(RAW_UDP_FILE_NAME,0666);
    if(ret == -1)
    {
        if(errno == EEXIST)
        {
            debug_fprintf(stderr, "File already exists. Will attempt to open.\n");
        }
        else
        {
            perror("Error creating fifo");
            exit(1);
        }
    }

    //open the fifo
    debug_fprintf(stderr, "Opening fifo %s\n", RAW_UDP_FILE_NAME);
    fifo = open(RAW_UDP_FILE_NAME, O_WRONLY);
    if(fifo == -1)
    {
        perror("Error opening fifo");
        exit(1);
    }

    //set up the signal handler
    newact.sa_handler = cleanup;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;

    //start listening for Ctrl-C
    sigaction(SIGINT, &newact, NULL);

    //receive packets and write into the fifos
    receive_packets(fifo);

    debug_fprintf(stderr, "Closing fifo\n");


    close(fifo);
*/
/////////////////////////
}


/*
 * do_analyze_on_gpu
 */
void do_analyze_on_gpu(){
	
	// Device memory pointer
	char         *devSignalData = NULL;
	cufftComplex *devFFTData    = NULL;
	float        *devPowerData  = NULL;
	float        *devAvgRe      = NULL;
	float        *devAvgIm      = NULL;
	float        *devPartSumRe  = NULL;
	float        *devPartSumIm  = NULL;
	outputStruct *devOutputData = NULL;

	// Memory size for device
	unsigned int devSignalDataMemSize = 0;
	unsigned int devFFTDataMemSize    = 0;
	unsigned int devPowerDataMemSize  = 0;
	unsigned int devPartSumMemSize    = 0;
	unsigned int devOutputDataMemSize = 0;


	// Calculate memory size
	devSignalDataMemSize = hostSignalDataMemSize;
	devFFTDataMemSize    = sizeof(cufftComplex) * signalLength;
	devPowerDataMemSize  = sizeof(float) * signalLength;
	devPartSumMemSize    = sizeof(float) * SUM_MAX_THREAD;
	devOutputDataMemSize    = hostOutputDataMemSize;

	// Allocate device memory
	cutilSafeCall( cudaMalloc( (void**) &devSignalData, devSignalDataMemSize) );
	cutilSafeCall( cudaMalloc( (void**) &devFFTData,    devFFTDataMemSize) );
	cutilSafeCall( cudaMalloc( (void**) &devPowerData,  devPowerDataMemSize) );
	cutilSafeCall( cudaMalloc( (void**) &devPartSumRe,  devPartSumMemSize) );
	cutilSafeCall( cudaMalloc( (void**) &devPartSumIm,  devPartSumMemSize) );
	cutilSafeCall( cudaMalloc( (void**) &devAvgRe, sizeof(float) * 1) );
	cutilSafeCall( cudaMalloc( (void**) &devAvgIm, sizeof(float) * 1) );
	cutilSafeCall( cudaMalloc( (void**) &devOutputData, devOutputDataMemSize) );

	// the row length and col length of matrix
	int matrixX, matrixY;

	// the value of 'matrixY' must be fixed!!! because this program includes only 16-point fft kernel.
	matrixY = 16;
	matrixX = signalLength / matrixY;

	// Initialize output file
	init_output_file();

	// timer
	unsigned int timer;
	cutCreateTimer(&timer);	


	// Main loop
	for(int iter=0; iter<LOOP_NUM; iter++){

		// timer
		cutResetTimer(timer);
		cutStartTimer(timer);

		// CPU -> GPU : move signal data from host to device
		cutilSafeCall( cudaMemcpy(devSignalData, hostSignalData, devSignalDataMemSize, cudaMemcpyHostToDevice));
		cudaThreadSynchronize();

		// GPU : convert char format signal data to float format
		convert_to_float_exec(devSignalData, devPartSumRe, devPartSumIm, devAvgRe, devAvgIm, devFFTData);

		// GPU : do fft
		do_four_step_fft(devFFTData, devPowerData, matrixX, matrixY);

		// GPU : detect strong power spectrum
		calc_over_threshold_exec(devPowerData, devOutputData);

		// GPU -> CPU : copy detect spectrum data from device to host
		cutilSafeCall( cudaMemcpy( hostOutputData, devOutputData, devOutputDataMemSize, cudaMemcpyDeviceToHost));
		cudaThreadSynchronize();

		// CPU : output detect power spectrum to file
		output_spectrum(hostOutputData, iter, 1);

		// timer
		cutStopTimer(timer);
		printf("time = %f, %d done...\n",cutGetTimerValue(timer), iter);

	}

	// Terminate output file
	terminate_output_file();


	// Free device memory
	cutilSafeCall( cudaFree( devSignalData ) );
	cutilSafeCall( cudaFree( devFFTData ) );
	cutilSafeCall( cudaFree( devPowerData ) );
	cutilSafeCall( cudaFree( devPartSumRe ) );
	cutilSafeCall( cudaFree( devPartSumIm ) );
	cutilSafeCall( cudaFree( devAvgRe ) );
	cutilSafeCall( cudaFree( devAvgIm ) );
}

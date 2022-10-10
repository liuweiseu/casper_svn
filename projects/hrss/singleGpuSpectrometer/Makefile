TARGETS := pasp_recv pasp_distribute  singleGpuSpectrometer
.PHONY: clean realclean new


CC = gcc
NCC = nvcc 
CFLAGS = -g -O2 -I/usr/local/cuda/include -L/usr/local/cuda/lib64
DEBUG = -D DEBUG
CUDA_INCLUDE_LIB = -lcufft

default: all

all: singleGpuSpectrometer pasp_recv pasp_distribute

# compile all pasp .o files the same way
%.o: %.c %.h pasp_config.h
	$(CC) $(CFLAGS) $(DEBUG) -c $<
		
# default rule to compile targets
$(TARGETS): %: Makefile %.o
	$(CC) $(CFLAGS) $(DEBUG) $*.o -o $*

singleGpuSpectrometer: singleGpuSpectrometer.o pasp.o pasp_process.o
	$(CC) $(CFLAGS) singleGpuSpectrometer.o pasp_process.o pasp.o -o singleGpuSpectrometer -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc /home/mwagner/NVIDIA_GPU_Computing_SDK/C/lib/libcutil_x86_64.a -L /home/mwagner/lib -l cufft

singleGpuSpectrometer.o : singleGpuSpectrometer.cu 
	$(NCC) $(CFLAGS) -o singleGpuSpectrometer.o -c singleGpuSpectrometer.cu -L /home/mwagner/lib -l cufft -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc -I ./
	
pasp.o: pasp.c
	$(CC) -c pasp.c -o pasp.o


clean: 
	@rm -vf *.o

realclean: clean
	@rm -vf $(TARGETS)
	
new: realclean all

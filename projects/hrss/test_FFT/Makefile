bench_fft : bench_fft.c fft_gpu.cu 
	cc -c random.c
	nvcc -c fft_gpu.cu -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc /home/mwagner/NVIDIA_GPU_Computing_SDK/C/lib/libcutil_x86_64.a -L /home/mwagner/lib -l cufft -lm -I /usr/local/cuda/include
	cc -Wall -o bench_fft bench_fft.c fft_gpu.o random.o -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc /home/mwagner/NVIDIA_GPU_Computing_SDK/C/lib/libcutil_x86_64.a -L /home/mwagner/lib  -L /usr/local/cuda/lib64/ -lcufft -lm -I /usr/local/cuda/include

bench_fft_th : bench_fft.c fft_gpu_threshold.cu 
	cc -c random.c
	nvcc -c fft_gpu_threshold.cu -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc /home/mwagner/NVIDIA_GPU_Computing_SDK/C/lib/libcutil_x86_64.a -L /home/mwagner/lib -l cufft -lm -I /usr/local/cuda/include
	cc -Wall -o bench_fft bench_fft.c fft_gpu_threshold.o random.o -I /home/mwagner/NVIDIA_GPU_Computing_SDK/C/common/inc /home/mwagner/NVIDIA_GPU_Computing_SDK/C/lib/libcutil_x86_64.a -L /home/mwagner/lib  -L /usr/local/cuda/lib64/ -lcufft -lm -I /usr/local/cuda/include

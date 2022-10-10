CC = gcc
CFLAGS = -Wall
INCS = -I/usr/local/grace/include
LIBS = -lm -lgrace_np -L/usr/local/grace/lib
TARGETS = seti_fft seti_recv seti_test_pfb

all: seti_fft.o seti_recv.o seti_test_pfb.o casper.o gracie.o setispec.o trial1.o trial2.o Makefile
	${CC} ${CFLAGS} seti_fft.o casper.o gracie.o setispec.o -o seti_fft ${LIBS}
	${CC} ${CFLAGS} seti_recv.o casper.o gracie.o setispec.o -o seti_recv ${LIBS}
	${CC} ${CFLAGS} seti_test_pfb.o gracie.o -o seti_test_pfb ${LIBS}
	${CC} ${CFLAGS} trial1.o setispec.o casper.o gracie.h -o trial1 
	${CC} ${CFLAGS} trial2.o setispec.o casper.o gracie.h -o trial2 

trial: setispec.o casper.o trial1.o trial2.o Makefile
	${CC} ${CFLAGS} trial1.o setispec.o casper.o gracie.h -o trial1 
	${CC} ${CFLAGS} trial2.o setispec.o casper.o gracie.h -o trial2 

seti_fft.o: seti_fft.c Makefile
	${CC} ${CFLAGS} -c seti_fft.c -o seti_fft.o ${INCS}

seti_recv.o: seti_recv.c Makefile
	${CC} ${CFLAGS} -c seti_recv.c -o seti_recv.o ${INCS}

seti_test_pfb.o: seti_test_pfb.c Makefile
	${CC} ${CFLAGS} -c seti_test_pfb.c -o seti_test_pfb.o ${INCS}

trial1.o: trial1.c Makefile
	${CC} ${CFLAGS} -c trial1.c -o trial1.o 

trial2.o: trial2.c Makefile
	${CC} ${CFLAGS} -c trial2.c -o trial2.o 

gracie.o: gracie.c Makefile
	${CC} ${CFLAGS} -c gracie.c -o gracie.o ${INCS}

setispec.o: setispec.c Makefile
	${CC} ${CFLAGS} -c setispec.c -o setispec.o ${INCS}

casper.o: casper.c Makefile
	${CC} ${CFLAGS} -c casper.c -o casper.o ${INCS}

clean:
	@rm -rf *.o

fuckit:
	rm -rf ${TARGETS} *.o


CC      = gcc
# CC      = i386-linux-uclibc-gcc

CFLAGS  = -I. -I../_home_cima_Wapp_Bin_Sources_Includes  -I../_home_phil_vw_h -lm -lnsl
OBJ	= jtest.o scram.o flip.o swapdata.o

stest:	${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o stest

jtest:	${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o jtest

clean:
	rm -rf *.o stest jtest

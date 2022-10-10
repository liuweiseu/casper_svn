CC = gcc
CFLAGS = -g -Wall
LDLIBS = -lm

OBJ=bug.o dio.o headio.o hio.o maskio.o pack.o uvio.o

libmir.a: $(OBJ)
	$(CC) $(CFLAGS) -c *.c
	ar rvu $@ $(OBJ)
	ranlib $@

clean:
	@rm -vf *.o *~
	@rm -vf libmir.a


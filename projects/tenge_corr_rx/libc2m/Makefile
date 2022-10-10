CC = gcc
CFLAGS = -g -Wall
LIBS = -L./libastro -L./mir
LDLIBS = -lm -lastro -lmir -llinpack

libc2m: Makefile libastro.a libmir.a c2m.h c2m.c mjd.c parse_conf.c
	$(CC) $(CFLAGS) -c c2m.c mjd.c parse_conf.c
	@ar rcs libc2m.a c2m.o mjd.o parse_conf.o

c2m: Makefile c2m.h c2m.c mjd.c parse_conf.c
	$(CC) $(CFLAGS) -o c2m c2m.c mjd.c parse_conf.c $(LIBS) $(LDLIBS)

libastro.a:
	@make -C libastro

libmir.a:
	@make -C mir

clean:
	@rm -vf *.o *~
	@rm -vf c2m
	@make -C mir clean
	@make -C libastro clobber


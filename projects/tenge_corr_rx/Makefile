CC = gcc
CFLAGS = -g -Wall
DEBUG = -D DEBUG

default: all

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEBUG) -c $<

N2M_DEPS = libc2m.a debug_macros.h
N2M_OBJS = net2mem.o ring_buffer.o caspern_packet.o integration_buffer.o
N2M_LIBS = -L./libc2m -L./libc2m/libastro -L./libc2m/mir
N2M_LDFLAGS = -lpthread -lc2m -lm -lastro -lmir
net2mem: Makefile $(N2M_DEPS) $(N2M_OBJS)
	$(CC) $(CFLAGS) $(DEBUG) $(N2M_OBJS) -o $@ $(N2M_LIBS) $(N2M_LDFLAGS)

M2M_DEPS = libc2m.a debug_macros.h
M2M_OBJS = mem2mir.o ring_buffer.o caspern_packet.o integration_buffer.o
M2M_LIBS = -L./libc2m -L./libc2m/libastro -L./libc2m/mir
M2M_LDFLAGS = -lpthread -lc2m -lm -lastro -lmir 
mem2mir: Makefile $(M2M_DEPS) $(M2M_OBJS)
	$(CC) $(CFLAGS) $(DEBUG) $(M2M_OBJS) -o $@ $(M2M_LIBS) $(M2M_LDFLAGS)

libc2m.a:
	@make -C libc2m

gen_data: Makefile gen_data.c
	$(CC) $(CFLAGS) -o gen_data gen_data.c

ring_buffer_test: Makefile ring_buffer_test.c ring_buffer.o
	$(CC) $(CFLAGS) $(DEBUG) ring_buffer_test.c ring_buffer.o -o ring_buffer_test -lpthread

integration_buffer_test: Makefile integration_buffer_test.c caspern_packet.o integration_buffer.o
	$(CC) $(CFLAGS) $(DEBUG) integration_buffer_test.c caspern_packet.o integration_buffer.o -o integration_buffer_test -lpthread

all: net2mem mem2mir

clean:
	@rm -vf *.o
	@rm -vf *~
	@rm -vf net2mem
	@rm -vf mem2mir
	@rm -vf gen_data
	@rm -vf *_test
	@make -C libc2m clean


CC=gcc
CFLAGS=-g -Wall
SOFLAGS=-c -fPIC -shared
SOLIBS=-L. -lfpga -lwrap

all: fifo_tx fifo_rx fifo_cat fifo_dump net_fifo

libs: libfpga.so libwrap.so

libfpga.so: libfpga.c libfpga.h
	$(CC) $(CFLAGS) $(SOFLAGS) libfpga.c -o libfpga.so 

libwrap.so: libwrap.c libwrap.h
	$(CC) $(CFLAGS) $(SOFLAGS) libwrap.c -o libwrap.so 

fifo_tx: fifo_tx.c libs
	$(CC) $(CFLAGS) fifo_tx.c -o fifo_tx $(SOLIBS)

fifo_rx: fifo_rx.c libs
	$(CC) $(CFLAGS) fifo_rx.c -o fifo_rx $(SOLIBS)

fifo_cat: fifo_cat.c libs
	$(CC) $(CFLAGS) fifo_cat.c -o fifo_cat $(SOLIBS)

fifo_dump: fifo_dump.c libs
	$(CC) $(CFLAGS) fifo_dump.c -o fifo_dump $(SOLIBS)

net_fifo: net_fifo.c libs
	$(CC) $(CFLAGS) net_fifo.c -o net_fifo $(SOLIBS)

isi_push: isi_push.c libs
	$(CC) $(CFLAGS) isi_push.c -o isi_push $(SOLIBS)

clean:
	@rm -vf *.o
	@rm -vf *.so
	@rm -vf *~
	@rm -vf fifo_tx
	@rm -vf fifo_rx
	@rm -vf fifo_cat
	@rm -vf fifo_dump
	@rm -vf net_fifo
	@rm -vf isi_push

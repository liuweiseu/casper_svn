PROJ=xyzmodem_tx
CC=gcc
CFLAGS=-Wall
SRC=$(PROJ).c
DEFINE=-DDEBUG

$(PROJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ $<

clean:
	rm -f *.o $(PROJ) $(PROJ).exe

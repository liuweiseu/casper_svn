PROJ=xport_gpio
CC=gcc
CFLAGS=-Wall -lutil
SRC=$(PROJ).c
#HDR=xportd.c
#DEFINE=-DDEBUG

$(PROJ): $(SRC)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ $<

clean:
	rm -f *.o $(PROJ)

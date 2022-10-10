PROJ=charfind
CC=gcc
CFLAGS=-Wall -lutil
SRC=$(PROJ).c
#DEFINE=-DDEBUG

$(PROJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ $<

clean:
	rm -f *.o $(PROJ) $(PROJ).exe

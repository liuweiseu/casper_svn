CC=gcc
CFLAGS=-Wall -lutil
SRC=xportd.c
#HDR=xportd.c
#DEFINE=-DDEBUG

xportd: $(SRC)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ $<

clean:
	rm -f *.o xportd

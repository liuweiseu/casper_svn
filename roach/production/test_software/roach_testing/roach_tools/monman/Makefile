CC=gcc
CFLAGS=-Wall
SRC=monman.c
HDR=monman.h
PLATFORM=$(shell uname | sed 's/_NT.*//')
ifeq ($(PLATFORM),CYGWIN)
DEFINE=-DCYGWIN
else
DEFINE=-DLINUX
endif
#DEFINE+=-DDEBUG

monman: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(DEFINE) -o $@ $<

ci: clean
	svn add --force *.c Makefile
	svn ci

clean:
	rm -rf *.o monman monman.exe

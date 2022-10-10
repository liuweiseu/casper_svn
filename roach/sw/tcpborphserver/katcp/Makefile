prefix=/usr/local

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall
#CFLAGS += $(shell test `whoami` = marc && echo "-DDEBUG -ggdb")
AR = ar
RM = rm -f
INSTALL = install

LIB = libkatcp.a

SRC = line.c netc.c dispatch.c loop.c sensor.c log.c
HDR = katcp.h katcl.h
OBJ = $(patsubst %.c,%.o,$(SRC))

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

%.o: %.c *.h 
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

clean:
	$(RM) $(LIB) *.o

install: all
	$(INSTALL) -d $(prefix)/include $(prefix)/lib
	$(INSTALL) $(LIB) $(prefix)/lib
	$(INSTALL) $(HDR) $(prefix)/include


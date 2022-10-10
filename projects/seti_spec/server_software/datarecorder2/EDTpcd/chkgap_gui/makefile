
#edtlib is either edt or pdv
EDTLIB=edt
EDTDIR = ..

CC=gcc

CFLAGS = -DUNIX -Wall -g -I$(EDTDIR) `pkg-config --cflags gtk+-2.0 libglade-2.0`

LDFLAGS += -L$(EDTDIR) -l$(EDTLIB) -lpthread -lm `pkg-config --libs gtk+-2.0 libglade-2.0`


OBJS= chkgap_gui_main.o \
chkgap_gui_callbacks.o chkgap_gui_histogram.o \
../check_gap_lib.o

checkgap_gui: $(OBJS) 
	$(CC) -o $@ $(OBJS) $(LDFLAGS)  

chkgap_gui_main.o: chkgap_gui_main.c
	$(CC) $(CFLAGS) -c chkgap_gui_main.c 

chkgap_gui_callbacks.o: chkgap_gui_callbacks.c chkgap_gui_callbacks.h
	$(CC) $(CFLAGS) -c chkgap_gui_callbacks.c  

chkgap_gui_histogram.o: chkgap_gui_histogram.c chkgap_gui_histogram.h
	$(CC) $(CFLAGS) -c chkgap_gui_histogram.c  


objclean:
	rm -rf *.o checkgap_gui 




# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

# Change it here or specify it on the "make" commandline

#DEBFLAGS = -O2 -g 
DEBFLAGS = -O2 

CC=gcc

CFLAGS = $(DEBFLAGS) $(BUILDOPTIONS) $(PSN_FLAG)

CFLAGS += -fPIC -I. 

LDFLAGS = -L. -ledt -lpthread -lcurses -Xlinker -rpath -Xlinker ./EDTpcd 

dualchannel: LDFLAGS += -lm

TARGS = \
	dualchannel \
	chkprbs15 \
	chkprbs7 \
	genprbs15 \
	genprbs7 \
	pcdtest \
	pcdtest_bb \
	testring \
	simple_read \
	simple_write \
	simple_getdata \
	simple_putdata \
	readssd \
	writessd \
	rdssdio \
	wrssdio \
	bitload \
	setdebug \
	sample \
	testssdout \
	wrpcd8 \
	rdpcd8 \
		testit \
		xtest \
		rd16 \
		rd4 \
		wr16 \
		rdwr16 \
		pdb \
		ssepdb \
		sserd \
		ssewr \
		ssedata \
		sserot \
		sseload \
		sstestdiff \
		ssbasetest \
		set_ss_vco \
		lxt \
		testssvco \
		dumpit \
		testcombo \
		testssmem \
		pcdin \
		pcdout \
		c3_set_e1 \
		c3_set_e3 \
		check_gap_ss \
		pdma pcdrequest \
		sslooptest \
		gptest \
		rfile \
		wfile \
		test_timeout \
		gs_temp \
		ocmload

PCDLIB = libedt.so
STATICPCDLIB = libedt.a

all: $(PCDLIB) $(TARGS) initpcd pciload hubload

# A note about Java:
#  Since not everyone has the javac program installed, in order
#  to compile the Java-dependant code (namely PcdRequest.jar) you will 
#  need to run "make java-dep", not "make all". 
#  (But this EDT software package should include the compiled Java 
#  stuff anyway, so unless you make changes to it, recompiling shouldn't
#  be necessary.)
java-dep: PcdRequest.jar


# Extract version number from headers.
VER = $(shell awk -F\" '/REL/ {print $$2}' $(INCLUDEDIR)/linux/version.h)

driver:
	cd module ; $(MAKE) 
 
objclean:	
	@echo "Cleaning object files"
	@-rm -f *.o *~
	@-rm -f  $(TARGS) initpcd $(PCDLIB)


LIBOBJS = libedt.o chkbuf.o edt_ss_vco.o edt_vco.o edt_error.o edt_bitload.o edt_xilinx.o initedt.o initpcd.o libpdma.o edt_bitfile.o lib_adt7461.o

$(LIBOBJS) : CFLAGS += -D_REENTRANT 

HUBLOADOBJS = \
	hubload.o \
	pciload_fn.o \
	pciload_4028xla.o 

load: 
	./edt_load

unload:
	./edt_unload

edt: 
	make -f Makefile.drv


# Check Gap stuff
check_gap_common: $(LIBOBJS) check_gap_lib.o check_gap_common.c
	$(CC) -o $@ $(LDFLAGS) check_gap_lib.o check_gap_common.c $(LOGOUT)
	@$(SHOWOUT)

checkgap_gui: check_gap_lib.o
	-cd chkgap_gui ; $(MAKE)
	-ln -s chkgap_gui/checkgap.glade ./
	-ln -s chkgap_gui/checkgap_gui ./

#
# Java program and JNI library compile stuff
#
SUN_OSFLAGS=-xO2 -z text -G -xO2 -mt -K pic -I/usr/java/include/solaris -h libPcdRequest.so
LINUX_OSFLAGS=-O2 -I/usr/src/linux/include -I/usr/java/include/linux -shared 
#
# Uncomment the appropriate OSFLAGS
#
#OSFLAGS=$(SUN_OSFLAGS)
OSFLAGS=$(LINUX_OSFLAGS)
JAVA_CFLAGS= $(OSFLAGS) -I. -I/usr/java/include -I/opt/EDTpcd -L. -L/opt/EDTpcd

PcdRequest.jar: PcdRequest.java Manifest.pcdrequest makefile
	javac PcdRequest.java ;
	jar cmf Manifest.pcdrequest PcdRequest.jar PcdRequest*.class ;
	chmod +x runjava

libPcdRequest.so: edt_bitfile.h edt_bitfile.c libPcdRequest.c PcdRequest.jar makefile
	-if [ -e /usr/java/include ] ; then \
	    $(CC) $(JAVA_CFLAGS) -o libPcdRequest.so edt_bitfile.c libPcdRequest.c -lm ; \
	fi

PCILOADOBJS = \
    pciload_4013e.o\
    pciload_4013xla.o\
    pciload_4028xla.o\
    pciload_fn.o\
    pciload_main.o

# This is setup so that 
# if you want to redirect output to a log
# you just need to define the macros LOGOUT and SHOWOUT
#


$(PCDLIB) : $(LIBOBJS)
	@echo $(CC) -o $(PCDLIB) -shared $(LIBOBJS) $(LOGOUT)
	@$(CC) -o $(PCDLIB) -shared $(LIBOBJS) $(LOGOUT)
	@$(SHOWOUT)

$(STATICPCDLIB) : $(LIBOBJS)
	@echo $(AR) -ur $(STATICPCDLIB) $(LIBOBJS) $(LOGOUT)
	@$(AR) -ur $(STATICPCDLIB) $(LIBOBJS) $(LOGOUT)
	@$(SHOWOUT)


$(TARGS) : % : %.o
	@echo $(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LOGOUT)
	@$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LOGOUT)
	@$(SHOWOUT)

hubload :  $(LIBOBJS) $(HUBLOADOBJS)
	@echo $(CC) -o $@ $(HUBLOADOBJS) $(LDFLAGS) $(LOGOUT)
	@$(CC) -o $@ $(HUBLOADOBJS) $(LDFLAGS) $(LOGOUT)
	@$(SHOWOUT)

pciload :  $(LIBOBJS) $(PCILOADOBJS)
	@echo $(CC) -o $@ $(PCILOADOBJS) $(LDFLAGS) $(LOGOUT)
	@$(CC) -o $@ $(PCILOADOBJS) $(LDFLAGS) $(LOGOUT)
	@$(SHOWOUT)

%.o : %.c
	@echo $(CC) $(CFLAGS) -c $< $(LOGOUT)
	@$(CC) $(CFLAGS) -c $< $(LOGOUT)
	@$(SHOWOUT)

initpcd :  initpcd_main.o $(PCDLIB) 
	@echo $(CC) -o initpcd initpcd_main.o $(LDFLAGS) $(LOGOUT)
	@$(CC) -o initpcd initpcd_main.o $(LDFLAGS) $(LOGOUT)
	@$(SHOWOUT)


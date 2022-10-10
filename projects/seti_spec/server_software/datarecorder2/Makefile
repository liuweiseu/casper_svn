EDTDIR		=	./EDTpcd
SETISPECDIR	=	./setispec
TKDIR		=	/usr/include/tk

CC		=	gcc
#CFLAGS          =       -g -D_REENTRANT -I$(EDTDIR) -I$(EDTDIR) -Iscram/phil -Iscram/aoui
CFLAGS          =       -g -D_REENTRANT -I$(EDTDIR) -IAO/_home_phil_vw_pnt_exportCode -IAO/_home_phil_vw_h -IAO/_home_cima_Wapp_Bin_Sources_Includes
# CFLAGS definition specific for setispec_dr target. Includes the setispec library instead the edt library. Redefined this way so the %.o target does not require modification.
setispec_dr:	CFLAGS          =       -Wstack-protector -fstack-protector-all -W -Wall -dH -g -D_REENTRANT -I$(TKDIR) -I$(SETISPECDIR) -IAO/_home_phil_vw_pnt_exportCode -IAO/_home_phil_vw_h -IAO/_home_cima_Wapp_Bin_Sources_Includes

SUFFIXES 	= 	.C .o

INCLUDES    = globals.h \
              dr2.h

STDLINK 	= 	-lstdc++ 

LINKLIBS 	= 	-lrt \
			-lpthread

EDTLINKLIBS	=	-ledt

AOLIBS      = -LAO/lib -lazzatoradec

DR2OBJS		=	globals.o \
			membuf.o \
			diskbuf.o \
			header.o \
			synth.o \
			observatory.o \
			idle.o \
			trigger.o \
			ui.o \
			signals.o \
			getdata.o \
            ringwatch.o \
            eng.o   \
            quicklook.o \
            ao_utils.o \
            error.o

EDTOBJS     = 	edt.o \
		adc_to_edt_funcs.o

SETISPECOBJS =	udp.o \
		$(SETISPECDIR)/udp_functions.o \
		$(SETISPECDIR)/edt_stubs.o
		
            

SCRAMOBJS   = AO/lib/scram.o \
            AO/lib/flip.o \
            AO/lib/swapdata.o

SERIALOBJS	=	serial.o

default: setispec_dr scram_peek
		
all: simple_getdata set_clock cmdsynth dr2 dr2_power_spectrum

%.o: %.C $(INCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

simple_getdata:	simple_getdata.C 
	$(CC) $(CFLAGS) simple_getdata.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR) $(STDLINK)

simple_getdata_with_arming:	simple_getdata_with_arming.C 
	$(CC) $(CFLAGS) simple_getdata_with_arming.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR) $(STDLINK)

simple_waveform:	simple_waveform.C 
	$(CC) $(CFLAGS) simple_waveform.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR) $(STDLINK)

arm:	arm.C 
	$(CC) $(CFLAGS) arm.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR) $(STDLINK)

cmdsynth:	cmdsynth.C $(SERIALOBJS) 
	$(CC) $(CFLAGS) $(STDLINK) cmdsynth.C serial.o -O -o $@

cmdeng:	cmdeng.C $(SERIALOBJS) 
	$(CC) $(CFLAGS) $(STDLINK) cmdeng.C serial.o -O -o $@ 

cmdeng2:	cmdeng2.C $(SERIALOBJS) 
	$(CC) $(CFLAGS) $(STDLINK) cmdeng2.C serial.o -O -o $@

set_clock:	set_clock.C 
	$(CC) $(CFLAGS) $(LINKLIBS) set_clock.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR) $(STDLINK)

dr2_reader:	dr2_reader.C 
	$(CC) $(CFLAGS) dr2_reader.C -O -o $@ $(STDLINK)

dr2_power_spectrum:	dr2_power_spectrum.C 
	$(CC) $(CFLAGS) $(STDLINK) dr2_power_spectrum.C -O -o $@

tapeutils:	tapeutils.C 
	$(CC) $(CFLAGS) $(STDLINK) tapeutils.C -O -o $@

scram_peek:    scram_peek.C AO/lib/scram.o AO/lib/flip.o AO/lib/swapdata.o 
	$(CC) $(CFLAGS) scram_peek.C $(STDLINK) error.o ao_utils.o -L. $(SCRAMOBJS) $(AOLIBS) -I$(TKDIR) $(LINKLIBS) -L$(SETISPECDIR) -lm  -o $@
	#$(CC) $(CFLAGS) scram_peek.C $(STDLINK) error.o ao_utils.o -L. $(SCRAMOBJS) $(AOLIBS) -lm -lsocket -ls4 -I../siren/include -L../siren/lib -o $@

dr2:    dr2.C $(DR2OBJS) $(EDTOBJS) $(SERIALOBJS) $(INCLUDES)
	$(CC) $(CFLAGS) $(DR2OBJS) $(EDTOBJS) $(SERIALOBJS) dr2.C $(STDLINK) $(LINKLIBS) $(EDTLINKLIBS) -L. -L$(EDTDIR) $(SCRAMOBJS) $(AOLIBS) -o $@

setispec_dr:    dr2.C $(DR2OBJS) $(SETISPECOBJS) $(SERIALOBJS) $(INCLUDES)
	$(CC) $(CFLAGS) $(DR2OBJS) $(SETISPECOBJS) $(SERIALOBJS) dr2.C $(STDLINK) $(LINKLIBS) -L. -L$(SETISPECDIR) $(SCRAMOBJS) $(AOLIBS) -o $@

# -rm so any errors from rm (ie file not found) will be ignored
clean:
	-rm dr2 simple_getdata set_clock *.o setispec_dr setispec/*.o scram_peek.o

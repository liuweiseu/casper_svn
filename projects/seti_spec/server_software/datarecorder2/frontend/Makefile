EDTDIR		=	/opt/EDTpcd

CC		=	gcc
CFLAGS          =       -g -D_REENTRANT -I$(EDTDIR) 

LINKLIBS 	= 	-lrt \
			-lpthread \
			-ledt

simple_waveform:	simple_waveform.C adc_to_edt_funcs.C
	$(CC) $(CFLAGS) simple_waveform.C adc_to_edt_funcs.C -O -o $@ $(LINKLIBS) -L. -L$(EDTDIR)

clean:
	rm simple_waveform

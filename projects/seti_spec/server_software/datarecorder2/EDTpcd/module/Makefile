####
#
#	Makefile for the Linux 2.6 EDT driver 
#
###

# target

VER_SERIES=$(shell uname -r | cut -f1-2 -d.)
VER_MINOR=$(shell uname -r | cut -f3 -d.)

GCC_VER=$(shell $(CC) --version | head -1 | cut -d" " -f3 | cut -d. -f1)

ifneq ($(strip $(CONFIG_PPC)),)
CPUTYPE=PPC
#CFLAGS += -DBIG_ENDIAN -D_BIG_ENDIAN -D__BIG_ENDIAN -D__BIG_ENDIAN_BITFIELD
CFLAGS += -DBIG_ENDIAN
else
CPUTYPE=
endif



ifeq ("$(VER_SERIES)","2.6")

ifneq ($(KERNELRELEASE),)

CFLAGS += -D_KERNEL -D__KERNEL__ -I$(PWD)/..

obj-m += edt.o

BASICOBJS = edt_lnx_kernel.o 

#DRVOBJS = edt_lnx.o edtioctl.o pdv.o p16d.o p11w.o pcd.o edt_dma.o edt_common.o edt_initdev.o edt_timeout.o



edt-objs += $(BASICOBJS) 


else


KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

KTARGETDIR= /lib/modules/`uname -r`/kernel/drivers/misc
TARGET=../`uname -r`/edt.ko

EDTDRVLIB=-l_edtdrv_$(VER_SERIES)$(CPUTYPE)_$(GCC_VER)

default: build install

build:
	$(MAKE) -C $(KDIR) V=1 SUBDIRS=$(PWD) modules
	ld -m elf_i386 -r -o edt.o edt_lnx_kernel.o -L. $(EDTDRVLIB) 
	ld -m elf_i386 -r -o edt.ko edt.o edt.mod.o 
	mkdir -p ../`uname -r`
	cp edt.ko ../`uname -r`

install: 
	mkdir -p $(KTARGETDIR) 
	install -c $(TARGET) $(KTARGETDIR)
	depmod -a

uninstall:
	rm /lib/modules/`uname -r`/kernel/drivers/misc/$(TARGET)
	depmod -a

load: default
	./edt_load

unload:
	./edt_unload

cleano:
	-rm -f *.o *.ko .*cmd 

endif

else

include makefile.drv

endif
	
objclean:
	@echo "Cleaning objects"
	@-rm -fr *.o *~ core .depend driverlib obj_* .*cmd

clean: objclean
	@echo "Cleaning source for driver"
	@-cleanlinks -q
	@-sccs clean 

source:
	@echo "Getting source for driver"
	@-sccs get -s SCCS
	@-chmod +x makelinks cleanlinks 
	@-makelinks -q
	


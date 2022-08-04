AR = ar --target elf32-i386
#deploy params
DEPSDIR = ./
DEPDDIR = ../zeos/
ZEOS_SOURCES= bootsect.S \
				build.c \
				devices.c \
				entry.S \
				hardware.c \
				interrupt.c \
				io.c \
				libc.c  \
				list.c\
				libzeos.a \
				mm.c \
				sched.c \
				sys.c \
				sys_call_table.S \
				system.c \
				system.lds\
				user.c \
				user.lds \
				utils.c \
				.bochsrc \
				.bochsrc_gdb \
				.gdbcmd\
				libauxjp.a\
				sem.c\

DEPOBJS = zeos_interrupt.o zeos_entry.o zeos_mm.o
DEPLIBRARY = libzeos.a

LIBSEMAPHORESOBJ=zeos_semaphores.o zeos_semaphores_syscall.o

# Create new dependences
all: libauxjp.a $(DEPLIBRARY) soalib

clean: clean_lib

clean_lib:
	rm -f $(DEPLIBRARY) libauxjp.a

# User Makefile
include Makefile.deploy

# New objects (code that will not be visible to the students)
zeos_entry.s: zeos_entry.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

zeos_interrupt.o:zeos_interrupt.c $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h

zeos_sys.o:zeos_sys.c $(INCLUDEDIR)/devices.h

zeos_mm.o:zeos_mm.c 

sched_policy.o:sched_policy.c $(INCLUDEDIR)/sched.h

$(DEPLIBRARY): $(DEPOBJS)
	$(AR) -r $(DEPLIBRARY) $(DEPOBJS)

zeos_semaphores.o: zeos_semaphores.c $(INCLUDEDIR)/sched.h 

zeos_semaphores_syscall.s: zeos_semaphores_syscall.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

libauxjp.a: $(LIBSEMAPHORESOBJ)
	$(AR) -r $@ $(LIBSEMAPHORESOBJ)

deployso2: $(DEPLIBRARY) system
	rm -rf $(DEPDDIR)
	mkdir $(DEPDDIR)
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build
	cp $(ZEOS_SOURCES) $(DEPDDIR)
	mkdir $(DEPDDIR)/include
	cp include/*.h $(DEPDDIR)/include
	cp Makefile.deploy $(DEPDDIR)Makefile
	tar zcvf zeos_so2.tar.gz $(DEPDDIR)

deploysoa:$(DEPLIBRARY) system
	rm -rf $(DEPDDIR)
	mkdir $(DEPDDIR)
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build
	cp $(ZEOS_SOURCES) $(DEPDDIR)
	mkdir $(DEPDDIR)/include
	cp include/*.h $(DEPDDIR)/include
	cp soa_supportE3/libschedperf.a soa_supportE3/libuser.a soa_supportE3/soa_schedPerf.xls $(DEPDDIR)
	cp soa_supportE3/schedperf.h $(DEPDDIR)/include
	cp Makefile.deploy $(DEPDDIR)/Makefile
	tar zcvf zeos_soa.tar.gz $(DEPDDIR)

deploy: all deployso2 deploysoa

soalib:
	cd soa_supportE3; make all

qemu: all
	qemu-system-i386 -fda zeos.bin -hda fs.img

debug: all
	@echo "target remote | qemu-system-i386 -fda zeos.bin -hda fs.img -gdb stdio"
	gdb system -x .gdbinit

disassboot:	## A reminder to disassemble the boot sector
	objdump -b binary -m i8086 -D bootsect

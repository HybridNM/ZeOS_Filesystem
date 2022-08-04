CFLAGS+= -m32 -ffreestanding -I. -I../include
LDFLAGS+= -melf_i386
OBJS_SYS= zeos_devices.o zeos_sched.o zeos_sys.o

OBJS_USR= zeos_libc.o

all: libschedperf.a libuser.a

libschedperf.a: $(OBJS_SYS)
		ar -q libschedperf.a $(OBJS_SYS)
		@echo "==== Created libschedperf.a ===="

libuser.a: $(OBJS_USR)
		ar -q libuser.a $(OBJS_USR)
		@echo "==== Created libuser.a ===="

clean:
	rm -f libuser.a libschedperf.a $(OBJS_SYS) $(OBJS_USR)

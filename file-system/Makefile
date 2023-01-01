CFLAGS=-I. -m32 -g -fno-omit-frame-pointer -ffreestanding -Wall -fno-PIC

OBJS=fat32.o
all: fat32.a

fat32.o: fat32.c fat32.h

fat32.a: ${OBJS}
	ar r fat32.a ${OBJS}

clean:
	rm -rf ${OBJS}

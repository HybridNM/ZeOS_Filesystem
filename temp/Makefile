CFLAGS=-I.  -m32 -g -fno-omit-frame-pointer -ffreestanding -Wall -fno-PIC

OBJS=ide.o io_patch.o
all: ide.a

io_patch.o: io_patch.c

ide.o: ide.c ide.h

ide.a: ${OBJS}
	ar r ide.a ${OBJS}

.PHONY: clean deploy deploylib deploysrc
clean:
	rm -rf ${OBJS}

realclean: clean
	rm -f ide.a *.tar.gz

deploy:  deploylib deploysrc

deploylib: ide.a
	tar zcvf disk-driver.tar.gz README.txt ide.h ide.a

deploysrc:
	tar zcvf disk-driver.src.tar.gz --exclude=.git *.c *.h README.txt Makefile

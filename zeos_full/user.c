#include <libc.h>

#define READ_TEST 0
#define WRITE_TEST 0
#define CREATE_TEST 1
#define LSEEK_TEST 0
#define UNLINK_TEST 0

char buff[24];
int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
	/* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	// char buffer[256];
	// int ticks;
	// ticks=gettime();
	// itoa(ticks, buffer);
	// write(1,buffer,strlen(buffer));


	///////////////////////
	// File System tests //
	///////////////////////

	int fd;

	// read test
	#if READ_TEST
		int ret_read;
		char filepathR[30] = "/DIR1/FILE1";
		char filebufferR[60];

		fd = open(filepathR, 0);
		ret_read = read(fd, filebufferR, 30);

		write(1, "Read contents: ", 17);
		if (!ret_read) write(1, filebufferR, strlen(filebufferR));
		write(1, "\n", 1);

	#endif


	// write test
	#if WRITE_TEST
		int ret_write;
		char filepathW[30] = "/DIR1/FILE1";
		char filebufferW[30] = "FILE1 written here";

		fd = open(filepathW, 1);
		ret_write = write(fd, filebufferW, 30);

		if (!ret_write) write(1, "Write successful\n", 17);
		else write(1, "Write unsuccessful\n", 19);

		if (LSEEK_TEST)
		{
			lseek(fd, 30, SEEK_SET);
			write(fd, filebufferW, 30);
		}
	#endif


	// open non existent file test
	#if CREATE_TEST
		int ret_open;
		char openFile[30] = "/DIR1/FILE5";

		ret_open = open(openFile, 0);
		if (ret_open>0) write(1, "File creation successful\n", 25);
		else write(1, "File creation unsuccessful\n", 27);

	#endif


	// unlink test
	#if UNLINK_TEST
		int ret_unlink;
		char deleteFile[30] = "/DIR1/FILE1";

		ret_unlink = unlink(deleteFile);

		if (!ret_unlink) write(1, "Unlink successful\n", 18);
		else write(1, "Unlink unsuccessful\n", 20);
	#endif


  while(1) { ;}
}

/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

#define SEEK_SET 0
#define SEEK_CUR 1

int write(int fd, char *buffer, int size);

int read(int fd, char *buffer, int size);

int open(char *path, int mode);

int close(int fd);

int unlink(char *path);

int lseek(int fd, int offset, int whence);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

#endif  /* __LIBC_H__ */

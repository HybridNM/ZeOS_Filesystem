/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}


int lseek(int fd, int offset, int whence) 
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $9, %%eax\n"
			"movl %1, %%ebx\n"
			"movl %2, %%ecx\n"
			"movl %3, %%edx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (fd), "g" (offset), "g" (whence)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}


// I/O Functions

int write(int fd, char *buffer, int nbytes) 
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $4, %%eax\n"
			"movl %1, %%ebx\n"
			"movl %2, %%ecx\n"
			"movl %3, %%edx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (fd), "g" (buffer), "g" (nbytes)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}

int read(int fd, char *buffer, int nbytes)
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $5, %%eax\n"
			"movl %1, %%ebx\n"
			"movl %2, %%ecx\n"
			"movl %3, %%edx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (fd), "g" (buffer), "g" (nbytes)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}

int open(char *path, int mode)
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $6, %%eax\n"
			"movl %1, %%ebx\n"
			"movl %2, %%ecx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (path), "g" (mode)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}


int close(int fd)
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $7, %%eax\n"
			"movl %1, %%ebx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (fd)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}


int unlink(char *path)
{
	int ret;
	__asm__ __volatile__ (
			"pushl %%ebx \n"
			"movl $8, %%eax\n"
			"movl %1, %%ebx\n"
			"int $0x80\n"
			"popl %%ebx \n"
			"movl %%eax, %0"
			: "=g" (ret)
			: "g" (path)
			);

	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}


int gettime(){
int ret;
__asm__ __volatile__ (
		"movl $10, %%eax\n"
		"int $0x80\n"
		"movl %%eax, %0"
		: "=g" (ret)	
		);
if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;

}

void perror() {


}

void exit() {
__asm__ __volatile__ (
		"movl $1, %eax\n"
		"int $0x80\n"
		);
}


int getpid(void) {
int ret;
__asm__ __volatile__ (
		"movl $20, %%eax\n"
		"int $0x80\n"
		"movl %%eax, %0"
		: "=g" (ret)
		: 
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

int fork(void) {
int ret;
__asm__ __volatile__ (
		"movl $2, %%eax\n"
		"int $0x80\n"
		"movl %%eax, %0"
		: "=g" (ret)
		: 
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}
int get_stats(int pid, struct stats *s)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $3, %%eax\n"
		"movl %1, %%ebx\n"
		"movl %2, %%ecx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (pid), "g" (s)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}
int clone(void (*function)(void), void* stack)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $19, %%eax\n"
		"movl %1, %%ebx\n"
		"movl %2, %%ecx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (function), "g" (stack)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

int sem_init(int n_sem, unsigned int value)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $21, %%eax\n"
		"movl %1, %%ebx\n"
		"movl %2, %%ecx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (n_sem), "g" (value)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

int sem_wait(int n_sem)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $22, %%eax\n"
		"movl %1, %%ebx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (n_sem)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

int sem_signal(int n_sem)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $23, %%eax\n"
		"movl %1, %%ebx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (n_sem)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

int sem_destroy(int n_sem)
{
int ret;
__asm__ __volatile__ (
		"pushl %%ebx \n"
		"movl $24, %%eax\n"
		"movl %1, %%ebx\n"
		"int $0x80\n"
		"popl %%ebx \n"
		"movl %%eax, %0"
		: "=g" (ret)
		: "g" (n_sem)
		);

if (ret < 0) {
	errno = -ret;
	ret = -1;
}
return ret;
}

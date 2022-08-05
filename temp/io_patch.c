/****
 **** THIS CODE MUST BE ADDED TO io.c
 ****/

typedef unsigned char	Byte;

/* Write byte 'value' to 'port' */
void outb (unsigned short port, Byte value )
{
	__asm__ __volatile__ ( "movb %0, %%al; outb %1" ::"a"(value), "Nd"(port));
}
/* Read 'cnt' longs (4bytes) from IO port 'port' to 'addr' */
void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl"
				: "=D" (addr), "=c" (cnt)
				: "d" (port), "0" (addr), "1" (cnt)
				: "memory", "cc");
}

/* Write 'cnt' longs(4bytes) from 'addr' to IO port 'port' */
void outsl(int port, const void *addr, int cnt)
{
  asm volatile("cld; rep outsl"
				: "=S" (addr), "=c" (cnt)
				: "d" (port), "0" (addr), "1" (cnt)
				: "cc");
}



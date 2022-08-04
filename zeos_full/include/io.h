/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void outb (unsigned short port, Byte value );
void insl(int port, void *addr, int cnt);
void outsl(int port, const void *addr, int cnt);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);

#endif  /* __IO_H__ */

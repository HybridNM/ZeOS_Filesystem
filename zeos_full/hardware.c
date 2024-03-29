/*
 * hardware.c 
 */


#include <types.h>

extern unsigned int *p_rdtr ;
DWord get_eflags(void)
{
  register DWord flags;
__asm__ __volatile__(
  "pushfl\n\t"
  "popl %0"
  : "=q" (flags) );

  return flags;
}

void set_eflags(void)
{
__asm__ __volatile__(
  "pushl $0\n\t"
  "popfl" );
}

void set_idt_reg(Register * idt)
{
__asm__ __volatile__(
  "lidtl (%0)"
  : /*no output*/
  : "r" (idt) );
}

void set_gdt_reg(Register * gdt)
{
__asm__ __volatile__(
  "lgdtl (%0)"
  : /*no output*/
  : "r" (gdt) );
}

void set_ldt_reg(Selector ldt)
{
__asm__ __volatile__(
  "lldtw %0"
  : /*no output*/
  : "r" (ldt) );
}

void set_task_reg(Selector tr)
{
__asm__ __volatile__(
  "ltrw %0"
  : /*no output*/
  : "r" (tr) );
}


void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip)
{
  __asm__ __volatile__ (
    "mov %0,%%es\n\t"
    "mov %0,%%ds\n\t"
    "movl %2, %%eax\n\t"
    "addl $12, %%eax\n\t"
    "movl %5,(%%eax)\n\t"
    "pushl %1\n\t"       /* user ss */
    "pushl %2\n\t"       /* user esp */
    "pushl %3\n\t"       /* user cs */
    "pushl %4\n\t"       /* user eip */
    "lret"
    : /*no output*/
    : "m" (ds), "m" (ss), "m" (esp), "m" (cs), "m" (eip), "d" (*p_rdtr));
    //: "r" (ds), "g" (ss), "g" (esp), "g" (cs), "g" (eip), "d" (*p_rdtr));
}

/*
 * enable_int: Set interruput mask
 *
 *    register 0x21:
 *    7 6 5 4 3 2 1 0
 *    x x x x x x x x
 *    
 *    bit 0 : Timer
 *    bit 1 : Keyboard
 *    bit 2 : PIC cascading
 *    bit 3 : 2nd Serial Port
 *    bit 4 : 1st Serial Port
 *    bit 5 : Reserved
 *    bit 6 : Floppy disk
 *    bit 7 : Reserved
 * 
 *
 *   x = 0 -> enabled
 *   x = 1 -> disabled
 *
 *   register 0xA1 (2n PIC connected to bit 2)
 *    7 6 5 4 3 2 1 0
 *    x x x x x x x x
 *    15  13  11  9
 *      14  12  10  8
 *
 *   bit 6 (14): IDE drive
 *   bit 7 (15): 2nd IDE drive
 */

void enable_int(void)
{
__asm__ __volatile__(
  "movb %0,%%al\n\t"
  "outb %%al,$0x21\n\t"
  "call delay\n\t"

  "movb %1,%%al\n\t"
  "outb %%al,$0xA1\n\t"
  "call delay\n\t"

  "sti"
  : /*no output*/
  : "i" (0xf8)       /* 0xF8 = 11111000 -> Timer, KB, PIC cascading enabled */
  , "i" (0x3f)       /* 0x3F = 00111111 -> IDE drive0, drive1 enabled */

  : "%al" );
}

void delay(void)
{
__asm__ __volatile__(
  "jmp a\na:\t"
  : : );
}


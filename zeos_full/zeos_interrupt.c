#include <sched.h>
#include <utils.h>
#include <list.h>
#include <io.h>

int clocks=0;
int segs=0;
int mins=0;

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

void zeos_show_clock()
{
    void printc_xy( Byte mx, Byte my, char c)
    {
        Word ch = (Word) (c & 0x00FF) | 0x0200;
		/*
        DWord screen = 0xb8000 + (my * 80 + mx) * 2;
        asm("movw %0, (%1)" : : "g"(ch), "r"(screen));
		*/
		Word *screen = (Word*)0xb8000;
		screen[(my * 80 + mx)] = ch;
    }
  char cmins[4];
  char csegs[4];

  clocks++;
  if (clocks==18)
  {
    segs++;
    clocks=0;
    if (segs==60)
    {
      segs=0;
      mins++;
    }
  itoa(mins, cmins);
  itoa(segs, csegs);
  
  if (mins>=10) printc_xy(70, 0, cmins[0]); else printc_xy(70, 0, '0');
  if (mins>=10) printc_xy(71, 0, cmins[1]); else printc_xy(71, 0, cmins[0]);
  printc_xy(72, 0, ':');
  if (segs>=10) printc_xy(73, 0, csegs[0]); else printc_xy(73, 0, '0');
  if (segs>=10) printc_xy(74, 0, csegs[1]); else printc_xy(74, 0, csegs[0]);
  }
}

extern struct list_head blocked;

#if 0
void zeos_IO_emulation(void (*enqueue)(struct task_struct* t))
{
  struct task_struct *t;
  struct list_head *cur_task, *next_cur;

  if (!list_empty(&blocked))
  {
    cur_task=list_first(&blocked);
    while (1)
    {
      next_cur=cur_task->next;
      t=list_head_to_task_struct(cur_task);
      t->remaining_io_ticks--;
      if (t->remaining_io_ticks==0)
      {
        list_del(&(t->list));
	if (enqueue!=NULL) (*enqueue)(t);
      }
      if (next_cur==&blocked) break;
      cur_task=next_cur;
    }
  }
}
#endif


void divide_error_routine()
{
  printk("\nDivide error fault\n");
  while(1);
}

void debug_routine()
{
  printk("\nDebug fault\n");
  while(1);
}

void nm1_routine()
{
  printk("\nnm1 fault\n");
  while(1);
}

void breakpoint_routine()
{
  printk("\nBreakpoint fault\n");
  while(1);
}

void overflow_routine()
{
  printk("\nOverflow fault\n");
  while(1);
}

void bounds_check_routine()
{
  printk("\nBounds check fault\n");
  while(1);
}

void invalid_opcode_routine()
{
  printk("\nInvalid opcode fault\n");
  while(1);
}

void device_not_available_routine()
{
  printk("\nDevice not available fault\n");
  while(1);
}

void double_fault_routine()
{
  printk("\ndouble fault\n");
  while(1);
}

void coprocessor_segment_overrun_routine()
{
  printk("\nCoprocessor segment overrun fault\n");
  while(1);
}

void invalid_tss_routine()
{
  printk("\nInvalid tss fault\n");
  while(1);
}

void segment_not_present_routine()
{
  printk("\nSegment not present fault\n");
  while(1);
}

void stack_exception_routine()
{
  printk("\nStack exception fault\n");
  while(1);
}


void general_protection_routine()
{
  printk("\nGeneral protection fault\n");
  while(1);
}

void page_fault_routine()
{
  printk("\nPage fault\n");
  while(1);
}

void intel_reserved_routine()
{
  printk("\nIntel reserved fault\n");
  while(1);
}

void floating_point_error_routine()
{
  printk("\nFloating point fault\n");
  while(1);
}

void alignment_check_routine()
{
  printk("\nAlignment check fault\n");
  while(1);
}

extern void divide_error_handler();
extern void debug_handler();
extern void nm1_handler();
extern void breakpoint_handler();
extern void overflow_handler();
extern void bounds_check_handler();
extern void invalid_opcode_handler();
extern void device_not_available_handler();
extern void double_fault_handler(long);
extern void coprocessor_segment_overrun_handler();
extern void invalid_tss_handler(long);
extern void segment_not_present_handler(long);
extern void stack_exception_handler(long);
extern void general_protection_handler(long);
extern void page_fault_handler(long);
extern void intel_reserved_handler(long);
extern void floating_point_error_handler();
extern void alignment_check_handler();

extern void system_call_handler();

extern void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

extern void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void set_handlers()
{
  setInterruptHandler(0, divide_error_handler, 0);
  setInterruptHandler(1, debug_handler, 0);
  setInterruptHandler(2, nm1_handler, 0);
  setInterruptHandler(3, breakpoint_handler, 0);
  setInterruptHandler(4, overflow_handler, 0);
  setInterruptHandler(5, bounds_check_handler, 0);
  setInterruptHandler(6, invalid_opcode_handler, 0);
  setInterruptHandler(7, device_not_available_handler, 0);
  setInterruptHandler(8, double_fault_handler, 0);
  setInterruptHandler(9, coprocessor_segment_overrun_handler, 0);
  setInterruptHandler(10, invalid_tss_handler, 0);
  setInterruptHandler(11, segment_not_present_handler, 0);
  setInterruptHandler(12, stack_exception_handler, 0);
  setInterruptHandler(13, general_protection_handler, 0);
  setInterruptHandler(14, page_fault_handler, 0);
  setInterruptHandler(15, intel_reserved_handler, 0);
  setInterruptHandler(16, floating_point_error_handler, 0);
  setInterruptHandler(17, alignment_check_handler, 0);
  
  //setTrapHandler(0x80, system_call_handler, 3);
}

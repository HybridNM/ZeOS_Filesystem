/*
 * zeos_entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es; \
		pushl %eax; \
		call user2system; \
		popl %eax;

#define RESTORE_ALL \
call system2user; \
	popl %ebx; \
	popl %ecx; \
	popl %edx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	popl %ds; \
	popl %es; \
	popl %fs; \
	popl %gs;

#define EOI \
movb $0x20, %al; \
outb %al, $0x20;


ENTRY (divide_error_handler)
	SAVE_ALL
	call divide_error_routine
	RESTORE_ALL
	IRET

ENTRY (debug_handler)
	SAVE_ALL
	call debug_routine
	RESTORE_ALL
	IRET

ENTRY (nm1_handler)
	SAVE_ALL
	call nm1_routine
	RESTORE_ALL
	IRET

ENTRY (breakpoint_handler)
	SAVE_ALL
	call breakpoint_routine
	RESTORE_ALL
	IRET

ENTRY (overflow_handler)
	SAVE_ALL
	call overflow_routine
	RESTORE_ALL
	IRET

ENTRY (bounds_check_handler)
	SAVE_ALL
	call bounds_check_routine
	RESTORE_ALL
	IRET

ENTRY (invalid_opcode_handler)
	SAVE_ALL
	call invalid_opcode_routine
	RESTORE_ALL
	IRET

ENTRY (device_not_available_handler)
	SAVE_ALL
	call device_not_available_routine
	RESTORE_ALL
	IRET

ENTRY (double_fault_handler)
	SAVE_ALL
	call double_fault_routine
	RESTORE_ALL
	IRET

ENTRY (coprocessor_segment_overrun_handler)
	SAVE_ALL
	call coprocessor_segment_overrun_routine
	RESTORE_ALL
	IRET

ENTRY (invalid_tss_handler)
	SAVE_ALL
	call invalid_tss_routine
	RESTORE_ALL
	IRET

ENTRY (segment_not_present_handler)
	SAVE_ALL
	call segment_not_present_routine
	RESTORE_ALL
	IRET

ENTRY (stack_exception_handler)
	SAVE_ALL
	call stack_exception_routine
	RESTORE_ALL
	IRET

ENTRY (general_protection_handler)
	SAVE_ALL
	call general_protection_routine
	RESTORE_ALL
	IRET

ENTRY (page_fault_handler)
	SAVE_ALL
	call page_fault_routine
	RESTORE_ALL
	IRET

ENTRY (intel_reserved_handler)
	SAVE_ALL
	call intel_reserved_routine
	RESTORE_ALL
	IRET

ENTRY (floating_point_error_handler)
	SAVE_ALL
	call floating_point_error_routine
	RESTORE_ALL
	IRET

ENTRY (alignment_check_handler)
	SAVE_ALL
	call alignment_check_routine
	RESTORE_ALL
	IRET

/*
ENTRY (system_call_handler)
	SAVE_ALL
	cmp $0, %eax
	jl err
	cmpl $40, %eax
	jg err
		
	call *sys_call_table(, %eax, 0x04)
	jmp fin
err:
	movl $-38, %eax
fin:
	movl %eax, 0x18(%esp)
	
	RESTORE_ALL
	IRET
	*/

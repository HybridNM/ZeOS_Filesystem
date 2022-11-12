# 0 "zeos_semaphores_syscall.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "zeos_semaphores_syscall.S"

# 1 "include/asm.h" 1
# 3 "zeos_semaphores_syscall.S" 2
# 1 "include/segment.h" 1
# 4 "zeos_semaphores_syscall.S" 2
# 1 "include/errno.h" 1
# 5 "zeos_semaphores_syscall.S" 2
# 75 "zeos_semaphores_syscall.S"
.globl zeos_call_handler; .type zeos_call_handler, @function; .align 0; zeos_call_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call *zeos_call_table(, %eax, 0x04)
 movl %eax, 0x18(%esp)
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl zeos_call_table; .type zeos_call_table, @function; .align 0; zeos_call_table:
 .long zeos_sys_sem_init
 .long zeos_sys_sem_destroy
 .long zeos_sys_sem_signal
 .long zeos_sys_sem_wait
 .long zeos_sys_nice

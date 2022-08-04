# 1 "zeos_entry.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "zeos_entry.S"




# 1 "include/asm.h" 1
# 6 "zeos_entry.S" 2
# 1 "include/segment.h" 1
# 7 "zeos_entry.S" 2
# 1 "include/errno.h" 1
# 8 "zeos_entry.S" 2
# 77 "zeos_entry.S"
.globl divide_error_handler; .type divide_error_handler, @function; .align 0; divide_error_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call divide_error_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl debug_handler; .type debug_handler, @function; .align 0; debug_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call debug_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl nm1_handler; .type nm1_handler, @function; .align 0; nm1_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call nm1_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl breakpoint_handler; .type breakpoint_handler, @function; .align 0; breakpoint_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call breakpoint_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl overflow_handler; .type overflow_handler, @function; .align 0; overflow_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call overflow_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl bounds_check_handler; .type bounds_check_handler, @function; .align 0; bounds_check_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call bounds_check_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl invalid_opcode_handler; .type invalid_opcode_handler, @function; .align 0; invalid_opcode_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call invalid_opcode_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl device_not_available_handler; .type device_not_available_handler, @function; .align 0; device_not_available_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call device_not_available_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl double_fault_handler; .type double_fault_handler, @function; .align 0; double_fault_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call double_fault_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl coprocessor_segment_overrun_handler; .type coprocessor_segment_overrun_handler, @function; .align 0; coprocessor_segment_overrun_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call coprocessor_segment_overrun_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl invalid_tss_handler; .type invalid_tss_handler, @function; .align 0; invalid_tss_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call invalid_tss_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl segment_not_present_handler; .type segment_not_present_handler, @function; .align 0; segment_not_present_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call segment_not_present_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl stack_exception_handler; .type stack_exception_handler, @function; .align 0; stack_exception_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call stack_exception_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl general_protection_handler; .type general_protection_handler, @function; .align 0; general_protection_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call general_protection_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl page_fault_handler; .type page_fault_handler, @function; .align 0; page_fault_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call page_fault_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl intel_reserved_handler; .type intel_reserved_handler, @function; .align 0; intel_reserved_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call intel_reserved_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl floating_point_error_handler; .type floating_point_error_handler, @function; .align 0; floating_point_error_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call floating_point_error_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

.globl alignment_check_handler; .type alignment_check_handler, @function; .align 0; alignment_check_handler:
 pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %edx; pushl %ecx; pushl %ebx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es; pushl %eax; call user2system; popl %eax;
 call alignment_check_routine
 call system2user; popl %ebx; popl %ecx; popl %edx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
 IRET

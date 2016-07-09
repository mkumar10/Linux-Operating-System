#define ASM 1
#include "syscalls_linkage.h"
#include "x86_desc.h"

.data
    MAX_SYSCALL_NUM  = 10
    MIN_SYSCALL_NUM =  1
    SYS_HALT = 1
    EBX_MASK = 0xFF

.text

.globl  syscall_handler


# Jump table for 10 system calls
syscall_jump_table:
    .long   0x0
    .long   halt_func   #halt
    .long   execute_func
    .long   read_func
    .long   write_func
    .long   open_func   #open
    .long   close_func
    .long   getargs_func
    .long   vidmap_func
    .long   set_handler_func
    .long   sigreturn_func

# The actual handler for system calls. Address should reside in IDT
syscall_handler:
    pushfl
    pushal

    # Todo: Check if syscall_number (EAX) is within valid range
    cmpl    $MAX_SYSCALL_NUM,   %eax
    jg      invalid

    cmpl    $MIN_SYSCALL_NUM,   %eax
    jnl     valid

invalid:
    movl $-1, TEMP
    jmp end
    # Push all the arguments (maximum 3) onto the stack
valid:
    cmpl    $SYS_HALT,  %eax
    jne     not_halt
    andl    $EBX_MASK,  %ebx
not_halt:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    # Change to kernel mode
    movw    $KERNEL_DS, %si
    movw    %si,        %ds

    # Jump to actual system call function
    call   *syscall_jump_table(,%eax,4)

    # Return from system call. EAX will have the return value.
    movl    %eax,   TEMP
    addl    $12,    %esp
end:
    popal
    popfl
    movl    TEMP,    %eax
    iret
TEMP:
    .long   0x0

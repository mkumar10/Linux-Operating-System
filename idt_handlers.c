#include "idt_handlers.h"
#define EXCEPTION_RET 256

// TODO: Macro for every exception handler.
// #define get_reg_info(...)
// do{
//   asm volatile("movl %%ebp, %%ebx"
//               :
//               :
//               :"ebx");
//   volatile uint32_t page_fault_addr, error_code, EIP, CS, EFLAGS;
//
//   asm volatile("movl  %%eax, %0"
//               :"=r"(page_fault_addr));
//   asm volatile("movl 4(%%ebx), %0"
//               :"=r"(error_code));
//   asm volatile("movl 8(%%ebx), %0"
//               :"=r"(EIP));
//   asm volatile("movl 12(%%ebx), %0"
//               :"=r"(CS));
//   asm volatile("movl 16(%%ebx), %0"
//               :"=r"(EFLAGS));
//
//
//   terminal_printf("Page Fault Exception.\n");
//   terminal_printf("Address:%#x\n", page_fault_addr);
//   terminal_printf("Error code:%#x\n", error_code);
//   terminal_printf("EIP:%#x\n", EIP);
//   terminal_printf("CS:%#x\n", CS);
//   terminal_printf("EFLAGS:%#x\n", EFLAGS);
//   // terminal_printf("Page Fault Exception.\n");
// }

/*
 * squash_user_prog
 *   DESCRIPTION: Infinite loop to squash user program for checkpoint 1.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of infinite loop.
 */
void squash_user_prog(){
  halt_retval[curr_term] = EXCEPTION_RET;
  halt_func(0);
  return;
}

/*
 * exception_0
 *   DESCRIPTION: Divide Error Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_0(){
    terminal_printf("Divide Error Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_1
 *   DESCRIPTION: Debug Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_1(){
    terminal_printf("Debug Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_2
 *   DESCRIPTION: NMI Interrupt Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_2(){
    terminal_printf("NMI Interrupt.\n");
    squash_user_prog();
    return;
}


/*
 * exception_3
 *   DESCRIPTION: Breakpoint Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_3(){
    terminal_printf("Breakpoint Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_4
 *   DESCRIPTION: Overflow Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_4(){
    terminal_printf("Overflow Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_5
 *   DESCRIPTION: BOUND Range Exceeded Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_5(){
    terminal_printf("BOUND Range Exceeded Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_6
 *   DESCRIPTION: Invalid Opcode Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_6(){
    terminal_printf("Invalid Opcode Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_7
 *   DESCRIPTION: Device Not Available Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_7(){
    terminal_printf("Device Not Available Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_8
 *   DESCRIPTION: Double Fault Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_8(){
    terminal_printf("Double Fault Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_9
 *   DESCRIPTION: Coprocessor Segment Overrun Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_9(){
    terminal_printf("Coprocessor Segment Overrun.\n");
    squash_user_prog();
    return;
}

/*
 * exception_10
 *   DESCRIPTION: Invalid TSS Exception Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_10(){
    terminal_printf("Invalid TSS Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_11
 *   DESCRIPTION: Segment Not Present Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_11(){
    terminal_printf("Segment Not Present.\n");
    squash_user_prog();
    return;
}

/*
 * exception_12
 *   DESCRIPTION: Stack Fault Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_12(){
    terminal_printf("Stack Fault Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_13
 *   DESCRIPTION: General Protection Exception Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_13(){
    terminal_printf("General Protection Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_14
 *   DESCRIPTION: Page Fault Exception Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_14(){
    asm volatile("movl %%cr2, %%eax"
                :
                :
                :"eax");
    asm volatile("movl %%ebp, %%ebx"
                :
                :
                :"ebx");
    volatile uint32_t page_fault_addr, error_code, EIP, CS, EFLAGS;

    asm volatile("movl  %%eax, %0"
                :"=r"(page_fault_addr));
    asm volatile("movl 4(%%ebx), %0"
                :"=r"(error_code));
    asm volatile("movl 8(%%ebx), %0"
                :"=r"(EIP));
    asm volatile("movl 12(%%ebx), %0"
                :"=r"(CS));
    asm volatile("movl 16(%%ebx), %0"
                :"=r"(EFLAGS));

    terminal_printf("Page Fault Exception.\n");
    terminal_printf("Address:%#x\n", page_fault_addr);
    terminal_printf("Error code:%#x\n", error_code);
    terminal_printf("EIP:%#x\n", EIP);
    terminal_printf("CS:%#x\n", CS);
    terminal_printf("EFLAGS:%#x\n", EFLAGS);
    // terminal_printf("Page Fault Exception.\n");

    squash_user_prog();
    return;
}

/*
 * exception_15
 *   DESCRIPTION:  Exception Reserved by Intel.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_15(){
    terminal_printf("Reserved by Intel.\n");
    squash_user_prog();
    return;
}

/*
 * exception_16
 *   DESCRIPTION: x87 FPU Floating Point Error Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_16(){
    terminal_printf("x87 FPU Floating Point Error.\n");
    squash_user_prog();
    return;
}

/*
 * exception_17
 *   DESCRIPTION: Alignment Check Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_17(){
    terminal_printf("Alignment Check Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_18
 *   DESCRIPTION: Machine Check Exception Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_18(){
    terminal_printf("Machine Check Exception.\n");
    squash_user_prog();
    return;
}

/*
 * exception_19
 *   DESCRIPTION: SIMD Floating Point Exception handler.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: System "freezes" because of squash_user_prog().
 */
void exception_19(){
    terminal_printf("SIMD Floating Point Exception.\n");
    squash_user_prog();
    return;
}

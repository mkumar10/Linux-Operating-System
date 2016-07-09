#define ASM 1
#include "interrupt_handler.h"

.data
    RTC =   0
    PIT =   1
    KB  =   2
.text

.globl  PIT_handler
.globl  RTC_handler
.globl  KB_handler

intr_jump_table:
    .long   RTC_IRQ_handler
    .long   pit_handler
    .long   get_c_keyboard

RTC_handler:
    pushal
    movl    $RTC,     %eax
    jmp     INTR_HANDLER

PIT_handler:
    pushal
    movl    $PIT,     %eax
    jmp     INTR_HANDLER

KB_handler:
    pushal
    movl    $KB,     %eax
    jmp     INTR_HANDLER

INTR_HANDLER:
    call    *intr_jump_table(,%eax,4)
    # back to the common handler, restore all the registers.
    popal
    iret

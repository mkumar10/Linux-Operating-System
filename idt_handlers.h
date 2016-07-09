#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "rtc_driver.h"
#include "syscalls.h"

void squash_user_prog();
void exception_0();
void exception_1();
void exception_2();
void exception_3();
void exception_4();
void exception_5();
void exception_6();
void exception_7();
void exception_8();
void exception_9();
void exception_10();
void exception_11();
void exception_12();
void exception_13();
void exception_14();
void exception_15();
void exception_16();
void exception_17();
void exception_18();
void exception_19();


#endif // _IDT_HANDLER_H

/* Todo: Consider rename to PS/2. since right now this only deals with the keyboard.
*/

#ifndef _PS_2_H
#define _PS_2_H

#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "terminal.h"
#include "schedule.h"

#define PS2_CONTROLLER_PORT_1 0x60
#define KB_IRQ_NUM 0x1
#define KB_IDT_ENTRY 0x21


// void return_int();
// void save_registers();
// void restore_registers();

void init_keyboard();
void get_c_keyboard();

#endif //_PS_2_H

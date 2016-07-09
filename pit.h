#ifndef _PIT_H
#define _PIT_H

#include "globals.h"
#include "schedule.h"
#include "lib.h"

#define PIT_IRQ_NUM 0
#define PIT_IDT_ENTRY 0x20

#define PIT_RW_PORT_CH0 0x40
#define PIT_CMD_PORT 0x43


#define PIT_MAX_FREQ 1193180
#define PIT_MODE_REG_VAL 0x36 		//Select Channel 0, LI/HO Byte Access, Mode 2 - Rate Generator
#define BYTE_MASK 0xFF



void pit_init();
void set_pit_freq(uint32_t freq);
void pit_handler();

#endif

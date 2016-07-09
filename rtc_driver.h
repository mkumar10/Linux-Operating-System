#ifndef _RTC_DRIVER_H
#define _RTC_DRIVER_H

#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "globals.h"

#define RTC_IRQ_NUM     8
#define INDEX_PORT      0x70
#define RTC_PORT_RW     0x71
#define NMI_BIT         0x80
#define RTC_REG_B       0x0B
#define RTC_REG_C       0x0C
#define RTC_REG_A       0x0A
#define RTC_IDT_ENTRY   0x28
#define RTC_HIGH_MASK	0xF0
#define RTC_LOW_MASK	0x0F

void rtc_init();
void RTC_IRQ_handler();


int rtc_open(const uint8_t* filename);
int rtc_read(int32_t fd, void* buf, int32_t nbytes);
int rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int rtc_close(int32_t fd);

// Helper function to set rtc to default rate when exiting from a user program.
void rtc_default_rate();

#endif

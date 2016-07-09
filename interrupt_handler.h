#ifndef _INTR_HANDLER_H_
#define _INTR_HANDLER_H_

#ifndef ASM

#include "rtc_driver.h"
#include "PS_2.h"
#include "pit.h"

extern void RTC_handler();
extern void KB_handler();
extern void PIT_handler();

#endif
#endif

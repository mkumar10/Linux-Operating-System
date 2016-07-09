#include "pit.h"

#define SHIFT_8 8

void pit_init()
{
	outb(PIT_MODE_REG_VAL, PIT_CMD_PORT);
	uint32_t freq = 100;
	set_pit_freq(freq);
	pit_counter = 0;
	enable_irq(PIT_IRQ_NUM);
}

void set_pit_freq(uint32_t freq)
{
	outb((PIT_MAX_FREQ/freq) & BYTE_MASK, PIT_RW_PORT_CH0);
	outb(((PIT_MAX_FREQ/freq) >> SHIFT_8) & BYTE_MASK, PIT_RW_PORT_CH0);
}

void pit_handler()
{
    // disable_irq(PIT_IRQ_NUM);
    send_eoi(PIT_IRQ_NUM);
    schedule(pit_counter);
    // terminal_printf("%d\n", pit_counter);
    pit_counter = (pit_counter + 1)%NUM_TERMS;
    // enable_irq(PIT_IRQ_NUM);
}

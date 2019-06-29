#ifndef __INTERRUPT_H
#define __INTERRUPT_H

enum interrupts {
	V_BLANK_INTERRUPT  = 0x01,
	LCD_STAT_INTERRUPT = 0x02,
	TIMER_INTERRUPT    = 0x04,
	SERIAL_INTERRUPT   = 0x08,
	JOYPAD_INTERRUPT   = 0x10
};

void interrupt_init();
void interrupt_handle();
void interrupt_trigger(enum interrupts interrupt);

#endif

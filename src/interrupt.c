#include "interrupt.h"
#include "memory.h"
#include "cpu.h"

static char * _interrupt_enable;
static char * _interrupt_waiting;

void interrupt_init() {
	_interrupt_enable = memory_dump() + 0xFFFF;
	_interrupt_waiting = memory_dump() + 0xFF0F;
}

void interrupt_handle() {
	char process_interrupts;
	
	// See if interrupt needs to be handled
	process_interrupts = (*_interrupt_waiting) & (*_interrupt_enable);
	if(process_interrupts) {
		// Disable interrupts
		cpu_state.ime = 0;
		
		// Push address onto stack
		cpu_state.registers.SP -= 2;
		memory_write16(cpu_state.registers.SP, cpu_state.registers.PC);
		// TODO: How many cycles to PUSH PC ???
		
		// Determine which interrupt to execute
		if(process_interrupts & V_BLANK_INTERRUPT) {
			// V_BLANK
			// Jump
			cpu_state.registers.PC = 0x40;
			
			// Clear Flag
			*_interrupt_waiting ^= 0x1;
		} else if(process_interrupts & LCD_STAT_INTERRUPT) {
			// LCD_STAT
			// Jump
			cpu_state.registers.PC = 0x48;
			
			// Clear Flag
			*_interrupt_waiting ^= 0x2;
		} else if(process_interrupts & TIMER_INTERRUPT) {
			// TIMER
			// Jump
			cpu_state.registers.PC = 0x50;
			
			// Clear Flag
			*_interrupt_waiting ^= 0x4;
		} else if(process_interrupts & SERIAL_INTERRUPT) {
			// SERIAL
			// Jump
			cpu_state.registers.PC = 0x58;
			
			// Clear Flag
			*_interrupt_waiting ^= 0x8;
		} else if(process_interrupts & JOYPAD_INTERRUPT) {
			// JOYPAD
			// Jump
			cpu_state.registers.PC = 0x60;
			
			// Clear Flag
			*_interrupt_waiting ^= 0x10;
		}
	}
}

void interrupt_trigger(enum interrupts interrupt) {
	*_interrupt_waiting |= interrupt;
}



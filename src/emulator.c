#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "lcd.h"
#include "graphics.h"
#include "rom.h"

/**
Functions
*/
void emulator_init() {
	// Initialize system
	cpu_init();
	memory_init();
	lcd_init();
	graphics_init();
}

int main(int argc, char ** argv) {
	emulator_init();
	rom_load(argv[1]);
	
	while(cpu_state.running) {
		cpu_step();
#ifdef DISASSEMBLE
		printf("$%04x %s\n", disassembly_pc, disassembly);
#endif
	}

	return 0;
}

#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "lcd.h"
#include "graphics.h"
#include "rom.h"
#include "debugger.h"

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
	int i, debugger, rom;
	
	emulator_init();
	
	debugger = 0;
	
	// Get commane line arguments
	for(i = 1; i < argc; i++) {
		// Load rom
		if(!strcmp(argv[i], "-f") && i+1 < argc) {
			rom_load(argv[++i]);
		}
		
		// Print help
		if(!strcmp(argv[i], "-h")) {
			printf("usage: %s\n", argv[0]);
			printf("\t-f\tROM File (.gb)\n");
			printf("\t-debug\tStart debugger\n");
			printf("\t-h\tDisplay this screen\n");
			return 0;
		}
		
		// Start debugger
		if(!strcmp(argv[i], "-debug")) {
			debugger = 1;
		}
	}
	
	if(debugger) {
		debugger_init();
		debugger_loop();
	} else {
		while(cpu_state.running) {
			cpu_step();
#ifdef DISASSEMBLE
			printf("$%04x %s\n", disassembly_pc, disassembly);
#endif
		}
	}

	return 0;
}

#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "lcd.h"
#include "graphics.h"
#include "rom.h"
#include "interrupt.h"
#include "debugger.h"

/**
Functions
*/
void emulator_init() {
	char window_title[256];
	
	// Print startup information
	sprintf(window_title, "%s v%d.%d",
		EMULATOR_TITLE,
		VERSION_MAJOR,
		VERSION_MINOR
	);
	printf("Created by "AUTHOR"\n");
	printf("%s\n", window_title);
	
	// Initialize system
	memory_init();
	interrupt_init();
	graphics_init(window_title);
	lcd_init();
	cpu_init();
}

int main(int argc, char ** argv) {
	int i;
	
#ifdef DISASSEMBLE
	int debugger;
	debugger = 0;
#endif
	
	emulator_init();
	
	// Get commane line arguments
	for(i = 1; i < argc; i++) {
		// Load rom
		if(!strcmp(argv[i], "-f") && i+1 < argc) {
			rom_load(argv[++i]);
		}
		
		// Print help
		if(!strcmp(argv[i], "-h")) {
			printf("usage: %s\n", argv[0]);
			printf("\t-f                  ROM File (.gb)\n");
#ifdef DISASSEMBLE
			printf("\t-debug              Start debugger\n");
#endif
			printf("\t-ignore-bootloader  Skip bootloader\n");
			printf("\t-h                  Display this screen\n");
			return 0;
		}

#ifdef DISASSEMBLE
		// Start debugger
		if(!strcmp(argv[i], "-debug")) {
			debugger = 1;
		}
#endif
		
		// Ignore bootloader
		if(!strcmp(argv[i], "-ignore-bootloader")) {
			cpu_rom_reset();
		}
	}
	
#ifdef DISASSEMBLE
	if(debugger) {
		debugger_init();
		debugger_loop();
	} else {
		while(cpu_state.running) {
			cpu_step();
			printf("$%04x %s\n", disassembly_pc, disassembly);
		}
	}
#else
	while(cpu_state.running) {
		cpu_step();
	}
#endif

	return 0;
}

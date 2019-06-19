#include "debugger.h"
#include "cpu.h"
#include "memory.h"

/**
Static Variables
*/
static struct debugger_settings settings;

/**
Static Functions
*/
static short readNumber(char * line);
static void debugger_continue();

/**
Functions
*/
void debugger_init() {
	settings.number_breakpoints = 0;
	settings.breakpoints[settings.number_breakpoints++] = 0x000;
	settings.breakpoints[settings.number_breakpoints++] = 0x100;
}

void debugger_loop() {
	char c;
	short s;
	
	char * line;
	size_t line_size;
	
	line_size = 32;
	line = malloc(line_size);
	
	for(;;) {
		// Display registers
		printf("A:  $%04x\t", cpu_state.registers.A);
		printf("F:  $%04x\n", cpu_state.registers.F);
		printf("B:  $%04x\t", cpu_state.registers.B);
		printf("C:  $%04x\n", cpu_state.registers.C);
		printf("D:  $%04x\t", cpu_state.registers.D);
		printf("E:  $%04x\n", cpu_state.registers.E);
		printf("H:  $%04x\t", cpu_state.registers.H);
		printf("L:  $%04x\n", cpu_state.registers.L);
		printf("AF: $%04x\n", cpu_state.registers.AF);
		printf("BC: $%04x\n", cpu_state.registers.BC);
		printf("DE: $%04x\n", cpu_state.registers.DE);
		printf("HL: $%04x\n", cpu_state.registers.HL);
		printf("SP: $%04x\t", cpu_state.registers.SP);
		printf("PC: $%04x\n", cpu_state.registers.PC);
		
		// Print prompt
		printf(DEBUG_SHELL_PROMPT);
		
		// Get input
		if(getline(&line, &line_size, stdin) < 0) {
			break;
		}
		
		// Remove whitespace
		while(((c = *line) == ' ' || c == '\n') && c != '\0') line++;
		if(c == '\0') continue;
		
		// Determine command
		switch(c)
		{
		case 'x':
			// eXamine memory
			s = readNumber(++line);
			printf("$%04x\t$%04x\n", s, ((char*)memory_dump())[s]);
			break;
		case 'b':
			// Set a Breakpoint
			s = readNumber(++line);
			settings.breakpoints[settings.number_breakpoints++] = s;
			printf("Set new breakpoint @ $%04x\n", s);
			break;
		case 's':
			// Single step
			cpu_step();
			break;
		case 'r':
			// Run from beginning
			cpu_reset();
			printf("Running...\n");
		case 'c':
			// Continue until next breakpoint/program exit
			cpu_step();
			debugger_continue();
			break;
		case 'q':
			// Exit
			return;
		}
		
#ifdef DISASSEMBLE
		// Print last instruction executed
		printf("$%04x %s\n", disassembly_pc, disassembly);
#endif
	}
}

static short readNumber(char * line) {
	char c;
	
	// Remove leading whitespace
	while((c = *line) == ' ' && c != '\0') line++;
	printf("%s\n", line);
	// Convert to number
	return strtol(line, NULL, 0);
}

static void debugger_continue() {
	int i;
	
	while(cpu_state.running) {
		// Determine if we need to break out (breakpoint hit)
		for(i = 0; i < settings.number_breakpoints; i++) {
			if(cpu_state.registers.PC == settings.breakpoints[i])
				return;
		}
		
		// If no breakpoints, continue execution
		cpu_step();
	}
}

#if !defined(__DEBUGGER_H) && defined(DISASSEMBLE)
#define __DEBUGGER_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define DEBUG_SHELL_PROMPT  "(debug) > "
#define DEBUG_DUMP_FILENAME "mem_dump.dat"

struct debugger_settings {
	unsigned short number_breakpoints;
	unsigned short breakpoints[128];
};

void debugger_init();
void debugger_loop();

#endif

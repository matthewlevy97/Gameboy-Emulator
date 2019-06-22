#ifndef __DEBUGGER_H
#define __DEBUGGER_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define DEBUG_SHELL_PROMPT "(debug) > "

struct debugger_settings {
	short number_breakpoints;
	short breakpoints[128];
};

void debugger_init();
void debugger_loop();

#endif

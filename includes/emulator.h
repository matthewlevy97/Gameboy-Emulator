#ifndef __EMULATOR_H
#define __EMULATOR_H

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

struct cycle_tracker {
	unsigned char cycles_remaining;
	unsigned char (*func)();
};

#endif

#ifndef __MEMORY_H
#define __MEMORY_H

#include <stdlib.h>
#include <string.h>

/**
$FFFF   	Interrupt Enable Flag
$FF80-$FFFE	Zero Page - 127 bytes
$FF00-$FF7F	Hardware I/O Registers
$FEA0-$FEFF	Unusable Memory
$FE00-$FE9F	OAM - Object Attribute Memory
$E000-$FDFF	Echo RAM - Reserved, Do Not Use
$D000-$DFFF	Internal RAM - Bank 1-7 (switchable - CGB only)
$C000-$CFFF	Internal RAM - Bank 0 (fixed)
$A000-$BFFF	Cartridge RAM (If Available)
$9C00-$9FFF	BG Map Data 2
$9800-$9BFF	BG Map Data 1
$8000-$97FF	Character RAM
$4000-$7FFF	Cartridge ROM - Switchable Banks 1-xx
$0150-$3FFF	Cartridge ROM - Bank 0 (fixed)
$0100-$014F	Cartridge Header Area
$0000-$00FF	Restart and Interrupt Vectors
*/
#define INTERNAL_MEMORY_SIZE 0xFFFF

struct memory_region {
	unsigned short base;
	unsigned short bound;
	unsigned char  (*read8)(unsigned short);
	unsigned short (*read16)(unsigned short);
	void  (*write8)(unsigned short, char);
	void  (*write16)(unsigned short, short);
};

void memory_init();
void memory_reset();

void memory_load(void * mem, unsigned short start, unsigned short size);
void * memory_dump();

unsigned char  memory_read8(unsigned short address);
unsigned short memory_read16(unsigned short address);

void memory_write8(unsigned short address, char val);
void memory_write16(unsigned short address, short val);

#endif

#include "memory.h"
#include "cpu.h"
char * memory;

void memory_reset() {
	if(!memory)
		memory = malloc(INTERNAL_MEMORY_SIZE);
	
	memset(memory, 0, INTERNAL_MEMORY_SIZE);
}
void * memory_dump() {
	return memory;
}
void memory_load(void * mem, unsigned short start, unsigned short size) {
	if((int)(start + size) > INTERNAL_MEMORY_SIZE) return;
	
	memcpy((memory + start), mem, size);
}

char  memory_read8(unsigned short address) {
	if(address >= 0xFEA0 && address <= 0xFEFF) { dump_regs(); exit(0); }
#ifdef DEBUG_MEMORY
	printf("[memory_read8] Address: $%04x\n", address);
#endif
	return memory[address];
}
short memory_read16(unsigned short address) {
	if(address >= 0xFEA0 && address <= 0xFEFF) { dump_regs(); exit(0); }
#ifdef DEBUG_MEMORY
	printf("[memory_read16] Address: $%04x\n", address);
#endif
	return *((short*)(memory + address));
}

void memory_write8(unsigned short address, char val) {
	if(address >= 0xFEA0 && address <= 0xFEFF) { dump_regs(); exit(0); }
#ifdef DEBUG_MEMORY
	printf("[memory_write8] Address: $%04x\tValue: $%02x\n", address, val);
#endif

	memory[address] = val;
}
void memory_write16(unsigned short address, short val) {
	if(address >= 0xFEA0 && address <= 0xFEFF) { dump_regs(); exit(0); }
#ifdef DEBUG_MEMORY
	printf("[memory_write16] Address: $%04x\tValue: $%02x\n", address, val);
#endif

	*(short*)(memory+address) = val;
}

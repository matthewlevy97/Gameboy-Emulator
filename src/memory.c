#include "memory.h"
#include "cpu.h"

/**
Static Functions
*/
static struct memory_region * getMemoryRegion(unsigned short address);

static unsigned char  io_port_read8(unsigned short address);
static unsigned short io_port_read16(unsigned short address);
static void io_port_write8(unsigned short address, char val);
static void io_port_write16(unsigned short address, short val);

/**
Static Variables
*/
static unsigned char * memory;

/**
Mapping for all memory regions
Base is inclusive, bound is not
If region not found here, revert to default behavior
*/
#define MEMORY_REGIONS_LEN 1
static struct memory_region memory_regions[] = {
	{
		.base=0xFF00, .bound=0xFF4C,
		.read8=&io_port_read8, .read16=&io_port_read16,
		.write8=&io_port_write8, .write16=&io_port_write16
	}
};

/**
Functions
*/
void memory_init() {
	memory = malloc(INTERNAL_MEMORY_SIZE);
	
	memory_reset();
}
void memory_reset() {
	memset(memory, 0, INTERNAL_MEMORY_SIZE);
}
void memory_load(void * mem, unsigned short start, unsigned short size) {
	if((int)(start + size) > INTERNAL_MEMORY_SIZE) return;
	
	memcpy((memory + start), mem, size);
}
void * memory_dump() {
	return memory;
}

unsigned char memory_read8(unsigned short address) {
	struct memory_region * region;
#ifdef DEBUG_MEMORY
	printf("[memory_read8] Address: $%04x\n", address);
#endif
	
	// Call the read8 for the region if one exists
	if((region = getMemoryRegion(address)))
		return region->read8(address);
	return memory[address];
}
unsigned short memory_read16(unsigned short address) {
	struct memory_region * region;
#ifdef DEBUG_MEMORY
	printf("[memory_read16] Address: $%04x\n", address);
#endif	
	
	// Call the read16 for the region if one exists
	if((region = getMemoryRegion(address)))
		return region->read16(address);
	return *((short*)(memory + address));
}

void memory_write8(unsigned short address, char val) {
	struct memory_region * region;
#ifdef DEBUG_MEMORY
	printf("[memory_write8] Address: $%04x\tValue: $%02x\n", address, val);
#endif
	
	// Call the write8 for the region if one exists
	if((region = getMemoryRegion(address)))
		region->write8(address, val);
	else
		memory[address] = val;
}
void memory_write16(unsigned short address, short val) {
	struct memory_region * region;
#ifdef DEBUG_MEMORY
	printf("[memory_write16] Address: $%04x\tValue: $%02x\n", address, val);
#endif
	
	// Call the write16 for the region if one exists
	if((region = getMemoryRegion(address)))
		region->write16(address, val);
	else
		*(short*)(memory+address) = val;
}

/**
Static Functions
*/
static struct memory_region * getMemoryRegion(unsigned short address) {
	for(int i = MEMORY_REGIONS_LEN; i--;) {
		if(address >= memory_regions[i].base && address < memory_regions[i].bound) {
			return &memory_regions[i];
		}
	}
	return NULL;
}

static unsigned char io_port_read8(unsigned short address) {
	return memory[address];
}
static unsigned short io_port_read16(unsigned short address) {
	return *((short*)(memory + address));
}
static void io_port_write8(unsigned short address, char val) {
	memory[address] = val;
}
static void io_port_write16(unsigned short address, short val) {
	*(short*)(memory+address) = val;
}


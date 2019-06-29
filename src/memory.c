#include "memory.h"
#include "cpu.h"
#include "lcd.h"
#include "ioports.h"

/**
Static Functions
*/
static struct memory_region * getMemoryRegion(unsigned short address);
static char isRegionLocked(unsigned short address, enum memory_op op);

static unsigned char  io_port_read8(unsigned short address);
static unsigned short io_port_read16(unsigned short address);
static void io_port_write8(unsigned short address, char val);
static void io_port_write16(unsigned short address, short val);

/**
Static Variables
*/
static unsigned char * memory;
static const char bootstrap_code[256] = {
	0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
	0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
	0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01,
	0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
	0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22,
	0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
	0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
	0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
	0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20,
	0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
	0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e,
	0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
	0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17,
	0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9,
	0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
	0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
	0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
	0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
	0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11,
	0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
	0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe,
	0x3e, 0x01, 0xe0, 0x50
};

/**
	Mapping for all memory regions
	Base is inclusive, bound is not
	If region not found here, revert to default behavior
*/
#define MEMORY_REGIONS_LEN 1
static struct memory_region
memory_regions[MEMORY_REGIONS_LEN] = {
	// IO PORTS
	{
		.base=0xFF00, .bound=0xFF4C,
		.read8=&io_port_read8, .read16=&io_port_read16,
		.write8=&io_port_write8, .write16=&io_port_write16
	}
};
#define MEMORY_LOCKED_REGIONS_LEN 1
static struct memory_locked_region
memory_locked_regions[MEMORY_LOCKED_REGIONS_LEN] = {
	// Anything not HRAM
	{
		.base=0x0000, .bound=0xFF80, .rwe_lock=0x00
	}
};

/**
Functions
*/
void memory_init() {
	memory = malloc(INTERNAL_MEMORY_SIZE);
	if(!memory)
		printf("[memory_init] Malloc failed\n");
	
	memory_reset();
}
void memory_reset() {
	memset(memory, 0, INTERNAL_MEMORY_SIZE);
	memcpy(memory, bootstrap_code, 256);
}
void * memory_dump() {
	return memory;
}
void memory_lockRegion(enum memory_lock_regions region, enum memory_op op) {
	if(op) op |= MEMORY_ENABLED;
	memory_locked_regions[region].rwe_lock = op;
}

unsigned char memory_read8(unsigned short address) {
	struct memory_region * region;
#ifdef DEBUG_MEMORY
	printf("[memory_read8] Address: $%04x\n", address);
#endif
	if(isRegionLocked(address, MEMORY_READ)) {
		printf("Cannot access memory region $%04x\n", address);
		exit(0);
	}
	
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
	if(isRegionLocked(address, MEMORY_READ)) {
		printf("Cannot access memory region $%04x\n", address);
		exit(0);
	}
	
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
	if(isRegionLocked(address, MEMORY_WRITE)) {
		printf("Cannot access memory region $%04x\n", address);
		exit(0);
	}
	
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
	if(isRegionLocked(address, MEMORY_WRITE)) {
		printf("Cannot access memory region $%04x\n", address);
		exit(0);
	}
	
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
static char isRegionLocked(unsigned short address, enum memory_op op) {
	for(int i = MEMORY_LOCKED_REGIONS_LEN; i--;) {
		if(memory_locked_regions[i].rwe_lock & 0x1 &&
			memory_locked_regions[i].rwe_lock & op &&
			address >= memory_locked_regions[i].base &&
			address < memory_locked_regions[i].bound) {
			return 1;
		}
	}
	return 0;
}

static unsigned char io_port_read8(unsigned short address) {
	return memory[address];
}
static unsigned short io_port_read16(unsigned short address) {
	return *((short*)(memory + address));
}
static void io_port_write8(unsigned short address, char val) {	
	memory[address] = val;
	
	// Do we need to do a DMA Transfer
	if(address == 0xFF46) lcd_dma_transfer(val);
}
static void io_port_write16(unsigned short address, short val) {
	*(short*)(memory+address) = val;
}


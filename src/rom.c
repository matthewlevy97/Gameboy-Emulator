#include "rom.h"
#include "memory.h"

/**
Static Variables
*/
// Used to hold the first 0x100 bytes of the Cartridge
static char   preamble[0x100];
static struct cartridge_header * header;

/**
Functions
*/
void rom_load(char * filename) {
	FILE * fp;
	char * ptr;
	
	fp = fopen(filename, "rb");
	ptr = memory_dump();
	
	// Read into preamble
	fread(preamble, 0x100, 1, fp);
	
	// Read in the header
	fread(ptr + 0x100, 0x4F, 1, fp);
	
	// Read in the first ROM bank
	fread(ptr + 0x14F, /*32kb*/ 1 << 15, 1, fp);
}

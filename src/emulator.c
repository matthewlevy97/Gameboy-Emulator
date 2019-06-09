#include "emulator.h"
#include "cpu.h"
#include "memory.h"

static void loadROM(char * filename) {
	struct stat st;
	void * rom;
	int fd, filesize;
	
	// Initialize system
	cpu_reset();
	memory_reset();
	
	// Open file (ROM)
	fd = open(filename, O_RDONLY, 0);
	if(fd < 0) {
		printf("[FATAL] Unable to open ROM: %s\n", filename);
		return;
	}
	
	// Get filesize
	stat(filename, &st);
	filesize = st.st_size;
		
	// Map to memory region
	rom = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
	if(rom == MAP_FAILED) {
		printf("[FATAL] Unable to mmap file\n");
		return;
	}
	
	// Load ROM into internal memory
	memory_load(rom, 0, 0x4000);
	
	// Close mapping/file
	munmap(rom, filesize);
	close(fd);
	
	for(int i = 0; i < 0xFFFF; i++) cpu_step();
	
	char * ptr;
	ptr = memory_dump();
	fd = open("mem_dump.dat", O_WRONLY, 0);
	write(fd, ptr, INTERNAL_MEMORY_SIZE);
	close(fd);
}

int main(int argc, char ** argv) {
	// Load ROM into memory
	loadROM(argv[1]);
	
	return 0;
}

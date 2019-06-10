#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "lcd.h"

/**
Static Variables
*/
static struct cycle_tracker trackers[] = {
	{.func = &cpu_step},
	{.func = &lcd_update}
};

/**
Static Functions
*/
static void loadROM(char * filename);

/**
Functions
*/
static void loadROM(char * filename) {
	struct stat st;
	void * rom;
	int fd, filesize;
		
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
}

int main(int argc, char ** argv) {
	int i, num_trackers;
	
	// Initialize system
	cpu_init();
	memory_init();
	lcd_init();

	// Load ROM into memory
	loadROM(argv[1]);
	
	num_trackers = sizeof(trackers) / sizeof(struct cycle_tracker);
	while(cpu_running) {
		for(i = num_trackers; i--;) {
			if(!(trackers[i].cycles_remaining--))
				trackers[i].cycles_remaining = trackers[i].func();
		}
	}
	
#ifdef DEBUG_MEMORY
	// Dump RAM
	char * ptr;
	int fd;
	ptr = memory_dump();
	fd = open("mem_dump.dat", O_WRONLY| O_CREAT, 0);
	write(fd, ptr, INTERNAL_MEMORY_SIZE);
	close(fd);
#endif
	return 0;
}

#ifndef __ROM_H
#define __ROM_H

#include <stdio.h>

struct cartridge_header {
	char entry_point[4];       // 0x100-0x103
	char nintendo_logo[48];    // 0x104-0x133
	struct {
		char gbc_title[11];
		char manufacturer_code[4];
		char gbc_flag;
	} title;                   // 0x134-0x143
	char new_licensee_code[2]; // 0x144-0x145
	char sgb_flag;             // 0x146
	char cartridge_type;       // 0x147
	char rom_size;             // 0x148
	char ram_size;             // 0x149
	char is_japanese;          // 0x14A
	char old_licensee_code;    // 0x14B
	char rom_version_number;   // 0x14C
	char header_checksum;      // 0x14D
	char global_checksum[2];   // 0x14E-0x14F
};

void rom_load(char * filename);
void rom_set_preamble();

#endif

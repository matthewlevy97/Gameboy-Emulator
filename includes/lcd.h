#ifndef __LCD_H
#define __LCD_H

#define LCD_TILE_SIZE 16

#define LCD_H_BLANK_CYCLES   4
#define LCD_V_BLANK_CYCLES   4
#define OAM_SCANLINE_CYCLES  4
#define VRAM_SCANLINE_CYCLES 4

/**
LCD Colors:
	00: White
	01: Light Gray
	10: Dark Gray
	11: Black
*/

void lcd_init();

void lcd_update(unsigned char cycles);

#endif

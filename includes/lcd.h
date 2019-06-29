#ifndef __LCD_H
#define __LCD_H

#define LCD_TILE_SIZE 16

/**
LCD Colors:
	00: White
	01: Light Gray
	10: Dark Gray
	11: Black
*/

void lcd_init();
void lcd_dma_transfer(unsigned char val);
void lcd_update(unsigned char cycles);

#endif

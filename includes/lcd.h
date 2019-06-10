#ifndef __LCD_H
#define __LCD_H

struct lcd_registers {
	unsigned char lcdc_control; // 0xFF40
	unsigned char lcdc_status;  // 0xFF41
	
	unsigned char scroll_y;     // 0xFF42
	unsigned char scroll_x;     // 0xFF43
	
	unsigned char lcdc_y;       // 0xFF44
	unsigned char ly_compare;   // 0xFF45
	
	unsigned char dma_transfer; // 0xFF46
	
	//  BG & Window Palette Data
	unsigned char bgp;          // 0xFF47
	
	//  Object Palette X Data 
	unsigned char obp0;         // 0xFF48
	unsigned char obp1;         // 0xFF49
	
	unsigned char window_y;     // 0xFF4A
	unsigned char window_x;     // 0xFF4B
};

void lcd_init();

unsigned char lcd_update();

#endif

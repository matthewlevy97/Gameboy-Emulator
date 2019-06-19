#ifndef __IO_PORTS_H
#define __IO_PORTS_H

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

/**
For easier referencing of entire io ports region
*/
struct io_ports {
	unsigned char padding1[4];    // 0xFF00 -> 0xFF03
	
	unsigned char div_reg;        // 0xFF04
	unsigned char timer_counter;  // 0xFF05
	unsigned char timer_modulo;   // 0xFF06
	unsigned char time_control;   // 0xFF07
	
	unsigned char padding2[7];    // 0xFF08 -> 0xFF0E
	
	unsigned char interrupt_flag; // 0xFF0F
	
	unsigned char padding3[48];   // 0xFF10 -> 0xFF3F
	
	struct lcd_registers lcd_registers; // 0xFF40 -> 0xFF4B
	
	unsigned char padding4[179];  // 0xFF4C -> 0xFFFE
	
	unsigned char interrupt_enable; // 0xFFFF
};

#endif

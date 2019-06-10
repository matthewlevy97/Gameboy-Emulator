#include "lcd.h"
#include "memory.h"

static struct lcd_registers * lcd_registers;

void lcd_init() {
	// Map the lcd_registers structure to the associated memory region
	lcd_registers = memory_dump() + 0xFF40;
	
	lcd_registers->lcdc_y = 0x90;
}

unsigned char lcd_update() {
#ifdef DEBUG_LCD
	printf("[lcd_update] lcdc_y: $%02x\n", lcd_registers->lcdc_y);
#endif
	return 4;
}

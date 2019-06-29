#include "lcd.h"
#include "cpu.h"
#include "memory.h"
#include "ioports.h"
#include "graphics.h"
#include "interrupt.h"

/**
Static Functions
*/
static void drawScanline();

/**
Static Variables
*/
static struct lcd_registers * lcd_registers;
static char * vram;

void lcd_init() {
	vram = memory_dump();
	
	// Map the lcd_registers structure to the associated memory region
	lcd_registers = (void*)vram + 0xFF40;
	
	// Correct the offset to VRAM
	vram += 0x8000;
}

void lcd_dma_transfer(unsigned char val) {
	char * mem;
	
	// Lock everything except HRAM
	memory_lockRegion(LRAM_LOCK, MEMORY_READ | MEMORY_WRITE);
	
	// Do transfer
	mem = memory_dump();
	memcpy(mem + 0xFE00, mem + (0x100 * val), 0xA0);
	
	// Takes 160 (0xA0) cycles to complete
	
	// Unlock LRAM
	memory_lockRegion(LRAM_LOCK, 0);
}

void lcd_update(unsigned char cycles) {
	// Determine if LCD Display is enabled
	if((lcd_registers->lcdc_control >> 7) ^ 0x1) {
		graphics_screen_off();
		return;
	}
	
	// Determine if enough cycles passed
	cpu_state.lcd_wait_cycles -= cycles;
	if(cpu_state.lcd_wait_cycles > 0) return;
	
	// Handle GPU status
	switch(lcd_registers->lcdc_status & 0x3) {
		case 0:
			// H_BLANK
			// TODO: Signal interrupt
			cpu_state.lcd_wait_cycles += 204;
			
			// Move to the next line
			lcd_registers->lcdc_y++;
			
			// Goto V_BLANK or OAM Scanline
			if(lcd_registers->lcdc_y >= 143)
				lcd_registers->lcdc_status |= 0x1; // V_BLANK
			else
				lcd_registers->lcdc_status |= 0x2; // OAM
			
			break;
		case 1:
			// V_BLANK (10 lines)
			// Signal interrupt
			interrupt_trigger(V_BLANK_INTERRUPT);
			
			cpu_state.lcd_wait_cycles += 456; // Cycles per line
			
			// Move to the next line
			lcd_registers->lcdc_y++;
		
			if(lcd_registers->lcdc_y >= 153) {
				// Set mode from 1 -> 2
				lcd_registers->lcdc_status ^= 0x3;
				
				// Set line to 0
				lcd_registers->lcdc_y = 0;
			}
			break;
		case 2:
			// Scanline (OAM)
			cpu_state.lcd_wait_cycles += 80;
			
			// Set mode from 2 -> 3
			lcd_registers->lcdc_status |= 0x1;
			break;
		case 3:
			// Scanline (VRAM)
			// Render scanline now
			drawScanline();
			
			cpu_state.lcd_wait_cycles += 172;
			
			// Set mode from 3 -> 0
			lcd_registers->lcdc_status ^= 0x3;
			break;
	}
	
	// Check for coincidence and set flag
	if(lcd_registers->ly_compare == lcd_registers->lcdc_y)
		lcd_registers->lcdc_status |= 4; // Set bit
	else
		lcd_registers->lcdc_status &= ~4; // Clear bit
}

static void drawScanline() {
	unsigned short vram_offset;
	unsigned char tile_data_region, bg_display_region;
	unsigned char tile_row, tile_col, pixel;
	char tileID, *tile, tile_line;
	char bits, i;
	
	// Get the row of the tile too look at (The '/8' is needed (integer division))
	tile_row = ((lcd_registers->lcdc_y + lcd_registers->scroll_y) / 8);
	
	/**
		Bit 4 == 1 << 4 == 16
		
		1 - 0x8000->0x8FFF (unsigned)
		0 - 0x8800->0x97FF (signed)
	*/
	tile_data_region = (lcd_registers->lcdc_control & 16) ? 1 : 0;
	
	/**
		Bit 3 == 1 << 3 == 8
		
		1 - 0x9C00->0x9FFF
		0 - 0x9800->0x9BFF
	*/
	bg_display_region = (lcd_registers->lcdc_control & 8) ? 1 : 0;
	
#ifdef DEBUG_LCD
	printf("Tile Region: %d\n", tile_data_region);
	printf("Background Region: %d\n", bg_display_region);
	printf("Scroll (X, Y): (%d, %d)\n", lcd_registers->scroll_x, lcd_registers->scroll_y);
	printf("LCDC_Y: %d\n", lcd_registers->lcdc_y);
	printf("Tile Row: %d\n", tile_row);
#endif
	/**
		Screen Width == 160
		Tile Width   == 8
		Since we draw 8 pixels at a time, only loop LCD_SCREEN_WIDTH / 8 times
	*/
	for(pixel = 0; pixel < LCD_SCREEN_WIDTH; pixel++) {
		tile_col = (lcd_registers->scroll_x + pixel) / 8;
		
		if(lcd_registers->lcdc_control & 0x1) {
			// BG Display Enabled
			
			// Get the tileID from BG Map Data
			tileID = *(vram + (bg_display_region ? 0x1C00 : 0x1800) + tile_row * 32 + tile_col);
			
			// Get the offset into vram
			if(tile_data_region) {
				vram_offset = ((unsigned)tileID) * LCD_TILE_SIZE;
			} else {
				vram_offset = 0x800;
				vram_offset += (tileID + 128) * LCD_TILE_SIZE;
			}
			
			// Get the tile (16 bytes long)
			tile = vram + vram_offset;
			
			// Get the current line in the tile
			tile_line = ((lcd_registers->lcdc_y + lcd_registers->scroll_y) & 7) * 2;
			tile += tile_line;
			
			// Probably a better way to do this, but it works for now
			i = (7 - pixel) & 0x7;
			bits = (tile[0] >> i) & 0x1;
			bits |= ((tile[1] >> i) & 0x1) << 1;
			
			// Determine color from palete
			// Gameboy Original
			switch((lcd_registers->bgp >> ((bits * 2))) & 0x3)
			{
				case 0x0:
					graphics_setColor(255, 255, 255, 255);
					break;
				case 0x1:
					graphics_setColor(170, 170, 170, 255);
					break;
				case 0x2:
					graphics_setColor(85, 85, 85, 255);
					break;
				case 0x3:
					graphics_setColor(0, 0, 0, 255);
					break;
			}
			
			graphics_drawPixel(pixel, lcd_registers->lcdc_y);
		}
		if(lcd_registers->lcdc_control & 0x2) {
			// OBJ (Sprite) Display Enabled
		}
	}
	
	graphics_render();
}

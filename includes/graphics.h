#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <SDL2/SDL.h>

#define LCD_SCREEN_HEIGHT 144
#define LCD_SCREEN_WIDTH  160

#define GRAPHICS_SCALE 4

void graphics_init();
void graphics_destroy();

void graphics_screen_off();

void graphics_setColor(int r, int g, int b, int a);
void graphics_drawPixel(int x, int y);
void graphics_clearScreen();
void graphics_render();

void graphics_update();

#endif

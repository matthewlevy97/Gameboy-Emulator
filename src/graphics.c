#include "graphics.h"
#include "cpu.h"

static SDL_Renderer * renderer;
static SDL_Window * window;

void graphics_init() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(
		LCD_SCREEN_WIDTH * GRAPHICS_SCALE,
		LCD_SCREEN_HEIGHT * GRAPHICS_SCALE,
		SDL_WINDOW_SHOWN,
		&window, &renderer
	);
	SDL_SetWindowTitle(window, "Gameboy Emulator v0.1");
	
	graphics_setColor(0, 0, 0, 255);
	graphics_clearScreen();
	
	graphics_setColor(255, 255, 255, 255);
}

void graphics_destroy() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void graphics_setColor(int r, int g, int b, int a) {
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
void graphics_drawPixel(int x, int y) {
	if(x < 0 || x > LCD_SCREEN_WIDTH || y < 0 || y > LCD_SCREEN_HEIGHT) {
		printf("OOB\n");
		return;
	}

	x *= GRAPHICS_SCALE;
	y *= GRAPHICS_SCALE;
		
	for(int i = 0; i < GRAPHICS_SCALE; i++) {
		for(int j = 0; j < GRAPHICS_SCALE; j++) {
			SDL_RenderDrawPoint(renderer, x + i, y + j);
		}
	}
}
void graphics_clearScreen() {
	SDL_RenderClear(renderer);
}
void graphics_render() {
	SDL_RenderPresent(renderer);
}

void graphics_update() {
	SDL_Event event;
	
	SDL_PollEvent(&event);
	switch(event.type)
	{
		case SDL_QUIT:
			graphics_destroy();
			cpu_state.running = 0;
			break;
	}
}

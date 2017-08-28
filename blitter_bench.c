
//
// Lorenzo Loconte - loreloc
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "blitter.h"

#define BENCH_LENGTH 2000000


int main(int argc, char* argv[])
{
	// crea il framebuffer in RGB565
	uint16_t* framebuffer = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint16_t));
	// crea lo sprite in BGRA8888
	uint8_t* sprite = calloc(SPRITE_SIZE * SPRITE_SIZE * 4, sizeof(uint8_t));

	// colora i buffer

	for(uint32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
		framebuffer[i] = 0x1234;
	
	for(uint32_t i = 0; i < SPRITE_SIZE * SPRITE_SIZE; ++i)
	{
		sprite[i * 4 + 0] = 0x83; // alpha
		sprite[i * 4 + 1] = 0xB5; // rosso
		sprite[i * 4 + 2] = 0x73; // verde
		sprite[i * 4 + 3] = 0x18; // blu
	}

	clock_t t1 = clock();

	// esegui il blit
	for(uint32_t i = 0; i < BENCH_LENGTH; ++i)
	{
		blitSprite(framebuffer, sprite, 320, 240);
	}

	clock_t t2 = clock();

	printf("Elapsed time: %f seconds\n", (float)(t2 - t1) / CLOCKS_PER_SEC);

	free(sprite);
	free(framebuffer);

	return 0;
}



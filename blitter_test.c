
//
// Lorenzo Loconte - loreloc
//

#include <stdio.h>
#include <stdlib.h>

#include "blitter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "deps/stb_image_write.h"

int main(int argc, char* argv[])
{
	// crea il framebuffer in RGB565
	uint16_t* framebuffer565 = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint16_t));
	// crea lo sprite in BGRA8888
	uint8_t* sprite = calloc(SPRITE_SIZE * SPRITE_SIZE * 4, sizeof(uint8_t));

	// colora i buffer

	for(uint32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
		framebuffer565[i] = 0x1234;
	
	for(uint32_t i = 0; i < SPRITE_SIZE * SPRITE_SIZE; ++i)
	{
		sprite[i * 4 + 0] = 0x83; // alpha
		sprite[i * 4 + 1] = 0xB5; // rosso
		sprite[i * 4 + 2] = 0x73; // verde
		sprite[i * 4 + 3] = 0x18; // blu
	}

	// disegna degli sprite
	blitSprite(framebuffer565, sprite,  320, 250);
	blitSprite(framebuffer565, sprite,  350, 230);
	blitSprite(framebuffer565, sprite,  335, 270);
	blitSprite(framebuffer565, sprite,  -50, 260);
	blitSprite(framebuffer565, sprite, 1017, 260);
	blitSprite(framebuffer565, sprite,  280, -11);
	blitSprite(framebuffer565, sprite,  600, 750);

	// converti il framebuffer nel formato RGB888

	uint8_t* framebuffer888 = calloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3, sizeof(uint8_t));

	for(uint32_t y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for(uint32_t x = 0; x < SCREEN_WIDTH; ++x)
		{
			uint32_t f5b = (framebuffer565[x + y * SCREEN_WIDTH]      ) & 0x1F;
			uint32_t f6g = (framebuffer565[x + y * SCREEN_WIDTH] >>  5) & 0x3F;
			uint32_t f5r = (framebuffer565[x + y * SCREEN_WIDTH] >> 11);

			framebuffer888[(x + y * SCREEN_WIDTH) * 3 + 0] = f5r << 3;
			framebuffer888[(x + y * SCREEN_WIDTH) * 3 + 1] = f6g << 2;
			framebuffer888[(x + y * SCREEN_WIDTH) * 3 + 2] = f5b << 3;
		}
	}

	// stampa in una bitmap il risultato
	if(!stbi_write_bmp("result.bmp", SCREEN_WIDTH, SCREEN_HEIGHT, 3, framebuffer888))
	{
		printf("Failed to print the result\n");

		free(framebuffer888);
		free(sprite);
		free(framebuffer565);

		return 1;
	}

	free(framebuffer888);
	free(sprite);
	free(framebuffer565);

	return 0;
}


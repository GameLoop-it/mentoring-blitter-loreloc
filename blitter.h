
//
// Lorenzo Loconte - loreloc
//

#pragma once

#include <stdint.h>

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT  768
#define SPRITE_SIZE     64

void blitSprite(uint16_t* framebuffer, uint8_t* sprite, int32_t x0, int32_t y0);


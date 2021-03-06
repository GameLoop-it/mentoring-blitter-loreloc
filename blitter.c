
//
// Lorenzo Loconte - loreloc
//

// formato RGB565
//
// 15      11             5           0
//  R R R R R | G G G G G G | B B B B B

// formato BGRA8888
//
// 31            24                16                 8                 0
//  B B B B B B B B | G G G G G G G G | R R R R R R R R | A A A A A A A A

#include "blitter.h"

// uint16_t* framebuffer  puntatore al framebuffer
// uint8_t* sprite        puntatore allo sprite
// int32_t x0             ascissa dell'angolo in alto a sinistra dello sprite
// int32_t y0             ordinata dell'angolo in alto a sinistra dello sprite
void blitSprite(uint16_t* framebuffer, uint8_t* sprite, int32_t x0, int32_t y0)
{
	// giusto per sicurezza :D
	if(!framebuffer || !sprite)
		return;

	const int32_t x1 = x0 + SPRITE_SIZE; // ascissa del vertice in alto a destra dello sprite
	const int32_t y1 = y0 + SPRITE_SIZE; // ordinata del vertice in basso a sinistra dello sprite

	// verifica se lo sprite si trova al di fuori dello schermo
	if(x1 <= 0 || y1 <= 0)
		return;
	if(x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT)
		return;

	// coordinate del vertice in alto a sinistra della regione dello sprite da disegnare
	int32_t sx0 = 0;
	int32_t sy0 = 0;

	// ordinata del vertice in basso a sinistra della regione dello sprite da disegnare
	int32_t sh = SPRITE_SIZE;

	// ascissa del vertice in alto a destra della regione dello sprite da disegnare
	int32_t sw = SPRITE_SIZE;

	// Se alcune parti dello sprite si trovano al di fuori dello schermo
	// ricalcola la regione dello sprite da disegnare

	if(x0 < 0)
	{
		sx0 = -x0;
		x0 = 0;
	}
	else if(x1 >= SCREEN_WIDTH)
	{
		sw = SCREEN_WIDTH - x0;
	}

	if(y0 < 0)
	{
		sy0 = -y0;
		y0 = 0;
	}
	else if(y1 >= SCREEN_HEIGHT)
	{
		sh = SCREEN_HEIGHT - y0;
	}

	for(int32_t y = y0, sy = sy0; sy < sh; ++sy, ++y)
	{
		for(int32_t x = x0, sx = sx0; sx < sw; ++sx, ++x)
		{
			// leggi i canali del pixel del framebuffer
			uint32_t f5b = (framebuffer[x + y * SCREEN_WIDTH]      ) & 0x1F;
			uint32_t f6g = (framebuffer[x + y * SCREEN_WIDTH] >>  5) & 0x3F;
			uint32_t f5r = (framebuffer[x + y * SCREEN_WIDTH] >> 11);

			// leggi i canali del pixel dello sprite
			uint8_t s8a = sprite[(sx + sy * SPRITE_SIZE) * 4 + 0];
			uint8_t s8r = sprite[(sx + sy * SPRITE_SIZE) * 4 + 1];
			uint8_t s8g = sprite[(sx + sy * SPRITE_SIZE) * 4 + 2];
			uint8_t s8b = sprite[(sx + sy * SPRITE_SIZE) * 4 + 3];

			// converti i canali del pixel dello sprite in RGB565
			uint32_t s5r = s8r >> 3; // scarta 3 bit
			uint32_t s6g = s8g >> 2; // scarta 2 bit
			uint32_t s5b = s8b >> 3; // scarta 3 bit

			// Inizialmente per eseguire l'alpha blending calcolavo l'alpha e l' 1-alpha
			// compresi nell'intervallo [0.0; 1.0] ed eseguivo il blending
			//
			// float alpha = alpha255 / 255.0f;
			// result = (source * alpha + destination * (1.0f - alpha))

			// Ho notato che c'erano due conversioni da intero a virgola mobile, una conversione da virgola mobile ad intero
			// e due moltiplicazioni a virgola mobile per ogni componente del pixel
			//
			// Ciò avrebbe influito decisamente sulle performance
			// Allora ho deciso di usare solo gli interi evitando moltiplicazioni in virgola mobile
			// Quindi non normalizzo il valore dell'alpha nell'intervallo [0.0; 1.0]
			//
			// In teoria per fare ciò avrei dovuto dividere tutto per 255, ma ho optato per uno shift a destra di 8 posti
			// equivalente ad una divisione per 256
			// Ho praticamente sacrificato un pò di precisione per velocizzare il blending
			//
			// result = (source * alpha + destination * (0xFF - alpha)) >> 8;

			// Con qualche semplificazione sono arrivato a questa formula
			// Di "impegnativo" richiede solo una moltiplicazione tra interi per ogni componente del pixel
			//
			// result = ((alpha * (source - destination)) >> 8) + destination;

			uint16_t p5r = ((s8a * (s5r - f5r)) >> 8) + f5r;
			uint16_t p6g = ((s8a * (s6g - f6g)) >> 8) + f6g;
			uint16_t p5b = ((s8a * (s5b - f5b)) >> 8) + f5b;

			// assembla il nuovo colore in formato RGB565
			uint16_t p565 = (p5r << 11) | (p6g << 5) | p5b;

			// salva il nuovo colore nel framebuffer
			framebuffer[x + y * SCREEN_WIDTH] = p565;
		}
	}
}


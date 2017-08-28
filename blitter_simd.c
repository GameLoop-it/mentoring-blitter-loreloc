
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

#include <emmintrin.h>

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

	// Ascissa del vertice in alto a destra della regione dello sprite da disegnare
	// che viene arrotondata per difetto al multiplo di 4 più vicino
	int32_t sw0 = SPRITE_SIZE;
	
	// ascissa del vertice in alto a destra della regione dello sprite da disegnare
	int32_t sw1 = SPRITE_SIZE;

	// Se alcune parti dello sprite si trovano al di fuori dello schermo
	// ricalcola la regione dello sprite da disegnare

	if(x0 < 0)
	{
		sx0 = -x0;
		x0 = 0;

		sw0 = sx0 + ((SPRITE_SIZE - sx0) & ~0x3);
	}
	else if(x1 >= SCREEN_WIDTH)
	{
		sw1 = SCREEN_WIDTH - x0;
		sw0 = sw1 & ~0x3;
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

	// mask per i primi 5, 6 e 8 bits
	const __m128i mask5 = _mm_set1_epi32(0x0000001F);
	const __m128i mask6 = _mm_set1_epi32(0x0000003F);
	const __m128i mask8 = _mm_set1_epi32(0x000000FF);

	for(int32_t y = y0, sy = sy0; sy < sh; ++sy, ++y)
	{
		// coordinata X a schermo
		int32_t x = x0;
		// coordinata X sullo sprite
		int32_t sx = sx0;

		// Esegui il blit 4 pixels alla volta usando le simd
		// per tutta la lunghezza multipla di 4 della regione dello sprite da disegnare
		//
		// Si supponga, ad esempio, che l'ultima colonna di pixels dello sprite si trovi al di fuori dello schermo
		// Quindi bisogna "blittare", ad ogni riga, 63 pixels invece di 64
		//
		// Viene eseguito il blit 4 pixels alla volta utilizzando le istruzioni simd per i primi 60 pixels
		// Dopodichè viene eseguito il blit degli ultimi 3 pixels uno alla volta
		//
		// sx0             sw0         sw1
		//  0  1  2  3 ... 60 61 62 || 63
		// __ __ __ __ ... __ __ __ || __

		for(; sx < sw0; sx += 4, x += 4)
		{
			// leggi 4 pixels dal framebuffer e spostali nei 4 doubleword
			__m128i fpixels = _mm_loadl_epi64((__m128i*)(&framebuffer[x + y * SCREEN_WIDTH]));
			fpixels = _mm_unpacklo_epi16(fpixels, fpixels);

			// calcola i canali dei 4 pixels del framebuffer
			__m128i f5b = _mm_and_si128(fpixels, mask5);
			__m128i f6g = _mm_and_si128(_mm_srli_epi32(fpixels,  5), mask6);
			__m128i f5r = _mm_and_si128(_mm_srli_epi32(fpixels, 11), mask5);

			// leggi 4 pixels dallo sprite
			__m128i spixels = _mm_loadu_si128((__m128i*)(&sprite[(sx + sy * SPRITE_SIZE) * 4]));

			// calcola l'alpha
			__m128i s8a = _mm_and_si128(spixels, mask8);

			// converti i canali dei 4 pixels dello sprite in RGB565
			__m128i s5r = _mm_and_si128(_mm_srli_epi32(spixels, 11), mask5);
			__m128i s6g = _mm_and_si128(_mm_srli_epi32(spixels, 18), mask6);
			__m128i s5b = _mm_srli_epi32(spixels, 27);

			// effettua il blending
			__m128i mr = _mm_mullo_epi16(_mm_sub_epi32(s5r, f5r), s8a);
			__m128i mg = _mm_mullo_epi16(_mm_sub_epi32(s6g, f6g), s8a);
			__m128i mb = _mm_mullo_epi16(_mm_sub_epi32(s5b, f5b), s8a);
			__m128i p5r = _mm_add_epi32(_mm_srai_epi16(mr, 8), f5r);
			__m128i p6g = _mm_add_epi32(_mm_srai_epi16(mg, 8), f6g);
			__m128i p5b = _mm_add_epi32(_mm_srai_epi16(mb, 8), f5b);

			// assembla i nuovi colori in formato RGB565
			__m128i p565 = _mm_or_si128(_mm_or_si128(_mm_slli_epi32(p5r, 11), _mm_slli_epi32(p6g, 5)), p5b);

			// sposta i 4 colori nel quadword basso
			p565 = _mm_shufflelo_epi16(p565, 0x88);
			p565 = _mm_shufflehi_epi16(p565, 0x88);
			p565 = _mm_shuffle_epi32(p565, 0x88);

			// salva i nuovi colori nel framebuffer
			_mm_storel_epi64((__m128i*)(&framebuffer[x + y * SCREEN_WIDTH]), p565);
		}

		// esegui il blit dei restanti pixels

		for(; sx < sw1; ++sx, ++x)
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


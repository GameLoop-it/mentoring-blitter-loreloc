
CC = gcc

CFALGS = -O2 -Wall

.PHONY: clean blitter_test blitter_bench

ifeq ($(SIMD), TRUE)
    BLITTER_SRC = blitter_simd.c
else
    BLITTER_SRC = blitter.c
endif

blitter_test: $(BLITTER_SRC) blitter_test.c
	$(CC) $(CFALGS) $(BLITTER_SRC) blitter_test.c -o blitter_test

blitter_bench: $(BLITTER_SRC) blitter_bench.c
	$(CC) $(CFALGS) $(BLITTER_SRC) blitter_bench.c -o blitter_bench

blitter.s: blitter.c
	$(CC) $(CFALGS) -S -masm=intel blitter.c
blitter_simd.s: blitter_simd.c
	$(CC) $(CFALGS) -S -masm=intel blitter_simd.c

clean:
	rm -f *.s *.exe *.bmp



CC = gcc

CFALGS = -O2 -Wall

.PHONY: clean

blitter_test: blitter.c blitter_test.c
	$(CC) $(CFALGS) blitter.c blitter_test.c -o blitter_test

blitter.s: blitter.c
	$(CC) $(CFALGS) -S -masm=intel blitter.c

clean:
	rm -f *.s *.exe *.bmp


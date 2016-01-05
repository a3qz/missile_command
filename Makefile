
final: final.c 
	gcc -o final final.c gfx4.h gfx4.o -lm -lX11


clean:
	rm final


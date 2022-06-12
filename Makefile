chip8: chip8.c
	gcc chip8.c -o chip8

btw: ./trash/btw.c
	gcc ./trash/btw.c -o ./trash/btw

clean: 
	rm chip8
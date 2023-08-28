chip8: chip8.c
	cc chip8.c -o chip8 $(shell pkg-config --cflags raylib) $(shell pkg-config --libs raylib)

btw: ./trash/btw.c
	gcc ./trash/btw.c -o ./trash/btw

clean: 
	# gcc chip8.c -o chip8
	rm chip8

#cc chip8.c -o chip8 $(shell pkg-config --libs --cflags raylib) "-I/opt/homebrew/Cellar/raylib/4.5.0/include/" 
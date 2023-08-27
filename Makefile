chip8: chip8.c
	eval cc chip8.c -o chip8 $(pkg-config --libs --cflags raylib) "-I/opt/homebrew/Cellar/raylib/4.5.0/include/" 

test: test.c
	eval cc test.c -o test $(pkg-config --libs --cflags raylib) 

btw: ./trash/btw.c
	gcc ./trash/btw.c -o ./trash/btw

clean: 
	# gcc chip8.c -o chip8
	rm chip8
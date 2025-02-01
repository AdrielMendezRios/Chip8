![](https://github.com/AdrielMendezRios/Chip8/blob/main/gifs/trip8.gif)   ![](https://github.com/AdrielMendezRios/Chip8/blob/main/gifs/Snake.gif)
![](https://github.com/AdrielMendezRios/Chip8/blob/main/gifs/flightrunner.gif)  ![](https://github.com/AdrielMendezRios/Chip8/blob/main/gifs/astros.gif)

# Chip8
A _very_ **simple** Chip8 emulator in C \[Work in progress\]

Writen all in a single C file and including only two raylib modules (core, and raymath).
This was my first endevour into writing an emulator and wanted to explore the simplicity and 'low-level' aspect
of the C language. 
its feature complete (implements all _necessary_ instructions, takes user input and renders to its own window)
some more features to add in the future will be:
- a debugger. to squash the hard-to-find game-breaking bugs
- in window game selection menu (right now you select your game through the command line)
- theming/ color picker

## how to run (on MacOS)
1. clone the directory
2. cd into chip8 directory
3. make a `programs` folder in the chip8 directory \[must be name `programs`\]
4. add Your ROM to the `programs` folder
5. with homebrew `brew install raylib`
6. run: `make chip8`
7. run: `make run`
8. select the menu number for your ROM of choice
9. select cpu speed.
10. play!

some ROMs that run great on this chip8 emulator
- https://johnearnest.github.io/chip8Archive/play.html?p=flightrunner
- https://johnearnest.github.io/chip8Archive/play.html?p=RPS
- https://johnearnest.github.io/chip8Archive/play.html?p=snake
- https://github.com/kripod/chip8-roms/blob/master/games/Airplane.ch8
- https://github.com/kripod/chip8-roms/blob/master/games/Astro%20Dodge%20%5BRevival%20Studios%2C%202008%5D.ch8
- https://github.com/kripod/chip8-roms/blob/master/games/Brick%20(Brix%20hack%2C%201990).ch8

Demos:
- https://github.com/loktar00/chip8/blob/master/roms/Chip8%20Picture.ch8
- https://github.com/loktar00/chip8/blob/master/roms/IBM%20Logo.ch8
- https://github.com/loktar00/chip8/blob/master/roms/Trip8%20Demo%20(2008)%20%5BRevival%20Studios%5D.ch8
- https://github.com/loktar00/chip8/blob/master/roms/Particle%20Demo%20%5BzeroZshadow%2C%202008%5D.ch8
- https://github.com/loktar00/chip8/blob/master/roms/Zero%20Demo%20%5BzeroZshadow%2C%202007%5D.ch8

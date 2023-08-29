# Chip8
A _very_ **simple** Chip8 emulator in C

Writen all in a single C file and including only two raylib modules (core, and raymath).
This was my first endevour into writing an emulator and wanted to explore the simplicity and 'low-level'-ness
of the C language. 
its feature complete (implements all _necessary_ instructions, takes user input and renders to its own window)
some more features to add in the future will be:
- a debugger. 
- in window game selection menu (right now you select your game through the command line)
- theming/ color picker

## how to run
1. clone the directory
2. cd into chip8 directory
3. run: make chip8
4. ./chip8
5. optionally, add you're own ROMs to the 'programs' folder then pick them from the menu after rerunning ./chip8 command

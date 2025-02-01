#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdlib.h>

#define MEMORY_SIZE 4096
#define MAX_ROM_SIZE (4096 - 200)
#define NUM_REGS 16
#define MAX_SUBROUTINES 16
#define WIDTH 64
#define HEIGHT 32

// Move struct definition and other declarations here
typedef struct Chip8 {
    uint16_t    pc, I, opcode, sp;
    uint16_t    stack[MAX_SUBROUTINES];
    uint8_t     v[NUM_REGS];
    uint8_t     delayTimer, soundTimer;
    uint8_t     memory[MEMORY_SIZE];
    int         screen[HEIGHT][WIDTH];
    size_t      rom_size;
    char        hrop[50];
    uint16_t    keypad;
} Chip8;

// Declare font data as external
extern const unsigned char fonts[];

// Function declarations
Chip8* chip8_init(void);
void clear_screen(Chip8 *chip);
void interpreter(Chip8 *chip);
void load_rom(Chip8 *chip, char* path);

#endif 
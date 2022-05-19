#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>



#define MEMORY_SIZE 4096
#define MAX_ROM_SIZE (4096 - 200)
#define NUM_REGS 16
#define MAX_SUBROUTINES 16
#define WIDTH 64
#define HEIGHT 32


typedef struct Chip8{
    unsigned short  pc, I, opcode, sp;
    unsigned short  stack[MAX_SUBROUTINES];
    unsigned char   v[NUM_REGS];
    unsigned char   delayTimer, soundTimer;
    unsigned char   memory[MEMORY_SIZE];
    int             screen[HEIGHT][WIDTH];
    size_t          rom_size;
    char            hrop[50]; // human readable opcode
} Chip8;

void getop(Chip8 *chip){
    chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc+1];
}

int flippix(Chip8 *chip, int x, int y){
    if (y < 0 || y >= HEIGHT) return 0;
    if (x < 0 || x >= WIDTH) return 0;
    chip->screen[y][x] ^= 1;
    return !chip->screen[y][x];
}

void init_mem(Chip8 *chip){
    for(int i = 0; i < MEMORY_SIZE; i++){
        chip->memory[i] = 0x0;
    }
}

void print_rom_in_memory(Chip8 *chip){
    printf("Rom in memory. size: %zu bytes\n", chip->rom_size);
    for(int i = 0x200; i < (0x200 + chip->rom_size); i++){
        printf("\tmemory[%d]: %x\n", i, chip->memory[i]);
    }
}

void print_memory(Chip8 *chip){
    printf("Chip8 memory space: \n");
    for(int i = 0x200; i <= (0x200 + chip->rom_size); i++){
        printf("\tmemory[%d]: %x\n", i, chip->memory[i]);
    }
}

void clear_screen(Chip8 *chip){
    memset(chip->screen, 0, sizeof(chip->screen));
}

void print_screen_debug(Chip8 *chip){
    printf("Screen: \n");
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(chip->screen[i][j] !=0)
                printf("%x", chip->screen[i][j]);
            else
                printf(" ");
        }
        printf("\n");
    }
}

void print_registers(Chip8 *chip){
    printf("Registers: \n");
    for(int i = 0; i < 16; i++){
        printf("\tv[%d]: %d\n", i, chip->v[i]);
    }
}

void print_filled_registers(Chip8 *chip){
    printf("Registers: \n");
    for(int i = 0; i < 16; i++){
        if(chip->v[i] != 0)
            printf("\tv[%d]: %02x\n", i, chip->v[i]);
    }
}

void interpreter(Chip8 *chip){
    getop(chip);
    uint16_t opcode = chip->opcode;
    printf("interpreter: 0x%x\n", opcode);
    uint8_t x = (opcode & 0x0F00)>> 8; // in AxyB get x
    uint8_t y = (opcode & 0x00F0) >> 4; // in AxyB get y

    switch (opcode & 0xF000){
        case 0x0000:
            /* fall in if opcode is an instruction starting with 0x0___*/
            switch(opcode){
                case 0x00E0:{ // CLR (clear screen)
                    strcpy(chip->hrop, "CLR\0");
                    clear_screen(chip);
                    chip->pc += 2;
                    break;
                }
                case 0x00EE: {// RET return from subroutine
                    unsigned short ret_addr = chip->stack[chip->sp];
                    chip->sp = chip->sp <=0 ? 0 : chip->sp--;
                    break;
                }
            }
            break;
        case 0x1000:{ // 1nnn: JP to addr nnn
            strcpy(chip->hrop, "JP to Addr nnn\0");
            chip->pc = (opcode & 0x0FFF);
            break;
        }
        case 0x2000:{ // 2nnn:  CALL subroutine at addr nnn
            // TODO:
            break;
        }
        case 0x3000:{ // 3xkk: SE Vx, byte. if v[x] == kk (immediate byte) skip next instruction (pc + 2)
            //TODO:
            break;
        }
        case 0x4000:{ // 0x4xkk: SNE Vx, byte. if v[x] != kk (immediate byte) skip next instruction (pc + 2)
            //TODO:
            break;
        }
        case 0x5000:{ // 0x5xy0: SE Vx, Vy. if v[x] == v[y] skip next instruction (pc + 2)
            //TODO:
            break;
        }
        case 0x6000:{ // 0x6xkk: LD Vx, byte. set v[x] = kk
            strcpy(chip->hrop, "LC Vx, byte\0");
            chip->v[x] = (opcode & 0x00FF);
            chip->pc += 2;
            break;
        }
        case 0x7000:{ // 0x7xkk: ADD Vx, byte. set v[x] += kk
            // char temp[50] = "ADD Vx, byte";
            // strcpy(chip->hrop, temp);
            chip->v[x] += (opcode & 0x00FF);
            chip->pc += 2;
            break;
        }
        case 0x8000:{
            /* fall in if its an instruction starting with 0x8___*/
            switch(opcode & 0x000F){ // find which version of 0x8__i the opcode has
                case 0x0:{ // LD Vx, Vy. store the value in v[y] in v[x]
                    //TODO
                    break;
                }
                case 0x1:{ // OR Vx, Vy. bitwise OR on the values of Vx and Vy, then store in Vx.
                    //TODO
                    break;
                }
                case 0x2:{ // AND Vx, Vy. bitwise AND on the values of Vx and Vy, then store in Vx
                    //TODO
                    break;
                }
                case 0x3:{ // XOR Vx, Vy. bitwise Exclusive OR on the values of Vx and Vy. then store in Vx.
                    //TODO
                    break;
                }
                case 0x4:{ //ADD Vx, Vy
                    /* 
                        ADD Vx, Vy. Add values in both registers and if the result if greater than 8 bits 
                        VF is set to 1, otherwise 0. the lowest 8bits are stored in Vx.
                    */
                    //TODO
                    break;
                }
                case 0x5:{ // SUB Vx, Vy
                    /*
                        SUB Vx - Vy. if Vx > Vy then Vf is set to 1, otherwise 0. then Vy subtracted from Vx
                        result stored in Vx.
                    */
                    //TODO
                    break;
                }
                case 0x6:{ // SHR Vx {, Vy}
                    /*
                        set Vx = Vx SHR 1.
                        if least-sig bit of Vx is 1, then VF is set to 1, otherwise 0. then Vx is divided by 2
                    */
                    //TODO
                    break;
                }
                case 0x7:{// SUBN Vx, Vy, set VF = NOT borrow.
                    /*
                        set Vx = Vy - Vx. if Vy > Vx, then Vf is set to 1, otherwise 0. then Vx is subtracted from Vy,
                        result stored in Vx.
                    */
                    //TODO
                    break;
                }
                case 0xE:{// SHL Vx {, Vy}
                    /*
                        set Vx = Vx SHL 1. if most-sig bit of Vx is 1, then VF is set to 1, otherwise 0.
                        then Vx is multiplied by 2.
                    */
                    //TODO
                    break;
                }
            }
            break;
        }
        case 0x9000:{ // SNE Vx, Vy. skip next instr. if  Vx != Vy. if true increase pc + 2
            // TODO:
            break;
        }
        case 0xA000:{ // LD I, Addr. set the Value of register I to nnn
            strcpy(chip->hrop, "LD I, Addr\0");
            chip->I  = (opcode & 0x0FFF);
            chip->pc += 2;
            break;
        }
        case 0xB000:{ // JP V0, addr. pc is set to nnn plus the value in V0.
            //TODO:
            break;
        }
        case 0xC000:{// RND Vx, byte. set Vx = random byte AND kk. 
            /*
                interpreter generates a random number from 0 to 255, which is ANDed with kk, the rrsult is stored in Vx.
            */
            //TODO:
            break;
        }
        case 0xD000:
			{
				/* DRW, Vx, Vy, nibble */
                strcpy(chip->hrop,"DRW Vx, Vy, nibble\0");
				unsigned char width = 8;
				unsigned char height = opcode & 0xF;
				int row, col;
				chip->v[0xF] = 0;
				for (row = 0; row < height; row++) {
					uint8_t sprite = chip->memory[chip->I + row];
					for (col = 0; col < width; col++) {
						if ((sprite & 0x80) > 0) {
							if (flippix(chip, chip->v[x] + col, chip->v[y] + row))  {
								chip->v[0xF] = 1;
								/* if register 0xF is set to 1, collision happened */
							}
						}
						sprite <<= 1;
					}
				}
                chip->pc += 2;
			}
			break;
        // case 0xD000: { // DRW Vx, Vy, nibble (DxyN)
        //     /*
        //         display n-byte sprite starting at memory location I at (vx, vy), set VF = Collision.

        //         The interpreter reads n bytes from memory, starting at the address stored in I. 
        //         these bytes are then displayed as sprites on the screeen at coordinates (Vx, Vy). 
        //         sprites are XORed onto the existing screen. if this causes any pixeles
        //         to be erased, VF is set to 1, otherwise it is set to 0. if the sprite is positioned so 
        //         part of it is outside the coordinates of the display it wraps around to the opposite side of the screen.
        //     */
        //     //TODO:
        //     chip->v[0xf] = 0;
        //     uint8_t s_width = 8;                    // sprite width
        //     uint8_t s_height = (opcode & 0x000F);   // sprite height
        //     int row, col; // x & y coordinate of screen
        //     for (row = 0; row < s_height; row++){
        //         uint8_t sprite_row = chip->memory[chip->I + row];
        //         for(col = 0; col < s_width; col++){
        //             if((sprite_row & 0x80) > 0){
        //                 if(flippix(chip, chip->v[x]+row, chip->v[y] + col)){
        //                     chip->v[0xf] = 1;
        //                 }
        //             }
        //             sprite_row <<= 1;
        //         }
        //     }
        // }
        // break;
        case 0xE09E: { // SKP Vx. skip next instr. if key with value stored in Vx is pressed.
            /*
                Cheaks the keyboard, if the key corresponding to the values of Vx is currently
                in the down position, pc is increased by 2.
            */
            // TODO:
            break;
        }
        case 0xE0A1:{ // SKPN Vx. skip instr. if key with values in Vx not pressed
            /*
                checks keyboard, and if the key corresponding to the value of Vx is currently 
                in the up position. pc is increased by 2.
            */
            // TODO:
            break;
        }
        case 0xF000:{
            /* fall in if opcode is an instruction starting with 0xF___*/
            switch(opcode & 0x00FF){  // find which version of 0xF__i the opcode has
                case 0x0007:{ // LD Vx, DT. set Vx = Delay Timer value.
                    //TODO:
                    break;
                }
                case 0x000A: {// LD Vx, K
                    /*
                        wait for a key press, store the value of the key into Vx.
                        All execution stops until a key is pressedm then the value of that key is stored in Vx.
                    */
                    // TODO:
                    break;
                }
                case 0x0015:{ // LD DT, Vx. set Delay timer = Vx.
                    //TODO:
                    break;
                }
                case 0x0018:{ // LD ST, Vx. set Sound Timer = Vx.
                    // TODO:
                    break;
                }
                case 0x001E: { // ADD I, Vx, set I += Vx.
                    //TODO:
                    break;
                }
                case 0x0029: { // LD F, Vx. set I = location of sprite for digit Vx.
                    /*
                        the value of U is set to the location for the hexadecimal sprite corresponding to the value
                        of Vx. 
                    */
                    //TODO:
                    break;
                }
                case 0x0033:{ // LD B, Vx. store BCD representation of Vx in memory location I, I+1, and I+2.
                    /*
                        the interpreter takes the decimal value of Vx, and places the hundreds digit in memory location at I,
                        the tens digit at memory location I+1, and the ones digit in location I+2.
                    */
                    //TODO:
                    break;
                }
                case 0x0055:{ // LD [I], Vx. store register V0 through Vx in memory starting at location I.
                    /*
                        the interpreser copies the vaues of resgisters V0 -> Vx into memory, starting at the address in I.
                    */
                    // TODO:
                    break;
                }
                case 0x0065:{ // LD Vx, [I]
                    /*
                        Read Registers V0 -> Vx from memory starting at location I.
                        the inerpreter reads values from memory starting at locaiton I into registers V0 -> Vx.
                    */
                    // TODO:
                    break;
                }
            }
        }

    }
}

Chip8* chip8_init(){
    Chip8 *chip = (Chip8*)malloc(sizeof(Chip8));
    chip->pc = 0x200;
    // chip->I = 0x200;
    // chip->opcode = 0x0;
    chip->sp = 0;
    // init_mem(chip);
    // clear_screen(chip);

    return chip;
}

void printState(Chip8 *chip){
    print_screen_debug(chip);
    printf("opcode: 0x%x --> %s\n", chip->opcode, chip->hrop);
    printf("PC:     0x%x\n", chip->pc);
    printf("I:      0x%x\n", chip->I);
    printf("sp:     %x\n", chip->sp);
    printf("instr at pc: 0x%x%02x\n", chip->memory[chip->pc], chip->memory[chip->pc+1] );
    print_filled_registers(chip);
    // print_registers(chip);
    // printf("memory[%d]: %x\n", chip->pc+1, chip->memory[chip->pc+1] );
    // print_rom_in_memory(chip);
    
}

/*
    TODO:
        start with a subset of instructions. these should be implemented first:
        [x] clear screen:           00E0
        [x] jump:                   1NNN
        [x] set register vX:        6XNN
        [x] add val to reg vX:      7XNN
        [x] set index register (I): ANNN
        [x] display/draw:           DXYN
        [x] also needed is the *load_rom* function     
*/

int load_rom(Chip8 *chip, char* path){
    FILE *rom = NULL;
    size_t rom_size;
    size_t read_size;
    uint8_t *rom_buff;

    rom = fopen(path, "rb");
    if(rom == NULL){
        fprintf(stderr, "%s","Error: could not open file in <path>" );
        exit(1);
    }

    fseek(rom, 0, SEEK_END);
    rom_size = ftell(rom);
    rewind(rom);

    rom_buff = (uint8_t *)malloc(sizeof(rom_size));
    if(rom_buff == NULL){
        fprintf(stderr, "%s", "Error: allocating memory for rom");
        exit(1);
    }

    read_size = fread(rom_buff, sizeof(uint8_t), (size_t) rom_size, rom);
    if(read_size != rom_size){
        fprintf(stderr, "%s", "Error: reading rom into rom buffer");
        exit(1);
    }

    if (rom_size > MAX_ROM_SIZE){
        fprintf(stderr, "%s", "Error: rom size larger than available space");
    }
    else{
        memcpy(chip->memory+0x200, rom_buff, rom_size);
        chip->rom_size = rom_size;
    }

    fclose(rom);
    free(rom_buff);

    return 1;
}



int main(){
    Chip8 *chip = chip8_init();
    char *p = "IBM.ch8";
    load_rom(chip, p);
    while(1){
        interpreter(chip);
        sleep(1);
        printState(chip);
        // print_screen_debug(chip);
        printf("\n");
    }
    return 0;
}
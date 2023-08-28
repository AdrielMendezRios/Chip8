/*
    written by Adriel Mendez
    [Work in progress]
    A very simple, Chip8 emulator written in C using raylib. it mostly works, 
    but there are still some bugs. most are rendering related but need a debugger
    further investigate.

    My goal with this project was to practice and learn more about the C language,
    how emulators work, and expose myself to the raylib library, as such i made it
    as simple as possible and wrote everything in a single self-contained file. 
    I still have a  lot to learn about all of these topics, but excited about 
    my progress.
    
    TODO:
    - The next steps is to split up the `interpreter` function into smaller functions
    of related instructions.
    - improve performance rendering the frame to the screen. 
    - make a chip8.h and clean up the chip8.c file
    

    implemented with the help of some amazing resources online.
    mainly:
        http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#Fx15 (cowgod's chip8 technical reference)
        https://tobiasvl.github.io/blog/write-a-chip-8-emulator/ (tobiasvl's blog post on writing a chip8 emulator)
        https://www.raylib.com/cheatsheet/cheatsheet.html (raylib's examples and Cheatsheet)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

// raylib modules
#include <raylib.h>
#include <raymath.h>


#define MEMORY_SIZE 4096
#define MAX_ROM_SIZE (4096 - 200)
#define NUM_REGS 16
#define MAX_SUBROUTINES 16
#define WIDTH 64
#define HEIGHT 32

// global variables
unsigned char frameBuffer[32][64];
RenderTexture2D frame_target;

// chip8 emulator Struct
typedef struct Chip8{
    uint16_t    pc, I, opcode, sp;
    uint16_t    stack[MAX_SUBROUTINES];
    uint8_t     v[NUM_REGS];
    uint8_t     delayTimer, soundTimer;
    uint8_t     memory[MEMORY_SIZE];
    int         screen[HEIGHT][WIDTH];
    size_t      rom_size;
    char        hrop[50]; // human readable opcode
} Chip8;

static const uint8_t fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// mapping from keyboard keys to keypad keys
int keypadMapping(int kb_key){
    switch (kb_key){
        case(KEY_ONE):  return 0x1;
        case(KEY_TWO):  return 0x2;
        case(KEY_THREE):return 0x3;
        case(KEY_FOUR): return 0xC;
        case(KEY_Q):    return 0x4;
        case(KEY_W):    return 0x5;
        case(KEY_E):    return 0x6;
        case(KEY_R):    return 0xD;
        case(KEY_A):    return 0x7;
        case(KEY_S):    return 0x8;
        case(KEY_D):    return 0x9;
        case(KEY_F):    return 0xE;
        case(KEY_Z):    return 0xA;
        case(KEY_X):    return 0x0;
        case(KEY_C):    return 0xB;
        case(KEY_V):    return 0xF;
        break;
    }
    return -1;
}

// mapping from keypad keys to keyboard keys
int keyboardMapping(int keypad_key){
    switch (keypad_key){
        case 0x1: return KEY_ONE;
        case 0x2: return KEY_TWO;
        case 0x3: return KEY_THREE;
        case 0xC: return KEY_FOUR;
        case 0x4: return KEY_Q;
        case 0x5: return KEY_W;
        case 0x6: return KEY_E;
        case 0xD: return KEY_R;
        case 0x7: return KEY_A;
        case 0x8: return KEY_S;
        case 0x9: return KEY_D;
        case 0xE: return KEY_F;
        case 0xA: return KEY_Z;
        case 0x0: return KEY_X;
        case 0xB: return KEY_C;
        case 0xF: return KEY_V;
        break;
    }
    return -1;
}

void loadfonts(Chip8 *chip) {
    size_t n = sizeof(fonts)/sizeof(*fonts);
    size_t i;
    for (i = 0; i < n; i++){
        chip->memory[i] = fonts[i];
    }
}

// stores current opcode into chip->opcode
void getop(Chip8 *chip){
    chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc+1];
    // printf("Generated opcode: %x", )
}

// not being used. kept as notes
int flippix(Chip8 *chip, int x, int y){
    if (y < 0 || y >= HEIGHT) return 0;
    if (x < 0 || x >= WIDTH) return 0;
    chip->screen[y][x] ^= 1;
    return !chip->screen[y][x];
}

// initializes chip8 emulators memory space
void init_mem(Chip8 *chip){
    for(int i = 0; i < MEMORY_SIZE; i++){
        chip->memory[i] = 0x0;
    }
}

// clears the emulators logical screen
void clear_screen(Chip8 *chip){
    memset(chip->screen, 0, sizeof(chip->screen));
}

// update_timer only updates the timers 60 times per every second
void update_timers(Chip8 *chip){
    static clock_t prev = 0;
    clock_t now = clock();
    if (now - prev > CLOCKS_PER_SEC/60){
        if (chip->delayTimer > 0) chip->delayTimer--;
        if (chip->soundTimer > 0) chip->soundTimer--;
        prev = now;
    }
}

// very basic sound function. [need better sound implementation]
void play_game_sound(unsigned char timer, Sound sound){
    if (timer > 0 ){
        PlaySound(sound);
    }else if (timer <=0){
        StopSound(sound);
    }

}

// renders the frame to the frame_target raylib texture
void render_frame() {
    // Clear the rendering target
    BeginTextureMode(frame_target);
    // ClearBackground(BLACK);

    // Draw the pixels onto the rendering target
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (frameBuffer[y][x]) {
                DrawPixel(x, y, WHITE);
            }
            else{
                DrawPixel(x, y, BLACK);
            }
        }
    }

    EndTextureMode();
}

// draws the frame_target texture to the screen
void draw_frame(){
    BeginDrawing();
        // ClearBackground(BLACK);
        DrawTexturePro(frame_target.texture, (Rectangle){0, 0, (float)frame_target.texture.width, (float)-frame_target.texture.height}, (Rectangle){0, 0, 640, 320}, (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
}


/****************************************************************                                

                        helper functions
                        for debugging                               

*****************************************************************/
void print_rom_in_memory(Chip8 *chip){
    printf("Rom in memory. size: %zu bytes\n", chip->rom_size);
    for(int i = chip->pc-8; i < chip->pc+12; i+=2){
        printf("%s", chip->pc == i ? "------>\t" : "\t");
        printf("memory[%x]: %02x%02x\n", i, chip->memory[i],chip->memory[i+1]);
    }
}

void print_rom_memory(Chip8 *chip){
    printf("Chip8 memory space: \n");
    for(int i = 0x200; i <= (0x200 + chip->rom_size); i++){
        printf("\tmemory[%d]: %x\n", i, chip->memory[i]);
    }
}

void print_emulator_memory_space(Chip8 *chip){
    printf("Chip8 emulator memory space: \n");
    for(int i = 0x0; i <= 0x200 ; i++){
        printf("\tmemory[%d]: %x\n", i, chip->memory[i]);
    }
}

void print_screen_debug(Chip8 *chip){
    printf("Screen: \n");
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(chip->screen[i][j] !=0)
                printf("%s", chip->screen[i][j] ? "*" : "");
            else
                printf(" ");
        }
        printf("\n");
    }
}

void print_registers(Chip8 *chip){
    printf("Registers: \n");
    for(int i = 0; i < 16; i++){
        printf("\tv[%x]: %d\n", i, chip->v[i]);
    }
}

void print_filled_registers(Chip8 *chip){
    printf("Registers: \n");
    for(int i = 0; i < 16; i++){
        if(chip->v[i] != 0)
            printf("\tv[%x]: %02x\n", i, chip->v[i]);
    }
}

void print_stack(Chip8 *chip){
    printf("Stack: \n");
    for(int i = 0; i < chip->sp; i++){
        printf("\t%04x\n", chip->stack[chip->sp]);
    }
    printf("\t%04x\n", chip->stack[chip->v[0xF]]);
    
}

void printState(Chip8 *chip){
    print_screen_debug(chip);
    // printf("opcode: 0x%04x --> %s\n", chip->opcode, chip->hrop);
    // printf("PC:     0x%x\n", chip->pc);
    // printf("I:      0x%x\n", chip->I);
    // printf("sp:     %x\n", chip->sp);
    // printf("Vf:     0x%04x\n", chip->v[0xF]);
    // printf("instr at pc: 0x%02x%02x\n", chip->memory[chip->pc], chip->memory[chip->pc+1] );
    // print_filled_registers(chip);
    // print_stack(chip);
    // print_rom_in_memory(chip);
    
}

/****************************************************************                                

                    end of helper functions                               

*****************************************************************/

// interprets the current chip8 opcode and executes the instruction
void interpreter(Chip8 *chip){
    
    getop(chip);
    uint16_t opcode = chip->opcode;
    // printf("interpreting: 0x%04x\n", opcode);
    uint8_t x = (opcode & 0x0F00)>> 8; // in AxyB get x
    uint8_t y = (opcode & 0x00F0) >> 4; // in AxyB get y
    uint8_t Vx = chip->v[x];
    uint8_t Vy = chip->v[y];
    uint8_t I = chip->I;



    switch (opcode & 0xF000){
        case 0x0000:
            /* fall in if opcode is an instruction starting with 0x0___*/
            switch(opcode){
                // Clear Screen
                case 0x00E0:{ // CLR (clear screen)
                    // strcpy(chip->hrop, "CLR\0");
                    clear_screen(chip);
                    chip->pc += 2;
                    break;
                }
                // Return from Subrouting
                case 0x00EE: {// RET 
                    // strcpy(chip->hrop, "RET. return from subroutine\0");
                    chip->pc = chip->stack[chip->sp--];
                    chip->pc +=2;
                    break;
                }
            }
            break;
        // JP to addr nnn
        case 0x1000:{ // 1nnn
            // strcpy(chip->hrop, "JP to Addr nnn\0");
            // sets pc to nnn

            chip->pc = (opcode & 0x0FFF);
            break;
        }
        // CALL subroutine at addr nnn
        case 0x2000:{ // 2nnn
            // strcpy(chip->hrop, "CALL subroutine at nnn\0");
            chip->stack[++chip->sp] = chip->pc;
            // Call subroutine at nnn
            chip->pc = (opcode & 0x0FFF);
            break;
        }
        // Skip if Equal [immediate]
        case 0x3000:{ // 3xkk: SE Vx, byte. if v[x] == kk (immediate byte) skip next instruction (pc + 2)
            
            // strcpy(chip->hrop, "3xkk: Skip if Vx == kk\0");
            if (Vx == (opcode & 0x00FF)){
                chip->pc += 4;
            }else {
                chip->pc += 2;
            }

            break;
        }
        // Skip if Not Equal [immediate]
        case 0x4000:{ // 0x4xkk: SNE Vx, byte. if v[x] != kk (immediate byte) skip next instruction (pc + 2)
            // strcpy(chip->hrop, "Skip if Vx != kk\0");
            if (Vx != (opcode & 0x00FF)) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;
        }
        // Skip if Equal [register]
        case 0x5000:{ // 0x5xy0: SE Vx, Vy. if v[x] == v[y] skip next instruction (pc + 2)
            // strcpy(chip->hrop, "Skip if Vx == Vy\0");
            if (Vx == Vy){
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;
        }
        // Load [immediate] into Vx register
        case 0x6000:{ // 0x6xkk: LD Vx, byte. set v[x] = kk
            // strcpy(chip->hrop, "LD byte into Vx\0");
            chip->v[x] = (opcode & 0x00FF);
            chip->pc += 2;
            break;
        }
        // Add [immediate] to Vx register
        case 0x7000:{ // 0x7xkk: ADD Vx, byte. set v[x] += kk
            // strcpy(chip->hrop, "ADD Vx, byte\0");
            chip->v[x] += (opcode & 0x00FF);
            chip->pc += 2;
            break;
        }
        //0x8.... == Logical and Arithmetic instructions
        case 0x8000:{
            /* fall in if its an instruction starting with 0x8___*/
            switch(opcode & 0x000F){ // find which version of 0x8__i the opcode has
                // Load(store) value in Vy register into Vx register
                case 0x0:{ //0x8xy0 LD Vx, Vy
                    // strcpy(chip->hrop, "LD Vx, Vy. store the value in v[y] in v[x]\0");
                    chip->v[x] = Vy;
                    chip->pc += 2;
                    break;
                }
                // Bitwise OR on the values of Vx and Vy, then store in Vx
                case 0x1:{ //0x8xy1. OR Vx, Vy
                    // strcpy(chip->hrop, "OR Vx, Vy. bitwise OR on the values of Vx and Vy, then store in Vx.\0");
                    chip->v[x] = Vx | Vy;
                    chip->pc += 2;
                    break;
                }
                // Bitwise AND on the values of Vx and Vy, then store in Vx
                case 0x2:{ //0x8xy2. AND Vx, Vy
                    // strcpy(chip->hrop, "AND Vx, Vy. bitwise AND on the values of Vx and Vy, then store in Vx\0");
                    chip->v[x] = Vx & Vy; 
                    chip->pc += 2;
                    break;
                }
                // Bitwise XOR on the values of Vx and Vy, then store in Vx
                case 0x3:{ //0x8xy3. XOR Vx, Vy
                    // strcpy(chip->hrop, "XOR Vx, Vy. bitwise Exclusive OR on the values of Vx and Vy. then store in Vx.\0");
                    chip->v[x] = Vx ^ Vy;
                    chip->pc += 2;
                    break;
                }
                /* 
                    ADD Vx, Vy. Add values in both registers and if the result if greater than 8 bits 
                    VF is set to 1, otherwise 0. the lowest 8bits are stored in Vx.
                */
                case 0x4:{ //0x8xy4. ADD Vx, Vy
                    // strcpy(chip->hrop, "ADD Vx, Vy\0");
                    uint16_t sum = Vx + Vy;

                    // set Vf to 1 if sum > 0xFF (255) or 0 if sum < 0xFF
                    chip->v[0xF] = sum > 0xFF;

                    // store the lower 8 bits of sum into Vx
                    chip->v[x] = sum & 0xFF;

                    chip->pc += 2;
                    break;
                }
                /*
                    SUB Vx - Vy. if Vx > Vy then Vf is set to 1, otherwise 0. then Vy subtracted from Vx
                    result stored in Vx.
                */
                case 0x5:{ //0x8xy5. SUB Vx, Vy
                    // strcpy(chip->hrop, "SUB Vx, Vy, set Vf = NOT borrow\0");
                    // // set Vf to 1 if Vx > Vy, otherwise set Vf to 0
                    chip->v[0xF] = Vx >= Vy;
                    
                    // subtract Vy from Vx, store difference in Vx
                    chip->v[x] = Vx - Vy;
                    
                    chip->pc += 2;

                    break;
                }
                /*
                    set Vx = Vx SHR 1. (shift right by 1)
                    if least-sig bit of Vx is 1, then VF is set to 1, otherwise 0. then Vx is divided by 2
                */
                case 0x6:{ //0x8xy6. SHR Vx {, Vy}
                    // strcpy(chip->hrop, "SHR Vx {, Vy}. Vx = Vx SHR 1\0");
                    chip->v[0xF] = Vx & 0x1;
                    chip->v[x] >>= 1;
                    // chip->v[x] /= 2;
                    chip->pc += 2;
                    break;
                }
                /*
                    set Vx = Vy - Vx. if Vy > Vx, then Vf is set to 1, otherwise 0. then Vx is subtracted from Vy,
                    result stored in Vx.
                */
                case 0x7:{// SUBN Vx, Vy, set VF = NOT borrow.
                    // strcpy(chip->hrop, "SUBN Vx, Vy, set Vf = NOT borrow\0");

                    // set Vf to 1 if Vy > Vx, otherwise set to 0
                    chip->v[0xF] = Vy >= Vx;
                    
                    // set Vx = Vy - Vx
                    chip->v[x] = Vy - Vx;

                    chip->pc +=2;
                    break;
                }
                /*
                    set Vx = Vx SHL 1. if most-sig bit of Vx is 1, then VF is set to 1, otherwise 0.
                    then Vx is multiplied by 2.
                */
                case 0xE:{// SHL Vx {, Vy}
                    // strcpy(chip->hrop, "SHL Vx {, Vy}. Vx = Vx SHL 1\0");
                    // set Vf to the left-most digit. (mask all but the left most bit, then shift it to right-most bit) 
                    chip->v[0xF] = (Vx & 0x80) >> 7;

                    chip->v[x] <<= 1; // shit left by 1 (aka multiply by 2)

                    chip->pc += 2;
                    break;
                }
            }
            break;
        } // end of 0x8000 instructions
        // Skip Next instruction if Vx NOT EQUAL to Vy, if EQUAL, increase pc + 2
        case 0x9000:{ // SNE Vx, Vy
            if (Vx != Vy){
                chip->pc += 4;
                break;
            }
            chip->pc += 2;
            break;
        }
        // Load Addr nnn to Register I
        case 0xA000:{ // LD I, Addr
            // strcpy(chip->hrop, "LD I, Addr\0");
            chip->I  = opcode & 0x0FFF;
            chip->pc += 2;
            break;
        }
        // Jump. set pc to nnn + value in V0
        case 0xB000:{ // JP V0, addr
            chip->pc = (opcode & 0x0FFF) + chip->v[0];
            break;
        }
        // (Bitwise AND) random Byte AND kk, then store in Vx
        case 0xC000:{// RND Vx, byte 
            /*
                interpreter generates a random number from 0 to 255, which is ANDed with kk, the result is stored in Vx.
            */
            srand(time(NULL));
            uint8_t r = rand() % 255; // random number from 0 - 255
            
            // r & kk
            chip->v[x] = r & (opcode & 0x00FF);
            
            chip->pc += 2;
            break;
        }
        // Draw Pixel to screen
        case 0xD000: {
            /* DRW, Vx, Vy, nibble (DxyN)*/
            /*
                display n-byte sprite starting at memory location I at (vx, vy), set VF = Collision.

                The interpreter reads n bytes from memory, starting at the address stored in I. 
                these bytes are then displayed as sprites on the screeen at coordinates (Vx, Vy). 
                sprites are XORed onto the existing screen. if this causes any pixeles
                to be erased, VF is set to 1, otherwise it is set to 0. if the sprite is positioned so 
                part of it is outside the coordinates of the display it wraps around to the opposite side of the screen.
            */
            // strcpy(chip->hrop,"DRW Vx, Vy, nibble\0");
            // Create a temporary buffer for the frame
            // unsigned char frameBuffer[32][64];

            // Copy the current screen state to the frame buffer
            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    frameBuffer[y][x] = chip->screen[y][x];
                }
            }

            unsigned char height = opcode & 0xF;
            chip->v[0xF] = 0;

            for (int row = 0; row < height; row++) {
                uint8_t sprite = chip->memory[chip->I + row];
                int spriteY = Vy + row;

                uint8_t bitmask = 0x80; // Start with the leftmost bit

                for (int col = 0; col < 8; col++) {
                    int spriteX = Vx + col;

                    if (sprite & bitmask) {
                        int screenX = spriteX; 
                        int screenY = spriteY; 

                        if (screenX >= 0 && screenX < 64 && screenY >= 0 && screenY < 32) {
                            chip->v[0xF] |= chip->screen[spriteY][spriteX]; // Set VF (collision flag)
                            chip->screen[spriteY][spriteX] ^= 1; // Toggle the pixel value

                            // Draw the pixel directly on the frame buffer
                            frameBuffer[spriteY][spriteX] = chip->screen[spriteY][spriteX] ? 1 : 0;
                        }
                    }

                    bitmask >>= 1; // Shift the bitmask to the right
                }
            }

            // // Clear the rendering target
            // BeginTextureMode(frame_target);
            //     // ClearBackground(BLACK);

            //     // Draw the pixels onto the rendering target
            //     for (int y = 0; y < 32; y++) {
            //         for (int x = 0; x < 64; x++) {
            //             if (frameBuffer[y][x]) {
            //                 DrawPixel(x, y, WHITE);
            //             }
            //             else{
            //                 DrawPixel(x, y, BLACK);
            //             }
            //         }
            //     }

            // EndTextureMode();
            // // Draw the texture on the Raylib window with scaling
            // BeginDrawing();

            //     // ClearBackground(BLACK);
            //     DrawTexturePro(frame_target.texture, (Rectangle){0, 0, (float)frame_target.texture.width, (float)-frame_target.texture.height}, (Rectangle){0, 0, 640, 320}, (Vector2){0, 0}, 0.0f, WHITE);

            // EndDrawing();

            chip->pc += 2;

        break;
        }
        // KEY PRESS INSTRUCTIONS
        case 0xE000:{
            /* fall in if opcode is an instruction starting with 0xE0__*/
            switch (opcode & 0x00FF){
                // skip instruction if key with value in Vx pressed
                case 0x009E:{ // SKP Vx
                    /*
                        Checks the keyboard, and if the key corresponding to the value of Vx is currently
                        in the down position, pc is increased by 2.
                    */
                    int target_key = Vx;
                    if (IsKeyDown(keyboardMapping(target_key))){ 
                        chip->pc += 2;
                    }

                    chip->pc += 2;
                    break;
                }
                // skip instruction if key with value in Vx not pressed
                case 0x00A1:{ // SKPN Vx
                    /*
                        checks keyboard, and if the key corresponding to the value of Vx is currently 
                        in the up position. pc is increased by 2.
                    */
                    int target_key = Vx;
                    if (IsKeyUp(keyboardMapping(target_key))){ // keypad_keys_pressed[target_key]
                        chip->pc += 2;
                    }
                    chip->pc += 2;
                    break;
                }
            }
            break;
        }
        case 0xF000:{
            /* fall in if opcode is an instruction starting with 0xF0__*/
            // find which version of 0xF__i the opcode has
            switch(opcode & 0x00FF){  
                // set Vx = Delay Timer value.
                case 0x0007:{ // LD Vx, DT 
                    // strcpy(chip->hrop,"LD Vx, DT (DelayTimer)\0");
                    chip->v[x] = chip->delayTimer;
                    chip->pc += 2;
                    break;
                }
                /*
                    wait for a key press, store the value of the key into Vx.
                    All execution stops until a key is pressed, then the value of that key is stored in Vx.
                */
                case 0x000A: {// LD Vx, K
                    // strcpy(chip->hrop,"LD Vx, Key\0");
                    int key = GetKeyPressed();
                    if (key != 0){
                        int keypad_key = keypadMapping(key);
                        if (keypad_key != -1){
                            chip->v[x] = keypad_key;
                            chip->pc += 2;
                        }
                    }
                    break;
                }
                // set Delay timer = Vx
                case 0x0015:{ // LD DT, Vx
                    // strcpy(chip->hrop,"LD DT, Vx. set Delay timer = Vx.\0");
                    chip->delayTimer = Vx;
                    chip->pc += 2;
                    break;
                }
                // set Sound Timer = Vx
                case 0x0018:{ // LD ST, Vx
                    // strcpy(chip->hrop,"LD ST, Vx. set Sound Timer = Vx.\0");
                    chip->soundTimer = Vx;
                    chip->pc += 2;
                    break;
                }
                // set I = I + Vx
                case 0x001E: { // ADD I, Vx
                    // strcpy(chip->hrop,"ADD I, Vx, set I += Vx.\0");
                    chip->I += Vx;
                    chip->pc += 2;
                    break;
                }
                // set I = location of Font sprite for digit Vx.
                case 0x0029: { // LD F, Vx. 
                    /*
                        the value of I is set to the location for the hexadecimal sprite corresponding to the value
                        of Vx. 
                    */
                    // strcpy(chip->hrop,"LD F, Vx. load hex font into I\0");
                    chip->I = Vx * 5;
                    chip->pc += 2;
                    break;
                }
                // store BCD representation of Vx in memory location I, I+1, and I+2.
                case 0x0033:{ // LD B, Vx
                    /*
                        the interpreter takes the decimal value of Vx, and places the hundreds digit in memory location at I,
                        the tens digit at memory location I+1, and the ones digit in location I+2.
                    */
                    // strcpy(chip->hrop,"LD B, Vx. store BCD representation of Vx in memory location I, I+1, and I+2.\0");
                    chip->memory[chip->I] = ( Vx/ 100) % 10;
					chip->memory[chip->I+1] = (Vx / 10) % 10;
					chip->memory[chip->I+2] = (Vx % 10);
                    // chip->memory[chip->I] = (chip->v[x] / 100) % 10;
					// chip->memory[chip->I+1] = (chip->v[x] / 10) % 10;
					// chip->memory[chip->I+2] = (chip->v[x] % 10);
                    chip->pc += 2;
                    break;
                }
                // store register V0 through Vx in memory starting at location I.
                case 0x0055:{ // LD [I], Vx 
                    /*
                        the interpreser copies the vaues of resgisters V0 -> Vx into memory, starting at the address in I.
                    */
                    // strcpy(chip->hrop,"LD [I], Vx. store register V0 through Vx into memory starting at I\0");
                    uint8_t i;
                    for(i = 0x0; i <= x; i++){
                        chip->memory[chip->I+i] = chip->v[i];
                    }
                    chip->pc += 2;
                    break;
                }
                // Read Registers V0 -> Vx from memory starting at location I.
                case 0x0065:{ // LD Vx, [I]
                    /*
                        Read Registers V0 -> Vx from memory starting at location I.
                        the inerpreter reads values from memory starting at locaiton I into registers V0 -> Vx.
                    */
                    // strcpy(chip->hrop,"LD Vx, [I] load register V0 -> Vx from memory starting at I\0");
                    uint8_t i;
                    for(i = 0; i <= x; i++){
                        chip->v[i] = chip->memory[chip->I + i];
                    }
                    chip->pc += 2;
                    break;
                }
            }
            break;
        }
    }
}

// initialize chip8 emulator
Chip8* chip8_init(){
    Chip8 *chip = (Chip8*)malloc(sizeof(Chip8));
    loadfonts(chip);
    chip->pc = 0x200;
    init_mem(chip);
    return chip;
}

// loads the fonts into chip8 memory
void load_rom(Chip8 *chip, char* path){
    FILE *rom = NULL;
    size_t rom_size;
    size_t read_size;
    uint8_t *rom_buff;

    rom = fopen(path, "rb");
    if(rom == NULL){
        fprintf(stderr, "%s","Error: could not open file in <path>\n" );
        exit(1);
    }

    fseek(rom, 0, SEEK_END);
    rom_size = ftell(rom);
    rewind(rom);

    rom_buff = (uint8_t *)malloc(rom_size);
    if(rom_buff == NULL){
        fprintf(stderr, "%s", "Error: allocating memory for rom\n");
        exit(1);
    }

    read_size = fread(rom_buff, sizeof(uint8_t), (size_t) rom_size, rom);
    if(read_size != rom_size){
        fprintf(stderr, "%s", "Error: reading rom into rom buffer\n");
        exit(1);
    }

    if (rom_size > MAX_ROM_SIZE){
        fprintf(stderr, "%s", "Error: rom size larger than available space\n");
    }
    else{
        memcpy(chip->memory+0x200, rom_buff, rom_size);
        chip->rom_size = rom_size;
        fprintf(stdout, "%s", "Rom loaded successfully\n");
    }

    fclose(rom);
    free(rom_buff);

    
}


int main(){
    // get list of programs in `programs` folder
    DIR *d;
    struct dirent *dir;
    // open directory stream
    d = opendir("programs");

    // get number of programs in `programs` folder
    int program_count = 0;
    while((dir = readdir(d)) != NULL){
        // only increase the counter if the file is a regular file
        if (dir->d_type == DT_REG){
            program_count++;
        }
    }

    // allocate enough memory to fill list of all programs in programs folder
    char** programs = (char**)malloc(sizeof(char*) * program_count);
    
    // reset directory stream
    rewinddir(d);

    // populate list with program names
    int i = 0;
    while((dir = readdir(d)) != NULL){
        // only add the file name to the list if the file is a regular file
        if (dir->d_type == DT_REG){
            programs[i] = dir->d_name;
            i++;
        }
    }
    
    // print list of programs for user to pick from
    for(int i = 0; i < program_count; i++){
        printf("%d. %s\n", i+1, programs[i]);
    }
    
    //close directory stream
    closedir(d);

    // get user choice
    printf("Which game would you like to play?[enter the number]\n");
    char str_choice;
    scanf("%s", &str_choice);
    int choice = atoi(&str_choice);

    char* folder = "programs/";
    char* p = (char*)malloc(sizeof(char) * (strlen(folder) + strlen(programs[choice-1]) + 1));
    strcat(p, folder);
    strcat(p, programs[choice-1]);
    printf("loading: \"%s\"\n", p);

    // making it look like the emulator is loading the rom (makes it less jarring when the window appears)    
    sleep(3);

    // initialize Chip8 emulator
    Chip8 *chip = chip8_init();
    load_rom(chip, p);

    // initialize Raylib window
    InitWindow(640, 320, "Chip8 Emulator by Adriel Méndez Ríos");
    SetTargetFPS(800);

    // initialize Raylib Audio
    InitAudioDevice();

    // initialize Raylib Texture/buffer
    frame_target = LoadRenderTexture(64, 32);

    // setup Raylib sound
    Wave wav = LoadWave("Sounds/medSineWave.wav");
    Sound st_sound = LoadSoundFromWave(wav);

    // Game Loop
    while(!WindowShouldClose()){
        interpreter(chip);
        update_timers(chip);
        // hack, need better solution
        if (chip->soundTimer > 0 && !IsSoundPlaying(st_sound)){
            play_game_sound(chip->soundTimer, st_sound);
        }
        render_frame();
        draw_frame();

    }

    // Clean up
    free(chip);
    free(p);
    for(int i = 0; i < program_count; i++){
        // free memory for each allocated program name
        free(programs[i]);
    }
    // free program list memory
    free(programs);

    UnloadRenderTexture(frame_target);
    UnloadSound(st_sound);
    UnloadWave(wav);
    CloseWindow();
    CloseAudioDevice();

    return 0;
}
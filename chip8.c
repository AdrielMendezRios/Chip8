/*
    written by Adriel Mendez
    [Work in progress]
    A very simple, Chip8 emulator written in C using raylib. its fully functional, 
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
#include </opt/homebrew/include/raylib.h>
#include </opt/homebrew/include/raymath.h>


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
    uint16_t    keypad;  // Add this for key state
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

// load fonts into Emulator memory
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
    BeginTextureMode(frame_target);
    static unsigned char previous_buffer[32][64] = {0};
    
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (frameBuffer[y][x] != previous_buffer[y][x]) {
                DrawPixel(x, y, frameBuffer[y][x] ? WHITE : BLACK);
                previous_buffer[y][x] = frameBuffer[y][x];
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

// Forward declarations of instruction handlers
void handle_0_instructions(Chip8 *chip, uint16_t opcode);
void handle_8_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t y, uint8_t Vx, uint8_t Vy);
void handle_E_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t Vx);
void handle_F_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t Vx);
void handle_display_draw(Chip8 *chip, uint16_t opcode, uint8_t Vx, uint8_t Vy);
void update_input(Chip8 *chip);

void interpreter(Chip8 *chip) {
    getop(chip);
    uint16_t opcode = chip->opcode;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t Vx = chip->v[x];
    uint8_t Vy = chip->v[y];

    // Update input state at CPU rate
    update_input(chip);

    switch (opcode & 0xF000) {
        case 0x0000:
            handle_0_instructions(chip, opcode);
            break;
        case 0x1000: // JP addr
            chip->pc = (opcode & 0x0FFF);
            break;
        case 0x2000: // CALL addr
            chip->stack[++chip->sp] = chip->pc;
            chip->pc = (opcode & 0x0FFF);
            break;
        case 0x3000: // SE Vx, byte
            if (Vx == (opcode & 0x00FF)){
                chip->pc += 4;
            }else {
                chip->pc += 2;
            }
            break;
        case 0x4000: // SNE Vx, byte
            if (Vx != (opcode & 0x00FF)) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;
        case 0x5000: // SE Vx, Vy
            if (Vx == Vy){
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;
        case 0x6000: // LD Vx, byte
            chip->v[x] = (opcode & 0x00FF);
            chip->pc += 2;
            break;
        case 0x7000: // ADD Vx, byte
            chip->v[x] += (opcode & 0x00FF);
            chip->pc += 2;
            break;
        case 0x8000:
            handle_8_instructions(chip, opcode, x, y, Vx, Vy);
            break;
        case 0x9000: // SNE Vx, Vy
            if (Vx != Vy){
                chip->pc += 4;
                break;
            }
            chip->pc += 2;
            break;
        case 0xA000: // LD I, Addr
            chip->I  = opcode & 0x0FFF;
            chip->pc += 2;
            break;
        case 0xB000: // JP V0, addr
            chip->pc = (opcode & 0x0FFF) + chip->v[0];
            break;
        case 0xC000: // RND Vx, byte
            srand(time(NULL));
            uint8_t r = rand() % 255;
            
            chip->v[x] = r & (opcode & 0x00FF);
            
            chip->pc += 2;
            break;
        case 0xD000:
            handle_display_draw(chip, opcode, Vx, Vy);
            break;
        case 0xE000:
            handle_E_instructions(chip, opcode, x, Vx);
            break;
        case 0xF000:
            handle_F_instructions(chip, opcode, x, Vx);
            break;
    }
}

// Implementation of instruction handlers
void handle_0_instructions(Chip8 *chip, uint16_t opcode) {
    switch(opcode) {
        case 0x00E0: // CLR
            clear_screen(chip);
            chip->pc += 2;
            break;
        case 0x00EE: // RET
            chip->pc = chip->stack[chip->sp--];
            chip->pc += 2;
            break;
    }
}

void handle_8_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t y, uint8_t Vx, uint8_t Vy) {
    switch(opcode & 0x000F) {
        case 0x0: // LD Vx, Vy
            chip->v[x] = Vy;
            chip->pc += 2;
            break;
        case 0x1: // OR Vx, Vy
            chip->v[x] = Vx | Vy;
            chip->pc += 2;
            break;
        case 0x2: // AND Vx, Vy
            chip->v[x] = Vx & Vy; 
            chip->pc += 2;
            break;
        case 0x3: // XOR Vx, Vy
            chip->v[x] = Vx ^ Vy;
            chip->pc += 2;
            break;
        case 0x4: { // ADD Vx, Vy
            uint16_t sum;
            sum = Vx + Vy;
            chip->v[0xF] = sum > 0xFF;
            chip->v[x] = sum & 0xFF;
            chip->pc += 2;
            break;
        }
        case 0x5: // SUB Vx, Vy
            chip->v[0xF] = Vx >= Vy;
            
            chip->v[x] = Vx - Vy;
            
            chip->pc += 2;
            break;
        case 0x6: // SHR Vx {, Vy}
            chip->v[0xF] = Vx & 0x1;
            chip->v[x] >>= 1;
            chip->pc += 2;
            break;
        case 0x7: // SUBN Vx, Vy, set Vf = NOT borrow.
            chip->v[0xF] = Vy >= Vx;
            
            chip->v[x] = Vy - Vx;

            chip->pc +=2;
            break;
        case 0xE: // SHL Vx {, Vy}
            chip->v[0xF] = (Vx & 0x80) >> 7;

            chip->v[x] <<= 1;

            chip->pc += 2;
            break;
    }
}

void handle_E_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t Vx) {
    switch (opcode & 0x00FF) {
        case 0x009E: // SKP Vx
            if (chip->keypad & (1 << Vx)) {
                chip->pc += 2;
            }
            chip->pc += 2;
            break;
        case 0x00A1: // SKNP Vx
            if (!(chip->keypad & (1 << Vx))) {
                chip->pc += 2;
            }
            chip->pc += 2;
            break;
    }
}

void handle_F_instructions(Chip8 *chip, uint16_t opcode, uint8_t x, uint8_t Vx) {
    switch(opcode & 0x00FF) {
        case 0x0007: // LD Vx, DT
            chip->v[x] = chip->delayTimer;
            chip->pc += 2;
            break;
        case 0x000A: { // LD Vx, K
            for (int i = 0; i < 16; i++) {
                if (chip->keypad & (1 << i)) {
                    chip->v[x] = i;
                    chip->pc += 2;
                    return;
                }
            }
            break;  // If no key pressed, PC doesn't advance
        }
        case 0x0015: // LD DT, Vx
            chip->delayTimer = Vx;
            chip->pc += 2;
            break;
        case 0x0018: // LD ST, Vx
            chip->soundTimer = Vx;
            chip->pc += 2;
            break;
        case 0x001E: // ADD I, Vx
            chip->I += Vx;
            chip->pc += 2;
            break;
        case 0x0029: // LD F, Vx. 
            chip->I = Vx * 5;
            chip->pc += 2;
            break;
        case 0x0033: // LD B, Vx
            chip->memory[chip->I] = ( Vx/ 100) % 10;
			chip->memory[chip->I+1] = (Vx / 10) % 10;
			chip->memory[chip->I+2] = (Vx % 10);
            chip->pc += 2;
            break;
        case 0x0055: { // LD [I], Vx 
            uint8_t i;
            for(i = 0x0; i <= x; i++){
                chip->memory[chip->I+i] = chip->v[i];
            }
            chip->pc += 2;
            break;
        }
        case 0x0065: { // LD Vx, [I]
            uint8_t i;
            for(i = 0; i <= x; i++){
                chip->v[i] = chip->memory[chip->I + i];
            }
            chip->pc += 2;
            break;
        }
    }
}

void handle_display_draw(Chip8 *chip, uint16_t opcode, uint8_t Vx, uint8_t Vy) {
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

        uint8_t bitmask = 0x80;

        for (int col = 0; col < 8; col++) {
            int spriteX = Vx + col;

            if (sprite & bitmask) {
                int screenX = spriteX; 
                int screenY = spriteY; 

                if (screenX >= 0 && screenX < 64 && screenY >= 0 && screenY < 32) {
                    chip->v[0xF] |= chip->screen[spriteY][spriteX];
                    chip->screen[spriteY][spriteX] ^= 1;

                    frameBuffer[spriteY][spriteX] = chip->screen[spriteY][spriteX] ? 1 : 0;
                }
            }

            bitmask >>= 1;
        }
    }

    chip->pc += 2;
}

// initialize chip8 emulator
Chip8* chip8_init(){
    Chip8 *chip = (Chip8*)malloc(sizeof(Chip8));
    loadfonts(chip);
    chip->pc = 0x200;
    init_mem(chip);
    return chip;
}

// initializes into chip8 memory
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
        fprintf(stderr, "%s", "Error: could not allocate memory for rom\n");
        exit(1);
    }

    read_size = fread(rom_buff, sizeof(uint8_t), (size_t) rom_size, rom);
    if(read_size != rom_size){
        fprintf(stderr, "%s", "Error: could not read rom into rom buffer\n");
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

// Define constants for timing
#define DISPLAY_HZ 120
#define CPU_HZ 3500000  // Increased significantly for better responsiveness

// Update keypad state at CPU rate (add to interpreter function before switch)
void update_input(Chip8 *chip) {
    chip->keypad = 0;
    for (int i = 0; i < 16; i++) {
        int key = keyboardMapping(i);
        if (key != -1 && IsKeyDown(key)) {
            chip->keypad |= (1 << i);
        }
    }
}

int main() {
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
    char input[32];
    int choice;
    printf("Which game would you like to play?[enter the number]\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        choice = strtol(input, NULL, 10);
    }

    // create path to program
    char* folder = "programs/";
    char* p = (char*)malloc(sizeof(char) * (strlen(folder) + strlen(programs[choice-1]) + 1));
    strcat(p, folder);
    strcat(p, programs[choice-1]);
    printf("loading: \"%s\"\n", p);

    // making it look like the emulator is loading the rom (makes it less jarring when the window appears imo)    
    sleep(2);

    // initialize Chip8 emulator
    Chip8 *chip = chip8_init();
    load_rom(chip, p);

    // initialize Raylib window
    InitWindow(640, 320, "Chip8 Emulator by Adriel Méndez Ríos");
    SetTargetFPS(DISPLAY_HZ);  // Lock display refresh to 60 Hz

    // initialize Raylib Audio
    InitAudioDevice();
    frame_target = LoadRenderTexture(64, 32);
    Wave wav = LoadWave("Sounds/medSineWave.wav");
    Sound st_sound = LoadSoundFromWave(wav);

    // For CPU timing
    double last_cpu_time = GetTime();
    double last_frame_time = GetTime();
    double cpu_interval = 1.0 / CPU_HZ;
    double frame_interval = 1.0 / DISPLAY_HZ;

    // Game Loop
    while(!WindowShouldClose()) {
        double current_time = GetTime();
        
        // Debug performance
        static double last_fps_time = 0;
        static int frame_counter = 0;
        static int instruction_counter = 0;
        frame_counter++;
        
        // Execute CPU instructions at CPU_HZ rate
        while (current_time - last_cpu_time >= cpu_interval) {
            interpreter(chip);
            instruction_counter++;
            last_cpu_time += cpu_interval;
        }

        // Update display at DISPLAY_HZ rate
        if (current_time - last_frame_time >= frame_interval) {
            if (current_time - last_fps_time >= 1.0) {
                printf("FPS: %d, CPU Instructions/sec: %d\n", 
                        frame_counter, 
                        instruction_counter);
                frame_counter = 0;
                instruction_counter = 0;
                last_fps_time = current_time;
            }

            update_timers(chip);
            if (chip->soundTimer > 0 && !IsSoundPlaying(st_sound)) {
                play_game_sound(chip->soundTimer, st_sound);
            }
            render_frame();
            BeginDrawing();
                ClearBackground(BLACK);
                DrawTexturePro(frame_target.texture, 
                    (Rectangle){0, 0, (float)frame_target.texture.width, (float)-frame_target.texture.height}, 
                    (Rectangle){0, 0, 640, 320}, 
                    (Vector2){0, 0}, 0.0f, WHITE);
                DrawFPS(10,10);
            EndDrawing();
            last_frame_time += frame_interval;
        }
    }

    // Clean up
    free(chip);
    free(p);
    free(programs);

    // free up Raylib resources
    UnloadRenderTexture(frame_target);
    UnloadSound(st_sound);
    UnloadWave(wav);
    CloseWindow();
    CloseAudioDevice();

    return 0;
}

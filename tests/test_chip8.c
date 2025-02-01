#include "unity.h"
#include "../chip8.h"

Chip8 *chip;

void setUp(void) {
    chip = chip8_init();
}

void tearDown(void) {
    free(chip);
}

void test_chip8_init(void) {
    TEST_ASSERT_NOT_NULL(chip);
    TEST_ASSERT_EQUAL_HEX16(0x200, chip->pc);
    TEST_ASSERT_EQUAL_HEX16(0, chip->sp);
    TEST_ASSERT_EQUAL_HEX16(0, chip->I);
    
    // Test font loading
    for (int i = 0; i < 80; i++) {
        TEST_ASSERT_EQUAL_HEX8(fonts[i], chip->memory[i]);
    }
}

void test_clear_screen(void) {
    // Set some pixels
    chip->screen[0][0] = 1;
    chip->screen[31][63] = 1;
    
    clear_screen(chip);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            TEST_ASSERT_EQUAL_INT(0, chip->screen[y][x]);
        }
    }
}

void test_opcode_00E0(void) {
    // Set up test state
    chip->memory[0x200] = 0x00;
    chip->memory[0x201] = 0xE0;
    chip->screen[0][0] = 1;
    
    interpreter(chip);
    
    // Verify screen was cleared
    TEST_ASSERT_EQUAL_INT(0, chip->screen[0][0]);
    TEST_ASSERT_EQUAL_HEX16(0x202, chip->pc);
}

void test_opcode_1NNN(void) {
    // Test jump instruction
    chip->memory[0x200] = 0x12;  // Jump to 0x234
    chip->memory[0x201] = 0x34;
    
    interpreter(chip);
    
    TEST_ASSERT_EQUAL_HEX16(0x234, chip->pc);
}

void test_opcode_6XNN(void) {
    // Test load immediate
    chip->memory[0x200] = 0x62;  // Load 0x42 into V2
    chip->memory[0x201] = 0x42;
    
    interpreter(chip);
    
    TEST_ASSERT_EQUAL_HEX8(0x42, chip->v[2]);
    TEST_ASSERT_EQUAL_HEX16(0x202, chip->pc);
}

void test_opcode_7XNN(void) {
    // Test add immediate
    chip->v[2] = 0x10;
    chip->memory[0x200] = 0x72;  // Add 0x05 to V2
    chip->memory[0x201] = 0x05;
    
    interpreter(chip);
    
    TEST_ASSERT_EQUAL_HEX8(0x15, chip->v[2]);
    TEST_ASSERT_EQUAL_HEX16(0x202, chip->pc);
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_chip8_init);
    RUN_TEST(test_clear_screen);
    RUN_TEST(test_opcode_00E0);
    RUN_TEST(test_opcode_1NNN);
    RUN_TEST(test_opcode_6XNN);
    RUN_TEST(test_opcode_7XNN);
    return UNITY_END();
} 
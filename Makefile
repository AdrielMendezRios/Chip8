CC = cc
RAYLIB_FLAGS = $(shell pkg-config --cflags raylib) $(shell pkg-config --libs raylib)
TEST_DIR = tests
SRC = chip8.c
TEST_SRC = $(TEST_DIR)/test_chip8.c $(TEST_DIR)/unity.c

# Main build target
chip8: $(SRC)
	$(CC) $^ -o $@ $(RAYLIB_FLAGS)

# Run the program
run: chip8
	./chip8

# Test targets
test: $(TEST_SRC) $(SRC)
	$(CC) -DTESTING -I$(TEST_DIR) $^ -o run_tests $(RAYLIB_FLAGS)
	./run_tests

# Clean both main build and test artifacts
clean:
	rm -f chip8 run_tests

.PHONY: run test clean


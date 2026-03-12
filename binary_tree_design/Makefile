CC       = gcc
CFLAGS   = -Wall -Wextra -Wpedantic -std=c11 -g -fsanitize=address
LDFLAGS  = -fsanitize=address

SRC_DIR  = src
TEST_DIR = tests

LIB_SRC  = $(SRC_DIR)/binary_tree.c
TEST_SRC = $(TEST_DIR)/test_binary_tree.c
TEST_BIN = test_binary_tree

.PHONY: all test valgrind clean

all: $(TEST_BIN)

$(TEST_BIN): $(LIB_SRC) $(TEST_SRC)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $^ -o $@ $(LDFLAGS)

test: $(TEST_BIN)
	./$(TEST_BIN)

valgrind: $(TEST_BIN)
	valgrind --leak-check=full --error-exitcode=1 ./$(TEST_BIN)

clean:
	rm -f $(TEST_BIN)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Idev
SRC := $(shell find dev -name "*.c")
OBJ := $(SRC:dev/%.c=build/%.o)
BIN = build/shrimp

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

build/%.o: dev/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
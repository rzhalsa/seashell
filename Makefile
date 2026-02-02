# Vars
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Idev -g
SRC := $(shell find dev -name "*.c")
OBJ := $(SRC:dev/%.c=build/%.o)
BIN = build/shrimp
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
TMPDIR = $(PWD)/tmp_install

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

build/%.o: dev/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# For CI testing
tmp_install: $(BIN)
	mkdir -p $(TMPDIR)/bin
	install -m 755 $(BIN) $(TMPDIR)/bin/shrimp

check: tmp_install
	./tests/run_all_tests.sh $(TMPDIR)/bin/shrimp

# For installing and uninstalling the shell binary to your pc.
# By default, installed to /usr/local/bin/shrimp
install: $(BIN)
	install -m 755 $(BIN) $(BINDIR)/shrimp

uninstall:
	rm -f $(BINDIR)/shrimp

# Remove the generated build and tmp_install directories if present
clean:
	- rm -rf build
	- rm -rf tmp_install
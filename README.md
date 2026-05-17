# Termioslib

A small C library that wraps common `termios` terminal setup code for terminal-based programs.

`termioslib` is meant to keep raw terminal mode, alternate screen setup, buffered terminal output, input reading, and ANSI color helpers out of application code.

## Features

- Creates a terminal context using an arena allocator
- Switches the terminal into raw mode
- Saves and restores the original terminal settings
- Enables and disables the terminal alternate screen buffer
- Reads keyboard input from `stdin`
- Buffers writes before flushing to `stdout`
- Writes single characters or `string8` values
- Supports true color ANSI foreground/background output
- Provides simple terminal/window tile structs for terminal UI projects

## Project Layout

```text
termioslib/
├── include/
│   └── termioslib/
│       └── termioslib.h
├── src/
│   └── termioslib.c
├── build/
│   └── libtermioslib.a
├── Makefile
├── compile_flags.txt
└── README.md
```

## Dependencies

This library depends on:

- `base`
- `arena`
- POSIX terminal APIs

Expected folder layout:

```text
libs/
├── base/
│   └── include/
│       └── base.h
├── arena/
│   ├── include/
│   │   └── arena.h
│   └── build/
│       └── libarena.a
└── termioslib/
    ├── include/
    │   └── termioslib/
    │       └── termioslib.h
    ├── src/
    │   └── termioslib.c
    └── Makefile
```

The default Makefile assumes:

```make
BASE_DIR ?= ../base
ARENA_DIR ?= ../arena
```

You can override those paths when building:

```sh
make BASE_DIR=/path/to/base ARENA_DIR=/path/to/arena
```

## Building

Build the static library:

```sh
make
```

This creates:

```text
build/libtermioslib.a
```

Clean build files:

```sh
make clean
```

Rebuild from scratch:

```sh
make rebuild
```

Print the Makefile configuration:

```sh
make print
```

## Usage

Include the header:

```c
#include "termioslib/termioslib.h"
```

Create a terminal context:

```c
mem_arena arena = arena_create(MiB(1));

term_context *term = term_create(KiB(4), &arena);
```

Write to the terminal:

```c
term_write(term, STR8_LIT("Hello from termioslib\n"));
term_flush(term);
```

Read input:

```c
u8 keys[32];

u32 key_count = term_read(term, keys, sizeof(keys));

for (u32 i = 0; i < key_count; i++) {
    if (keys[i] == 'q') {
        // quit
    }
}
```

Restore the terminal before exiting:

```c
term_quit(term);
```

## Example

```c
#include "base.h"
#include "arena.h"
#include "termioslib/termioslib.h"

int main(void) {
    mem_arena arena = arena_create(MiB(1));

    term_context *term = term_create(KiB(4), &arena);

    b32 running = true;

    while (running) {
        u8 keys[32];
        u32 key_count = term_read(term, keys, sizeof(keys));

        for (u32 i = 0; i < key_count; i++) {
            if (keys[i] == 'q') {
                running = false;
            }
        }

        term_write(term, STR8_LIT("\x1b[2J"));
        term_write(term, STR8_LIT("\x1b[H"));
        term_write(term, STR8_LIT("Press q to quit\n"));

        term_flush(term);
    }

    term_quit(term);

    return 0;
}
```

## Linking in Another Project

Example project Makefile:

```make
CC := gcc

BUILD_DIR := build
SRC_DIR := src

BASE_DIR := ../libs/base
ARENA_DIR := ../libs/arena
TERMIOSLIB_DIR := ../libs/termioslib

CPPFLAGS := \
	-I$(BASE_DIR)/include \
	-I$(ARENA_DIR)/include \
	-I$(TERMIOSLIB_DIR)/include

CFLAGS := -Wall -Wextra -Wpedantic -g

LDFLAGS := \
	-L$(ARENA_DIR)/build \
	-L$(TERMIOSLIB_DIR)/build

LDLIBS := -ltermioslib -larena

SRC := $(SRC_DIR)/main.c
BIN := $(BUILD_DIR)/my_app

.PHONY: all clean run rebuild arena termioslib libs

all: libs $(BIN)

libs: arena termioslib

arena:
	$(MAKE) -C $(ARENA_DIR)

termioslib:
	$(MAKE) -C $(TERMIOSLIB_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(SRC) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

run: all
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(ARENA_DIR) clean
	$(MAKE) -C $(TERMIOSLIB_DIR) clean

rebuild: clean all
```

## API

### Types

```c
typedef struct {
    struct termios orig_termios;
    u32 buf_capacity;
    u32 buf_size;
    u8 *buf;
} term_context;
```

Stores terminal state and an output buffer.

```c
typedef struct {
    u8 r, g, b;
} win_col;
```

Represents an RGB color.

```c
typedef struct {
    u8 c;
    win_col fg_col;
    win_col bg_col;
} win_tile;
```

Represents a terminal tile with a character, foreground color, and background color.

```c
typedef struct {
    u32 width, height;
    win_tile *data;
} win_buf;
```

Represents a 2D terminal buffer.

### Functions

```c
term_context *term_create(u32 capacity, mem_arena *arena);
```

Creates a terminal context, switches the terminal into raw mode, enters the alternate screen buffer, clears the screen, and allocates the write buffer from the arena.

```c
void term_quit(term_context *term);
```

Leaves the alternate screen buffer and restores the original terminal settings.

```c
u32 term_read(term_context *term, u8 *keys, u32 capacity);
```

Reads keyboard input into `keys`.

```c
void term_write(term_context *term, string8 str);
```

Writes a `string8` into the terminal output buffer.

```c
void term_write_c(term_context *term, u8 c);
```

Writes a single character into the terminal output buffer.

```c
void term_flush(term_context *term);
```

Flushes the terminal output buffer to `stdout`.

```c
b32 col_eq(win_col a, win_col b);
```

Returns whether two colors are equal.

```c
void set_col(term_context *term, win_col c, b32 fg);
```

Writes an ANSI true color escape sequence to set the foreground or background color.

Use `fg = true` for foreground color.

Use `fg = false` for background color.

## Notes

`term_create` changes terminal settings. Always call `term_quit` before the program exits.

For programs with multiple exit paths, use a cleanup path:

```c
int main(void) {
    mem_arena arena = arena_create(MiB(1));
    term_context *term = term_create(KiB(4), &arena);

    b32 running = true;

    while (running) {
        // program loop
    }

cleanup:
    term_quit(term);
    return 0;
}
```

This library is intended for terminal programs, small games, terminal UIs, and experiments where direct terminal control is useful.

## Editor / LSP Setup

A `compile_flags.txt` file is included for language servers such as `clangd`.

Current compile flags:

```text
-std=c11
-Wall
-Wextra
-Wpedantic
-g
-Iinclude
-I../base/include
-I../arena/include
```

If your folder structure is different, update the include paths in `compile_flags.txt`.

## License

No license has been specified yet.

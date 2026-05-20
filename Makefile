CC := gcc
AR := ar

BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include
EXAMPLES_DIR := examples

# Path to your external base library.
# Override from command line if needed:
# make BASE_DIR=../base
BASE_DIR ?= ../base
BASE_INCLUDE_DIR := $(BASE_DIR)/include
BASE_BUILD_DIR := $(BASE_DIR)/build

ARENA_DIR ?= ../arena
ARENA_INCLUDE_DIR := $(ARENA_DIR)/include
ARENA_BUILD_DIR := $(ARENA_DIR)/build

LIB_NAME := libtermioslib.a

CFLAGS := -Wall -Wextra -Wpedantic -g
CPPFLAGS := -I$(INCLUDE_DIR) -I$(BASE_INCLUDE_DIR) -I$(ARENA_INCLUDE_DIR)

LDFLAGS := -L$(BUILD_DIR) -L$(BASE_BUILD_DIR) -L$(ARENA_BUILD_DIR)
LDLIBS := -ltermioslib -larena -lbase

SRC := $(SRC_DIR)/termioslib.c
OBJ := $(BUILD_DIR)/termioslib.o
LIB := $(BUILD_DIR)/$(LIB_NAME)

EXAMPLE_SRC := $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_BIN := $(patsubst $(EXAMPLES_DIR)/%.c,$(BUILD_DIR)/examples/%,$(EXAMPLE_SRC))

.PHONY: all clean rebuild print examples

all: $(LIB)

examples: $(LIB) $(EXAMPLE_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/examples:
	mkdir -p $(BUILD_DIR)/examples

$(OBJ): $(SRC) $(INCLUDE_DIR)/termioslib/termioslib.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(BUILD_DIR)/examples/%: $(EXAMPLES_DIR)/%.c $(LIB) | $(BUILD_DIR)/examples
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

print:
	@echo "CC:                 $(CC)"
	@echo "AR:                 $(AR)"
	@echo "SRC:                $(SRC)"
	@echo "OBJ:                $(OBJ)"
	@echo "LIB:                $(LIB)"
	@echo "INCLUDE_DIR:        $(INCLUDE_DIR)"
	@echo "BASE_DIR:           $(BASE_DIR)"
	@echo "BASE_INCLUDE_DIR:   $(BASE_INCLUDE_DIR)"
	@echo "BASE_BUILD_DIR:     $(BASE_BUILD_DIR)"
	@echo "ARENA_DIR:          $(ARENA_DIR)"
	@echo "ARENA_INCLUDE_DIR:  $(ARENA_INCLUDE_DIR)"
	@echo "ARENA_BUILD_DIR:    $(ARENA_BUILD_DIR)"
	@echo "EXAMPLES_DIR:       $(EXAMPLES_DIR)"
	@echo "EXAMPLE_SRC:        $(EXAMPLE_SRC)"
	@echo "EXAMPLE_BIN:        $(EXAMPLE_BIN)"

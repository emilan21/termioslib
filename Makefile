CC := gcc
AR := ar

BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include

# Path to your external base library.
# Override from command line if needed:
# make BASE_DIR=../base
BASE_DIR ?= ../base
BASE_INCLUDE_DIR := $(BASE_DIR)/include

ARENA_DIR ?= ../arena
ARENA_INCLUDE_DIR := $(ARENA_DIR)/include

LIB_NAME := libtermioslib.a

CFLAGS := -Wall -Wextra -Wpedantic -g
CPPFLAGS := -I$(INCLUDE_DIR) -I$(BASE_INCLUDE_DIR) -I$(ARENA_INCLUDE_DIR)

SRC := $(SRC_DIR)/termioslib.c
OBJ := $(BUILD_DIR)/termioslib.o
LIB := $(BUILD_DIR)/$(LIB_NAME)

.PHONY: all clean rebuild print

all: $(LIB)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ): $(SRC) $(INCLUDE_DIR)/termioslib/termioslib.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

print:
	@echo "CC:               	$(CC)"
	@echo "AR:               	$(AR)"
	@echo "SRC:              	$(SRC)"
	@echo "OBJ:              	$(OBJ)"
	@echo "LIB:              	$(LIB)"
	@echo "INCLUDE_DIR:      	$(INCLUDE_DIR)"
	@echo "BASE_DIR:         	$(BASE_DIR)"
	@echo "BASE_INCLUDE_DIR: 	$(BASE_INCLUDE_DIR)"
	@echo "ARENA_DIR_DIR:    	$(ARENA_DIR)"
	@echo "ARENA_INCLUDE_DIR: 	$(ARENA_INCLUDE_DIR)"

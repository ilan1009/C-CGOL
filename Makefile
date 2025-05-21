CC = gcc
CFLAGS = -Wall -Wextra -std=c23 -Iinclude -Iinclude/glad -O3 -march=native -flto -funroll-loops -MMD -MP
LDFLAGS = -lglfw -lGL -lm -ldl

# src
SRC := $(wildcard src/*.c) src/glad/glad.c
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)  # Dependency files
BIN := build/my_project

# shaders
SHADER_DIR := build/shaders
SHADERS := $(wildcard src/shaders/*.vert src/shaders/*.frag)
SHADER_TARGETS := $(patsubst src/shaders/%,$(SHADER_DIR)/%,$(SHADERS))

$(BIN): $(OBJ) $(SHADER_TARGETS)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHADER_DIR)/%: src/shaders/%
	@mkdir -p $(@D)
	cp $< $@

clean:
	rm -rf build

-include $(DEP)

.PHONY: clean
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c23 -Iinclude -Iinclude/glad -O3 -march=native -flto -funroll-loops -MMD -MP
LDFLAGS = -lglfw -lGL -lm -ldl

# Source files
SRC := $(wildcard src/*.c) src/glad/glad.c
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)  # Dependency files
BIN := build/my_project

# Shader files
SHADER_DIR := build/shaders
SHADERS := $(wildcard src/shaders/*.vert src/shaders/*.frag)
SHADER_TARGETS := $(patsubst src/shaders/%,$(SHADER_DIR)/%,$(SHADERS))

# Main target
$(BIN): $(OBJ) $(SHADER_TARGETS)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compile C files
build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Copy shaders
$(SHADER_DIR)/%: src/shaders/%
	@mkdir -p $(@D)
	cp $< $@

# Clean
clean:
	rm -rf build

# Include dependencies
-include $(DEP)

.PHONY: clean
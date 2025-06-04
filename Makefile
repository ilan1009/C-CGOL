CC = gcc
CFLAGS = -Wall -Wextra -std=c23 -O3 -march=native -flto -funroll-loops \
         -Iinclude -Isrc
LDFLAGS = -lglfw -lGL -lm -ldl

SRC_DIR := src
LIBS_DIR := libs
BUILD_DIR := build
SHADER_DIR := $(BUILD_DIR)/shaders

SRC := $(wildcard $(SRC_DIR)/*.c)
LIBS := $(wildcard $(LIBS_DIR)/glad/*.c) $(wildcard $(LIBS_DIR)/tinyfiledialogs/*.c)
ALL_SRC := $(SRC) $(LIBS)

OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(ALL_SRC))

DEP := $(OBJ:.o=.d)

BIN := $(BUILD_DIR)/CCGOL

SHADERS := $(wildcard $(SRC_DIR)/shaders/*.vert $(SRC_DIR)/shaders/*.frag)
SHADER_TARGETS := $(patsubst $(SRC_DIR)/shaders/%,$(SHADER_DIR)/%,$(SHADERS))

# Build binary
$(BIN): $(OBJ) $(SHADER_TARGETS)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# copy shaders
$(SHADER_DIR)/%: $(SRC_DIR)/shaders/%
	@mkdir -p $(@D)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEP)

.PHONY: clean

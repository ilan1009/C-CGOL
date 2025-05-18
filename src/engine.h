#ifndef ENGINE_H
#define ENGINE_H

#include "uthash.h"
#include "coordinate.h"
#include "coordinate_set.h"
#include <stdbool.h>

// game constants
#define ALIVE true
#define DEAD false

#define FATE_STAY 0
#define FATE_DEATH -1
#define FATE_BIRTH 1

#define POOL_SIZE 10000
extern size_t pool_index;


extern CoordinateSetEntry* alive_cells;

void engine_init(int width, int height);
void engine_cleanup(void);

void birth_cell(Coordinate pos);
void kill_cell(Coordinate pos);

static inline bool get_cell_state(Coordinate pos) {
    CoordinateSetEntry* found;
    HASH_FIND(hh, alive_cells, &pos, sizeof(Coordinate), found);
    return found != NULL;
}
int count_alive_neighbors(Coordinate pos);

// decide if cell is fated to be birthed, die, or stay the same in the next generation
int decide_cell_fate(Coordinate pos);

// add a coordinate to the candidates set
static inline void add_candidate(CoordinateSetEntry** candidates, Coordinate coord);

// add alive cells and their neighbors to candidates set
void add_cell_and_neighbors(CoordinateSetEntry** candidates, Coordinate cell, int width, int height);

static inline void free_coordinate_set(CoordinateSetEntry** set);

void engine_step(void); // advance the game by one generation

#endif
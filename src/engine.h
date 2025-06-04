// engine.h
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

extern CoordinateSetEntry* alive_cells;

void engine_init(int width, int height);
void engine_cleanup(void);

void birth_cell(Coordinate pos);
void kill_cell(Coordinate pos);


// add a coordinate to the candidates set
static inline void add_to_coordinate_set(CoordinateSetEntry** candidates, Coordinate coord) {
    CoordinateSetEntry* existing_entry;
    
    HASH_FIND(hh, *candidates, &coord, sizeof(Coordinate), existing_entry);
    
    if (!existing_entry) {
        CoordinateSetEntry* new_cell = malloc(sizeof(CoordinateSetEntry));
        if (new_cell) {
            new_cell->coord = coord;
            HASH_ADD(hh, *candidates, coord, sizeof(Coordinate), new_cell);
        }
    }
}


static inline void free_coordinate_set(CoordinateSetEntry** set) {
    CoordinateSetEntry *entry, *tmp;
    HASH_ITER(hh, *set, entry, tmp) {
        HASH_DEL(*set, entry);
        free(entry);
    }
    *set = NULL; // nullify pointer after cleanup
}

void engine_step(void); // advance the game by one generation

#endif
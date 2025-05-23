// engine.c - Using proper dynamic sets
#include "engine.h"
#include "coordinate.h"
#include "coordinate_set.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool first_step;


static int grid_width = 0;
static int grid_height = 0;

CoordinateSetEntry* alive_cells = NULL;
static CoordinateSetEntry* candidates = NULL;

typedef struct {
    Coordinate coord;
    int count;
    UT_hash_handle hh;
} NeighborCount;

static const int DIRECTIONS_X[8] = { 0, -1,  1, -1,  1,  0, -1,  1 };
static const int DIRECTIONS_Y[8] = {-1, -1, -1,  0,  0,  1,  1,  1 };

static const int DELTA_X[9] = { 0, -1,  1, -1,  0,  1, -1,  0,  1 };
static const int DELTA_Y[9] = {-1, -1, -1,  0,  0,  0,  1,  1,  1 };

void engine_init(int width, int height) {
    first_step = true;
    
    grid_width = width;
    grid_height = height;
}

void engine_cleanup(void) {
    CoordinateSetEntry *current, *tmp;
    HASH_ITER(hh, alive_cells, current, tmp) {
        HASH_DEL(alive_cells, current);
        free(current);
    }
    alive_cells = NULL;
    
    HASH_ITER(hh, candidates, current, tmp) {
        HASH_DEL(candidates, current);
        free(current);
    }
    candidates = NULL;
}

// Proper coordinate wrapping that handles all cases
static inline void wrap_coordinate_inplace(int* x, int* y) {
    *x = ((*x % grid_width) + grid_width) % grid_width;
    *y = ((*y % grid_height) + grid_height) % grid_height;
}

void birth_cell(Coordinate pos) {
    CoordinateSetEntry* found;
    HASH_FIND(hh, alive_cells, &pos, sizeof(Coordinate), found);
    if (!found) {
        CoordinateSetEntry* new_cell = malloc(sizeof(CoordinateSetEntry));
        new_cell->coord = pos;
        HASH_ADD(hh, alive_cells, coord, sizeof(Coordinate), new_cell);
    }
}

void kill_cell(Coordinate pos) {
    CoordinateSetEntry* found;
    HASH_FIND(hh, alive_cells, &pos, sizeof(Coordinate), found);
    if (found) {
        HASH_DEL(alive_cells, found);
        free(found);
    }
}

// Optimized neighbor counting
static inline int count_alive_neighbors(int x, int y) {
    int count = 0;
    CoordinateSetEntry* found;
    Coordinate neighbor;
    
    for (int i = 0; i < 8; i++) {
        neighbor.x = x + DIRECTIONS_X[i];
        neighbor.y = y + DIRECTIONS_Y[i];
        wrap_coordinate_inplace(&neighbor.x, &neighbor.y);
        
        HASH_FIND(hh, alive_cells, &neighbor, sizeof(Coordinate), found);
        if (found) count++;
    }
    
    return count;
}

// Clear and free a coordinate set
static inline void clear_coordinate_set(CoordinateSetEntry** set) {
    CoordinateSetEntry *current, *tmp;
    HASH_ITER(hh, *set, current, tmp) {
        HASH_DEL(*set, current);
        free(current);
    }
    *set = NULL;
}

static NeighborCount* build_neighbor_counts(CoordinateSetEntry* candidates) {
    NeighborCount* counts = NULL;
    CoordinateSetEntry *cell, *tmp;
    
    HASH_ITER(hh, candidates, cell, tmp) {
        int target_x = cell->coord.x;
        int target_y = cell->coord.y;
        wrap_coordinate_inplace(&target_x, &target_y);
        
        Coordinate target = {target_x, target_y};
        
        
        // count neighbors for this coordinate, there are no duplicates in candidates
        int alive_neighbors = count_alive_neighbors(target_x, target_y);
        
        NeighborCount* entry;
        entry = malloc(sizeof(NeighborCount));
        entry->coord = target;
        entry->count = alive_neighbors;
        HASH_ADD(hh, counts, coord, sizeof(Coordinate), entry);
    
    }
    
    return counts;
}

void engine_step(void) {
    CoordinateSetEntry* cell;
    CoordinateSetEntry* tmp;
    
    if (first_step) {
        first_step = false;
        HASH_ITER(hh, alive_cells, cell, tmp) {
            // Add cell and its neighbors to changed set
            for (int i = 0; i < 9; i++) {
                Coordinate affected;
                affected.x = cell->coord.x + DELTA_X[i];
                affected.y = cell->coord.y + DELTA_Y[i];
                wrap_coordinate_inplace(&affected.x, &affected.y);
                add_to_coordinate_set(&candidates, affected);
            }
        }
    }
    
    NeighborCount* neighbor_counts = build_neighbor_counts(candidates);
    
    CoordinateSetEntry* to_birth = NULL;
    CoordinateSetEntry* to_die = NULL;
    
    NeighborCount *nc, *nc_tmp;
    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        CoordinateSetEntry* alive;
        HASH_FIND(hh, alive_cells, &nc->coord, sizeof(Coordinate), alive);
        
        int is_alive = (alive != NULL);
        int neighbors = nc->count;
        
        // cgol rules
        if (is_alive) {
            if (neighbors < 2 || neighbors > 3) {
                add_to_coordinate_set(&to_die, nc->coord);
            }
        } else {
            if (neighbors == 3) {
                add_to_coordinate_set(&to_birth, nc->coord);
            }
        }
    }
    
    // cleanup
    clear_coordinate_set(&candidates);
    
    HASH_ITER(hh, to_die, cell, tmp) {
        kill_cell(cell->coord);
        
        // add neighbors and self to candidates
        for (int i = 0; i < 9; i++) {
            Coordinate neighbor;
            neighbor.x = cell->coord.x + DELTA_X[i];
            neighbor.y = cell->coord.y + DELTA_Y[i];
            wrap_coordinate_inplace(&neighbor.x, &neighbor.y);
            add_to_coordinate_set(&candidates, neighbor);
        }
    }
    
    HASH_ITER(hh, to_birth, cell, tmp) {
        birth_cell(cell->coord);
        
        for (int i = 0; i < 9; i++) {
            Coordinate neighbor;
            neighbor.x = cell->coord.x + DELTA_X[i];
            neighbor.y = cell->coord.y + DELTA_Y[i];
            wrap_coordinate_inplace(&neighbor.x, &neighbor.y);
            add_to_coordinate_set(&candidates, neighbor);
        }
    }
    
    // cleanup
    clear_coordinate_set(&to_birth);
    clear_coordinate_set(&to_die);
    
    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        HASH_DEL(neighbor_counts, nc);
        free(nc);
    }
}
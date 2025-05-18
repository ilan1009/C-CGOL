// engine.c
#include "engine.h"
#include "coordinate.h"
#include "coordinate_set.h"
#include <stdlib.h>
#include <stdio.h>


static int grid_width = 0;
static int grid_height = 0;

CoordinateSetEntry* alive_cells = NULL;

typedef struct {
    Coordinate coord;
    int count;
    UT_hash_handle hh;
} NeighborCount;



void engine_init(int width, int height) {
    grid_width = width;
    grid_height = height;
}

void engine_cleanup(void) {
    CoordinateSetEntry *current, *tmp;
    HASH_ITER(hh, alive_cells, current, tmp) {
        HASH_DEL(alive_cells, current);
        free(current);
    }
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

static const Coordinate DIRECTIONS[8] = {
    {  0, -1 }, { -1, -1 }, { 1, -1 }, { -1, 0 },
    {  1,  0 }, {  0,  1 }, { -1,  1 }, { 1, 1 }
};

int count_alive_neighbors(Coordinate pos) {
    int count = 0;
    for (int i = 0; i < 8; ++i) {
        Coordinate neighbor = {
            (pos.x + DIRECTIONS[i].x + grid_width) % grid_width,
            (pos.y + DIRECTIONS[i].y + grid_height) % grid_height
        };
        if (get_cell_state(neighbor) == ALIVE) {
            count++;
        }
    }
    return count;
}

static inline Coordinate wrap_coordinate(int x, int y) {
    return (Coordinate){
        (x + grid_width) % grid_width,
        (y + grid_height) % grid_height
    };
}

static NeighborCount* build_neighbor_counts(void) {
    NeighborCount* counts = NULL;
    CoordinateSetEntry *cell, *tmp;

    HASH_ITER(hh, alive_cells, cell, tmp) {
        for (int i = 0; i < 8; i++) {
            Coordinate nb = wrap_coordinate(
                cell->coord.x + DIRECTIONS[i].x,
                cell->coord.y + DIRECTIONS[i].y
            );
            
            NeighborCount* entry;
            HASH_FIND(hh, counts, &nb, sizeof(Coordinate), entry);
            if (entry) {
                entry->count++;
            } else {
                entry = malloc(sizeof(NeighborCount));
                *entry = (NeighborCount){nb, 1};
                HASH_ADD(hh, counts, coord, sizeof(Coordinate), entry);
            }
        }
    }
    return counts;
}

static inline int decide_fate(int neighbors, bool alive) {
    return (alive && (neighbors < 2 || neighbors > 3)) ? FATE_DEATH :
          (!alive && neighbors == 3) ? FATE_BIRTH : FATE_STAY;
}

static inline void add_candidate(CoordinateSetEntry** candidates, Coordinate coord) {
    CoordinateSetEntry* new_cell = malloc(sizeof(CoordinateSetEntry));
    if (new_cell) {
        new_cell->coord = coord;
        HASH_ADD(hh, *candidates, coord, sizeof(Coordinate), new_cell);
    }
}

void add_cell_and_neighbors(CoordinateSetEntry** candidates, Coordinate cell, int width, int height) {
    // add neighbors
    for (int i = 0; i < 8; ++i) {
        Coordinate neighbor = {
            (cell.x + DIRECTIONS[i].x + width) % width,
            (cell.y + DIRECTIONS[i].y + height) % height
        };
        CoordinateSetEntry* found;
        HASH_FIND(hh, *candidates, &neighbor, sizeof(Coordinate), found);
        if (!found) {
            add_candidate(candidates, neighbor);
        }
    }
    // add self
    CoordinateSetEntry* self;
    HASH_FIND(hh, *candidates, &cell, sizeof(Coordinate), self);
    if (!self) {
        add_candidate(candidates, cell);
    }
}

void free_coordinate_set(CoordinateSetEntry** set) {
    CoordinateSetEntry *entry, *tmp;
    HASH_ITER(hh, *set, entry, tmp) {
        HASH_DEL(*set, entry);
        free(entry);
    }
    *set = NULL; // important: nullify pointer after cleanup
}

void engine_step(void) {
    NeighborCount* neighbor_counts = build_neighbor_counts();
    CoordinateSetEntry *to_birth = NULL, *to_kill = NULL;

    // Process currently alive cells
    CoordinateSetEntry *cell, *tmp;
    HASH_ITER(hh, alive_cells, cell, tmp) {
        NeighborCount* nc;
        HASH_FIND(hh, neighbor_counts, &cell->coord, sizeof(Coordinate), nc);
        int fate = decide_fate(nc ? nc->count : 0, true);
        if (fate == FATE_DEATH) add_candidate(&to_kill, cell->coord);
    }

    // Process neighbor counts for potential births
    NeighborCount *nc, *nc_tmp;
    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        CoordinateSetEntry* existing;
        HASH_FIND(hh, alive_cells, &nc->coord, sizeof(Coordinate), existing);
        if (!existing) {
            int fate = decide_fate(nc->count, false);
            if (fate == FATE_BIRTH) add_candidate(&to_birth, nc->coord);
        }
    }

    // Apply changes
    HASH_ITER(hh, to_kill, cell, tmp) kill_cell(cell->coord);
    HASH_ITER(hh, to_birth, cell, tmp) birth_cell(cell->coord);

    // Cleanup
    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        HASH_DEL(neighbor_counts, nc);
        free(nc);
    }
    free_coordinate_set(&to_birth);
    free_coordinate_set(&to_kill);
}

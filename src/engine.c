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


static inline Coordinate wrap_coordinate(int x, int y) {
    return (Coordinate){
        (x + grid_width) % grid_width,
        (y + grid_height) % grid_height
    };
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
                *entry = (NeighborCount){ .coord = nb, .count = 1 };

                HASH_ADD(hh, counts, coord, sizeof(Coordinate), entry);
            }
        }
    }
    return counts;
}




void engine_step(void) {
    NeighborCount* neighbor_counts = build_neighbor_counts();

    CoordinateSetEntry *to_birth = NULL, *to_kill = NULL;

    // alive cells
    CoordinateSetEntry *cell, *tmp;
    HASH_ITER(hh, alive_cells, cell, tmp) {
        NeighborCount* nc;
        HASH_FIND(hh, neighbor_counts, &cell->coord, sizeof(Coordinate), nc);
        int fate;
        if (nc) {
            fate = decide_fate(nc->count, true);
        }
        else {
            fate = FATE_DEATH;
        }

        if (fate == FATE_DEATH) add_to_coordinate_set(&to_kill, cell->coord);
    }

    // neighbors
    NeighborCount *nc, *nc_tmp;
    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        CoordinateSetEntry* alive;
        HASH_FIND(hh, alive_cells, &nc->coord, sizeof(Coordinate), alive);
        if (!alive) { // if dead
            int fate = decide_fate(nc->count, false);
            if (fate == FATE_BIRTH) add_to_coordinate_set(&to_birth, nc->coord);
        }
    }

    HASH_ITER(hh, to_kill, cell, tmp) kill_cell(cell->coord);
    HASH_ITER(hh, to_birth, cell, tmp) birth_cell(cell->coord);


    HASH_ITER(hh, neighbor_counts, nc, nc_tmp) {
        HASH_DEL(neighbor_counts, nc);
        free(nc);
    }
    free_coordinate_set(&to_birth);
    free_coordinate_set(&to_kill);
}

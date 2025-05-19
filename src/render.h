#ifndef RENDER_H
#define RENDER_H

#include "coordinate_set.h"
#include <stdbool.h>

typedef struct {
    unsigned int vao, vbo, instance_vbo, shader;
    float cell_size;
    float projection[16]; // Column-major 4x4 matrix

    int* cells;
    int cells_capacity;
} Renderer;


void render_init(Renderer* renderer, float cell_size);
void render_resize(Renderer* renderer, int width, int height);
void render_grid(Renderer* renderer, CoordinateSetEntry* alive_cells);
void render_cleanup(Renderer* renderer);

#endif


#include "render.h"
#include "shader_loader.h"
#include "linmath.h"
#include "window.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>

void render_init(Renderer* renderer, float cell_size) {
    if (!gladLoadGL()) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(EXIT_FAILURE);
    }


    // Initialize shader
    printf("initialising shader...\n");
    renderer->shader = create_shader_program(
        "src/shaders/grid.vert",
        "src/shaders/grid.frag"
    );
    printf("shader initialised\n");
    renderer->cell_size = cell_size;

    renderer->cells = NULL;
    renderer->cells_capacity = 0;


    // Initialize VAO/VBO
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->instance_vbo);
    glBindVertexArray(renderer->vao);
    
    // Only instance VBO needed for point coordinates
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, GRID_WIDTH * GRID_HEIGHT * 2 * sizeof(int), NULL, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_INT, 2 * sizeof(int), (void*)0);
    
    // No attribute divisor needed for points
    glBindVertexArray(0);
    
    // Enable point sprites
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void render_resize(Renderer* renderer, int width, int height) {
    mat4x4 ortho;
    mat4x4_ortho(ortho, 0, width, height, 0, -1, 1); // Y-flip
    memcpy(renderer->projection, ortho, sizeof(ortho));
}

void render_grid(Renderer* renderer, CoordinateSetEntry* alive_cells) {
    // Count cells
    int count = 0;
    for (CoordinateSetEntry* c = alive_cells; c; c = c->hh.next) count++;

    // Resize buffer if needed
    if (count > renderer->cells_capacity) {
        free(renderer->cells);  // only if it exists
        renderer->cells = malloc(count * 2 * sizeof(int));
        renderer->cells_capacity = count;
    }

    int* cells = renderer->cells;
    int i = 0;
    for (CoordinateSetEntry* c = alive_cells; c; c = c->hh.next) {
        cells[i++] = c->coord.x;
        cells[i++] = c->coord.y;
    }

    // Update GPU buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 2 * sizeof(int), cells);

    // Draw
    glUseProgram(renderer->shader);
    glUniformMatrix4fv(
        glGetUniformLocation(renderer->shader, "uProjection"),
        1, GL_FALSE, renderer->projection
    );
    glUniform1f(glGetUniformLocation(renderer->shader, "uCellSize"), renderer->cell_size);
    glBindVertexArray(renderer->vao);
    glDrawArrays(GL_POINTS, 0, count); // Just draw points
}

void render_cleanup(Renderer* renderer) {
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->instance_vbo);
    glDeleteProgram(renderer->shader);
}

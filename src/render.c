// src/render.c
#include "render.h"
#include "shader_loader.h"
#include "linmath.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>

static const float quad_vertices[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // Triangle 1
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Triangle 2
};

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

    // Initialize VAO/VBO
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glGenBuffers(1, &renderer->instance_vbo);

    glBindVertexArray(renderer->vao);

    // Quad VBO
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // Instance VBO (init with NULL)
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instance_vbo);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 2, GL_INT, 2 * sizeof(int), (void*)0);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}

void render_resize(Renderer* renderer, int width, int height) {
    mat4x4 ortho;
    mat4x4_ortho(ortho, 0, width, height, 0, -1, 1); // Note: height and 0 are swapped
    memcpy(renderer->projection, ortho, sizeof(ortho));
}

void render_grid(Renderer* renderer, CoordinateSetEntry* alive_cells) {
    // Count cells
    int count = 0;
    for (CoordinateSetEntry* c = alive_cells; c; c = c->hh.next) count++;

    // Pack coordinates
    int* cells = malloc(count * 2 * sizeof(int));
    int i = 0;
    for (CoordinateSetEntry* c = alive_cells; c; c = c->hh.next) {
        cells[i++] = c->coord.x;
        cells[i++] = c->coord.y;
    }

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(int), cells, GL_STREAM_DRAW);
    free(cells);

    // Draw
    glUseProgram(renderer->shader);
    glUniformMatrix4fv(
        glGetUniformLocation(renderer->shader, "uProjection"),
        1, GL_FALSE, renderer->projection
    );
    glUniform1f(glGetUniformLocation(renderer->shader, "uCellSize"), renderer->cell_size);

    glBindVertexArray(renderer->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);
}

void render_cleanup(Renderer* renderer) {
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->instance_vbo);
    glDeleteProgram(renderer->shader);
}
#include "main.h"
#include "engine.h"
#include "window.h"
#include "render.h"
#include "game.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

extern CoordinateSetEntry* alive_cells;

int main(void) {
    printf("program started\n");
    setbuf(stdout, NULL);

    engine_init(GRID_WIDTH, GRID_HEIGHT);

    GLFWwindow *window;
    init_glfw(&window);

    Renderer renderer;
    render_init(&renderer, CELL_SIZE);

    // Query the framebuffer size
    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

    // Resize the renderer to match the framebuffer size
    render_resize(&renderer, framebuffer_width, framebuffer_height);
    printf("Renderer initialised\n");

    glClear(GL_COLOR_BUFFER_BIT);
    render_grid(&renderer, alive_cells);
    glfwSwapBuffers(window);
    
    init_game(window, &renderer);

    game_loop();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void init_glfw(GLFWwindow **window) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CGOL", NULL, NULL);
    if (!*window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(*window);
    glfwSetWindowAspectRatio(*window, 1, 1);
    glfwSwapInterval(1);
}

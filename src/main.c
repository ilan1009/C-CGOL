#include "main.h"
#include "engine.h"
#include "window.h"
#include "render.h"
#include "game.h"
#include "tinyfiledialogs.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>


extern CoordinateSetEntry* alive_cells;


int main(void) {
    
    printf("program started\n");
    setbuf(stdout, NULL);

    // initialise engine
    engine_init(GRID_WIDTH, GRID_HEIGHT);

    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT
                                        , "CGOL", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    Renderer renderer;
    render_init(&renderer, CELL_SIZE);
    render_resize(&renderer, WINDOW_WIDTH, WINDOW_HEIGHT); 
    printf("Renderer initialised\n");


    // i have no clue, don't ask.
    glClear(GL_COLOR_BUFFER_BIT);
    render_grid(&renderer, alive_cells);
    glfwSwapBuffers(window);

    int speed = 50;  // Start at half speed
    const int max_speed = 100;
    const int min_speed = 0;

    bool vsync = true;
    glfwSwapInterval(1); 

    // Last time we processed a step
    double last_step_time = glfwGetTime();
    int generation_count = 0;


    double previous_time = last_step_time;
    int generations_per_second = 0;
    int generations_last_second = 0;

    update_dashboard(0,0,0);
    
    bool was_space = false;
    bool was_l_pressed = false;
    double delay = (speed == max_speed) ? 0.0 : get_speed_delay(speed);

    while (!glfwWindowShouldClose(window)) {
        bool dashboard_needs_update = false;
        double now = glfwGetTime();

        // 1) Always poll events:
        glfwPollEvents();
        bool space_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool up_pressed    = glfwGetKey(window, GLFW_KEY_UP   ) == GLFW_PRESS;
        bool down_pressed  = glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS;
        bool l_pressed = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

        // 2) Update per-second counters
        if (now - previous_time > 1.0) {
            generations_per_second = generations_last_second;
            generations_last_second = 0;
            previous_time = now;
            dashboard_needs_update = true;
        }

        // 3) Adjust speed and vsync (remove embedded waits for consistent behavior)
        bool speed_changed = false;
        if (up_pressed && speed < max_speed) {
            speed++;
            speed_changed = true;
            if (speed >= 90 && vsync) {
                glfwSwapInterval(0);
                vsync = false;
            }
        }
        if (down_pressed && speed > min_speed) {
            speed--;
            speed_changed = true;
            if (speed < 90 && !vsync) {
                glfwSwapInterval(1);
                vsync = true;
            }
        }
        if (speed_changed) {
            delay = (speed == max_speed) ? 0.0 : get_speed_delay(speed);

            dashboard_needs_update = true;
        }

        if (l_pressed && !was_l_pressed) {
            const char *filter[] = { "*.rle" };
            const char *path = tinyfd_openFileDialog(
                "Load RLE", "", 1, filter, "RLE files", 0
            );
            if (path) {
                char input[100];
                int x = 0, y = 0;

                sscanf(tinyfd_inputBox("X coordinate", "Enter X position to load the RLE:", "20"), "%d", &x);
                sscanf(tinyfd_inputBox("Y coordinate", "Enter Y position to load the RLE:", "20"), "%d", &y);
                
                load_rle(path, x, y);
            }
        }
        was_l_pressed = l_pressed;

        // delay

        if (space_pressed && !was_space) {
            last_step_time = now - delay;
        }
        was_space = space_pressed;

        // delay logic
        bool did_step = false;
        if (space_pressed && (now - last_step_time >= delay)) {
            engine_step();
            ++generation_count;
            ++generations_last_second;
            last_step_time = now;
            did_step = true;
        }

        if ((speed_changed || did_step || dashboard_needs_update)) {
            update_dashboard(speed, generations_per_second, generation_count);
        }

        // 8) Render every frame
        glClear(GL_COLOR_BUFFER_BIT);
        render_grid(&renderer, alive_cells);
        glfwSwapBuffers(window);

        // 9) Throttle only when not at max speed
        if (speed != max_speed) {
            if (!did_step) {
                // cap loop to desired delay
                glfwWaitEventsTimeout(delay);
            }
        }
        // if at max_speed, immediately continue loop with no waits
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
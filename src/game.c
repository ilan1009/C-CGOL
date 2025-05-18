#include "game.h"
#include "coordinate.h"
#include "coordinate_set.h"
#include "engine.h"
#include "window.h"
#include "render.h"

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "tinyfiledialogs.h"


static double now;
static double last_speed_adjust_time = 0;


void load_rle(const char* filename, int start_x, int start_y) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open RLE file");
        return;
    }

    char line[1024];
    int x = 0, y = 0;
    bool rle_started = false;

    while (fgets(line, sizeof(line), file)) {
        // skip comments
        if (line[0] == '#') continue;

        // skip header
        if (!rle_started) {
            if (strstr(line, "x") && strstr(line, "y")) {
                rle_started = true;  // Next lines will be RLE data
            }
            continue;
        }
        if (start_x < 0 || start_x+x > GRID_WIDTH || start_y<0 || start_y+y > GRID_HEIGHT) return;

        // Parse RLE data
        const char* p = line;
        int run_count = 0;

        while (*p) {
            if (isdigit(*p)) {
                run_count = run_count * 10 + (*p - '0');
            } else if (*p == 'b' || *p == 'o') {
                if (run_count == 0) run_count = 1;

                for (int i = 0; i < run_count; ++i) {
                    if (*p == 'o') {
                        Coordinate c = { start_x + x, start_y + y };
                        birth_cell(c);
                    }
                    x++;
                }

                run_count = 0;
            } else if (*p == '$') {
                if (run_count == 0) run_count = 1;
                y += run_count;
                x = 0;
                run_count = 0;
            } else if (*p == '!') {
                fclose(file);
                return;
            }
            p++;
        }
    }

    fclose(file);
}

// black magic
double get_speed_delay(int speed) {
    if (speed <= 0) return 1.0;
    if (speed >= 100) return 0.0;
    
    if (speed <= 80) {
        double factor = (double)(80 - speed) / 80.0;
        return 1.0 * pow(factor, 1.5) + 0.01;
    } else {
        double factor = (double)(99 - speed) / 19.0;
        return 0.01 * factor + 0.001;
    }
}

void update_dashboard(int speed, int generations_per_second, int generation){
    printf(DASH_TEMPLATE, speed, generations_per_second, 
        generation);
}


bool handle_input(GLFWwindow *window, int *speed, bool *vsync, double *delay, bool *paused, bool *load_requested) {
    static bool prev_space = false;
    static bool prev_l = false;

    bool updated = false;

    bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool up = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    bool down = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    bool l = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;

    // pause toggle
    if (space && !prev_space) {
        *paused = !*paused;
        updated = true;
    }
    prev_space = space;

    //load
    if (l && !prev_l) {
        *load_requested = true;
    }
    prev_l = l;

    //speed
    if (up && *speed < MAX_SPEED && now - last_speed_adjust_time > 0.1) {
        (*speed) += 1; 
        updated = true;
        last_speed_adjust_time = now; 
    } else if (down && *speed > MIN_SPEED && now - last_speed_adjust_time > 0.1) {
        (*speed) -= 1; 
        updated = true;
        last_speed_adjust_time = now; 
    }

    // vsync 
    if (*speed > 80 && *vsync) {
        glfwSwapInterval(0); // Disable VSync
        *vsync = false;
    } else if (*speed <= 80 && !*vsync) {
        glfwSwapInterval(1); // Enable VSync
        *vsync = true;
    }

    // Recompute delay
    if (updated) {
        *delay = (*speed == MAX_SPEED) ? 0.0 : get_speed_delay(*speed);
    }

    return updated;
}

void load_rle_dialog(void) {
    const char *filter[] = { "*.rle" };
    const char *path = tinyfd_openFileDialog("Load RLE", "", 1, filter, "RLE files", 0);
    if (!path) return;

    int x = 0, y = 0;
    sscanf(tinyfd_inputBox("X coordinate", "Enter X position to load the RLE:", "20"), "%d", &x);
    sscanf(tinyfd_inputBox("Y coordinate", "Enter Y position to load the RLE:", "20"), "%d", &y);
    load_rle(path, x, y);
}


void game_loop(GLFWwindow *window, Renderer *renderer) {
    int speed = 50;
    bool vsync = true;
    double delay = (speed == MAX_SPEED) ? 0.0 : get_speed_delay(speed);
    double last_step_time = glfwGetTime();
    int generation_count = 0;

    double previous_time = last_step_time;
    int generations_per_second = 0;
    int generations_last_second = 0;

    update_dashboard(0, 0, 0);

    bool paused = true;
    bool load_requested = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        now = glfwGetTime();
        bool did_step = false;

        // Update per-second generation counter
        if (now - previous_time > 1.0) {
            generations_per_second = generations_last_second;
            generations_last_second = 0;
            previous_time = now;
            update_dashboard(speed, generations_per_second, generation_count);
        }

        if (handle_input(window, &speed, &vsync, &delay, &paused, &load_requested)) {
            update_dashboard(speed, generations_per_second, generation_count);
        }

        if (load_requested) {
            load_rle_dialog();
            load_requested = false;
        }

        // simulating
        if (!paused && (speed == MAX_SPEED || now - last_step_time >= delay)) {
            engine_step();
            ++generation_count;
            ++generations_last_second;
            last_step_time = now;
            did_step = true;
            if (speed != MAX_SPEED) {
                update_dashboard(speed, generations_per_second, generation_count);
            }
        }

        // rendering

        glClear(GL_COLOR_BUFFER_BIT);
        render_grid(renderer, alive_cells);
        glfwSwapBuffers(window);

        //throttle_loop(delay, speed, did_step);
    }
}


void throttle_loop(double delay, int speed, bool did_step) {
    if (speed != MAX_SPEED && !did_step) {
        glfwWaitEventsTimeout(delay);
    }
}

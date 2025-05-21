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


Message messages[MAX_MESSAGES];
static int bottom_message_index = 0;

Userstate user_state;
Gamestate game_state;
Renderstate render_state;


void load_rle(const char* path, int start_x, int start_y) {
    FILE* file = fopen(path, "r");
    if (!file) {
        init_message("failed to load rle");
        return;
    }
    int x = 0, y = 0;

    char line[1024];
    bool rle_started = false;

    while (fgets(line, sizeof(line), file)) {
        // skip comments
        if (line[0] == '#') continue;

        // process header
        if (!rle_started) {
            // example:
            // x = 3, y = 3, rule = B3/S23
            if (strstr(line, "x") && strstr(line, "y")) {
                rle_started = true;  // next lines will be RLE data
            }
            continue;
        }


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
                init_message("loaded rle");

                return;
                
            }
            p++;
        }
    }


    fclose(file);
}

// black magic if it was bad
double get_speed_delay() {
    int speed = user_state.speed; 

    if (speed <= 0) return 1.0;
    if (speed >= 100) return 0.0;

    if (speed <= 80) {
        double factor = (80.0 - speed) / 80.0;
        return pow(factor, 1.5) + 0.01;
    } else {
        double factor = (99.0 - speed) / 19.0;
        return 0.01 * factor + 0.001;
    }
}

void init_message(char* msg_content){
    if (bottom_message_index >= MAX_MESSAGES) {
        bottom_message_index = 0;
    }
    Message *message = &messages[bottom_message_index];
    message->msg_content = msg_content; // allocates and copies
    message->msg_start_time = glfwGetTime();
    bottom_message_index++;
}

void handle_messages(){
    for (int i = 0; i < MAX_MESSAGES; ++i){
        if (messages[i].msg_content == NULL) continue;

        if (now - messages[i].msg_start_time > 2.0) {
            messages[i].msg_content = NULL;
        }
        else {
            printf("%s\n", messages[i].msg_content);
        }
    }
}
#define DASH_TEMPLATE "Speed: %-3d\n"\
                        "Generations / s: %dHz\n"\
                        "Generation: %-6d\n"\
                        
void update_dashboard(){   
    printf("\033[H\033[J"); 
    printf(DASH_TEMPLATE, user_state.speed, render_state.generations_per_second, game_state.generation_count);
    printf("%s | %s\n", user_state.fast_forward ? "FAST FORWARD" : (user_state.paused ? "   PAUSED   " : " SIMULATING "), user_state.vsync ? "VSYNC" : "     ");
    handle_messages();
}


bool handle_input() {
    static bool prev_space = false;
    static bool prev_l = false;
    static bool prev_v = false;
    static bool prev_r = false;

    bool updated = false;

    bool space = glfwGetKey(render_state.window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool up = glfwGetKey(render_state.window, GLFW_KEY_UP) == GLFW_PRESS;
    bool down = glfwGetKey(render_state.window, GLFW_KEY_DOWN) == GLFW_PRESS;
    bool l = glfwGetKey(render_state.window, GLFW_KEY_L) == GLFW_PRESS;
    bool tab = glfwGetKey(render_state.window, GLFW_KEY_TAB) == GLFW_PRESS;
    bool v = glfwGetKey(render_state.window, GLFW_KEY_V) == GLFW_PRESS;
    bool r = glfwGetKey(render_state.window, GLFW_KEY_R) == GLFW_PRESS;

    // pause toggle
    if (space && !prev_space) {
        user_state.paused = !user_state.paused;
        updated = true;
    }
    prev_space = space;

    //load
    if (l && !prev_l) {
        user_state.load_requested = true;
    }
    prev_l = l;

    //reset simulation
    if (r && !prev_r) {
        user_state.reset_requested = true;
    }
    prev_r = r;

    //speed
    if (up && user_state.speed < MAX_SPEED && now - last_speed_adjust_time > 0.1) {
        user_state.speed += 1; 
        updated = true;
        last_speed_adjust_time = now; 
        game_state.delay = get_speed_delay();
    } else if (down && user_state.speed > MIN_SPEED && now - last_speed_adjust_time > 0.1) {
        user_state.speed -= 1; 
        updated = true;
        last_speed_adjust_time = now; 
        game_state.delay = get_speed_delay();
    }  

    // vsync
    if (user_state.fast_forward && user_state.vsync){
        updated = true;
        user_state.vsync = false;
        glfwSwapInterval(0);
    }
    if (v && !prev_v) {
        user_state.vsync = !user_state.vsync;
        glfwSwapInterval(user_state.vsync ? 1 : 0);
        updated = true;
    }
    prev_v = v;

    user_state.fast_forward = tab;
    
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

void init_game(GLFWwindow* window, Renderer* renderer){
    render_state.window = window;
    render_state.renderer = renderer;
    render_state.generations_per_second = 0;
    render_state.generations_last_second = 0;

    user_state.speed = 50;
    user_state.vsync = true;

    user_state.paused = true;
    user_state.fast_forward = false;
    user_state.load_requested = false;
    user_state.reset_requested = false;

    game_state.delay = get_speed_delay(user_state.speed);

    game_state.last_step_time = glfwGetTime();
    game_state.generation_count = 0;
    game_state.previous_time = glfwGetTime();   
}

void game_loop() {
    update_dashboard();

    while (!glfwWindowShouldClose(render_state.window)) {
        glfwPollEvents();

        now = glfwGetTime();

        // update gen/s
        if (now - game_state.previous_time > 1.0) {
            render_state.generations_per_second = render_state.generations_last_second;
            render_state.generations_last_second = 0;
            game_state.previous_time = now;
            update_dashboard();
        }

        if (handle_input()) {
            update_dashboard();
        }

        if (user_state.load_requested) {
            load_rle_dialog();
            user_state.load_requested = false;
            update_dashboard();
        }

        if (user_state.reset_requested) {
            reset_game();
            return;
        }

        // simulating
        if (user_state.fast_forward || (!user_state.paused &&  (now - game_state.last_step_time >= game_state.delay))) {
            engine_step();
            ++game_state.generation_count;
            ++render_state.generations_last_second;
            game_state.last_step_time = now;
            if (!user_state.fast_forward) {
                update_dashboard();
            }
        }

        // rendering

        glClear(GL_COLOR_BUFFER_BIT);
        render_grid(render_state.renderer, alive_cells);
        glfwSwapBuffers(render_state.window);

        //throttle_loop(delay, speed, did_step);
    }
}

void reset_game(){
    game_state.generation_count = 0;

    game_state.last_step_time = glfwGetTime();
    game_state.previous_time = glfwGetTime();   


    render_state.generations_per_second = 0;
    render_state.generations_last_second = 0;

    user_state.paused = true;
    user_state.fast_forward = false;
    user_state.load_requested = false;
    user_state.reset_requested = false;

    engine_cleanup();
    engine_init(GRID_WIDTH, GRID_HEIGHT);

    init_message("game reset");
    game_loop();
}
void throttle_loop(double delay, int speed, bool did_step) {
    if (speed != MAX_SPEED && !did_step) {
        glfwWaitEventsTimeout(delay);
    }
}

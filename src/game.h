#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "render.h"




#define INITIAL_SPEED 50;
#define MAX_SPEED 100
#define MIN_SPEED 0
#define VSYNC_THRESHOLD 90

typedef struct
{
    int speed;
    bool vsync;
    
    bool paused;
    bool fast_forward;

    bool load_requested;
    bool reset_requested;

} Userstate;

typedef struct
{
    double delay;

    double last_step_time;
    int generation_count;

    double previous_time;

} Gamestate;

typedef struct
{
    GLFWwindow* window;
    Renderer* renderer;

    int generations_per_second;
    int generations_last_second;
} Renderstate;



void load_rle(const char* filename, int start_x, int start_y);
double get_speed_delay();
void update_dashboard();

void init_game(GLFWwindow* window, Renderer* renderer);

void game_loop();
void throttle_loop(double delay, int speed, bool did_step);
bool handle_input();

void reset_game();


#endif
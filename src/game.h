#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "render.h"

#define DASH_TEMPLATE "\033[H\033[J"\
                        "Speed: %-3d\n"\
                        "Generations / s: %dHz\n"\
                        "Generation: %-6d"\


#define MAX_SPEED 100
#define MIN_SPEED 0
#define VSYNC_THRESHOLD 90

void load_rle(const char* filename, int start_x, int start_y);
double get_speed_delay(int speed);
void update_dashboard(int speed, int generations_per_second, int generation);
void game_loop(GLFWwindow *window, Renderer *renderer);
void throttle_loop(double delay, int speed, bool did_step);
bool handle_input(GLFWwindow *window, int *speed, bool *vsync, double *delay, bool *paused, bool *load_requested);


#endif
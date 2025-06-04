#include "window.h"

int WINDOW_WIDTH;
int WINDOW_HEIGHT;

int GRID_WIDTH;
int GRID_HEIGHT;

int CELL_SIZE;

void init_window_parameters(int window_size, int grid_size){
    WINDOW_WIDTH = window_size;
    WINDOW_HEIGHT = window_size;

    GRID_WIDTH = grid_size;
    GRID_HEIGHT = grid_size;

    CELL_SIZE = window_size / grid_size;
}
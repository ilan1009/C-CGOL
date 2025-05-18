#include "game.h"
#include "coordinate.h"
#include "coordinate_set.h"
#include "engine.h"
#include "window.h"

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

const char* dash_template = "\033[H\033[J"  // Clear screen
                            "Speed: %-3d\n"
                            "Simulation Speed: %dHz\n"
                            "Generation: %-6d";

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

double get_speed_delay(int speed) {
    if (speed <= 0) return 1.0;
    if (speed >= 100) return 0.0;
    
    if (speed <= 80) {
        // Exponential decrease from 1.0 to 0.01
        double factor = (double)(80 - speed) / 80.0;
        return 1.0 * pow(factor, 1.5) + 0.01;
    } else {
        // Linear decrease from 0.01 to 0.001
        double factor = (double)(99 - speed) / 19.0;
        return 0.01 * factor + 0.001;
    }
}

void update_dashboard(int speed, int generations_per_second, int generation){
    printf(dash_template, speed, generations_per_second, 
        generation);
}
#ifndef GAME_H
#define GAME_H

void load_rle(const char* filename, int start_x, int start_y);
double get_speed_delay(int speed);
void update_dashboard(int speed, int generations_per_second, int generation);

#endif
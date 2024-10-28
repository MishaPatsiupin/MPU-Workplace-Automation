#ifndef DISPLAY_H
#define DISPLAY_H

#include "globals.h"

struct waterind_settings {
    int type = -1; // 0 - auto, 1 - time
    int max_control_value_auto = 0;
    int time_s_auto = 0;
    int time_s_time = 0;
    int period_time = 0;
};

extern waterind_settings waterind_settings;

struct window_settings {
    int type = -1; //0 - auto, 1 - time, 2 - mix
    int low_temp_auto = 0;
    int max_temp_auto = 0;
    int time_time = 0;
    int periud_time = 0;
};

extern window_settings window_settings;

void display_debug_moisture1();
void display_debug_moisture2();
void display_debug_watering();
void display_debug_window();
void update_debug_moisture(int old_pos);
void update_debug_watering();
void update_debug_window();
void turn_on_backlight();
void turn_off_backlight();
void display_data_update(int string_pos, int string);
void display_data(int status, float temperature, int weather, int moisture1, int moisture2, int liquid_sensor_water, int liquid_sensor_plant);

extern void (*menu_functions[])();


void handle_backlight();
void handle_menu_navigation();
void handle_value_adjustments();
void handle_right_press_for_pos1();
void handle_right_press_for_pos2();
void handle_left_press_for_pos2();
void handle_right_press_for_pos3();
void handle_left_press_for_pos3();

void measure_air(int sensor_pin);
void measure_water(int sensor_pin);


#endif // DISPLAY_H
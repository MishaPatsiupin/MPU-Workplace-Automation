#ifndef DATA_CAPTURE_H
#define DATA_CAPTURE_H

#include "globals.h"
#include "display.h"




struct sensor_data {
    //bme280
    float temperature = 0;
    float humidity = 90;
    float pressure = 995;

    //moisture
    int moisture1 = 100;
    int moisture2 = 0;

    //liquid
    int liquid_sensor_water = 0;
    int liquid_sensor_plant = 0;
};
extern sensor_data now_sensor_data;

void init_devices();
sensor_data read_data_sensors();
float read_pressure();
float read_humidity();
float read_temperature();
int read_moisture(int pin);
int read_liquid_sensor_water();
int read_liquid_sensor_plant();
void servo_window_control(bool window_flag_local);
void window_control(float temp);
void pump_control(int moisture1, int moisture2);
void start_pump();
void stop_pump();
int update_status();
int check_weather_condition(float pressure);


#endif //DATA_CAPTURE_H

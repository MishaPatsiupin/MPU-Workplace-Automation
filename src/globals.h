#ifndef GLOBALS_H
#define GLOBALS_H
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EncButton.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>
#include <ServoSmooth.h>

//---------- ПИНЫ ----------

// Шина i2c
#define MY_SDA_PIN 21
#define MY_SCL_PIN 22

// Энкодер
#define MY_SW 16
#define MY_DT 17
#define MY_CLK 5

// Устройства исполнители
#define WATER_PUMP 32
#define WINDOW 33

// Датчики влажности почвы
#define CS_1 34
// Переменные калибровки
extern int moisture1_air;
extern int moisture1_water;
#define CS_2 35
extern int moisture2_air;
extern int moisture2_water;

// Датчики наличия жидкости
#define LIQUID_SENSOR_PLANT 26
#define LIQUID_SENSOR_WATER 25


//------------АДРЕСА I2C УСТРОЙСТВ------------
#define MY_LCD2004_ADDRESS 0x27
#define MY_BME280_ADDRESS 0x76
#define MY_RTC_ADDRESS 0x68

//----------- ОБЪЕКТЫ И ПЕРЕМЕННЫЕ -----------
extern LiquidCrystal_I2C lcd;
extern Adafruit_BME280 bme;
extern RTC_DS1307 rtc;
extern ServoSmooth servo_window;
extern EncButton eb;

extern int pos;
extern bool in_menu;
extern int current_menu;
extern byte custom_wind[];
extern byte custom_sun[];
extern byte grad[];

extern long interval;
extern unsigned long previous_millis;

extern int control_moisture1_value;
extern int control_moisture2_value;

extern bool window_flag;
extern unsigned long last_window_control_time;
















#endif //GLOBALS_H

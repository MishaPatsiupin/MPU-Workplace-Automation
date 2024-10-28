#include "globals.h"

//определение устройств
LiquidCrystal_I2C lcd(MY_LCD2004_ADDRESS, 20, 4);
EncButton eb(16, 17, 5);

//главный таймер
long interval = 2000;
unsigned long previous_millis = 0;

//переменные для работы с меню
int pos = 0; // Текущая позиция
bool in_menu = false; // Флаг для отслеживания состояния меню
int current_menu = 0; // Индекс текущего меню

// Контрольные значения
int control_moisture1_value = 0;
int control_moisture2_value = 0;

// Пользовательские символы
byte custom_wind[] = {B11100, B00011, B11000, B00111, B00000, B11100, B00011, B11000};
byte custom_sun[] = {B00000,B10001,B01010,B11011,B00100,B11011,B01010,B10001};
byte grad[] = {B10000,B00110,B01000,B01000,B01000,B00110,B00000,B00000};





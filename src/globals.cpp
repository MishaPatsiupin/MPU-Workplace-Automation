//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

// Подключение глобальных переменных
#include "globals.h"

//определение устройств
RTC_DS1307 rtc; // Устройство для работы с RTC
Adafruit_BME280 bme; // Устройство для работы с датчиком BME280
LiquidCrystal_I2C lcd(MY_LCD2004_ADDRESS, 20, 4); // Устройство для работы с дисплеем
ServoSmooth servo_window; // Устройство для работы с сервоприводом окна
EncButton eb (MY_SW, MY_DT, MY_CLK); // Устройство для работы с энкодером

//главный таймер
long interval = 2000; // Интервал таймера
unsigned long previous_millis = 0; // Предыдущее значение времени

//переменные для работы с меню
int pos = 0; // Текущая позиция
bool in_menu = false; // Флаг для отслеживания состояния меню
int current_menu = 0; // Индекс текущего меню

// Контрольные значения
int control_moisture1_value = 35; // Контрольное значение влажности 1
int control_moisture2_value = 35; // Контрольное значение влажности 2
bool pump_flag = false; // Флаг состояния помпы
unsigned long last_pump_control_time = 0; // Время последнего управления помпой

// Символы облочности, солнечной погоды, градус
byte custom_wind[] = {B11100, B00011, B11000, B00111, B00000, B11100, B00011, B11000}; // Символ ветра
byte custom_sun[] = {B00000,B10001,B01010,B11011,B00100,B11011,B01010,B10001}; // Символ солнца
byte grad[] = {B10000,B00110,B01000,B01000,B01000,B00110,B00000,B00000}; // Символ градуса

// Контрольные переменные для работы с датчиками влажности почвы
int moisture1_air = 2639; // Значение с датчика в воздухе датчика 1
int moisture1_water = 995; // Значение с датчика в воде датчика 1
int moisture2_air = 2590; // Значение с датчика в воздухе датчика 2
int moisture2_water = 930; // Значение с датчика в воде датчика 2

// Переменные для работы с окном
bool window_flag = false; // Флаг состояния окна
unsigned long last_window_control_time = 0; // Время последнего управления окном
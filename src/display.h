//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

#ifndef DISPLAY_H
#define DISPLAY_H

#include "globals.h"  // Подключение глобальных переменных

// Структура для хранения настроек полива
struct waterind_settings {
    int type = -1;  // Тип полива: 0 - авто, 1 - по времени
    int max_control_value_auto = 0;  // Максимальное значение контроля для авто режима
    int time_s_auto = 0;  // Время полива в авто режиме
    int time_s_time = 0;  // Время полива в режиме по времени
    int period_time = 0;  // Период времени для режима по времени
};

extern struct waterind_settings waterind_settings;  // Экземпляр структуры для хранения настроек полива

// Структура настроек освящения
struct relay_settings {
    int type = -1;  // Тип управления окном: 0 - отключено, 1 - по времени
    int start_hour = 7;  // Час начала
    int start_minute = 0;  // Минута начала
    int end_hour = 8;  // Час конца
    int end_minute = 0;  // Минута конца
    bool relay_flag = false;  // Флаг состояния реле
};

extern struct relay_settings relay_settings;  // Экземпляр структуры для хранения настроек окна

struct old_data {
    char date_time[17];
    int status;
    float temperature;
    int weather;
    int moisture1;
    int moisture2;
    int liquid_sensor_water;
    int liquid_sensor_plant;
};
extern struct old_data old_data;  // Экземпляр структуры для хранения старых данных

//Функция: отображение отладочной информации о влажности 1
//Принимает: -
//Возвращает: -
void display_debug_moisture1();

//Функция: отображение отладочной информации о влажности 2
//Принимает: -
//Возвращает: -
void display_debug_moisture2();

//Функция: отображение отладочной информации о поливе
//Принимает: -
//Возвращает: -
void display_debug_watering();

//Функция: отображение отладочной информации об окне
//Принимает: -
//Возвращает: -
void display_debug_relay();

//Функция: обновление отладочной информации о влажности
//Принимает: старую позицию
//Возвращает: -
void update_debug_moisture(int old_pos);

//Функция: обновление отладочной информации о поливе
//Принимает: -
//Возвращает: -
void update_debug_watering();

//Функция: обновление отладочной информации об окне
//Принимает: -
//Возвращает: -
void update_debug_relay();

//Функция: включение подсветки
//Принимает: -
//Возвращает: -
void turn_on_backlight();

//Функция: выключение подсветки
//Принимает: -
//Возвращает: -
void turn_off_backlight();

//Функция: обновление данных на дисплее
//Принимает: позицию строки, строку
//Возвращает: -
void display_data_update(int string_pos, int string);//а где она

//Функция: отображение данных на дисплее
//Принимает: статус, температура, состояние погоды, влажность 1, влажность 2, состояние датчика воды, состояние датчика жидкости в растении
//Возвращает: -
void display_data(int status, float temperature, int weather, int moisture1, int moisture2, int liquid_sensor_water, int liquid_sensor_plant);

extern void (*menu_functions[])();  // Массив указателей на функции меню

//Функция: обработка подсветки
//Принимает: -
//Возвращает: -
void handle_backlight();

//Функция: обработка навигации по меню
//Принимает: -
//Возвращает: -
void handle_menu_navigation();

//Функция: обработка изменения значений
//Принимает: -
//Возвращает: -
void handle_value_adjustments();

//Функция: обработка нажатия вправо для позиции 1
//Принимает: -
//Возвращает: -
void handle_right_press_for_pos1();

//Функция: обработка нажатия вправо для позиции 2
//Принимает: -
//Возвращает: -
void handle_right_press_for_pos2();

//Функция: обработка нажатия влево для позиции 2
//Принимает: -
//Возвращает: -
void handle_left_press_for_pos2();

//Функция: обработка нажатия вправо для позиции 3
//Принимает: -
//Возвращает: -
void handle_right_press_for_pos3();

//Функция: обработка нажатия влево для позиции 3
//Принимает: -
//Возвращает: -
void handle_left_press_for_pos3();

//Функция: измерение влажности воздуха
//Принимает: номер пина
//Возвращает: -
void measure_air(int sensor_pin);

//Функция: измерение влажности воды
//Принимает: номер пина
//Возвращает: -
void measure_water(int sensor_pin);

void update_relay_flag();

#endif // DISPLAY_H
// Патюпин М.С. ГР250503 КП
// Микропроцессорное устройство контроля параметров тепличного комбината

#ifndef DATA_CAPTURE_H
#define DATA_CAPTURE_H

#include "globals.h"  // Подключение глобальных переменных
#include "display.h"  // Подключение файла для обновления дисплеем

// Структура для хранения данных с датчиков
struct sensor_data {
    // bme280
    float temperature = 0;      // Температура
    float humidity = 90;        // Влажность
    float pressure = 995;       // Давление

    // Влажность почвы
    int moisture1 = 100;        // Влажность почвы 1
    int moisture2 = 0;          // Влажность почвы 2

    // Датчики жидкости
    int liquid_sensor_water = 0;  // Датчик наличия воды
    int liquid_sensor_plant = 0;   // Датчик наличия жидкости в растении
};

extern sensor_data now_sensor_data;  // Экземпляр структуры для хранения текущих данных с датчиков

// Функция: инициализация устройств
// Принимает: -
// Возвращает: -
void init_devices();

// Функция: считывание данных с датчиков
// Принимает: -
// Возвращает: данные с датчиков
sensor_data read_data_sensors();

// Функция: считывание давления
// Принимает: -
// Возвращает: значение давления
float read_pressure();

// Функция: считывание влажности
// Принимает: -
// Возвращает: значение влажности
float read_humidity();

// Функция: считывание температуры
// Принимает: -
// Возвращает: значение температуры
float read_temperature();

// Функция: считывание влажности почвы
// Принимает: номер пина
// Возвращает: процент влажности почвы
int read_moisture(int pin);

// Функция: считывание состояния датчика наличия воды
// Принимает: -
// Возвращает: состояние датчика наличия воды
int read_liquid_sensor_water();

// Функция: считывание состояния датчика наличия жидкости в растении
// Принимает: -
// Возвращает: состояние датчика наличия жидкости в растении
int read_liquid_sensor_plant();

// Функция: управление сервоприводом окна
// Принимает: состояние окна
// Возвращает: -
void servo_window_control(bool window_flag_local, bool init = false);

// Функция: управление окном в зависимости от температуры
// Принимает: температура
// Возвращает: -
void window_control(float temp);

// Функция: управление насосом
// Принимает: влажность 1, влажность 2, состояние датчика воды, состояние датчика потопа
// Возвращает: -
void pump_control(int moisture1, int moisture2, int water_sensor, int flood_sensor);

// Функция: включение помпы
// Принимает: -
// Возвращает: -
void start_pump();

// Функция: выключение помпы
// Принимает: -
// Возвращает: -
void stop_pump();

// Функция: обновление статуса
// Принимает: -
// Возвращает: статус
int update_status();

// Функция: проверка погодных условий
// Принимает: давление
// Возвращает: состояние погоды
int check_weather_condition(float pressure);

#endif // DATA_CAPTURE_H
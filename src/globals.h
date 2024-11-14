//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

#ifndef GLOBALS_H
#define GLOBALS_H

// Подключение библиотек для работы с различными устройствами
#include <Wire.h> // Библиотека для работы с шиной I2C
#include <LiquidCrystal_I2C.h> // Библиотека для работы с LCD дисплеем
#include <EncButton.h> // Библиотека для работы с энкодером
#include <Adafruit_BME280.h> // Библиотека для работы с датчиком BME280
#include <RTClib.h> // Библиотека для работы с RTC
#include <ServoSmooth.h> // Библиотека для работы с сервоприводом

//---------- ПИНЫ ----------

// Шина i2c
#define MY_SDA_PIN 21 // Пин для SDA
#define MY_SCL_PIN 22 // Пин для SCL

// Энкодер
#define MY_SW 5 // Пин для SW энкодера
#define MY_DT 17 // Пин для DT энкодера
#define MY_CLK 16 // Пин для кнопки энкодера

// Устройства исполнители
#define WATER_PUMP 32 // Пин для управления помпой
#define WINDOW 33 // Пин для управления окном

// Датчики влажности почвы
#define CS_1 34 // Пин для первого датчика влажности почвы
// Переменные калибровки для первого датчика влажности почвы
extern int moisture1_air; // Значение влажности воздуха для первого датчика
extern int moisture1_water; // Значение влажности воды для первого датчика
#define CS_2 35 // Пин для второго датчика влажности почвы
// Переменные калибровки для второго датчика влажности почвы
extern int moisture2_air; // Значение влажности воздуха для второго датчика
extern int moisture2_water; // Значение влажности воды для второго датчика

// Датчики наличия жидкости
#define LIQUID_SENSOR_PLANT 26 // Пин для датчика наличия жидкости в растении
#define LIQUID_SENSOR_WATER 25 // Пин для датчика наличия воды

//------------АДРЕСА I2C УСТРОЙСТВ------------
#define MY_LCD2004_ADDRESS 0x27 // Адрес LCD дисплея
#define MY_BME280_ADDRESS 0x76 // Адрес датчика BME280
#define MY_RTC_ADDRESS 0x68 // Адрес RTC

//----------- ОБЪЕКТЫ И ПЕРЕМЕННЫЕ -----------
// Объекты для работы с различными устройствами
extern LiquidCrystal_I2C lcd; // Объект для работы с LCD дисплеем
extern Adafruit_BME280 bme; // Объект для работы с датчиком BME280
extern RTC_DS1307 rtc; // Объект для работы с RTC
extern ServoSmooth servo_window; // Объект для работы с сервоприводом окна
extern EncButton eb; // Объект для работы с энкодером

// Переменные для работы с меню
extern int pos; // Текущая позиция в меню
extern bool in_menu; // Флаг для отслеживания состояния меню
extern int current_menu; // Индекс текущего меню

// Пользовательские символы для отображения на дисплее
extern byte custom_wind[]; // Символ ветра
extern byte custom_sun[]; // Символ солнца
extern byte grad[]; // Символ градуса

// Главный таймер
extern long interval; // Интервал таймера
extern unsigned long previous_millis; // Предыдущее время события

// Контрольные значения для датчиков влажности почвы
extern int control_moisture1_value; // Контрольное значение влажности 1
extern int control_moisture2_value; // Контрольное значение влажности 2
extern bool pump_flag; // Флаг состояния помпы
extern unsigned long last_pump_control_time; // Время последнего управления помпой

// Переменные для работы с окном
extern bool window_flag; // Флаг состояния окна
extern unsigned long last_window_control_time; // Время последнего управления окном

#endif //GLOBALS_H
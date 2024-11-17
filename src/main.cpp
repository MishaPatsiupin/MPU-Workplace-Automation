//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

// Подключение библиотеки для работы с API
#include <api.h>

// Подключение заголовочных файлов
#include "display.h" // Заголовочный файл для работы с дисплеем
#include "globals.h" // Заголовочный файл с глобальными переменными
#include "data_capture.h" // Заголовочный файл для работы с данными

// Структура для хранения настроек окна
struct window_settings window_settings;

// Структура для хранения настроек полива
struct waterind_settings waterind_settings;

// Структура для хранения данных с датчиков
sensor_data now_sensor_data;

//Функция: инициализация устройств и подключение к Wi-Fi
//Принимает: -
//Возвращает: -
void setup() {
    Wire.begin(MY_SDA_PIN, MY_SCL_PIN); // Инициализация шины I2C
    Serial.begin(115200); // Инициализация последовательного порта
    init_devices(); // Инициализация устройств
    stop_pump();
    servo_window_control(false, true);

    // Создание задачи для отправки данных по Wi-Fi на втором ядре
    xTaskCreatePinnedToCore(
        sendDataTask,   // Функция задачи
        "SendDataTask", // Имя задачи
        10000,          // Размер стека задачи
        NULL,           // Параметр задачи
        1,              // Приоритет задачи
        NULL,           // Дескриптор задачи
        1               // Ядро, на котором будет выполняться задача (1 - второе ядро)
    );
}

//Функция: основной цикл программы
//Принимает: -
//Возвращает: -
void loop() {
    eb.tick(); // Обработка событий энкодера
    servo_window.tick(); // Обработка событий сервопривода окна

    unsigned long current_millis = millis(); // Текущее время работы

    if (current_millis - previous_millis >= interval) {
        previous_millis = current_millis;

        now_sensor_data = read_data_sensors(); // Чтение данных с датчиков
        window_control(now_sensor_data.temperature); // Управление окном на основе температуры
        pump_control(now_sensor_data.moisture1, now_sensor_data.moisture2, now_sensor_data.liquid_sensor_water, now_sensor_data.liquid_sensor_plant); // Управление помпой на основе данных с датчиков

        if (!in_menu) {
            display_data(update_status(), now_sensor_data.temperature, check_weather_condition(now_sensor_data.pressure), now_sensor_data.moisture1, now_sensor_data.moisture2,
                         now_sensor_data.liquid_sensor_water, now_sensor_data.liquid_sensor_plant); // Обновление данных на дисплее
        }
    }

    handle_backlight(); // Обновление подсветки дисплея
    handle_menu_navigation(); // Обработка навигации по меню
    handle_value_adjustments(); // Обработка изменения значений
}
#include <api.h>

#include "display.h"
#include "globals.h"
#include "data_capture.h"

struct window_settings window_settings;
struct waterind_settings waterind_settings;
sensor_data now_sensor_data;

void setup() {
    Wire.begin(MY_SDA_PIN, MY_SCL_PIN);
    Serial.begin(115200);
    init_devices();


    // Подключение к Wi-Fi
    connectToWiFi();
    Serial.print("API доступно по адресу: http://");
    Serial.println(WiFi.localIP());
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

void loop() {
    eb.tick();
    servo_window.tick();

    unsigned long current_millis = millis();

    if (current_millis - previous_millis >= interval) {
        previous_millis = current_millis;

        now_sensor_data = read_data_sensors();
        window_control(now_sensor_data.temperature);
        pump_control(now_sensor_data.moisture1, now_sensor_data.moisture2, now_sensor_data.liquid_sensor_water, now_sensor_data.liquid_sensor_plant);
        if (!in_menu) {
            display_data(update_status(), now_sensor_data.temperature, check_weather_condition(now_sensor_data.pressure), now_sensor_data.moisture1, now_sensor_data.moisture2,
                         now_sensor_data.liquid_sensor_water, now_sensor_data.liquid_sensor_plant);
        }
    }

    handle_backlight();
    handle_menu_navigation();
    handle_value_adjustments();
}

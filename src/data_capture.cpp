#include "data_capture.h"

void init_devices() {
    //инциализация дисплея
    lcd.begin(20, 04);
    turn_off_backlight();
    lcd.createChar(0, custom_wind);
    lcd.createChar(1, custom_sun);
    lcd.createChar(2, grad);

    //Инициализация датчика BME280
    if (!bme.begin(MY_BME280_ADDRESS)) {
        Serial.println("Could not find BME280");
        lcd.clear();
        lcd.print("BME280 not found");
    } else {
        Serial.println("BME280 initialized successfully");
    }

    //Инициализация RTC
    if (!rtc.begin()) {
        Serial.println("Could not find RTC");
        lcd.clear();
        lcd.print("BME280 not found");
    } else {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("RTC initialized successfully");
    }

    //Инициализация датчиков влажности почвы
    pinMode(CS_1, INPUT);
    pinMode(CS_2, INPUT);
    //Инициализация датчиков наличия жидкости
    pinMode(LIQUID_SENSOR_PLANT, INPUT);
    pinMode(LIQUID_SENSOR_WATER, INPUT);

    //Инициализация устройств исполнителей
    pinMode(WATER_PUMP, OUTPUT);
    servo_window.attach(WINDOW);
    servo_window.setSpeed(130); // ограничить скорость
    servo_window.setAccel(0.1); // установить ускорение (разгон и торможение)
}

sensor_data read_data_sensors() {
    sensor_data data{};
    data.temperature = read_temperature();
    data.humidity = read_humidity();
    data.pressure = read_pressure();

    data.moisture1 = read_moisture(CS_1);
    data.moisture2 = read_moisture(CS_2);

    data.liquid_sensor_water = read_liquid_sensor_water();
    data.liquid_sensor_plant = read_liquid_sensor_plant();

    return data;
}

float read_temperature() {
    return bme.readTemperature();
}

float read_humidity() {
    return bme.readHumidity();
}

float read_pressure() {
    return (bme.readPressure() / 100.0F) * 0.75006F;
}

int read_moisture(int pin) {
    int measured_val = analogRead(pin);
    int soil_moisture_percent = map(measured_val, (pin == CS_1) ? moisture1_air : moisture2_air,
                                    (pin == CS_1) ? moisture1_water : moisture2_water, 0, 100);

    if (soil_moisture_percent < 0 or soil_moisture_percent > 100) {
        Serial.print("Error moisture ");
        Serial.print(pin == CS_1 ? 1 : 2);
        Serial.print(" -> ");
        Serial.println(soil_moisture_percent);
        soil_moisture_percent = -1;
    }
    return soil_moisture_percent;
}

int read_liquid_sensor_water() {
    return digitalRead(LIQUID_SENSOR_WATER);
}

int read_liquid_sensor_plant() {
    return digitalRead(LIQUID_SENSOR_PLANT);
}

void measure_air(int sensor_pin) {
    int measured_val = 0;
    int highest_val = 0;
    unsigned long previous_millis_air = 0;
    const long interval_air = 500; // Интервал обновления в миллисекундах

    for (int i = 0; i < 10; i++) {
        unsigned long current_millis = millis();
        if (current_millis - previous_millis_air >= interval_air) {
            previous_millis_air = current_millis;
            measured_val = analogRead(sensor_pin);
            if (measured_val > highest_val) {
                highest_val = measured_val;
            }
        }
    }
    Serial.print("Highest current AIR value: ");
    Serial.println(highest_val);

    switch (sensor_pin) {
        case CS_1:
            moisture1_air = highest_val;
            break;
        case CS_2:
            moisture2_air = highest_val;
            break;
    }
}

void measure_water(int sensor_pin) {
    int lowest_val = 30000;
    int measured_val = 0;
    unsigned long previous_millis_water = 0;
    const long interval_water = 500; // Интервал обновления в миллисекундах

    for (int i = 0; i < 10; i++) {
        unsigned long current_millis = millis();
        if (current_millis - previous_millis_water >= interval_water) {
            previous_millis_water = current_millis;
            measured_val = analogRead(sensor_pin);
            if (measured_val < lowest_val) {
                lowest_val = measured_val;
            }
        }
    }
    Serial.print("Lowest current WATER value: ");
    Serial.println(lowest_val);

    switch (sensor_pin) {
        case CS_1:
            moisture1_water = lowest_val;
            break;
        case CS_2:
            moisture2_water = lowest_val;
            break;
    }
}

bool window_open_due_to_temp = false;

void window_control(float temp) {
    unsigned long current_time = millis();

    switch (window_settings.type) {
        case 0: {
            Serial.println("Mode: Auto");
            if (temp < window_settings.low_temp_auto) {
                Serial.println("Temperature below low_temp_auto, closing window");
                window_flag = false;
                servo_window_control(false);
            } else if (temp > window_settings.max_temp_auto) {
                Serial.println("Temperature above max_temp_auto, opening window");
                window_flag = true;
                servo_window_control(true);
            }
            break;
        }
        case 1: {
            Serial.println("Mode: Time-based");
            unsigned long time_time_ms = window_settings.time_time * 1000;
            unsigned long periud_time_ms = window_settings.periud_time * 1000;

            Serial.print("Time time (ms): ");
            Serial.println(time_time_ms);
            Serial.print("Period time (ms): ");
            Serial.println(periud_time_ms);

            if (window_flag) {
                Serial.println("Window is currently open");
                if (current_time - last_window_control_time >= time_time_ms) {
                    Serial.println("Time to close the window");
                    window_flag = false;
                    servo_window_control(false);
                    last_window_control_time = current_time;
                } else {
                    Serial.println("Not yet time to close the window");
                }
            } else {
                Serial.println("Window is currently closed");
                if (current_time - last_window_control_time >= periud_time_ms) {
                    Serial.println("Time to open the window");
                    window_flag = true;
                    servo_window_control(true);
                    last_window_control_time = current_time;
                } else {
                    Serial.println("Not yet time to open the window");
                }
            }
            break;
        }
        case 2: {
            Serial.println("Mode: Mix");
            unsigned long time_time_ms = window_settings.time_time * 1000;
            unsigned long periud_time_ms = window_settings.periud_time * 1000;

            Serial.print("Time time (ms): ");
            Serial.println(time_time_ms);
            Serial.print("Period time (ms): ");
            Serial.println(periud_time_ms);

            if (window_flag) {
                Serial.println("Window is currently open");
                if (current_time - last_window_control_time >= time_time_ms) {
                    Serial.println("Time to close the window");
                    window_flag = false;
                    window_open_due_to_temp = false;
                    servo_window_control(false);
                    last_window_control_time = current_time;
                } else {
                    Serial.println("Not yet time to close the window");
                }
            } else {
                Serial.println("Window is currently closed");
                if (temp > window_settings.max_temp_auto) {
                    Serial.println("Temperature above max_temp_auto, opening window");
                    window_flag = true;
                    window_open_due_to_temp = true;
                    servo_window_control(true);
                    last_window_control_time = current_time;
                } else if (current_time - last_window_control_time >= periud_time_ms) {
                    Serial.println("Time to open the window");
                    window_flag = true;
                    window_open_due_to_temp = false;
                    servo_window_control(true);
                    last_window_control_time = current_time;
                } else {
                    Serial.println("Not yet time to open the window");
                }
            }

            // Additional logic to keep the window open if the temperature is above max_temp_auto
            if (window_flag && temp > window_settings.max_temp_auto) {
                Serial.println("Temperature above max_temp_auto, keeping window open");
                window_open_due_to_temp = true;
            } else if (window_open_due_to_temp && temp < window_settings.low_temp_auto) {
                Serial.println("Temperature below low_temp_auto, closing window");
                window_flag = false;
                window_open_due_to_temp = false;
                servo_window_control(false);
                last_window_control_time = current_time;
            }
            break;
        }
    }
}

void servo_window_control(bool window_flag_local) {
    servo_window.setTargetDeg(window_flag_local ? 0 : 270);
    Serial.println("Window state: " + String(window_flag_local ? "open" : "closed"));
}

void pump_control(int moisture1, int moisture2) {
    unsigned long current_time = millis();

    switch (waterind_settings.type) {
        case 0: { // Auto mode
            Serial.println("Mode: Auto");
            if (moisture1 > control_moisture1_value || moisture2 > control_moisture2_value) {
                if (!pump_flag) {
                    Serial.println("Moisture below control value, starting pump");
                    pump_flag = true;
                    last_pump_control_time = current_time;
                    start_pump();
                }
            }

            if (pump_flag) {
                if (moisture1 >= waterind_settings.max_control_value_auto || moisture2 >= waterind_settings.max_control_value_auto) {
                    Serial.println("Moisture above max control value, stopping pump");
                    pump_flag = false;
                    stop_pump();
                } else if (current_time - last_pump_control_time >= waterind_settings.time_s_auto * 1000) {
                    Serial.println("Auto watering time elapsed, stopping pump");
                    pump_flag = false;
                    stop_pump();
                }
            }
            break;
        }
        case 1: { // Time-based mode
            Serial.println("Mode: Time-based");
            unsigned long period_time_ms = waterind_settings.period_time * 1000;
            unsigned long time_s_time_ms = waterind_settings.time_s_time * 1000;

            if (!pump_flag && current_time - last_pump_control_time >= period_time_ms) {
                Serial.println("Period time elapsed, starting pump");
                pump_flag = true;
                last_pump_control_time = current_time;
                start_pump();
            }

            if (pump_flag && current_time - last_pump_control_time >= time_s_time_ms) {
                Serial.println("Watering time elapsed, stopping pump");
                pump_flag = false;
                stop_pump();
            }
            break;
        }
    }
}

void start_pump() {
    // Логика включения помпы
    digitalWrite(WATER_PUMP, HIGH);
    Serial.println("Pump is ON");
}

void stop_pump() {
    // Логика выключения помпы
    digitalWrite(WATER_PUMP, LOW);
    Serial.println("Pump is OFF");
}

int update_status() {
    if (pump_flag && window_flag) {
        return 3; // "wat_vent"
    } else if (pump_flag) {
        return 1; // "watering"
    } else if (window_flag) {
        return 2; // "vent"
    } else {
        return 0; // "wait"
    }
}

int check_weather_condition(float pressure) {
    // Диапазоны давления (примерные значения, могут быть изменены)
    const float cloudy_or_rainy_min = 710.0;
    const float cloudy_or_rainy_max = 745.0;
    const float sunny_min = 745.1;
    const float sunny_max = 790.0;
    Serial.println("Pressure: " + String(pressure));

    if (pressure >= cloudy_or_rainy_min && pressure <= cloudy_or_rainy_max) {
        return 1; // Облачно или дождь
    } else if (pressure >= sunny_min && pressure <= sunny_max) {
        return 0; // Солнечно
    } else {
        // Значение давления вне известных диапазонов
        return -1; // Неизвестное состояние
    }
}
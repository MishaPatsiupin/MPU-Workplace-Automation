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


// struct window_settings {
//     int type = -1; //0 - auto, 1 - time, 2 - mix
//     int low_temp_auto = 0;
//     int max_temp_auto = 0;
//     int time_time = 0;
//     int periud_time = 0;
// };


void window_control(float temp) {
    unsigned long current_time = millis();
    Serial.print("Current time: ");
    Serial.println(current_time);
    Serial.print("Window settings type: ");
    Serial.println(window_settings.type);
    Serial.print("Temperature: ");
    Serial.println(temp);

    switch (window_settings.type) {
        case 0: {
            Serial.println("Mode: Auto");
            if (temp < window_settings.low_temp_auto) {
                Serial.println("Temperature below low_temp_auto, opening window");
                window_flag = true;
                servo_window_control(true);
            } else if (temp > window_settings.max_temp_auto) {
                Serial.println("Temperature above max_temp_auto, closing window");
                window_flag = false;
                servo_window_control(false);
            } else {
                Serial.println("Temperature within range, no action taken");
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
    }
}

void servo_window_control(bool window_flag_local) {
    servo_window.setTargetDeg(window_flag_local ? 0 : 270);
    Serial.println("Window state: " + window_flag ? "open" : "closed");
}

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
    servo_window.setSpeed(130);    // ограничить скорость
    servo_window.setAccel(0.1);    // установить ускорение (разгон и торможение)

}



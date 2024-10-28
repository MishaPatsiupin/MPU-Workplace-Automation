#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EncButton.h>

#define MY_LCD2004_ADDRESS 0x27
LiquidCrystal_I2C lcd(MY_LCD2004_ADDRESS, 20, 4);
EncButton eb(16, 17, 5);

const char *my_status[] = {
    "wait",
    "watering",
    "vent",
    "wat_vent"
};

const char *debug_moisture1[] = {
    "DEBUG M1: ",
    "set value air",
    "set value water",
    "set control val"
};

const char *debug_moisture2[] = {
    "DEBUG M2: ",
    "set value air",
    "set value water",
    "set control val"
};

struct waterind_settings {
    int type = -1; // 0-auto, 1-time
    int max_control_value_auto = 0;
    int time_s_auto = 0;
    int time_s_time = 0;
    int period_time = 0;
};

waterind_settings waterind_settings;

const char *type_watering[] = {
    "auto",
    "time"
};

const char *debug_watering[] = {
    "DEBUG WATERING: ",
    "type: ",
    "", // Пустая строка для max %
    "" // Пустая строка для time
};

const char *debug_watering_auto[] = {
    "max %: ", // от большего из control_moisture1_value и control_moisture2_value до 100, шаг 5
    "time: " // шаг 15
};

const char *debug_watering_time[] = {
    "time: ", // шаг 15 c
    "period: " // шаг 30 m
};

struct window_settings {
    int type = -1; // 0-auto, 1-time, 2-mix
    int low_temp_auto = 0;
    int max_temp_auto = 0;
    int time_time = 0;
    int periud_time = 0;
};

window_settings window_settings;

const char *type_window[] = {
    "auto",
    "time",
    "mix"
};

const char *debug_window[] = {
    "DEBUG WINDOW: ",
    "type: ",
    "",
    ""
};

const char *debug_window_auto[] = {
    "Low 'C: ", // от -40 до 50, шаг 5
    "Max 'C: " // от -40 до 50, шаг 5
};

const char *debug_window_time[] = {
    "time: ", // шаг 5
    "period: " // шаг 30
};

long interval = 2000;
unsigned long previousMillis = 0;
int pos = 0; // Текущая позиция
bool inMenu = false; // Флаг для отслеживания состояния меню

// Глобальные переменные для значений
int control_moisture1_value = 0;
int control_moisture2_value = 0;

// Предварительные объявления функций
void display_debug_moisture1();

void display_debug_moisture2();

void display_debug_watering();

void display_debug_window();

void update_debug_moisture(int oldPos);

void update_debug_watering();

void update_debug_window();

void turnOnBacklight();

void turnOffBacklight();

void display_data_update(int string_pos, int string);

void displayData(int status, float temperature, int weather, int moisture1, int moisture2, int liquid_sensor_water,
                 int liquid_sensor_plant);


// Указатели на функции для меню
void (*menuFunctions[])() = {
    display_debug_moisture1, display_debug_moisture2, display_debug_watering, display_debug_window
};

int currentMenu = 0; // Индекс текущего меню

void setup() {
    Wire.begin(21, 22);
    Serial.begin(115200);
    lcd.begin(20, 04);
    turnOffBacklight();
    eb.counter = 0;
}

void loop() {
    eb.tick();
    unsigned long currentMillis = millis();

    // Обновление данных и главного дисплея(///////)
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        if (!inMenu) {
            displayData(2, 25.5, 1, 55, 60, 1, 0);
        }
    }

    // Включаем подсветку при повороте
    if (eb.turn()) turnOnBacklight();

    // Проверка нажатия вправо для перехода в меню
    if (!inMenu && eb.rightH()) {
        inMenu = true;
        pos = 0;
        menuFunctions[currentMenu]();
    }

    // Если в меню
    if (inMenu) {
        // Обработка поворота
        if (eb.turn()) {
            int oldPos = pos;

            if (eb.left()) {
                pos = (pos > 0) ? pos - 1 : 0; // Поворот влево
            }
            if (eb.right()) {
                pos = (pos < 3) ? pos + 1 : 3; // Поворот вправо
            }

            // Обновляем только изменённые строки
            update_debug_moisture(oldPos); // Обновляем указатель
        }

        // Проверка нажатия для переключения между меню
        if (pos == 0 && eb.rightH()) {
            currentMenu = (currentMenu + 1) % 4; // Зацикленный переход
            menuFunctions[currentMenu](); // Отобразить текущее меню
            return; // Выход из текущего меню
        }
        if (pos == 0 && eb.leftH()) {
            currentMenu = (currentMenu - 1 + 4) % 4; // Зацикленный переход назад
            menuFunctions[currentMenu](); // Отобразить текущее меню
            return; // Выход из текущего меню
        }

        // Проверка нажатия для увеличения значений
        if (pos == 1 && eb.rightH()) {
            Serial.println("Move right, position: " + String(pos));
            switch (currentMenu) {
                case 0: {
                    Serial.println("Calling function for correction 1 air");
                    break;
                }
                case 1: {
                    Serial.println("Calling function for correction 2 air");
                    break;
                }
                case 2:
                    if (waterind_settings.type == 0 || waterind_settings.type == -1) waterind_settings.type = 1;
                    else if (waterind_settings.type == 1) waterind_settings.type = 0;

                    update_debug_watering();
                    break;
                case 3:
                    window_settings.type += 1;
                    if (window_settings.type == 3) window_settings.type = 0;

                    update_debug_window();
                    break;
            }
        }

        if (pos == 2 && eb.rightH()) {
            Serial.println("Move right, position: " + String(pos));
            switch (currentMenu) {
                case 0:
                    Serial.println("Calling function for correction 1 water");
                    break;
                case 1:
                    Serial.println("Calling function for correction 2 water");
                    break;
                case 2: {
                    if (waterind_settings.type == 0) {
                        if (waterind_settings.max_control_value_auto <= 95) {
                            waterind_settings.max_control_value_auto += 5;
                        }
                    } else if (waterind_settings.type == 1) {
                        if (waterind_settings.time_s_time <= 175) {
                            waterind_settings.time_s_time += 5;
                        }
                    }
                    update_debug_watering();
                    break;
                }
                case 3:
                    if (window_settings.type == 0) {
                        if (window_settings.low_temp_auto <= 35) {
                            window_settings.low_temp_auto += 5;
                        }
                    } else if (window_settings.type == 1) {
                        if (window_settings.time_time <= 175) {
                            window_settings.time_time += 5;
                        }
                    }
                    update_debug_window();
                    break;
            }
        }

        if (pos == 2 && eb.leftH()) {
            Serial.println("Move left, position: " + String(pos));
            switch (currentMenu) {
                case 2: {
                    if (waterind_settings.type == 0) {
                        if (waterind_settings.max_control_value_auto >= 5) {
                            waterind_settings.max_control_value_auto -= 5;
                        }
                    } else if (waterind_settings.type == 1) {
                        if (waterind_settings.time_s_time >= 5) {
                            waterind_settings.time_s_time -= 5;
                        }
                    }
                    update_debug_watering();
                    break;
                }
                case 3:
                    if (window_settings.type == 0) {
                        if (window_settings.low_temp_auto >= -35) {
                            window_settings.low_temp_auto -= 5;
                        }
                    } else if (window_settings.type == 1) {
                        if (window_settings.time_time >= 5) {
                            window_settings.time_time -= 5;
                        }
                    }
                update_debug_window();
                break;
            }
        }

        if (pos == 3 && eb.rightH()) {
            Serial.println("Move right, position: " + String(pos));
            switch (currentMenu) {
                case 0:
                    if (control_moisture1_value < 96) {
                        control_moisture1_value += 5;
                        Serial.println("Correction 1 -> " + String(control_moisture1_value));
                        update_debug_moisture(pos);
                    }
                    break;
                case 1:
                    if (control_moisture2_value < 96) {
                        control_moisture2_value += 5;
                        Serial.println("Correction 2 -> " + String(control_moisture2_value));
                        update_debug_moisture(pos);
                    }
                    break;
                case 2: {
                    if (waterind_settings.type == 0) {
                        if (waterind_settings.time_s_auto <= 95) {
                            waterind_settings.time_s_auto += 5;
                        }
                    } else if (waterind_settings.type == 1) {
                        if (waterind_settings.period_time <= 175) {
                            waterind_settings.period_time += 5;
                        }
                    }
                    update_debug_watering();
                    break;
                }
                case 3: {
if (window_settings.type == 0) {
    if (window_settings.max_temp_auto <= 35) {
        window_settings.max_temp_auto += 5;
    }
} else if (window_settings.type == 1) {
    if (window_settings.periud_time <= 175) {
        window_settings.periud_time += 5;
    }
}

                    update_debug_window();
                    break;
                }
            }
        }

        if (eb.leftH() && pos == 3) {
            Serial.println("Move left, position: " + String(pos));
            switch (currentMenu) {
                case 0:
                    if (control_moisture1_value > 4) {
                        control_moisture1_value -= 5;
                        Serial.println("Correction 1 -> " + String(control_moisture1_value));
                        update_debug_moisture(pos);
                    }
                    break;
                case 1:
                    if (control_moisture2_value > 4) {
                        control_moisture2_value -= 5;
                        Serial.println("Correction 2 -> " + String(control_moisture2_value));
                        update_debug_moisture(pos);
                    }
                    break;
                case 2: {
                    if (waterind_settings.type == 0) {
                        if (waterind_settings.time_s_auto >= 5) {
                            waterind_settings.time_s_auto -= 5;
                        }
                    } else if (waterind_settings.type == 1) {
                        if (waterind_settings.period_time >= 5) {
                            waterind_settings.period_time -= 5;
                        }
                    }
                    update_debug_watering();
                }
                case 3: {
                    if (window_settings.type == 0) {
                        if (window_settings.max_temp_auto >= -35) {
                            window_settings.max_temp_auto -= 5;
                        }
                    } else if (window_settings.type == 1) {
                        if (window_settings.periud_time >= 5) {
                            window_settings.periud_time -= 5;
                        }
                    }
                    update_debug_window();
                    break;
                }
            }
        }
    }

    // Проверка нажатия для выхода из меню
    if (eb.timeout(10000)) {
        turnOffBacklight();
        inMenu = false;
    }
}

void displayData(int status, float temperature, int weather, int moisture1, int moisture2, int liquid_sensor_water,
                 int liquid_sensor_plant) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("01/11 12:00");
    lcd.setCursor(0, 1);
    lcd.print("STATUS: ");
    lcd.print(my_status[status]);
    lcd.setCursor(0, 2);
    lcd.print("HUMIDITY: ");
    lcd.print(moisture1);
    lcd.print("_");
    lcd.print(moisture2);
    lcd.print("%");
    lcd.setCursor(0, 3);
    lcd.print("WATER plt-wtr: ");
    lcd.print(liquid_sensor_plant ? "N" : "Y");
    lcd.print("-");
    lcd.print(liquid_sensor_water ? "N" : "Y");
    lcd.setCursor(19, 0);
    lcd.write(weather ? 0 : 1);
    lcd.setCursor(15, 1);
    lcd.print(temperature);
}

void display_debug_moisture1() {
    lcd.clear();
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(1, i); // Отступ в 1 символ
        lcd.print(debug_moisture1[i]); // Обычные строки
    }
    lcd.setCursor(0, pos); // Указатель на первой колонке
    lcd.print(">"); // Символ указателя
}

void display_debug_moisture2() {
    lcd.clear();
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(1, i); // Отступ в 1 символ
        lcd.print(debug_moisture2[i]); // Обычные строки
    }
    lcd.setCursor(0, pos); // Указатель на первой колонке
    lcd.print(">"); // Символ указателя
}

void display_debug_watering() {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(debug_watering[0]); // "DEBUG WATERING: "
    lcd.setCursor(1, 1);
    lcd.print(debug_watering[1]); // "type: "
    lcd.print(type_watering[waterind_settings.type]);

    lcd.setCursor(1, 2);
    lcd.print(debug_watering[2]);
    lcd.setCursor(1, 3);
    lcd.print(debug_watering[3]);

    lcd.setCursor(15, 2);
    if (waterind_settings.type == 0) {
        lcd.print(waterind_settings.max_control_value_auto);
    } else if (waterind_settings.type == 1) {
        lcd.print(waterind_settings.time_s_time);
        Serial.print(waterind_settings.time_s_time);
    }
    lcd.setCursor(15, 3);
    if (waterind_settings.type == 0) {
        lcd.print(waterind_settings.time_s_auto);
    } else if (waterind_settings.type == 1) {
        lcd.print(waterind_settings.period_time);
    }

    lcd.setCursor(0, pos); // Указатель на первой колонке
    lcd.print(">"); // Символ указателя
}

void display_debug_window() {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(debug_window[0]); // "DEBUG WINDOW: "
    lcd.setCursor(1, 1);
    lcd.print(debug_window[1]); // "type: "
    lcd.print(type_window[window_settings.type]);

    lcd.setCursor(1, 2);
    lcd.print(debug_window[2]);
    lcd.setCursor(1, 3);
    lcd.print(debug_window[3]);

    lcd.setCursor(15, 2);
    if (window_settings.type == 0) {
        lcd.print(window_settings.low_temp_auto);
    } else if (window_settings.type == 1) {
        lcd.print(window_settings.time_time);
    }
    lcd.setCursor(15, 3);
    if (window_settings.type == 0) {
        lcd.print(window_settings.max_temp_auto);
    } else if (window_settings.type == 1) {
        lcd.print(window_settings.periud_time);
    }

    lcd.setCursor(0, pos); // Указатель на первой колонке
    lcd.print(">"); // Символ указателя
}

// Обновление выделенной строки
void update_debug_moisture(int oldPos) {
    // Удаляем указатель с предыдущей позиции
    lcd.setCursor(0, oldPos);
    lcd.print(" "); // Затираем старую строку

    // Обновляем новую позицию
    lcd.setCursor(0, pos);
    lcd.print(">"); // Выделяем новую строку

    // Выводим текущее значение сбоку
    lcd.setCursor(17, 3); // Колонка для значения
    lcd.print("   ");
    lcd.setCursor(17, 3);
    if (currentMenu == 0) {
        lcd.print(control_moisture1_value); // Для moisture1
    } else if (currentMenu == 1) {
        lcd.print(control_moisture2_value); // Для moisture2
    } else if (currentMenu == 2) {
        // Добавьте логику для отображения данных полива
    } else if (currentMenu == 3) {
        // Добавьте логику для отображения данных окон
    }
}

// Обновление данных полива в зависимости от типа
void update_debug_watering() {
    if (waterind_settings.type >= 0 && waterind_settings.type <= 1) {
        debug_watering[2] = (waterind_settings.type == 0) ? debug_watering_auto[0] : debug_watering_time[0];
        debug_watering[3] = (waterind_settings.type == 0) ? debug_watering_auto[1] : debug_watering_time[1];
    } else {
        debug_watering[2] = "";
        debug_watering[3] = "";
    }
    display_debug_watering();
}

void update_debug_window() {
    if (window_settings.type == 0) {
        // Auto
        debug_window[2] = debug_window_auto[0];
        debug_window[3] = debug_window_auto[1];
    } else if (window_settings.type == 1) {
        // Time
        debug_window[2] = debug_window_time[0];
        debug_window[3] = debug_window_time[1];
    } else if (window_settings.type == 2) {
        // Mix
        debug_window[2] = ""; // Add appropriate debug strings for mix type if needed
        debug_window[3] = "";
    } else {
        debug_window[2] = "";
        debug_window[3] = "";
    }
    display_debug_window();
}

// Функция для включения подсветки
void turnOnBacklight() {
    lcd.backlight(); // Включаем подсветку
}

// Функция для выключения подсветки
void turnOffBacklight() {
    lcd.noBacklight(); // Выключаем подсветку
}

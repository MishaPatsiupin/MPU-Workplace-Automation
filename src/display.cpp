//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

#include "display.h"

void (*menu_functions[])() = {
    display_debug_moisture1, display_debug_moisture2, display_debug_watering, display_debug_window
};

const char *my_status[] = {
    "wait",
    "water.",
    "vent",
    "watVent"
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

const char *type_watering[] = {
    "auto",
    "time"
};

const char *debug_watering[] = {
    "DEBUG WATERING: ",
    "type: ",
    "",
    ""
};

const char *debug_watering_auto[] = {
    "max %: ",
    "time: "
};

const char *debug_watering_time[] = {
    "time: ",
    "period: "
};

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
    "Low 'C: ",
    "Max 'C: "
};

const char *debug_window_time[] = {
    "time: ",
    "period: "
};


void display_data(int status, float temperature, int weather, int moisture1, int moisture2, int liquid_sensor_water,
                  int liquid_sensor_plant) {
    DateTime now = rtc.now();
    char date_time[17];
    snprintf(date_time, sizeof(date_time), "%02d/%02d %02d:%02d", now.day(), now.month(), now.hour(), now.minute());


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(date_time);
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
        lcd.setCursor(1, i);
        lcd.print(debug_moisture1[i]);
    }
    lcd.setCursor(0, pos);
    lcd.print(">");
}

void display_debug_moisture2() {
    lcd.clear();
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(1, i);
        lcd.print(debug_moisture2[i]);
    }
    lcd.setCursor(0, pos);
    lcd.print(">");
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

    lcd.setCursor(0, pos);
    lcd.print(">");
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

    lcd.setCursor(0, pos);
    lcd.print(">");
}


void update_debug_moisture(int old_pos) {
    lcd.setCursor(0, old_pos);
    lcd.print(" ");


    lcd.setCursor(0, pos);
    lcd.print(">");


    lcd.setCursor(17, 3);
    lcd.print("   ");
    lcd.setCursor(17, 3);
    if (current_menu == 0) {
        lcd.print(control_moisture1_value); // Для moisture1
    } else if (current_menu == 1) {
        lcd.print(control_moisture2_value); // Для moisture2
    }
}

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
        debug_window[2] = "";
        debug_window[3] = "";
    } else {
        debug_window[2] = "";
        debug_window[3] = "";
    }
    display_debug_window();
}



void turn_on_backlight() {
    lcd.backlight();
}

void turn_off_backlight() {
    lcd.noBacklight();
}

// Handle functions
void handle_backlight() {
    if (eb.turn()) turn_on_backlight();
}

void handle_menu_navigation() {
    if (!in_menu && eb.rightH()) {
        in_menu = true;
        pos = 0;
        menu_functions[current_menu]();
    }
}

void handle_value_adjustments() {
    if (in_menu) {
        if (eb.turn()) {
            int old_pos = pos;

            if (eb.left()) {
                pos = (pos > 0) ? pos - 1 : 0;
            }
            if (eb.right()) {
                pos = (pos < 3) ? pos + 1 : 3;
            }

            update_debug_moisture(old_pos);
        }

        if (pos == 0 && eb.rightH()) {
            current_menu = (current_menu + 1) % 4;
            menu_functions[current_menu]();
            return;
        }
        if (pos == 0 && eb.leftH()) {
            current_menu = (current_menu - 1 + 4) % 4;
            menu_functions[current_menu]();
            return;
        }

        if (pos == 1 && eb.rightH()) {
            handle_right_press_for_pos1();
        }

        if (pos == 2 && eb.rightH()) {
            handle_right_press_for_pos2();
        }

        if (pos == 2 && eb.leftH()) {
            handle_left_press_for_pos2();
        }

        if (pos == 3 && eb.rightH()) {
            handle_right_press_for_pos3();
        }

        if (pos == 3 && eb.leftH()) {
            handle_left_press_for_pos3();
        }
    }

    if (eb.timeout(10000)) {
        turn_off_backlight();
        in_menu = false;
    }
}

void handle_right_press_for_pos1() {
    Serial.println("Move right, position: " + String(pos));
    switch (current_menu) {
        case 0:
            Serial.println("Calling function for correction 1 air");
            measure_air(CS_1);
            break;
        case 1:
            Serial.println("Calling function for correction 2 air");
            measure_air(CS_2);
            break;
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

void handle_right_press_for_pos2() {
    Serial.println("Move right, position: " + String(pos));
    switch (current_menu) {
        case 0:
            Serial.println("Calling function for correction 1 water");
            measure_water(CS_1);
            break;
        case 1:
            Serial.println("Calling function for correction 2 water");
            measure_water(CS_2);
            break;
        case 2:
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
        case 3:
            if (window_settings.type == 0) {
                if (window_settings.low_temp_auto <= 35) {
                    window_settings.low_temp_auto += 1;
                }
                if (window_settings.max_temp_auto <= window_settings.low_temp_auto) {
                    window_settings.max_temp_auto = window_settings.low_temp_auto + 1;
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

void handle_left_press_for_pos2() {
    Serial.println("Move left, position: " + String(pos));
    switch (current_menu) {
        case 2:
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
        case 3:
            if (window_settings.type == 0) {
                if (window_settings.low_temp_auto >= -39) {
                    window_settings.low_temp_auto -= 1;
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

void handle_right_press_for_pos3() {
    Serial.println("Move right, position: " + String(pos));
    switch (current_menu) {
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
        case 2:
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
        case 3:
            if (window_settings.type == 0) {
                if (window_settings.max_temp_auto <= 39) {
                    window_settings.max_temp_auto += 1;
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

void handle_left_press_for_pos3() {
    Serial.println("Move left, position: " + String(pos));
    switch (current_menu) {
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
        case 2:
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
            break;
        case 3:
            if (window_settings.type == 0) {
                if (window_settings.max_temp_auto >= -39) {
                    window_settings.max_temp_auto -= 1;
                }
                if (window_settings.low_temp_auto >= window_settings.max_temp_auto) {
                    window_settings.low_temp_auto = window_settings.max_temp_auto - 1;
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

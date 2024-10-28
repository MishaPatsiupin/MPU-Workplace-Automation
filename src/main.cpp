#include "display.h"
#include "globals.h"


struct window_settings window_settings;
struct waterind_settings waterind_settings;

void setup() {
    Wire.begin(21, 22);
    Serial.begin(115200);
    lcd.begin(20, 04);
    turn_off_backlight();
    eb.counter = 0;
}

void loop() {
    eb.tick();
    unsigned long current_millis = millis();

    update_main_display(current_millis);
    handle_backlight();
    handle_menu_navigation();
    handle_value_adjustments();
}
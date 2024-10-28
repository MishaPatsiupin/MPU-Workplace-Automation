#include "display.h"
#include "globals.h"
#include "data_capture.h"

struct window_settings window_settings;
struct waterind_settings waterind_settings;

void setup() {
    Wire.begin(MY_SDA_PIN, MY_SCL_PIN);
    Serial.begin(115200);
    init_devices();


}

void loop() {
    eb.tick();
    unsigned long current_millis = millis();

    update_main_display(current_millis);
    handle_backlight();
    handle_menu_navigation();
    handle_value_adjustments();
}
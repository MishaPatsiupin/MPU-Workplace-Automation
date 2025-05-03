//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

#include "api.h"
#include <Preferences.h>
#include <esp_wifi.h>
#include <HTTPClient.h>

Preferences preferences;

// Конфигурация
const char* apSSID = "ESP32_AP";
const char* apPassword = "password123";
const IPAddress apIP(192, 168, 4, 1);

// Статические IP для STA
IPAddress localIP(192, 168, 1, 80);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);



bool show_status = false;

// Инициализация веб-сервера на порту 80
WebServer server(8080);

// Инициализация NTP клиента
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 60000);

bool serverStarted = false;
void setupServer();

// Настройка точки доступа
void setupAccessPoint() {
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID, apPassword, 1, 0, 4);
    Serial.println("Access Point настроен");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void setupWiFiClient() {
    preferences.putString("ssid", "AndroidAP4763");
    preferences.putString("pass", "123456Bears");
    String savedSSID = preferences.getString("ssid", "AndroidAP4763");
    String savedPass = preferences.getString("pass", "123456Bears");
//    Serial.print("Saved SSID: ");
//    Serial.println(savedSSID);
//    Serial.print("Saved Password: ");
//    Serial.println(savedPass);

    if (savedSSID.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        if (WiFi.waitForConnectResult(5000) == WL_CONNECTED) {
            Serial.println("Fast reconnect OK");
            preferences.putString("ssid", WiFi.SSID());
            preferences.putString("pass", WiFi.psk());
            return;
        }
    }

    // Fallback to default credentials if saved credentials fail
    WiFi.begin("AndroidAP4763", "123456Bears");
    if (WiFi.waitForConnectResult(5000) == WL_CONNECTED) {
        preferences.putString("ssid", WiFi.SSID());
        preferences.putString("pass", WiFi.psk());
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

static unsigned long lastExecutionTime = 0;
void connectToWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        unsigned long currentTime = millis();
        if (currentTime - lastExecutionTime >= 60000) { // 60000 ms = 1 minute
            lastExecutionTime = currentTime;
            setupWiFiClient();
            setupServer();
        }
    }
}

String getCurrentTimeISO8601() {
    DateTime now = rtc.now();
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    return String(buffer);
}

float roundToTwoDecimals(float value) {
    return round(value * 100.0) / 100.0;
}

void handleGetData() {
    Serial.println("Received request: /tepliza/data");
    DynamicJsonDocument doc(1024);
    doc["this_time"] = getCurrentTimeISO8601();
    doc["temperature"] = roundToTwoDecimals(now_sensor_data.temperature);
    doc["humidity"] = roundToTwoDecimals(now_sensor_data.humidity);
    doc["pressure"] = roundToTwoDecimals(now_sensor_data.pressure);
    doc["moisture1"] = now_sensor_data.moisture1 > 0 ? now_sensor_data.moisture1 : 0;
    doc["moisture2"] = now_sensor_data.moisture2 > 0 ? now_sensor_data.moisture2 : 0;
    doc["liquid_sensor_water"] = !(now_sensor_data.liquid_sensor_water > 0);
    doc["liquid_sensor_plant"] = !(now_sensor_data.liquid_sensor_plant > 0);

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleGetState() {
    Serial.println("Received request: /tepliza/state");
    DynamicJsonDocument doc(1024);
    doc["window_flag"] = window_flag;
    doc["water_pomp_flag"] = pump_flag;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleSetPomp() {
    Serial.println("Received request: /tepliza/set/pomp");
    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, server.arg("plain"));
        if (doc.containsKey("state")) {
            pump_flag = doc["state"];
            if (pump_flag) {
                start_pump();
            } else {
                stop_pump();
            }
            server.send(200, "application/json", "{\"message\":\"Water pump state updated\",\"new_state\":" + String(pump_flag) + "}");
        } else {
            server.send(400, "application/json", "{\"error\":\"No state provided\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No state provided\"}");
    }
}

void handleSetVentilation() {
    Serial.println("Received request: /tepliza/set/ventilation");
    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, server.arg("plain"));
        if (doc.containsKey("state")) {
            bool window_flag_local = doc["state"];
            servo_window_control(window_flag_local);
            window_flag = window_flag_local;
            server.send(200, "application/json", "{\"message\":\"Ventilation state updated\",\"new_state\":" + String(window_flag) + "}");
        } else {
            server.send(400, "application/json", "{\"error\":\"No state provided\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No state provided\"}");
    }
}

void handleGetDataGradka() {
    Serial.println("Received request: /gradka/data");
    DynamicJsonDocument doc(1024);
    doc["this_time"] = getCurrentTimeISO8601();
    doc["temperature"] = roundToTwoDecimals(now_sensor_data.temperature);
    doc["humidity"] = roundToTwoDecimals(now_sensor_data.humidity);
    doc["pressure"] = roundToTwoDecimals(now_sensor_data.pressure);
    doc["moisture1"] = (now_sensor_data.moisture1 + now_sensor_data.moisture2) / 2;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void setupServer() {
    if (WiFi.isConnected() && !serverStarted) {
        Serial.print("API доступно по адресу: http://");
        Serial.println(WiFi.localIP());

        server.on("/tepliza/data", HTTP_GET, handleGetData);
        server.on("/tepliza/state", HTTP_GET, handleGetState);
        server.on("/tepliza/set/pomp", HTTP_POST, handleSetPomp);
        server.on("/tepliza/set/ventilation", HTTP_POST, handleSetVentilation);
        server.on("/gradka/data", HTTP_GET, handleGetDataGradka);

        server.begin();
        Serial.println("HTTP server started");
        serverStarted = true;
    } else {
        Serial.println("WiFi не подключен. HTTP сервер не запущен.");
        serverStarted = false;
    }
}

void sendDataTask(void * parameter) {

    setupAccessPoint();
    //setupServer();



    while (true) {//loop

        //---для предоставления состояния к мобильному приложению
        connectToWiFi();
        
        server.handleClient();
        //***



        delay(2);
    }
}


unsigned long display_start_time = 0; // Время начала отображения информации
void wifi_status() {
    // Проверка состояния кнопки энкодера
    if (show_status) {
        display_start_time = millis(); // Сохранение времени начала отображения
        lcd.clear();

        lcd.setCursor(0, 0);
        if (WiFi.isConnected()) {
            lcd.print("IP: ");
            lcd.print(WiFi.localIP());
        } else {
            lcd.print("Connecting to WiFi..");
        }

    }
    show_status = false;
}

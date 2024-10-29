#include "api.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Параметры Wi-Fi
const char* ssid = "AndroidAP4763";
const char* password = "123456Bears";

WebServer server(80);

// Инициализация NTP клиента
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600); // Часовой пояс +3


// Функция для подключения к Wi-Fi
void connectToWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Обновление времени и даты через NTP
    timeClient.begin();
    if (timeClient.update()) {
        rtc.adjust(DateTime(timeClient.getEpochTime()));
        Serial.println("Time updated from NTP");
    } else {
        Serial.println("Failed to update time from NTP");
    }
}

// Функция для получения текущего времени в формате ISO 8601
String getCurrentTimeISO8601() {
    DateTime now = rtc.now();
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
    return String(buffer);
}

// Функция для округления числа до двух знаков после запятой
float roundToTwoDecimals(float value) {
    return round(value * 100.0) / 100.0;
}

// Обработчик для получения данных
void handleGetData() {
    DynamicJsonDocument doc(1024);
    doc["this_time"] = getCurrentTimeISO8601();
    doc["temperature"] = roundToTwoDecimals(now_sensor_data.temperature);
    doc["humidity"] = roundToTwoDecimals(now_sensor_data.humidity);
    doc["pressure"] = roundToTwoDecimals(now_sensor_data.pressure);
    doc["moisture1"] = now_sensor_data.moisture1;
    doc["moisture2"] = now_sensor_data.moisture2;
    doc["liquid_sensor_water"] = !(now_sensor_data.liquid_sensor_water > 0);
    doc["liquid_sensor_plant"] = !(now_sensor_data.liquid_sensor_plant > 0);

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

// Обработчик для получения состояния
void handleGetState() {
    DynamicJsonDocument doc(1024);
    doc["window_flag"] = window_flag;
    doc["water_pomp_flag"] = pump_flag;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

// Обработчик для установки состояния помпы
void handleSetPomp() {
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

// Обработчик для установки состояния вентиляции
void handleSetVentilation() {
    if (server.hasArg("plain")) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, server.arg("plain"));
        if (doc.containsKey("state")) {
            window_flag = doc["state"];
            servo_window_control(window_flag);
            server.send(200, "application/json", "{\"message\":\"Ventilation state updated\",\"new_state\":" + String(window_flag) + "}");
        } else {
            server.send(400, "application/json", "{\"error\":\"No state provided\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No state provided\"}");
    }
}

// Обработчик для получения данных для gradka
void handleGetDataGradka() {
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

// Функция задачи для второго ядра
void sendDataTask(void * parameter) {
    connectToWiFi();

    server.on("/tepliza/data", HTTP_GET, handleGetData);
    server.on("/tepliza/state", HTTP_GET, handleGetState);
    server.on("/tepliza/set/pomp", HTTP_POST, handleSetPomp);
    server.on("/tepliza/set/ventilation", HTTP_POST, handleSetVentilation);
    server.on("/gradka/data", HTTP_GET, handleGetDataGradka);

    server.begin();
    Serial.println("HTTP server started");

    while (true) {
        server.handleClient();
        delay(2);
    }
}


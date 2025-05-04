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

WebServer serverAP(80); // Веб-сервер для точки доступа

// Инициализация NTP клиента
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 60000);

void updateRTCFromNTP() {
    if (WiFi.isConnected()) {
        Serial.println("Обновление времени на RTC через NTP...");
        timeClient.update();
        unsigned long epochTime = timeClient.getEpochTime();
        if (epochTime > 0) {
            rtc.adjust(DateTime(epochTime));
            Serial.println("Время успешно обновлено на RTC.");
        } else {
            Serial.println("Ошибка: не удалось получить время через NTP.");
        }
    } else {
        Serial.println("WiFi не подключен. Невозможно обновить время на RTC.");
    }
}

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
        if (currentTime - lastExecutionTime >= 30000) { // 60000 ms = 1 minute
            lastExecutionTime = currentTime;
            setupWiFiClient();
            setupServer();
            updateRTCFromNTP();
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
    //doc["window_flag"] = window_flag;
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
                send_start_pomp();
            } else {
                send_stop_pomp();
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
            relay_control(window_flag_local);
           // window_flag = window_flag_local;
           // server.send(200, "application/json", "{\"message\":\"Ventilation state updated\",\"new_state\":" + String(window_flag) + "}");
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

String sendHttpRequest(const String& url) {
    HTTPClient http;
    //Serial.print("Sending request to: ");
    //Serial.println(url);
    String response = "-1";
    if (http.begin(url)) {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        //Serial.print("Response: ");
        response = http.getString();
        //Serial.println(response);
      } else {
        //Serial.print("HTTP error: ");
        //Serial.println(response);
      }
      http.end();
    } else {
      Serial.println("Failed to connect to server");
    }
    return response;
  }

esp32_c3_supermini_data read_http_c3_supermini(){
    esp32_c3_supermini_data data;
    String response = sendHttpRequest("http://192.168.4.2:80/data");
    if (response != "-1") {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        data.moisture1 = doc["soil1"];
        data.moisture2 = doc["soil2"];
        data.liquid_sensor_plant = doc["water"];
        return data;
    } else {
        data.moisture1 = -1;
        data.moisture2 = -1;
        data.liquid_sensor_plant = -1;
        return data;
    } 
    
}

int read_moisture_number(int number){
 String response = sendHttpRequest("http://192.168.4.2:80/data");
 //Serial.println("Response esp32-c3-supermini: " + response);
 if (response != "-1") {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    int moisture1 = doc["soil1"];
    int moisture2 = doc["soil2"];
    if (number == 1) {
        return moisture1;
    } else if (number == 2) {
        return moisture2;
    }
}
    return -1; 
}

bool send_start_pomp() {
    String response = sendHttpRequest("http://192.168.4.3:80/state/on");
    HTTPClient http;

    // Проверяем HTTP-код ответа
    if (response != "-1") { // HTTP_CODE_OK = 200
        Serial.println("Pump started successfully");
        Serial.print("Response(send_start_pomp): ");
        Serial.println(response); // Выводим ответ для проверки
        return true; // Успешный ответ
    } else {
        Serial.println("Failed to start pump");
        return false; // Ошибка
    }
}

bool send_stop_pomp() {
    String response = sendHttpRequest("http://192.168.4.3:80/state/off");

    // Проверяем HTTP-код ответа
    if (response != "-1") { // HTTP_CODE_OK = 200
        Serial.println("Pump stop successfully");
        Serial.print("Response(send_stop_pomp): ");
        Serial.println(response); // Выводим ответ для проверки
        return true; // Успешный ответ
    } else {
        Serial.println("Failed to stop pump");
        return false; // Ошибка
    }
}

bool send_on_relay(){//4!
    String response = sendHttpRequest("http://192.168.4.4:80/state/on");

    if (response != "-1") { // HTTP_CODE_OK = 200
        Serial.println("Relay ON successfully");
        Serial.print("Response(send_on_relay): ");
        Serial.println(response); // Выводим ответ для проверки
        return true; // Успешный ответ
    } else {
        Serial.println("Failed to turn on relay");
        return false; // Ошибка
    }
}
bool send_off_relay(){
    String response = sendHttpRequest("http://192.168.4.4:80/state/off");

    if (response != "-1") { // HTTP_CODE_OK = 200
        Serial.println("Relay OFF successfully");
        Serial.print("Response(send_off_relay): ");
        Serial.println(response); // Выводим ответ для проверки
        return true; // Успешный ответ
    } else {
        Serial.println("Failed to turn on relay");
        return false; // Ошибка
    }
}

void sendDataTask(void * parameter) {
    vTaskDelay(30 / portTICK_PERIOD_MS);
    setupAccessPoint();
    //setupServer();

    while (true) {//loop
        //eb.tick();
        //---для предоставления состояния к мобильному приложению
        connectToWiFi();
        server.handleClient();
        //***
        


        //delay(2);
    }
}


unsigned long display_start_time = 0; // Время начала отображения информации
void wifi_status() {
    // Проверка состояния кнопки энкодера
    if (show_status) {
        display_start_time = millis(); // Сохранение времени начала отображения
        lcd.clear();

        // Строка 0: IP или статус подключения
        lcd.setCursor(0, 0);
        if (WiFi.isConnected()) {
            lcd.print("IP: ");
            lcd.print(WiFi.localIP());
        } else {
            lcd.print("Connecting...");
        }

        // Вывод MAC-адресов подключенных устройств
        wifi_sta_list_t stationList;
        esp_wifi_ap_get_sta_list(&stationList);

        for (int i = 0; i < stationList.num && i < 3; i++) { // Ограничение на 3 устройства
            wifi_sta_info_t station = stationList.sta[i];
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                     station.mac[0], station.mac[1], station.mac[2],
                     station.mac[3], station.mac[4], station.mac[5]);

            // Строки 1, 2, 3: MAC-адреса
            lcd.setCursor(0, i + 1);
            lcd.print(macStr);
        }

        // Если меньше 3 устройств, оставшиеся строки остаются пустыми
        for (int i = stationList.num; i < 3; i++) {
            lcd.setCursor(0, i + 1);
            lcd.print("                "); // Очистка строки
        }
    }
    show_status = false;
}

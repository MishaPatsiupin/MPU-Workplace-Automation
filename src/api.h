//Патюпин М.С. ГР250503 КП
//Микропроцессорное устройство контроля параметров тепличного комбината

#ifndef API_H
#define API_H

// Подключение библиотеки для работы с веб-сервером
#include <WebServer.h>
// Подключение библиотеки для работы с JSON
#include <ArduinoJson.h>
// Подключение библиотеки для работы с WiFi
#include <WiFi.h>
// Подключение библиотеки для работы с UDP
#include <WiFiUdp.h>
// Подключение библиотеки для работы с NTP клиентом
#include <NTPClient.h>
// Подключение заголовочного файла для захвата данных
#include "data_capture.h"

// Функция: подключение к WiFi
// Принимает: -
// Возвращает: -
void connectToWiFi();
// Функция: обработчик для получения данных
// Принимает: -
// Возвращает: -
void handleGetData();
// Функция: обработчик для получения состояния
// Принимает: -
// Возвращает: -
void handleGetState();
// Функция: обработчик для установки состояния помпы
// Принимает: -
// Возвращает: -
void handleSetPomp();
// Функция: обработчик для установки состояния вентиляции
// Принимает: -
// Возвращает: -
void handleSetVentilation();
// Функция: задача для второго ядра
// Принимает: указатель на параметр
// Возвращает: -
void sendDataTask(void * parameter);

#endif //API_H
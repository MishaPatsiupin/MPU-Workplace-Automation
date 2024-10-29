#ifndef API_H
#define API_H

#include <WebServer.h>
#include <ArduinoJson.h>

#include "data_capture.h"

void connectToWiFi();
void handleGetData();
void handleGetState();
void handleSetPomp();
void handleSetVentilation();
void sendDataTask(void * parameter);

#endif //API_H
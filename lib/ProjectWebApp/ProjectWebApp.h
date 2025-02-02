#ifndef _PROJECTWEBAPP_H
#define _PROJECTWEBAPP_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

void webapp_init(JsonDocument *config_placeholder, bool *updateFlag_placeholder);
void write_json();
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

#endif
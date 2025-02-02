#include "ProjectWebApp.h"

JsonDocument config_json_document;
bool *updateFlag;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void webapp_init(JsonDocument *config_placeholder, bool *updateFlag_placeholder) {
  config_json_document = *config_placeholder;
  updateFlag = updateFlag_placeholder;

  //Inisialisasi Website
  server.on("/main", HTTP_GET, [](AsyncWebServerRequest *request){
    // Mengirimkan halaman website ke pengguna
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Memulai WebApp
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.begin();
}
// Fungsi untuk Menyimpan Config dalam file config.json
void write_json() {
  // Menulis data baru ke dalam file config.jsons
  File file = SPIFFS.open("/config.json", "w");
  if (file) {
    serializeJson(config_json_document, file);
    file.close();
  } else {
    Serial.println("Gagal mengakses file config.json untuk menulis data");
    while (true) {}
  }
}

// Fungsi untuk memproses request dari webapp melalui WebSocket
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Klien Terhubung : %u\n", client->id());

    // Mengirim file config.json yang tersimpan ke webapp
    File file = SPIFFS.open("/config.json", "r");

    if (file) {
      // Membaca data pada file config.json
      deserializeJson(config_json_document, file);
      file.close();
    }
    String jsonString;
    serializeJson(config_json_document, jsonString);
    client->text(jsonString);

  } else if (type == WS_EVT_DATA) {
    // Menerima data JSON baru
    String jsonString;
    for (size_t i = 0; i < len; i++) {
      jsonString += (char)data[i];
    }

    // Memproses dan memperbarui data file config.json
    DeserializationError error = deserializeJson(config_json_document, jsonString);
    if (!error) {
      write_json();
      Serial.println("config.json berhasil diperbarui");
      Serial.println(jsonString);
      // Memberikan tanda bahwa file json sudah di perbaharui
      *updateFlag = true;
    }
    // Memberikan notifikasi untuk men-refresh webpage
    client->text("REFRESH");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Klien Terputus : %u\n", client->id());
  }
}
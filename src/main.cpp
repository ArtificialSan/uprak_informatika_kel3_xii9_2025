#define R1 19
#define G1 13
#define BL1 18
#define R2 5
#define G2 12
#define BL2 17
#define CH_A 16
#define CH_B 14
#define CH_C 4
#define CH_D 27
#define CH_E -1 // TIDAK DIPERLUKAN UNTUK 64x32
#define CLK 2
#define LAT 26
#define OE 15

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANELS_NUMBER 1
#define FRAME_SIZE PANEL_WIDTH * PANEL_HEIGHT * 2

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FastLED.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"

#include <SPIFFSGIFPlayer.h>
#include <MatrixTemplates.h>
#include <ProjectWebApp.h>

#include <BreeSerif_Regular16pt7b.h>

RTC_DS3231 rtc;
MatrixPanel_I2S_DMA *dma_display = nullptr;

int year, month, date, hour, minute, second;
bool optionOpenClose;
int openHour, openMinute, openSecond;
int closeHour, closeMinute, closeSecond;
bool optionText;
String text;
String swipe;
String color;
bool optionAnimation;
String animation;

JsonDocument config;

const char* ssid = "E-RASI";
const char* password = "12345678";
const char* host = "e-rasi";

bool isUpdated = false;

// Fungsi untuk men-load config dalam file config.json
void read_json() {
  DeserializationError error;

  File file = SPIFFS.open("/config.json", "r");
  if (file) {
    // Membaca data pada file config.json
    error = deserializeJson(config, file);
    file.close();
  }

  if (error) {
    Serial.print(F("Gagal memdapat isi file config.json, eksekusi dihentikan..."));
    while (true) {}
  } else {
    // Mengambil data dari file config.json
    year = config["time"]["year"].as<int>();
    month = config["time"]["month"].as<int>();
    date = config["time"]["date"].as<int>();
    hour = config["time"]["hour"].as<int>();
    minute = config["time"]["minute"].as<int>();
    second = config["time"]["second"].as<int>();

    optionOpenClose = config["optionOpenClose"].as<bool>();
    openHour = config["configOpenClose"]["openTime"]["hour"].as<int>();
    openMinute = config["configOpenClose"]["openTime"]["minute"].as<int>();
    openSecond = config["configOpenClose"]["openTime"]["second"].as<int>();
    closeHour = config["configOpenClose"]["closeTime"]["hour"].as<int>();
    closeMinute = config["configOpenClose"]["closeTime"]["minute"].as<int>();
    closeSecond = config["configOpenClose"]["closeTime"]["second"].as<int>();

    optionText = config["optionText"].as<bool>();
    text = config["configText"]["text"].as<String>();
    swipe = config["configText"]["swipe"].as<String>();
    color = config["configText"]["color"].as<String>();

    optionAnimation = config["optionAnimation"].as<bool>();
    animation = config["configAnimation"]["animation"].as<String>();

    isUpdated = false;
  }
}

void rtc_update() {
  rtc.adjust(DateTime(year, month, date, hour, minute, second));
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi SPIFFS
  if (!SPIFFS.begin(false)) {
    Serial.println("SPIFFS gagal diinisialisasi");
    return;
  }

  // Mengecek apakah file config.json ada, jika tidak ada membuat file config.json baru
  if (!SPIFFS.exists("/config.json")) {
    Serial.println("/config.json tidak ditemukan, membuat file baru...");
    String defaultConfig = "{\"time\":{\"year\":1970,\"month\":1,\"date\":1,\"hour\":0,\"minute\":0,\"second\":0},\"optionOpenClose\":false,\"configOpenClose\":{\"openTime\":{\"hour\":0,\"minute\":0},\"closeTime\":{\"hour\":0,\"minute\":0}},\"optionText\":false,\"configText\":{\"text\":\"Hello World!\",\"swipe\":\"L\",\"color\":\"#ff0000\"},\"optionAnimation\":false,\"configAnimation\":{\"animation\":\"1\"}}"; // Data JSON default
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    if (!file) {
      Serial.println("Gagal membuat /config.json");
    } else {
      file.print(defaultConfig);
      file.close();
      Serial.println("/config.json berhasil dibuat");
    }
  }

  // Membaca data yang tersimpan di memori dalam format JSON
  read_json();

  // Inisialisasi RTC DS3231
  if (!rtc.begin()) {
    Serial.println("RTC tidak ditemukan");
    //while (1);
  } else {
    Serial.println("RTC ditemukan, menyetel waktu ...");
  }

  if (rtc.lostPower()) {
    rtc_update();
  }

  rtc.writeSqwPinMode(DS3231_OFF);

  // Inisialisasi WiFi
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  Serial.println("Jaringan dibuat");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP : ");
  Serial.println(IP);

  // Inisialisasi MDNS
  MDNS.begin(host);
  Serial.printf("Host: http://%s.local/main\n", host);

  // Inisialisasi Panel LED
  HUB75_I2S_CFG::i2s_pins _pins={R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};

  HUB75_I2S_CFG mxconfig (
    PANEL_WIDTH,                        // Width Panel Length (LEDs)
    PANEL_HEIGHT,                       // Height Panel Length (LEDs)
    PANELS_NUMBER,                      // Panel Chain (count)
    _pins,                              // Custom Pins for Custom Hardware
    HUB75_I2S_CFG::FM6124,              // Panel Used Driver (FM6124, FM6126A, ICN2038S, MBI5124, SM5266P)
    false,                              // Using Double (DMA) Buffer
    HUB75_I2S_CFG::HZ_10M,              // I2S Clock Speed
    1U,                                 // Latch Blanking
    true,                               // Clockphase
    60U                                 // Minimum Refresh Rate  
  );

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  dma_display->setBrightness8(255);    // Kecerahan 0-255, 0 - 0%, 255 - 100%

  // Alokasikan memori untuk menjalankan panel LED
  if(!(dma_display->begin()) ) {
      Serial.println("****** !KABOOM! I2S memory allocation failed ***********");
  }

  dma_display->cp437(true);

  dma_display->setRotation(0);
  dma_display->clearScreen();

  webapp_init(&config, &isUpdated);
  gif_player_init(dma_display, PANEL_WIDTH, PANEL_HEIGHT);
  matrix_templates_init(dma_display, PANEL_WIDTH, PANEL_HEIGHT);

   // Play_Text(true, "e-rasi.local/main", "L", (GFXfont*)&BreeSerif_Regular16pt7b, "#ffffff", 40, &isUpdated);
}

void loop() {
  dma_display->clearScreen();
  
  if (isUpdated) 
  {
    read_json();
    rtc_update();
  }

  Play_Open_Close(optionOpenClose, &rtc, openHour, openMinute, closeHour, closeMinute, &isUpdated);
 
  Play_Text(optionText, text, swipe, (GFXfont*)&BreeSerif_Regular16pt7b, color, 40, &isUpdated);
  
  switch (animation.charAt(0))
  {
  case '1':
    Play_Animation(optionAnimation, (char *)"/nyan-cat-64x32px.gif", 10, 8000, &isUpdated);
    break;
  case '2':
    Play_Animation(optionAnimation, (char *)"/horse-gif-64x32px.gif", 10, 8000, &isUpdated);
    break;
  case '3':
    Play_Animation(optionAnimation, (char *)"/heart-gif-64x32px.gif", 10, 8000, &isUpdated);
    break;
  default:
    break;
  }
}
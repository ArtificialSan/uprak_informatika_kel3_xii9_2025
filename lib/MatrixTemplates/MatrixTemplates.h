#ifndef MATRIXTEMPLATES_H
#define MATRIXTEMPLATES_H

#include "Arduino.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <RTClib.h>
#include <OPEN_FONT.h>
#include <CLOSED_FONT.h>
#include <Font5x7Fixed.h>

void matrix_templates_init (MatrixPanel_I2S_DMA *display_placeholder, int width, int height);
uint16_t convertHEXto565(String hexColor);
void Play_Text(bool isShow, String input_text, String orientation, GFXfont *font, String input_color, int speed, bool *stopFlag);
void hsvToRgb(uint16_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b);
void drawSnakeBorder(int frame, uint16_t color, int snakeLength, int numSnakes);
void drawOpenTemplate(int *open_hour, int *open_minute, int *close_hour, int *close_minute, int speed, int duration, bool *stopFlag);
void Play_Text_Effect_HSV(String input_text, GFXfont *font,int x , int y, int speed, int duration, bool *stopFlag);
void Play_Text_Effect_Snake_Border(String input_text, GFXfont *font, int x, int y, uint16_t color, int snakeLength, int numSnakes, int speed, int duration, bool *stopFlag);
void Play_Open_Close(bool isShow, RTC_DS3231 *rtc_placeholder, int open_hour, int open_minute, int close_hour, int close_minute, bool *stopFlag);

#endif
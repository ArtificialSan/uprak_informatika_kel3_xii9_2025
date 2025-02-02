#include "MatrixTemplates.h"

MatrixPanel_I2S_DMA *display_template;
int LEDPANEL_WIDTH_TEMPLATE, LEDPANEL_HEIGHT_TEMPLATE;

void matrix_templates_init (MatrixPanel_I2S_DMA *display_placeholder, int width, int height) {
    display_template = display_placeholder;
    LEDPANEL_WIDTH_TEMPLATE = width;
    LEDPANEL_HEIGHT_TEMPLATE = height;
}

uint16_t convertHEXto565(String hexColor) {
  if (hexColor.startsWith("#")) {
    hexColor.remove(0, 1);
  }

  uint8_t red = strtol(hexColor.substring(0, 2).c_str(), NULL, 16);
  uint8_t green = strtol(hexColor.substring(2, 4).c_str(), NULL, 16);
  uint8_t blue = strtol(hexColor.substring(4, 6).c_str(), NULL, 16);

  return display_template->color565(red, green, blue);
} 

void Play_Text(bool isShow, String input_text, String orientation, GFXfont *font, String input_color, int speed, bool *stopFlag) {
  if (isShow && orientation == "L") {
    bool endOfText = false;
  
    const int yPostition = 25;  // For font size = 16 keep this value
    int calculateActualTextLength = 0;
    String text_placeholder = input_text;
    int textLength = text_placeholder.length();
    uint16_t colorConverted = convertHEXto565(input_color);

    for (int i = 0; i < textLength; i++) {
      calculateActualTextLength += (font->glyph[(int)text_placeholder.charAt(i) - 32].xAdvance);
    }

    display_template->setFont(font);
    display_template->setTextColor(colorConverted);

    for (int i = 0; i < LEDPANEL_WIDTH_TEMPLATE + calculateActualTextLength; i++) {
      display_template->clearScreen();
      display_template->flush();
      int xPosition = 0;
      for (int j = 0; j < textLength; j++) {
        if (*stopFlag == true) {
          break;
        }
        int charActualLength = (font->glyph[(int)text_placeholder.charAt(j) - 32].xAdvance);
        display_template->drawChar( ( LEDPANEL_WIDTH_TEMPLATE - i ) + xPosition, yPostition, text_placeholder.charAt(j), colorConverted, display_template->color565(0, 0, 0), 1);
        xPosition += charActualLength;
      }
      if (*stopFlag == true) {
        break;
      }
      delay(speed);
    }
    display_template->clearScreen();
    display_template->flush();
  }
  if (isShow && orientation == "R") { // Change 'L' to 'R' for right orientation
    bool endOfText = false;
  
    const int yPostition = 28;  // For font size = 20 keep this value
    int calculateActualTextLength = 0;
    String text_placeholder = input_text;
    int textLength = text_placeholder.length();
    uint16_t colorConverted = convertHEXto565(input_color);

    for (int i = 0; i < textLength; i++) {
      calculateActualTextLength += (font->glyph[(int)text_placeholder.charAt(i) - 32].xAdvance);
    }

    display_template->setFont(font);
    display_template->setTextColor(colorConverted);

    for (int i = 0; i < LEDPANEL_WIDTH_TEMPLATE + calculateActualTextLength; i++) {
      display_template->clearScreen();
      display_template->flush();
      int xPosition = 0;
      for (int j = 0; j < textLength; j++) {
        if (*stopFlag == true) {
          break;
        }
        int charActualLength = (font->glyph[(int)text_placeholder.charAt(j) - 32].xAdvance);
        display_template->drawChar( i - xPosition, yPostition, text_placeholder.charAt(j), colorConverted, display_template->color565(0, 0, 0), 1);
        xPosition += charActualLength;
      }
      if (*stopFlag == true) {
        break;
      }
      delay(speed);
    }
  }
}

void hsvToRgb(uint16_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float hf = h / 60.0;
  int i = floor(hf);
  float f = hf - i;
  float pv = v * (1 - s / 255.0);
  float qv = v * (1 - s / 255.0 * f);
  float tv = v * (1 - s / 255.0 * (1 - f));

  switch (i) {
    case 0:
      r = v;
      g = tv;
      b = pv;
      break;
    case 1:
      r = qv;
      g = v;
      b = pv;
      break;
    case 2:
      r = pv;
      g = v;
      b = tv;
      break;
    case 3:
      r = pv;
      g = qv;
      b = v;
      break;
    case 4:
      r = tv;
      g = pv;
      b = v;
      break;
    case 5:
      r = v;
      g = pv;
      b = qv;
      break;
    default:
      r = v;
      g = v;
      b = v;
      break;
  }
}

void drawSnakeBorder(int frame, uint16_t color, int snakeLength, int numSnakes) {
  int x = 0, y = 0;
  int width = LEDPANEL_WIDTH_TEMPLATE - 1;
  int height = LEDPANEL_HEIGHT_TEMPLATE - 1;
  int perimeter = 2 * (width + height);

  // Clear previous border
  display_template->fillRect(x, y, width + 1, height + 1, display_template->color565(0, 0, 0));

  // Draw snakes
  for (int n = 0; n < numSnakes; n++) {
    int offset = (n * perimeter) / numSnakes;
    for (int i = 0; i < snakeLength; i++) {
      int snakePos = (frame + i + offset) % perimeter;
      if (snakePos < width) {
        display_template->drawPixel(x + snakePos, y, color);
      } else if (snakePos < width + height) {
        display_template->drawPixel(x + width, y + (snakePos - width), color);
      } else if (snakePos < 2 * width + height) {
        display_template->drawPixel(x + (2 * width + height - snakePos), y + height, color);
      } else {
        display_template->drawPixel(x, y + (2 * (width + height) - snakePos), color);
      }
    }
  }
}

void Play_Text_Effect_HSV(String input_text, GFXfont *font,int x , int y, int speed, int duration, bool *stopFlag) {
  int startMillis = millis();
  while (*stopFlag == false && millis() - startMillis <= duration ) {
    static uint16_t hue = 0; // Initialize hue value
    uint8_t r, g, b;
    hsvToRgb(hue, 255, 255, r, g, b);
    uint16_t color_ = display_template->color565(r, g, b); // Convert RGB to 565 format

    display_template->setFont(font);
    display_template->setCursor(x, y);    // 2, 29 for open
    display_template->setTextColor(color_);
    display_template->print(input_text);
    hue += 1; // Increment hue value
    if (hue >= 360) {
      hue = 0; // Reset hue value if it exceeds 360
      break; // Stop the loop if hue value reaches 360
    }
    delay(speed); // Adjust delay for desired t
  }
  display_template->clearScreen();
}

void Play_Text_Effect_Snake_Border(String input_text, GFXfont *font, int x, int y, uint16_t color, int snakeLength, int numSnakes, int speed, int duration, bool *stopFlag) {
  int frame = 0;

  int startMillis = millis();

  while (*stopFlag == false && millis() - startMillis <= duration) {
    drawSnakeBorder(frame, color, snakeLength, numSnakes);
    display_template->setFont(font);
    display_template->setCursor(x, y);    // 2, 29 for open
    display_template->setTextColor(color);
    display_template->print(input_text);
    frame += 1;
    delay(speed);
  }
  display_template->clearScreen();
}

void Play_Open_Close(bool isShow, RTC_DS3231 *rtc_placeholder, int open_hour, int open_minute, int close_hour, int close_minute, bool *stopFlag) {
  if (isShow) {
    DateTime now = rtc_placeholder->now();
    int currentHour = now.hour();
    int currentMinute = now.minute();

    if ((currentHour > open_hour || (currentHour == open_hour && currentMinute >= open_minute)) && (currentHour < close_hour || (currentHour == close_hour && currentMinute < close_minute))) {
      Play_Text_Effect_HSV("OPEN", (GFXfont *)&OPEN, 2, 29, 50, 10000, stopFlag);
      display_template->clearScreen();
      drawOpenTemplate(&open_hour, &open_minute, &close_hour, &close_minute, 70, 10000, stopFlag);
    } else if (((currentHour < open_hour || (currentHour == open_hour && currentMinute < open_minute)) || (currentHour > close_hour || (currentHour == close_hour && currentMinute >= close_minute)))) {
      Play_Text_Effect_Snake_Border("CLOSED", (GFXfont *)&CLOSED, 3, 28, display_template->color565(255, 0, 0), 15, 8, 50, 5000, stopFlag);
    }
  }
}

void drawOpenTemplate(int *open_hour, int *open_minute, int *close_hour, int *close_minute, int speed, int duration, bool *stopFlag) {
  int startMillis = millis();
  int frame = 0;

  char openTime[6];
  char closeTime[6];
  sprintf(openTime, "%02d:%02d", *open_hour, *open_minute);
  sprintf(closeTime, "%02d:%02d", *close_hour, *close_minute);

  while (*stopFlag == false && millis() - startMillis <= duration) {
    display_template->clearScreen();

    drawSnakeBorder(frame, display_template->color565(255, 255, 255), 20, 8);
    frame += 1;

    display_template->setFont(&Font5x7Fixed);
    display_template->setTextColor(display_template->color565(255, 0, 0));
    display_template->setCursor(3, 10);
    display_template->print("WE");
    display_template->setTextColor(display_template->color565(0, 255, 0));
    display_template->setCursor(3, 19);
    display_template->print("ARE");
    display_template->setTextColor(display_template->color565(0, 0, 255));
    display_template->setCursor(3, 28);
    display_template->print("OPEN!");
    display_template->drawFastVLine(31, 1, 30, display_template->color565(255, 255, 255));
    display_template->setTextColor(display_template->color565(0, 255, 255));
    display_template->setCursor(35, 11);
    display_template->print(openTime);
    display_template->setTextColor(display_template->color565(170, 32, 81));
    display_template->setCursor(46, 18);
    display_template->print("~");
    display_template->setTextColor(display_template->color565(138, 9, 178));
    display_template->setCursor(35, 26);
    display_template->print(closeTime);

    delay(speed);
  }
  display_template->clearScreen();
}
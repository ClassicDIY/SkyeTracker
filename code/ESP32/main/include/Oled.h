
#pragma once
#ifdef Has_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Enumerations.h"

namespace CLASSICDIY {
#define LEVEL_FONT 5
#define STATUS_FONT 2
#define HDR_FONT 2
#define MODE_FONT 2
#define DETAIL_FONT 1
#define NumChar(font) SCREEN_WIDTH / (6 * font)

#define LEVEL_Y 24
#define STATUS_Y 0
#define BUF_SIZE 32

class Oled {
 public:
   void Init();
   void update(uint16_t level, const char *state);
   void update(const char* hdr, const char* mode, const char* detail);
   void update(const char* hdr, const char* mode, int count);

 private:
   uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
};

} // namespace CLASSICDIY
#endif
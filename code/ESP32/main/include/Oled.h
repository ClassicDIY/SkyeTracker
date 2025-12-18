
#pragma once
#ifdef Has_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "IDisplayServiceInterface.h"
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

class Oled : public IDisplayServiceInterface {
 public:
   void Init();
   void Display(const char *state, uint16_t level);
   void Display(const char* hdr1, const char* detail1, const char* hdr2, const char* detail2);
   void Display(const char* hdr1, const char* detail1, const char* hdr2, int count);

 private:
   uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
};

} // namespace CLASSICDIY
#endif

#pragma once
#ifdef Has_TFT
#include <TFT_eSPI.h> // Graphics library
#include "IDisplayServiceInterface.h"
#include "Enumerations.h"
#include "Sun.h"
#include "LinearActuatorNoPot.h"

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

struct TFTCache {
   String mode;
   String state;
   String sun;
   String horizontal;
   String vertical;
   String wind;
};

struct TFTHeaderCache {
   String hdr1;
   String detail1;
   String hdr2;
   String detail2;
   int count = -1; // use -1 to indicate “no previous count”
};

class TFT : public IDisplayServiceInterface {
 public:
   void Init();
   void Display(const char *hdr1, const char *detail1, const char *hdr2, const char *detail2);
   void Display(const char *hdr1, const char *detail1, const char *hdr2, int count);
   void Update(JsonDocument &doc);

 private:
   void drawIfChanged(const String &newVal, String &oldVal, int x, int y, uint16_t color);
   uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
   uint16_t _hSplit = 70;
   TFTCache _cache;
   TFTHeaderCache _headerCache;
};

} // namespace CLASSICDIY
#endif
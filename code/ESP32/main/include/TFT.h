
#pragma once
#ifdef Has_TFT
#include <TFT_eSPI.h> // Graphics library
#include "IOledServiceInterface.h"
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

class TFT : public IOledServiceInterface {
 public:
   void Init();
   void Display(const char *state, uint16_t level);
   void Display(const char* hdr1, const char* detail1, const char* hdr2, const char* detail2);
   void Display(const char* hdr1, const char* detail1, const char* hdr2, int count);
   void Update(const char *TrackerMode, const char *TrackerState, Sun* sun, LinearActuatorNoPot* horizontalActuator, LinearActuatorNoPot* verticalActuator);

 private:
   uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
   uint16_t _hSplit = 70;
};

} // namespace CLASSICDIY
#endif
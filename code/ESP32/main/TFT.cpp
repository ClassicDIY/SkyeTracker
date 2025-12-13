#ifdef Has_TFT
#include "TFT.h"
#include "Log.h"
#include "defines.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library

namespace CLASSICDIY {

void TFT::Init() {
   tft.init();
   tft.setRotation(1); // Landscape
   tft.fillScreen(TFT_SKYBLUE);
   logd("Screen init: Width %d, Height %d", tft.width(), tft.height());
}

void TFT::Display(const char *pch, uint16_t level) {
   tft.fillRect(0, 0, tft.width(), _hSplit, TFT_BLACK);
   tft.setTextSize(STATUS_FONT);
   tft.setTextColor(TFT_GREEN);
   tft.setCursor(32, STATUS_Y);
   String state = pch;
   while (state.length() < 10) { // pad right to 10 characters to fill the display line
      state += ' ';
   }
   tft.setCursor(xOffset(2, state.length()), STATUS_Y); // center it
   tft.print(state.c_str());
   char buffer[64];
   sprintf(buffer, "%d%%", level);
   tft.setCursor(xOffset(5, strlen(buffer)), LEVEL_Y);
   tft.setTextSize(LEVEL_FONT);
   tft.setTextColor(TFT_GREEN);
   tft.print(buffer);
}

uint8_t TFT::xOffset(uint8_t textSize, uint8_t numberOfCharaters) {
   uint8_t textPixels = textSize * 6;
   uint8_t rVal = (tft.width() - (numberOfCharaters * textPixels)) / 2;
   return rVal;
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, int count) {
   tft.fillRect(0, 0, tft.width(), _hSplit, TFT_BLACK);
   tft.setTextSize(2);
   tft.setTextSize(HDR_FONT);
   tft.setTextColor(TFT_GREEN);
   tft.setCursor(0, 0);
   char buf[BUF_SIZE];
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr1, NumChar(HDR_FONT));
   tft.println(buf); // limit hdr to 8 char for font size 2
   tft.setTextSize(DETAIL_FONT);
   tft.setCursor(0, 18);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail1, NumChar(DETAIL_FONT));
   tft.println(buf);
   tft.setTextSize(MODE_FONT);
   tft.setCursor(0, 36);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr2, NumChar(MODE_FONT));
   tft.print(buf);
   if (count > 0) {
      tft.printf(":%d", count);
   }
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, const char *detail2) {
   tft.fillRect(0, 0, tft.width(), _hSplit, TFT_BLACK);
   tft.setTextSize(HDR_FONT);
   tft.setTextColor(TFT_GREEN);
   tft.setCursor(0, 0);
   char buf[BUF_SIZE];
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr1, NumChar(HDR_FONT));
   tft.println(buf);
   tft.setTextSize(DETAIL_FONT);
   tft.setCursor(0, 18);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail1, NumChar(DETAIL_FONT));
   tft.println(buf);
   tft.setTextSize(MODE_FONT);
   tft.setCursor(0, 36);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr2, NumChar(MODE_FONT));
   tft.println(buf);
   tft.setTextSize(DETAIL_FONT);
   tft.setCursor(0, 54);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail2, NumChar(DETAIL_FONT));
   tft.println(buf);
}

void TFT::Update(const char *TrackerStateString, Sun *sun, LinearActuatorNoPot *horizontalActuator, LinearActuatorNoPot *verticalActuator) {

   tft.fillRect(0, _hSplit, tft.width(), tft.height(), TFT_BLACK);
   tft.setTextSize(HDR_FONT);
   tft.setTextColor(TFT_GREEN);
   tft.setCursor(0, _hSplit);
   tft.println(TrackerStateString);
   if (sun->ItsDark()) {
      tft.println("It's Dark");
   } else {
      tft.printf("Azimuth %.2f\nElevation %.2f\n", sun->azimuth(), sun->elevation());
   }
   tft.printf("\nHorizontal Actuator\n Position %.2f\"\n Angle %.2f\n", horizontalActuator->CurrentPosition(),
              horizontalActuator->CurrentAngle());
   tft.printf("Vertical Actuator\n Position %.2f\"\n Angle %.2f", verticalActuator->CurrentPosition(), verticalActuator->CurrentAngle());
}

} // namespace CLASSICDIY

#endif
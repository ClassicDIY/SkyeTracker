#ifdef Has_TFT
#include "TFT.h"
#include "Log.h"
#include "defines.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library

namespace CLASSICDIY {

void TFT::Init() {
   tft.init();
   tft.setRotation(1); // Landscape
   tft.fillScreen(TFT_BLACK);
   tft.drawLine(0, _hSplit, tft.width(), _hSplit, TFT_YELLOW);
   logd("Screen init: Width %d, Height %d", tft.width(), tft.height());
}

uint8_t TFT::xOffset(uint8_t textSize, uint8_t numberOfCharaters) {
   uint8_t textPixels = textSize * 6;
   uint8_t rVal = (tft.width() - (numberOfCharaters * textPixels)) / 2;
   return rVal;
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, int count) {
   tft.setTextFont(1);
   int y = 0;
   // hdr1
   tft.setTextSize(HDR_FONT);
   drawIfChanged(String(hdr1), _headerCache.hdr1, 0, y, TFT_GREEN);
   y += 18;
   // detail1
   tft.setTextSize(DETAIL_FONT);
   drawIfChanged(String(detail1), _headerCache.detail1, 0, y, TFT_GREEN);
   y += 18;
   // hdr2 + count
   tft.setTextSize(MODE_FONT);
   String hdr2Full = String(hdr2);
   if (count > 0) {
      hdr2Full += ":" + String(count);
   }
   drawIfChanged(hdr2Full, _headerCache.hdr2, 0, y, TFT_GREEN);
   _headerCache.count = count;
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, const char *detail2) {
   tft.setTextFont(1);
   int y = 0;
   // hdr1
   tft.setTextSize(HDR_FONT);
   drawIfChanged(String(hdr1), _headerCache.hdr1, 0, y, TFT_GREEN);
   y += 18;
   // detail1
   tft.setTextSize(DETAIL_FONT);
   drawIfChanged(String(detail1), _headerCache.detail1, 0, y, TFT_GREEN);
   y += 18;
   // hdr2
   tft.setTextSize(MODE_FONT);
   drawIfChanged(String(hdr2), _headerCache.hdr2, 0, y, TFT_GREEN);
   y += 18;
   // detail2
   tft.setTextSize(DETAIL_FONT);
   drawIfChanged(String(detail2), _headerCache.detail2, 0, y, TFT_GREEN);
}

void TFT::Update(JsonDocument &doc) {
   tft.setFreeFont(&FreeSerif9pt7b);
   tft.setTextSize(DETAIL_FONT);
   int y = _hSplit + 20;
   drawIfChanged(doc["mode"].as<String>(), _cache.mode, 0, y, TFT_GREEN);
   y += 20;
   drawIfChanged(doc["state"].as<String>(), _cache.state, 0, y, TFT_GREEN);
   y += 20;
   drawIfChanged("Azimuth: " + doc["azimuth"].as<String>() + "° Elevation: " + doc["elevation"].as<String>() + "°", _cache.sun, 0, y, TFT_GREEN);
   y += 20;
   drawIfChanged("Horizontal @: " + doc["horizontal_extent"].as<String>() + "\" Angle: " + doc["horizontal_angle"].as<String>() + "°", _cache.horizontal, 0, y, TFT_GREEN);
   y += 20;
   drawIfChanged("Vertical @: " + doc["vertical_extent"].as<String>()+ "\" Angle: " + doc["vertical_angle"].as<String>() + "°", _cache.vertical, 0, y, TFT_GREEN);
   y += 20;
   if (!doc["wind_speed"].isNull()) {
      drawIfChanged("Wind speed: " + doc["wind_speed"].as<String>() + " km/hr", _cache.wind, 0, y, TFT_GREEN);
   }
}

void TFT::drawIfChanged(const String &newVal, String &oldVal, int x, int y, uint16_t color) {
   if (newVal != oldVal) {
      tft.setTextColor(TFT_BLACK, TFT_BLACK); // erases old text
      tft.drawString(oldVal, x, y);
      tft.setTextColor(color, TFT_BLACK); // new text
      tft.drawString(newVal, x, y);
      oldVal = newVal;
   }
}

} // namespace CLASSICDIY

#endif
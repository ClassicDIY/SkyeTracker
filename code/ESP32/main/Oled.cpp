#ifdef Has_OLED
#include "Wire.h"
#include "Oled.h"
#include "Log.h"
#include "defines.h"

Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

namespace CLASSICDIY {

void Oled::Init() {
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
}

void Oled::Display(const char *pch, uint16_t level) {
   oled_display.clearDisplay();
   oled_display.setTextSize(STATUS_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.setCursor(32, STATUS_Y);
   String state = pch;
   while (state.length() < 10) { // pad right to 10 characters to fill the display line
      state += ' ';
   }
   oled_display.setCursor(xOffset(2, state.length()), STATUS_Y); // center it
   oled_display.print(state.c_str());
   char buffer[64];
   sprintf(buffer, "%d%%", level);
   oled_display.setCursor(xOffset(5, strlen(buffer)), LEVEL_Y);
   oled_display.setTextSize(LEVEL_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.print(buffer);
   oled_display.display();
}

uint8_t Oled::xOffset(uint8_t textSize, uint8_t numberOfCharaters) {
   uint8_t textPixels = textSize * 6;
   uint8_t rVal = (SCREEN_WIDTH - (numberOfCharaters * textPixels)) / 2;
   return rVal;
}

void Oled::Display(const char* hdr1, const char* detail1, const char* hdr2, int count) {
   logv("Oled update: %s,  %s, %d", hdr, mode, count);
   oled_display.clearDisplay();
   oled_display.setTextSize(2);
   oled_display.clearDisplay();
   oled_display.setTextSize(HDR_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.setCursor(0, 0);
   char buf[BUF_SIZE];
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr1, NumChar(HDR_FONT));
   oled_display.println(buf); // limit hdr to 8 char for font size 2
   oled_display.setTextSize(DETAIL_FONT);
   oled_display.setCursor(0, 18);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail1, NumChar(DETAIL_FONT));
   oled_display.println(buf);
   oled_display.setTextSize(MODE_FONT);
   oled_display.setCursor(0, 36);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr2, NumChar(MODE_FONT));
   oled_display.print(buf);
   if (count > 0) {
      oled_display.printf(":%d", count);
   }
   oled_display.display();
}

void Oled::Display(const char* hdr1, const char* detail1, const char* hdr2, const char* detail2) {
   logv("Oled update: %s,  %s, %s", hdr, mode, detail);
   oled_display.clearDisplay();
   oled_display.setTextSize(HDR_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.setCursor(0, 0);
   char buf[BUF_SIZE];
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr1, NumChar(HDR_FONT));
   oled_display.println(buf); // limit hdr to 8 char for font size 2
   oled_display.setTextSize(DETAIL_FONT);
   oled_display.setCursor(0, 18);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail1, NumChar(DETAIL_FONT));
   oled_display.println(buf);
   oled_display.setTextSize(MODE_FONT);
   oled_display.setCursor(0, 36);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, hdr2, NumChar(MODE_FONT));
   oled_display.println(buf);
   oled_display.setTextSize(DETAIL_FONT);
   oled_display.setCursor(0, 54);
   memset(buf, 0, BUF_SIZE);
   strncpy(buf, detail2, NumChar(DETAIL_FONT));
   oled_display.println(buf);
   oled_display.display();
}
} // namespace CLASSICDIY

#endif
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

namespace CLASSICDIY {
class IDisplayServiceInterface {
 public:
   virtual void Display(const char* hdr1, const char* detail1, const char* hdr2, const char* detail2) = 0;
   virtual void Display(const char* hdr1, const char* detail1, const char* hdr2, int count) = 0;
};
} // namespace CLASSICDIY
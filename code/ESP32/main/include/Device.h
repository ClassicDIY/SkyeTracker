#pragma once
#include <Arduino.h>
#include <memory>
#include "GPIO_pins.h"
#include "Enumerations.h"
#include "Oled.h"
#include "tft.h"
#ifdef Lilygo_Relay_6CH
#include <ShiftRegister74HC595.h>
#endif

namespace CLASSICDIY {
class Device {
 protected:
   void Init();
   void InitCommon();
   void Run();
   void SetRelay(const uint8_t index, const uint8_t value);
   boolean GetRelay(const uint8_t index);
   void SetRTC(struct tm *tm);
#ifdef Lilygo_Relay_6CH
   std::shared_ptr<ShiftRegister74HC595<1>> _reg;
#endif
#ifdef Has_OLED
   Oled _oled = Oled();
#endif
#ifdef Has_TFT
   TFT _tft = TFT();
#endif
   NetworkState _networkState = Boot;
   unsigned long _lastBlinkTime = 0;
   bool _blinkStateOn = false;
   bool _running = false;
};

} // namespace CLASSICDIY
#include <Arduino.h>
#include "Wire.h"
#include <ArduinoJson.h>
#include "Log.h"
#include "Device.h"

#ifdef Has_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#ifdef HasRTC
#include "RTClib.h"
RTC_PCF8563 rtc;
#endif

#ifdef EDGEBOX
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
#endif

namespace CLASSICDIY {

#ifdef ESP_32Dev

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif

   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   pinMode(WIFI_STATUS_PIN, OUTPUT);
}

void Device::Run() {
   // handle blink led, fast : NotConnected slow: AP connected On: Station connected
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         digitalWrite(WIFI_STATUS_PIN, _blinkStateOn ? HIGH : LOW);
      }
   } else {
      digitalWrite(WIFI_STATUS_PIN, HIGH);
   }
}

// void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_Coils[index], value); }

// boolean Device::GetRelay(const uint8_t index) { return digitalRead(_Coils[index]) == 0 ? false : true; }

// bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif

#ifdef Waveshare_Relay_6CH

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   for (int i = 0; i < DO_PINS; i++) {
      pinMode(_Coils[i], OUTPUT);
   }
   pinMode(RGB_LED_PIN, OUTPUT);     // Initialize the control GPIO of RGB
   pinMode(GPIO_PIN_Buzzer, OUTPUT); // Initialize the control GPIO of Buzzer

   ledcSetup(PWM_Channel, Frequency, Resolution); // Set PWM channel
   ledcAttachPin(GPIO_PIN_Buzzer, PWM_Channel);   // Connect the channel to the corresponding pin
}

void Device::Run() {
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         neopixelWrite(RGB_LED_PIN, _blinkStateOn ? 60 : 0, _blinkStateOn ? 0 : 60, 0);
      }
   } else if (!_running) {
      neopixelWrite(RGB_LED_PIN, 0, 0, 60);
      _running = true;
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_Coils[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_Coils[index]) == 0 ? false : true; }

bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif
#ifdef Lilygo_Relay_6CH

void Device::Init() {
   _reg = std::make_shared<ShiftRegister74HC595<1>>(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH);
   pinMode(HT74HC595_OUT_EN, OUTPUT);
   digitalWrite(HT74HC595_OUT_EN, HIGH);
   _reg->setAllLow();
   logd("Set HT74HC595_OUT_EN to low level to enable relay output");
   digitalWrite(HT74HC595_OUT_EN, LOW);
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
#ifdef HasRTC
   if (!rtc.begin(&Wire)) {
      loge("Couldn't find RTC");
   }
   if (rtc.lostPower()) {
      logw("RTC is NOT initialized");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
   rtc.start();
   DateTime dt = rtc.now();
   logi("Date Time: %s", dt.timestamp().c_str());
   struct tm tm;
   tm.tm_year = dt.year() - 1900;
   tm.tm_mon = dt.month() - 1;
   tm.tm_mday = dt.day();
   tm.tm_hour = dt.hour();
   tm.tm_min = dt.minute();
   tm.tm_sec = dt.second();
   time_t t = mktime(&tm);
   struct timeval tv = {.tv_sec = t, .tv_usec = 0};
   settimeofday(&tv, nullptr);
#endif
}

void Device::Run() {
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         _reg->set(7, _blinkStateOn ? HIGH : LOW);
      }
   } else if (!_running) {
      _reg->set(7, LOW);
      _reg->set(6, HIGH);
      _running = true;
   }
}
void Device::SetRTC(struct tm *tm) {
      // ✅ Write back to RTC
      DateTime dt(
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
      );
      rtc.adjust(dt);

}

void Device::SetRelay(const uint8_t index, const uint8_t value) { _reg->set(index, value); }

boolean Device::GetRelay(const uint8_t index) { return _reg->get(index); }

#endif
#ifdef Lilygo_Relay_4CH

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   for (int i = 0; i < DO_PINS; i++) {
      pinMode(_Coils[i], OUTPUT);
   }
   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   pinMode(WIFI_STATUS_PIN, OUTPUT);
}

void Device::Run() {
   // handle blink led, fast : NotConnected slow: AP connected On: Station connected
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         digitalWrite(WIFI_STATUS_PIN, _blinkStateOn ? HIGH : LOW);
      }
   } else {
      digitalWrite(WIFI_STATUS_PIN, HIGH);
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_Coils[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_Coils[index]) == 0 ? false : true; }

bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif

} // namespace CLASSICDIY
#include <Arduino.h>
#include <memory>
#include "Wire.h"
#include <ArduinoJson.h>
#include "Log.h"
#include "Device.h"

#ifdef Has_OLED_Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
namespace CLASSICDIY {

#ifdef ESP_32Dev

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
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
#ifdef EDGEBOX

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
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
#ifndef LOG_TO_SERIAL_PORT // disable logs to use LED wifi status
                           // use LED if the log level is none (edgeBox shares the LED pin with the serial TX gpio)
   pinMode(WIFI_STATUS_PIN, OUTPUT);
#endif
   if (!ads.begin(0x48, &Wire)) {
      loge("Failed to initialize ADS.");
   }
}

void Device::Run() {
#ifndef LOG_TO_SERIAL_PORT // disable logs to use LED wifi status
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
#endif
   // transfer analog data from sensors to RegisterSet
   for (int i = 0; i < AI_PINS; i++) {
      _AnalogSensors[i].Run();
      _analogInputRegisters.set(i, _AnalogSensors[i].Level());
   }
   // transfer digital data from sensors to CoilData
   for (int i = 0; i < DI_PINS; i++) {
      _digitalInputDiscretes.set(i, GetDigitalLevel(i));
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_Coils[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_Coils[index]) == 0 ? false : true; }

bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif
#ifdef NORVI_GSM_AE02

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   for (int i = 0; i < DO_PINS; i++) {
      pinMode(_Coils[i], OUTPUT);
   }
}

void Device::Run() {}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_Coils[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_Coils[index]) == 0 ? false : true; }

bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif

#ifdef Waveshare_Relay_6CH

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
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
      logw("RTC is NOT initialized, let's set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
   // rtc.adjust(DateTime("Apr 16 2020","18:34:56"));
   rtc.start();
   DateTime now = rtc.now();
   logi("Date Time: %s", now.timestamp().c_str());
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

#include <ShiftRegister74HC595.h>
std::shared_ptr<ShiftRegister74HC595<1>> HT74HC595 =
    std::make_shared<ShiftRegister74HC595<1>>(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH);

void Device::Init() {
   pinMode(HT74HC595_OUT_EN, OUTPUT);
   digitalWrite(HT74HC595_OUT_EN, HIGH);
   HT74HC595->setAllLow();
   logd("Set HT74HC595_OUT_EN to low level to enable relay output");
   digitalWrite(HT74HC595_OUT_EN, LOW);

   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
}

void Device::Run() {
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         HT74HC595->set(7, _blinkStateOn ? HIGH : LOW);
      }
   } else if (!_running) {
      HT74HC595->set(7, LOW);
      HT74HC595->set(6, HIGH);
      _running = true;
   }
}

// void Device::SetRelay(const uint8_t index, const uint8_t value) { HT74HC595->set(index, value); }

// boolean Device::GetRelay(const uint8_t index) { return HT74HC595->get(index); }

// bool Device::GetDigitalLevel(const uint8_t index) { return (bool)digitalRead(_DigitalSensors[index]); }

#endif
#ifdef Lilygo_Relay_4CH

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
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
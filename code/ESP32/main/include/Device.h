#pragma once
#include <Arduino.h>
#include "GPIO_pins.h"
#include "Enumerations.h"

namespace CLASSICDIY {
class Device {
 protected:
   void Init();
   void Run();
   // void SetRelay(const uint8_t index, const uint8_t value);
   // boolean GetRelay(const uint8_t index);
   // bool GetDigitalLevel(const uint8_t index);

   NetworkState _networkState = Boot;
   unsigned long _lastBlinkTime = 0;
   bool _blinkStateOn = false;
   bool _running = false;

   // #ifdef EDGEBOX
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1, DO2, DO3, DO4, DO5};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {DI0, DI1, DI2, DI3};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {AI0, AI1, AI2, AI3};
   //    PWMOutput _PWMOutputs[AO_PINS] = {AO0, AO1};
   // #elif NORVI_GSM_AE02
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {DI0, DI1, DI2, DI3, DI4, DI5, DI6, DI7};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {AI0, AI1, AI2, AI3};
   //    PWMOutput _PWMOutputs[AO_PINS] = {};
   // #elif LILYGO_T_SIM7600G
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {DI0, DI1};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {};
   //    PWMOutput _PWMOutputs[AO_PINS] = {};
   // #elif Waveshare_Relay_6CH
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1, DO2, DO3, DO4, DO5};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {};
   //    PWMOutput _PWMOutputs[AO_PINS] = {};
   // #elif Lilygo_Relay_4CH
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1, DO2, DO3};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {};
   //    PWMOutput _PWMOutputs[AO_PINS] = {};
   // #elif ESP_32Dev
   //    gpio_num_t _Coils[DO_PINS] = {DO0, DO1};
   //    gpio_num_t _DigitalSensors[DI_PINS] = {DI0, DI1};
   //    AnalogSensor _AnalogSensors[AI_PINS] = {};
   //    PWMOutput _PWMOutputs[AO_PINS] = {};
   // #endif
};

} // namespace CLASSICDIY
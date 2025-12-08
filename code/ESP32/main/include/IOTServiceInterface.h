#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Enumerations.h"

namespace CLASSICDIY {
class IOTServiceInterface {
 public:
#ifdef MQTT
   // MQTT related methods
   virtual boolean Publish(const char *subtopic, const char *value, boolean retained) = 0;
   virtual boolean Publish(const char *subtopic, float value, boolean retained) = 0;
   virtual boolean PublishMessage(const char *topic, JsonDocument &payload, boolean retained) = 0;
   virtual String getRootTopicPrefix() = 0;
   virtual u_int getUniqueId() = 0;
   virtual String getThingName() = 0;
#endif
#ifdef Modbus
   // Modbus related methods
   virtual uint16_t getMBBaseAddress(IOTypes type) = 0;
   virtual void registerMBTCPWorkers(FunctionCode fc, MBSworker worker) = 0;
#endif
};
} // namespace CLASSICDIY
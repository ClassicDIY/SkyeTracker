#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "mqtt_client.h"
#ifdef HasModbus
#include <ModbusServerTCPasync.h>
#endif
#include "Enumerations.h"
#include "IDisplayServiceInterface.h"

namespace CLASSICDIY {
class IOTCallbackInterface {
 public:
   virtual void onNetworkState(NetworkState state) = 0;
   virtual String appTemplateProcessor(const String &var);
   virtual void onSaveSetting(JsonDocument &doc) = 0;
   virtual void onLoadSetting(JsonDocument &doc) = 0;
#ifdef HasMQTT
   virtual void onMqttConnect(esp_mqtt_client_handle_t& client) = 0;
   virtual void onMqttMessage(char *topic, char *payload) = 0;
#endif
#if defined(HasModbus) && defined(HasRS485)
   virtual bool onModbusMessage(ModbusMessage &msg);
#endif
#if  defined(Has_OLED) || defined(Has_TFT)
   virtual IDisplayServiceInterface& getDisplayInterface() = 0;
#endif
};
} // namespace CLASSICDIY
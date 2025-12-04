#pragma once
#include <Arduino.h>
#include "ArduinoJson.h"
#include <EEPROM.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#ifdef HasModbus
#include <ModbusServerTCPasync.h>
#include <ModbusServerRTU.h>
#include <ModbusClientRTU.h>
#endif
#ifdef HasMQTT
#include "mqtt_client.h"
#endif
#include "time.h"
#include <sstream>
#include <string>
#include "Defines.h"
#include "Enumerations.h"
#include "OTA.h"
#include "IOTServiceInterface.h"
#include "IOTCallbackInterface.h"

namespace CLASSICDIY {
class IOT : public IOTServiceInterface {
 public:
   IOT() {};
   void Init(IOTCallbackInterface *iotCB, AsyncWebServer *pwebServer);
   void Run();
   u_int getUniqueId() { return _uniqueId; };
   std::string getThingName();
   NetworkState getNetworkState() { return _networkState; }
   void GoOnline();

#ifdef HasMQTT
   std::string getRootTopicPrefix();
   boolean Publish(const char *subtopic, const char *value, boolean retained = false);
   boolean Publish(const char *subtopic, JsonDocument &payload, boolean retained = false);
   boolean Publish(const char *subtopic, float value, boolean retained = false);
   boolean PublishMessage(const char *topic, JsonDocument &payload, boolean retained);
#endif

#ifdef HasModbus
   boolean ModbusBridgeEnabled();
   void registerMBTCPWorkers(FunctionCode fc, MBSworker worker);
   Modbus::Error SendToModbusBridgeAsync(ModbusMessage &request);
   uint16_t getMBBaseAddress(IOTypes type);
#else
   boolean ModbusBridgeEnabled() { return false; };
#endif

 private:
   OTA _OTA = OTA();
   AsyncWebServer *_pwebServer;
   NetworkState _networkState = Boot;
   NetworkSelection _NetworkSelection = NotConnected;
   bool _blinkStateOn = false;
   String _AP_SSID = TAG;
   String _AP_Password = DEFAULT_AP_PASSWORD;
   bool _AP_Connected = false;
   String _SSID;
   String _WiFi_Password;
   String _APN;
   String _SIM_Username;
   String _SIM_Password;
   String _SIM_PIN;
   bool _useDHCP = false;
   char _Current_IP[16] = "";
   String _Static_IP;
   String _Subnet_Mask;
   String _Gateway_IP;
   uint32_t _settingsChecksum = 0;
   bool _needToReboot = false;
   String _bodyBuffer;

#ifdef HasMQTT
   bool _useMQTT = false;
   String _mqttServer;
   uint16_t _mqttPort = 1883;
   String _mqttUserName;
   String _mqttUserPassword;
   char _willTopic[STR_LEN * 2];
   char _rootTopicPrefix[STR_LEN];
   esp_mqtt_client_handle_t _mqtt_client_handle = 0;
   void ConnectToMQTTServer();
   void HandleMQTT(int32_t event_id, void *event_data);
   void StopMQTT();
   static void mqttReconnectTimerCF(TimerHandle_t xTimer) {
      // Retrieve the instance of the class (stored as the timer's ID)
      IOT *instance = static_cast<IOT *>(pvTimerGetTimerID(xTimer));
      if (instance != nullptr) {
         instance->ConnectToMQTTServer();
      }
   }
   static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
      IOT *instance = static_cast<IOT *>(handler_args);
      if (instance != nullptr) {
         instance->HandleMQTT(event_id, event_data);
      }
   }
#endif

#ifdef HasModbus
   bool _useModbus = false;
   ModbusMode _ModbusMode = TCP;
   uint16_t _modbusPort = 502;
   unsigned long _modbusBaudRate = 9600;
   uart_parity_t _modbusParity = UART_PARITY_DISABLE;
   uart_stop_bits_t _modbusStopBits = UART_STOP_BITS_1;
   uint16_t _modbusID = 1;

   bool _useModbusBridge = false;
   unsigned long _clientRTUBaud = 9600;
   uart_parity_t _clientRTUParity = UART_PARITY_DISABLE;
   uart_stop_bits_t _clientRTUStopBits = UART_STOP_BITS_1;
   uint32_t _Token = 1000;
   uint32_t nextToken() {
      _Token++;
      _Token %= 65535;
      return _Token;
   }
   uint16_t _input_register_base_addr = INPUT_REGISTER_BASE_ADDRESS;
   uint16_t _coil_base_addr = COIL_BASE_ADDRESS;
   uint16_t _discrete_input_base_addr = DISCRETE_BASE_ADDRESS;
   uint16_t _holding_register_base_addr = HOLDING_REGISTER_BASE_ADDRESS;
#endif

   IOTCallbackInterface *_iotCB;
   u_int _uniqueId = 0; // unique id from mac address NIC segment
   unsigned long _lastBlinkTime = 0;
   unsigned long _lastBootTimeStamp = millis();
   unsigned long _waitInAPTimeStamp = millis();
   unsigned long _NetworkConnectionStart = 0;
   unsigned long _GPIO0_PressedCountdown = 0;
   unsigned long _FlasherIPConfigStart = millis();
   void RedirectToHome(AsyncWebServerRequest *request);
   void UpdateOledDisplay();
   void GoOffline();
   void saveSettings();
   void loadSettings();
   void loadSettingsFromJson(JsonDocument &doc);
   void saveSettingsToJson(JsonDocument &doc);
   void setState(NetworkState newState);
#ifdef HasLTE
   void wakeup_modem(void);
   esp_netif_t *_netif = NULL;
   esp_err_t ConnectModem();
   void DisconnectModem();
   esp_eth_handle_t _eth_handle = NULL;
   esp_eth_netif_glue_handle_t _eth_netif_glue;
#endif
#ifdef HasEthernet
   esp_err_t ConnectEthernet();
   void DisconnectEthernet();
#endif
   void HandleIPEvent(int32_t event_id, void *event_data);
   static void on_ip_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
      IOT *instance = static_cast<IOT *>(arg);
      if (instance) {
         instance->HandleIPEvent(event_id, event_data);
      }
   }
};

} // namespace CLASSICDIY

#include <sys/time.h>
#include <thread>
#include <chrono>
#include <ESPmDNS.h>
#ifdef UseLittleFS
#include <LittleFS.h>
#else
#include "iot_script.js"
#include "style.css"
#endif
#ifdef HasEthernet
#include <SPI.h>
#include <Ethernet.h>
#include <esp_eth.h>
#include "esp_eth_mac.h"
#include "driver/spi_master.h"
#include "esp_wifi.h"
#endif
#ifdef HasLTE
#include "network_dce.h"
#include "esp_netif_ppp.h"
#endif
#include "Log.h"
#include "WebLog.h"
#include "IOT.h"
#include "IOT.htm"
#include "HelperFunctions.h"

namespace CLASSICDIY {

static DNSServer _dnsServer;
static WebLog _webLog;
#ifdef HasMQTT
TimerHandle_t mqttReconnectTimer;
#endif
#ifdef HasModbus
static ModbusServerTCPasync _MBserver;
static ModbusServerRTU _MBRTUserver(MODBUS_RTU_TIMEOUT);
#endif

static AsyncAuthenticationMiddleware basicAuth;
const char *NetworkSelectionStrings[] = {"", "AP Mode", "WiFi", "Ethernet", "Modem"};
const char *NetworkStateStrings[] = {"Boot", "Ap State", "Connecting", "OnLine", "OffLine"};

// #pragma region Setup
void IOT::Init(IOTCallbackInterface *iotCB, AsyncWebServer *pwebServer) {
   _iotCB = iotCB;
   _pwebServer = pwebServer;
#ifdef WIFI_STATUS_PIN
   pinMode(WIFI_STATUS_PIN,
           OUTPUT); // use LED for wifi AP status (note:edgeBox shares the LED pin with the serial TX gpio)
#endif
   pinMode(GPIO_NUM_0, INPUT_PULLUP);
#ifdef FACTORY_RESET_PIN // use digital input pin for factory reset
   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   EEPROM.begin(EEPROM_SIZE);
   if (digitalRead(FACTORY_RESET_PIN) == LOW) {
      logi("Factory Reset");
      EEPROM.write(0, 0);
      EEPROM.commit();
      saveSettings();
   }
#elif BUTTONS // use analog pin for factory reset
   EEPROM.begin(EEPROM_SIZE);
   uint16_t analogValue = analogRead(BUTTONS);
   logd("button value (%d)", analogValue);
   if (analogValue > 3000) {
      logi("**********************Factory Reset*************************(%d)", analogValue);
      EEPROM.write(0, 0);
      EEPROM.commit();
      saveSettings();
   }
#endif
   loadSettings();
#ifdef HasRS485
   if (RS485_RTS != -1) {
      pinMode(RS485_RTS, OUTPUT);
      digitalWrite(RS485_RTS, LOW);
   }
   if (_ModbusMode == RTU) {
      // Set up Serial2 connected to Modbus RTU server
      RTUutils::prepareHardwareSerial(Serial2);
      SerialConfig conf = getSerialConfig(_modbusParity, _modbusStopBits);
      logd("Serial baud: %d conf: 0x%x", _modbusBaudRate, conf);
      Serial2.begin(_modbusBaudRate, conf, RS485_RXD, RS485_TXD);
      while (!Serial2) {
      }
   }
#endif
#ifdef HasMQTT
   mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(8000), pdFALSE, this, mqttReconnectTimerCF);
#endif
   WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
      String s;
      JsonDocument doc;
      switch (event) {
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
         logd("AP_STADISCONNECTED");
         if (WiFi.softAPgetStationNum() == 0) {
            _AP_Connected = false;
            GoOffline();
         }
         break;
      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
         logd("AP_STAIPASSIGNED");
         sprintf(_Current_IP, "%s", WiFi.softAPIP().toString().c_str());
         logd("Current_IP: %s", _Current_IP);
         _AP_Connected = true;
         GoOnline();
         break;
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
         logd("STA_GOT_IP");
         doc["IP"] = WiFi.localIP().toString().c_str();
         sprintf(_Current_IP, "%s", WiFi.localIP().toString().c_str());
         logi("Got IP Address");
         logi("~~~~~~~~~~~");
         logi("IP: %s", _Current_IP);
         logi("IPMASK: %s", WiFi.subnetMask().toString().c_str());
         logi("Gateway: %s", WiFi.gatewayIP().toString().c_str());
         logi("~~~~~~~~~~~");
         doc["ApPassword"] = DEFAULT_AP_PASSWORD;
         serializeJson(doc, s);
         s += '\n';
         Serial.printf(s.c_str()); // send json to flash tool
         GoOnline();
         break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
         logw("STA_DISCONNECTED");
         GoOffline();
         break;
      default:
         logd("[WiFi-event] event: %d", event);
         break;
      }
   });
   // generate unique id from mac address NIC segment
   uint8_t chipid[6];
   esp_efuse_mac_get_default(chipid);
   _uniqueId = chipid[3] << 16;
   _uniqueId += chipid[4] << 8;
   _uniqueId += chipid[5];
   _lastBootTimeStamp = millis();
   _pwebServer->on("/reboot", [this](AsyncWebServerRequest *request) {
      RedirectToHome(request);
      _needToReboot = true;
   });
   _pwebServer->onNotFound([this](AsyncWebServerRequest *request) {
      logv("uri not found! %s", request->url().c_str());
      RedirectToHome(request);
   });
   basicAuth.setUsername("admin");
   basicAuth.setPassword(_AP_Password.c_str());
   basicAuth.setAuthFailureMessage("Authentication failed!");
   basicAuth.setAuthType(_NetworkSelection <= APMode ? AsyncAuthType::AUTH_NONE : AsyncAuthType::AUTH_BASIC); // skip credentials in APMode
   basicAuth.generateHash();
   // Serve favicon.ico
   //   server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
   //     request->send_P(200, "image/x-icon", (const char*)favicon_ico, sizeof(favicon_ico));
   //   });

   _pwebServer->on("/style.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
#ifdef UseLittleFS
      if (LittleFS.exists("/style.css")) {
         // Serve from filesystem
         request->send(LittleFS, "/style.css", "text/css");
      }
#else
      request->send(200, "text/css", iot_style);
#endif
   });
   _pwebServer->on("/script.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
#ifdef UseLittleFS
      if (LittleFS.exists("/iot_script.js")) {
         // Serve from filesystem
         request->send(LittleFS, "/iot_script.js", "application/javascript", true, [this](const String &var) {
            logd("script template: %s", var.c_str());
            return _iotCB->appTemplateProcessor(var);
         });
      }
#else
      request->send(200, "application/javascript", iot_script, [this](const String &var) {
         logd("script template: %s", var.c_str());
         return _iotCB->appTemplateProcessor(var);
      });
#endif
   });
   // Return the /settings web page
   _pwebServer
       ->on("/settings", HTTP_GET,
            [this](AsyncWebServerRequest *request) {
               request->send(200, "text/html", network_config_top, [this](const String &var) {
                  // logd("html template: %s", var.c_str());
                  if (var == "title") {
                     return _AP_SSID;
                  }
                  if (var == "header") {
                     return _AP_SSID;
                  }
                  if (var == "version") {
                     return String(APP_VERSION);
                  }
                  if (var == "iot_fields") {
                     String fields = network_config;
                     fields.replace("%n%", _AP_SSID);
                     fields.replace("%version%", APP_VERSION);
#ifndef HasEthernet
                     fields.replace("%ETH%", "class='hidden'");
#endif
#ifndef HasLTE
                     fields.replace("%4G%", "class='hidden'");
#endif
#ifdef HasMQTT
                     String mqtt = config_mqtt;
                     fields += mqtt;
#endif
#ifdef HasModbus
                     String modbus = config_modbus;
                     // hide unused modbus functions
                     modbus.replace("{inputRegDivClass}", InputRegistersDiv);
                     modbus.replace("{coilDivClass}", CoilsDiv);
                     modbus.replace("{discreteDivClass}", DiscretesDiv);
                     modbus.replace("{holdingRegDivClass}", HoldingRegistersDiv);
                     fields += modbus;
#endif
                     return fields;
                  }
                  if (var == "config_links") {
#ifdef HasOTA
                     return String(config_links);
#else 
            return String(config_links_no_ota);
#endif
                  }
                  return _iotCB->appTemplateProcessor(var);
               });
            })
       .addMiddleware(&basicAuth);
   _pwebServer->on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request) { logd("/ **************************** submit called with %d args", request->args()); });

   _pwebServer->on("/iot_fields", HTTP_GET, [this](AsyncWebServerRequest *request) {
      JsonDocument doc;
      saveSettingsToJson(doc);
      logd("HTTP_GET iot_fields: %s", formattedJson(doc).c_str());
      String s;
      serializeJson(doc, s);
      request->send(200, "text/html", s);
   });
   _pwebServer->on(
       "/iot_fields", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          // Called after all chunks are received
          logv("Full body received: %s", _bodyBuffer.c_str());
          // Parse JSON safely
          JsonDocument doc; // adjust size to expected payload
          DeserializationError err = deserializeJson(doc, _bodyBuffer);
          if (err) {
             logd("JSON parse failed: %s", err.c_str());
          } else {
             logd("HTTP_POST iot_fields: %s", formattedJson(doc).c_str());
             loadSettingsFromJson(doc);
             saveSettings();
             RedirectToHome(request);
          }
          request->send(200, "application/json", "{\"status\":\"ok\"}");
          _bodyBuffer = ""; // clear for next request
       },
       NULL, // file upload handler (not used here)
       [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
          logv("Chunk received: len=%d, index=%d, total=%d", len, index, total);
          // Append chunk to buffer
          _bodyBuffer.reserve(total); // reserve once for efficiency
          for (size_t i = 0; i < len; i++) {
             _bodyBuffer += (char)data[i];
          }
          if (index + len == total) {
             logd("Upload complete!");
          }
       });
}

void IOT::RedirectToHome(AsyncWebServerRequest *request) {
   // logd("Redirecting from: %s", request->url().c_str());
   String page = redirect_html;
   page.replace("%n%", _SSID);
   page.replace("{ip}", _Current_IP);
   request->send(200, "text/html", page);
}

void IOT::loadSettingsFromJson(JsonDocument &iot) {
   _AP_SSID = iot["AP_SSID"].isNull() ? TAG : iot["AP_SSID"].as<String>();
   _AP_Password = iot["AP_Pw"].isNull() ? DEFAULT_AP_PASSWORD : iot["AP_Pw"].as<String>();
   _NetworkSelection = iot["Network"].isNull() ? APMode : iot["Network"].as<NetworkSelection>();
   String ssid = iot["SSID"].isNull() ? "" : iot["SSID"].as<String>();
   ssid.trim();
   _SSID = ssid.c_str();
   _WiFi_Password = iot["WiFi_Pw"].isNull() ? "" : iot["WiFi_Pw"].as<String>();
   _APN = iot["APN"].isNull() ? "" : iot["APN"].as<String>();
   _SIM_Username = iot["SIM_USERNAME"].isNull() ? "" : iot["SIM_USERNAME"].as<String>();
   _SIM_Password = iot["SIM_PASSWORD"].isNull() ? "" : iot["SIM_PASSWORD"].as<String>();
   _SIM_PIN = iot["SIM_PIN"].isNull() ? "" : iot["SIM_PIN"].as<String>();
   _useDHCP = iot["useDHCP"].isNull() ? false : iot["useDHCP"].as<bool>();
   _Static_IP = iot["ETH_SIP"].isNull() ? "" : iot["ETH_SIP"].as<String>();
   _Subnet_Mask = iot["ETH_SM"].isNull() ? "" : iot["ETH_SM"].as<String>();
   _Gateway_IP = iot["ETH_GW"].isNull() ? "" : iot["ETH_GW"].as<String>();
#ifdef HasMQTT
   _useMQTT = iot["useMQTT"].isNull() ? false : iot["useMQTT"].as<bool>();
   _mqttServer = iot["mqttServer"].isNull() ? "" : iot["mqttServer"].as<String>();
   _mqttPort = iot["mqttPort"].isNull() ? 1883 : iot["mqttPort"].as<uint16_t>();
   _mqttUserName = iot["mqttUser"].isNull() ? "" : iot["mqttUser"].as<String>();
   _mqttUserPassword = iot["mqttPw"].isNull() ? "" : iot["mqttPw"].as<String>();
#endif
#ifdef HasModbus
   _useModbus = iot["useModbus"].isNull() ? false : iot["useModbus"].as<bool>();
   _ModbusMode = iot["modbusMode"].isNull() ? TCP : iot["modbusMode"].as<ModbusMode>();
   _modbusBaudRate = iot["svrRTUBaud"].isNull() ? 9600 : iot["svrRTUBaud"].as<uint32_t>();
   _modbusParity = iot["svrRTUParity"].isNull() ? UART_PARITY_DISABLE : iot["svrRTUParity"].as<uart_parity_t>();
   _modbusStopBits = iot["svrRTUStopBits"].isNull() ? UART_STOP_BITS_1 : iot["svrRTUStopBits"].as<uart_stop_bits_t>();
   _modbusPort = iot["modbusPort"].isNull() ? 502 : iot["modbusPort"].as<uint16_t>();
   _modbusID = iot["modbusID"].isNull() ? 1 : iot["modbusID"].as<uint16_t>();
   _input_register_base_addr = iot["inputRegBase"].isNull() ? INPUT_REGISTER_BASE_ADDRESS : iot["inputRegBase"].as<uint16_t>();
   _coil_base_addr = iot["coilBase"].isNull() ? COIL_BASE_ADDRESS : iot["coilBase"].as<uint16_t>();
   _discrete_input_base_addr = iot["discreteBase"].isNull() ? DISCRETE_BASE_ADDRESS : iot["discreteBase"].as<uint16_t>();
   _holding_register_base_addr = iot["holdingRegBase"].isNull() ? HOLDING_REGISTER_BASE_ADDRESS : iot["holdingRegBase"].as<uint16_t>();
#endif
}

void IOT::loadSettings() {
   String jsonString;
   char ch;
   for (int i = 0; i < EEPROM_SIZE; ++i) {
      ch = EEPROM.read(i);
      if (ch == '\0')
         break; // Stop at the null terminator
      jsonString += ch;
      _settingsChecksum += ch;
   }
   JsonDocument doc;
   DeserializationError error = deserializeJson(doc, jsonString);
   if (error) {
      loge("Failed to load data from EEPROM, using defaults: %s", error.c_str());
      saveSettings(); // save default values
   } else {
      logd("JSON loaded from EEPROM: %d", jsonString.length());
   }
   loadSettingsFromJson(doc);
   _iotCB->onLoadSetting(doc); // load app settings
}

void IOT::saveSettingsToJson(JsonDocument &iot) {
   iot["version"] = APP_VERSION;
   iot["AP_SSID"] = _AP_SSID;
   iot["AP_Pw"] = _AP_Password;
   iot["Network"] = _NetworkSelection;
   iot["SSID"] = _SSID;
   iot["WiFi_Pw"] = _WiFi_Password;
   iot["APN"] = _APN;
   iot["SIM_USERNAME"] = _SIM_Username;
   iot["SIM_PASSWORD"] = _SIM_Password;
   iot["SIM_PIN"] = _SIM_PIN;
   iot["useDHCP"] = _useDHCP;
   iot["ETH_SIP"] = _Static_IP;
   iot["ETH_SM"] = _Subnet_Mask;
   iot["ETH_GW"] = _Gateway_IP;
#ifdef HasMQTT
   iot["useMQTT"] = _useMQTT;
   iot["mqttServer"] = _mqttServer;
   iot["mqttPort"] = _mqttPort;
   iot["mqttUser"] = _mqttUserName;
   iot["mqttPw"] = _mqttUserPassword;
#endif
#ifdef HasModbus
   iot["useModbus"] = _useModbus;
   iot["modbusMode"] = _ModbusMode;
   iot["svrRTUBaud"] = _modbusBaudRate;
   iot["svrRTUParity"] = _modbusParity;
   iot["svrRTUStopBits"] = _modbusStopBits;
   iot["modbusPort"] = _modbusPort;
   iot["modbusID"] = _modbusID;
   iot["inputRegBase"] = _input_register_base_addr;
   iot["coilBase"] = _coil_base_addr;
   iot["discreteBase"] = _discrete_input_base_addr;
   iot["holdingRegBase"] = _holding_register_base_addr;
#endif
}

void IOT::saveSettings() {
   JsonDocument doc;
   saveSettingsToJson(doc);
   _iotCB->onSaveSetting(doc);
   logd("Saving: %s", formattedJson(doc).c_str());
   String jsonString;
   serializeJson(doc, jsonString);
   uint32_t sum = 0;
   for (int i = 0; i < jsonString.length(); ++i) {
      int8_t byte = jsonString[i];
      EEPROM.write(i, byte);
      sum += byte;
   }
   EEPROM.write(jsonString.length(), '\0'); // Null-terminate the string
   EEPROM.commit();
   logd("JSON saved, required EEPROM size: %d", jsonString.length());
   _needToReboot = _settingsChecksum != sum;
   if (_needToReboot)
      logd("******* Need to reboot! ***");
}

String IOT::getThingName() { return _AP_SSID; }

void IOT::Run() {
   uint32_t now = millis();
   if (_networkState == Boot && _NetworkSelection == NotConnected) { // Network not setup?, see if flasher is trying to send us the SSID/Pw
      if (Serial.peek() == '{') {
         String s = Serial.readStringUntil('}');
         s += "}";
         JsonDocument doc;
         DeserializationError err = deserializeJson(doc, s);
         if (err) {
            loge("deserializeJson() failed: %s", err.c_str());
         } else {
            if (doc["ssid"].is<const char *>() && doc["password"].is<const char *>()) {
               _SSID = doc["ssid"].as<String>();
               logd("Setting ssid: %s", _SSID.c_str());
               _WiFi_Password = doc["password"].as<String>();
               logd("Setting password: %s", _WiFi_Password.c_str());
               _NetworkSelection = WiFiMode;
               saveSettings();
               esp_restart();
            } else {
               logw("Received invalid json: %s", s.c_str());
            }
         }
      } else {
         Serial.read(); // discard data
      }
      if ((now - _FlasherIPConfigStart) > FLASHER_TIMEOUT) { // wait for flasher tool to send Wifi info
         logd("Done waiting for flasher!");
         setState(ApState); // switch to AP mode for AP_TIMEOUT
      }
   } else if (_networkState == Boot) { // have network selection, start with wifiAP for AP_TIMEOUT then STA mode
      setState(ApState);               // switch to AP mode for AP_TIMEOUT
   } else if (_networkState == ApState) {
      if (_NetworkSelection != APMode) {                   // don't try to connect if in APMode
         if (_AP_Connected == false) {                     // if AP client is connected, stay in AP mode
            if ((now - _waitInAPTimeStamp) > AP_TIMEOUT) { // switch to selected network after waiting in APMode for AP_TIMEOUT duration
               if (_SSID.length() > 0) {                   // is it setup yet?
                  logd("Connecting to network: %d", _NetworkSelection);
                  setState(Connecting);
               }
            } else {
#ifdef Has_OLED
               int countdown = (AP_TIMEOUT - (millis() - _waitInAPTimeStamp)) / 1000;
               _iotCB->update("AP Mode", countdown);
#endif
            }
         }
      }
      if (_AP_Connected) {
         _dnsServer.processNextRequest();
         _webLog.process();
      }
   } else if (_networkState == Connecting) {
      if ((millis() - _NetworkConnectionStart) > WIFI_CONNECTION_TIMEOUT) {
         // -- Network not available, fall back to AP mode.
         logw("Giving up on Network connection.");
         WiFi.disconnect();
         setState(ApState);
      }
   } else if (_networkState == OffLine) { // went offline, try again...
      logw("went offline, try again...");
      setState(Connecting);
   } else if (_networkState == OnLine) {
      _webLog.process();
   }
   if (digitalRead(GPIO_NUM_0) != LOW) { // GPIO0 pressed for GPIO0_FactoryResetCountdown? initiate a factory reset
      _GPIO0_PressedCountdown = millis();
   }
   if (GPIO0_FactoryResetCountdown < millis() - _GPIO0_PressedCountdown) {
      logi("Factory Reset");
      EEPROM.write(0, 0);
      EEPROM.commit();
      _needToReboot = true;
   }
   if (_needToReboot) {
      GoOffline();
      delay(500);
      esp_restart();
   }
   vTaskDelay(pdMS_TO_TICKS(20));
   return;
}

// #pragma endregion Setup

// #pragma region Network

void IOT::GoOnline() {
   logd("GoOnline called");
   _pwebServer->begin();
   _webLog.begin(_pwebServer);
#ifdef HasOTA
   _OTA.begin(_pwebServer);
#endif
   if (_AP_Connected) {
      _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
   }
   if (_networkState > ApState) {
      if (_NetworkSelection == EthernetMode || _NetworkSelection == WiFiMode) {
         MDNS.begin(_AP_SSID.c_str());
         MDNS.addService("http", "tcp", ASYNC_WEBSERVER_PORT);
         logd("Active mDNS services: %d", MDNS.queryService("http", "tcp"));
      }
#ifdef HasModbus
      if (_useModbus && !_MBserver.isRunning()) {
         if (_ModbusMode == TCP) {
            if (!_MBserver.isRunning()) {
               _MBserver.start(_modbusPort, 5, 0); // listen for modbus requests
               logd("Modbus TCP started");
            }
         } else {
            _MBRTUserver.begin(Serial2);
            _MBRTUserver.useModbusRTU();
            logd("Modbus RTU started");
         }
      }
#endif
#ifdef HasMQTT
      xTimerStart(mqttReconnectTimer, 0);
#endif
      setState(OnLine);
   }
}

void IOT::GoOffline() {
   logd("GoOffline");
#ifdef HasMQTT
   xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
   StopMQTT();
#endif
   _webLog.end();
   _dnsServer.stop();

#ifdef HasModbus
   logd("GoOffline RTU");
   if (_ModbusMode == RTU) {
      _MBRTUserver.end();
   }
#endif
   MDNS.end();
   if (_networkState == OnLine) {
      setState(OffLine);
   }
}

void IOT::setState(NetworkState newState) {
   NetworkState oldState = _networkState;
   _networkState = newState;
   logd("_networkState: %s", _networkState == Boot         ? "Boot"
                             : _networkState == ApState    ? "ApState"
                             : _networkState == Connecting ? "Connecting"
                             : _networkState == OnLine     ? "OnLine"
                                                           : "OffLine");

#ifdef Has_OLED
   String mode;
   String detail;
   if (_networkState == OnLine) {
      mode = NetworkSelectionStrings[_NetworkSelection];
      detail = _Current_IP;
   } else {
      mode = NetworkStateStrings[_networkState];
      detail = "...";
   }
   _iotCB->update(mode.c_str(), detail.c_str());
#endif
   switch (newState) {
   case OffLine:
      if (_NetworkSelection == WiFiMode) {
         logd("Going offline, set back to AP mode");
         WiFi.disconnect(true); // true = erase WiFi credentials
         WiFi.mode(WIFI_AP);
#ifdef HasEthernet
      } else if (_NetworkSelection == EthernetMode) {
         DisconnectEthernet();
#endif
#ifdef HasLTE
      } else if (_NetworkSelection == ModemMode) {
         DisconnectModem();
#endif
      }
      delay(100);
      break;
   case ApState:
      if ((oldState == Connecting) || (oldState == OnLine)) {
         if (_NetworkSelection == WiFiMode) {
            WiFi.disconnect(true); // true = erase WiFi credentials
#ifdef HasEthernet
         } else if (_NetworkSelection == EthernetMode) {
            DisconnectEthernet();
#endif
#ifdef HasLTE
         } else if (_NetworkSelection == ModemMode) {
            DisconnectModem();
#endif
         }
         delay(100);
      }
      WiFi.mode(WIFI_AP);
      if (WiFi.softAP(_AP_SSID, _AP_Password, 1, false)) {
         IPAddress IP = WiFi.softAPIP();
         logi("WiFi AP SSID: %s PW: %s", _AP_SSID.c_str(), _AP_Password.c_str());
         logd("AP IP address: %s", IP.toString().c_str());
      }
      _waitInAPTimeStamp = millis();
      break;
   case Connecting:
      _NetworkConnectionStart = millis();
      if (_NetworkSelection == WiFiMode) {
         logd("WiFiMode, trying to connect to %s", _SSID.c_str());
         WiFi.setHostname(_AP_SSID.c_str());
         WiFi.mode(WIFI_STA);
         WiFi.begin(_SSID, _WiFi_Password);
      } else if (_NetworkSelection == EthernetMode) {
#ifdef HasEthernet
         if (ConnectEthernet() == ESP_OK) {
            logd("Ethernet succeeded");
         } else {
            loge("Failed to connect to Ethernet");
         }
#endif
#ifdef HasLTE
      } else if (_NetworkSelection == ModemMode) {
         if (ConnectModem() == ESP_OK) {
            logd("Modem succeeded");
         } else {
            loge("Failed to connect to 4G Modem");
         }
#endif
      }
      break;
   case OnLine:
      logd("State: Online");
      break;
   default:
      break;
   }
   _iotCB->onNetworkState(newState);
}

void IOT::HandleIPEvent(int32_t event_id, void *event_data) {
   ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
   if (event_id == IP_EVENT_PPP_GOT_IP || event_id == IP_EVENT_ETH_GOT_IP) {
      const esp_netif_ip_info_t *ip_info = &event->ip_info;
      logi("Got IP Address");
      logi("~~~~~~~~~~~");
      logi("IP:" IPSTR, IP2STR(&ip_info->ip));
      sprintf(_Current_IP, IPSTR, IP2STR(&ip_info->ip));
      logi("IPMASK:" IPSTR, IP2STR(&ip_info->netmask));
      logi("Gateway:" IPSTR, IP2STR(&ip_info->gw));
      logi("~~~~~~~~~~~");
      GoOnline();
   } else if (event_id == IP_EVENT_PPP_LOST_IP) {
      logi("Modem Disconnect from PPP Server");
      GoOffline();
   } else if (event_id == IP_EVENT_ETH_LOST_IP) {
      logi("Ethernet Disconnect");
      GoOffline();
   } else if (event_id == IP_EVENT_GOT_IP6) {
      ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
      logi("Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
   } else {
      logd("IP event! %d", (int)event_id);
   }
}
#ifdef HasEthernet
esp_err_t IOT::ConnectEthernet() {

   esp_err_t ret = ESP_OK;
   logd("ConnectEthernet");
   if ((ret = gpio_install_isr_service(0)) != ESP_OK) {
      if (ret == ESP_ERR_INVALID_STATE) {
         logw("GPIO ISR handler has been already installed");
         ret = ESP_OK; // ISR handler has been already installed so no issues
      } else {
         logd("GPIO ISR handler install failed");
      }
   }
   spi_bus_config_t buscfg = {
       .mosi_io_num = ETH_MOSI,
       .miso_io_num = ETH_MISO,
       .sclk_io_num = ETH_SCK,
       .quadwp_io_num = -1,
       .quadhd_io_num = -1,
   };
   if ((ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)) != ESP_OK) {
      logd("SPI host #1 init failed");
      return ret;
   }
   uint8_t base_mac_addr[6];
   if ((ret = esp_efuse_mac_get_default(base_mac_addr)) == ESP_OK) {
      uint8_t local_mac_1[6];
      esp_derive_local_mac(local_mac_1, base_mac_addr);
      logi("ETH MAC: %02X:%02X:%02X:%02X:%02X:%02X", local_mac_1[0], local_mac_1[1], local_mac_1[2], local_mac_1[3], local_mac_1[4], local_mac_1[5]);
      eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG(); // Init common MAC and PHY configs to default
      eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
      phy_config.phy_addr = 1;
      phy_config.reset_gpio_num = ETH_RST;
      spi_device_interface_config_t spi_devcfg = {
          .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
          .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
          .mode = 0,
          .clock_speed_hz = 25 * 1000 * 1000,
          .spics_io_num = ETH_SS,
          .queue_size = 20,
      };
      spi_device_handle_t spi_handle;
      if ((ret = spi_bus_add_device(SPI2_HOST, &spi_devcfg, &spi_handle)) != ESP_OK) {
         loge("spi_bus_add_device failed");
         return ret;
      }
      eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
      w5500_config.int_gpio_num = ETH_INT;
      esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
      esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
      _eth_handle = NULL;
      esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac, phy);
      if ((ret = esp_eth_driver_install(&eth_config_spi, &_eth_handle)) != ESP_OK) {
         loge("esp_eth_driver_install failed");
         return ret;
      }
      if ((ret = esp_eth_ioctl(_eth_handle, ETH_CMD_S_MAC_ADDR, local_mac_1)) != ESP_OK) // set mac address
      {
         logd("SPI Ethernet MAC address config failed");
      }
      esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH(); // Initialize the Ethernet interface
      _netif = esp_netif_new(&cfg);
      assert(_netif);
      if (!_useDHCP) {
         esp_netif_dhcpc_stop(_netif);
         esp_netif_ip_info_t ipInfo;
         IPAddress ip;
         ip.fromString(_Static_IP);
         ipInfo.ip.addr = static_cast<uint32_t>(ip);
         ip.fromString(_Subnet_Mask);
         ipInfo.netmask.addr = static_cast<uint32_t>(ip);
         ip.fromString(_Gateway_IP);
         ipInfo.gw.addr = static_cast<uint32_t>(ip);
         if ((ret = esp_netif_set_ip_info(_netif, &ipInfo)) != ESP_OK) {
            loge("esp_netif_set_ip_info failed: %d", ret);
            return ret;
         }
      }
      _eth_netif_glue = esp_eth_new_netif_glue(_eth_handle);
      if ((ret = esp_netif_attach(_netif, _eth_netif_glue)) != ESP_OK) {
         loge("esp_netif_attach failed");
         return ret;
      }
      if ((ret = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, this)) != ESP_OK) {
         loge("esp_event_handler_register IP_EVENT->IP_EVENT_ETH_GOT_IP failed");
         return ret;
      }
      if ((ret = esp_eth_start(_eth_handle)) != ESP_OK) {
         loge("esp_netif_attach failed");
         return ret;
      }
   }
   return ret;
}

void IOT::DisconnectEthernet() {
   if (_eth_handle != NULL) {
      ESP_ERROR_CHECK(esp_eth_stop(_eth_handle));
      _eth_handle = NULL;
      if (_eth_netif_glue != NULL) {
         ESP_ERROR_CHECK(esp_eth_del_netif_glue(_eth_netif_glue));
         _eth_netif_glue = NULL;
      }
      if (_netif != NULL) {
         esp_netif_destroy(_netif);
         _netif = NULL;
      }
      ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, on_ip_event));
   }
}

#endif

#ifdef HasLTE
void IOT::wakeup_modem(void) {
   pinMode(LTE_PWR_EN, OUTPUT);
#ifdef LTE_AIRPLANE_MODE
   pinMode(LTE_AIRPLANE_MODE, OUTPUT); // turn off airplane mode
   digitalWrite(LTE_AIRPLANE_MODE, HIGH);
#endif
   digitalWrite(LTE_PWR_EN, LOW);
   delay(1000);
   logd("Power on the modem");
   digitalWrite(LTE_PWR_EN, HIGH);
   delay(2000);
   logd("Modem is powered up and ready");
}

esp_err_t IOT::ConnectModem() {
   logd("ConnectModem");
   esp_err_t ret = ESP_OK;
   wakeup_modem();
   esp_netif_config_t ppp_netif_config = ESP_NETIF_DEFAULT_PPP(); // Initialize lwip network interface in PPP mode
   _netif = esp_netif_new(&ppp_netif_config);
   assert(_netif);
   ESP_ERROR_CHECK(modem_init_network(_netif, _APN.c_str(),
                                      _SIM_PIN.c_str())); // Initialize the PPP network and register for IP event
   if (_SIM_Username.length() > 0) {
      esp_netif_ppp_set_auth(_netif, NETIF_PPP_AUTHTYPE_PAP, _SIM_Username.c_str(), _SIM_Password.c_str());
   }
   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, on_ip_event, this));
   int retryCount = 3;
   while (retryCount-- != 0) {
      if (!modem_check_sync()) {
         logw("Modem does not respond, maybe in DATA mode? ...exiting network mode");
         modem_stop_network();
         if (!modem_check_sync()) {
            logw("Modem does not respond to AT ...restarting");
            modem_reset();
            logi("Restarted");
         }
         continue;
      }
      if (!modem_check_signal()) {
         logw("Poor signal ...will check after 5s");
         vTaskDelay(pdMS_TO_TICKS(5000));
         continue;
      }
      if (!modem_start_network()) {
         loge("Modem could not enter network mode ...will retry after 10s");
         vTaskDelay(pdMS_TO_TICKS(10000));
         continue;
      }
      break;
   }
   logi("Modem has acquired network");
   return ret;
}

void IOT::DisconnectModem() {
   if (digitalRead(LTE_PWR_EN) == HIGH) {
#ifdef LTE_AIRPLANE_MODE
      digitalWrite(LTE_AIRPLANE_MODE, LOW); // turn on airplane mode
#endif
      digitalWrite(LTE_PWR_EN, LOW); // turn off power to the modem
      modem_stop_network();
      modem_deinit_network();
      if (_netif != NULL) {
         esp_netif_destroy(_netif);
         _netif = NULL;
      }
      ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, on_ip_event));
   }
}
#endif

// #pragma endregion Network

#ifdef HasModbus

void IOT::registerMBTCPWorkers(FunctionCode fc, MBSworker worker) {
   if (_ModbusMode == TCP) {
      _MBserver.registerWorker(_modbusID, fc, worker);
   } else {
      _MBRTUserver.registerWorker(_modbusID, fc, worker);
   }
}

uint16_t IOT::getMBBaseAddress(IOTypes type) {
   switch (type) {
   case DigitalInputs:
      return _discrete_input_base_addr;
      break;
   case DigitalOutputs:
      return _coil_base_addr;
      break;

   case AnalogInputs:
      return _input_register_base_addr;
      break;

   case AnalogOutputs:
      return _holding_register_base_addr;
      break;
   }
   return 0;
}

#endif

#ifdef HasMQTT
void IOT::HandleMQTT(int32_t event_id, void *event_data) {
   auto event = (esp_mqtt_event_handle_t)event_data;
   esp_mqtt_client_handle_t client = event->client;
   JsonDocument doc;
   switch ((esp_mqtt_event_id_t)event_id) {
   case MQTT_EVENT_CONNECTED:
      logi("Connected to MQTT.");
      char buf[128];
      sprintf(buf, "%s/set/#", _rootTopicPrefix);
      esp_mqtt_client_subscribe(client, buf, 0);
      _iotCB->onMqttConnect();
      esp_mqtt_client_publish(client, _willTopic, "Online", 0, 1, 0);
      break;
   case MQTT_EVENT_DISCONNECTED:
      logw("Disconnected from MQTT");
      if (_networkState == OnLine) {
         xTimerStart(mqttReconnectTimer, 5000);
      }
      break;

   case MQTT_EVENT_SUBSCRIBED:
      logi("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
   case MQTT_EVENT_UNSUBSCRIBED:
      logi("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
   case MQTT_EVENT_PUBLISHED:
      logi("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
   case MQTT_EVENT_DATA:
      char topicBuf[256];
      snprintf(topicBuf, sizeof(topicBuf), "%.*s", event->topic_len, event->topic);
      char payloadBuf[256];
      snprintf(payloadBuf, sizeof(payloadBuf), "%.*s", event->data_len, event->data);
      logd("MQTT Message arrived [%s] %s", topicBuf, payloadBuf);
      _iotCB->onMqttMessage(topicBuf, payloadBuf);
      // if (deserializeJson(doc, event->data)) // not json!
      // {
      // 	logd("MQTT payload {%s} is not valid JSON!", event->data);
      // }
      // else
      // {
      // 	if (doc.containsKey("status"))
      // 	{
      // 		doc.clear();
      // 		doc["sw_version"] = APP_VERSION;
      // 		// doc["IP"] = WiFi.localIP().toString().c_str();
      // 		// doc["SSID"] = WiFi.SSID();
      // 		doc["uptime"] = formatDuration(millis() - _lastBootTimeStamp);
      // 		Publish("status", doc, true);
      // 	}
      // 	else
      // 	{
      // 		_iotCB->onMqttMessage(topicBuf, doc);
      // 	}
      // }
      break;
   case MQTT_EVENT_ERROR:
      loge("MQTT_EVENT_ERROR");
      if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
         logi("Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
      }
      break;
   default:
      logi("Other event id:%d", event->event_id);
      break;
   }
}

void IOT::ConnectToMQTTServer() {
   if (_networkState == OnLine) {
      if (_useMQTT && _mqttServer.length() > 0) // mqtt configured?
      {
         logd("Connecting to MQTT: %s:%d", _mqttServer.c_str(), _mqttPort);
         int len = strlen(_AP_SSID.c_str());
         strncpy(_rootTopicPrefix, _AP_SSID.c_str(), len);
         logd("rootTopicPrefix: %s", _rootTopicPrefix);
         sprintf(_willTopic, "%s/tele/LWT", _rootTopicPrefix);
         logd("_willTopic: %s", _willTopic);
         esp_mqtt_client_config_t mqtt_cfg = {};
         mqtt_cfg.host = _mqttServer.c_str();
         mqtt_cfg.port = _mqttPort;
         mqtt_cfg.username = _mqttUserName.c_str();
         mqtt_cfg.password = _mqttUserPassword.c_str();
         mqtt_cfg.client_id = _AP_SSID.c_str();
         mqtt_cfg.lwt_topic = _willTopic;
         mqtt_cfg.lwt_retain = 1;
         mqtt_cfg.lwt_msg = "Offline";
         mqtt_cfg.lwt_msg_len = 7;
         mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
         // mqtt_cfg.cert_pem = (const char *)hivemq_ca_pem_start,
         // mqtt_cfg.skip_cert_common_name_check = true; // allow self-signed certs
         _mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);
         esp_mqtt_client_register_event(_mqtt_client_handle, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);
         esp_mqtt_client_start(_mqtt_client_handle);
      }
   }
}

boolean IOT::Publish(const char *subtopic, JsonDocument &payload, boolean retained) {
   String s;
   serializeJson(payload, s);
   return Publish(subtopic, s.c_str(), retained);
}

boolean IOT::Publish(const char *subtopic, const char *value, boolean retained) {
   boolean rVal = false;
   if (_mqtt_client_handle != 0) {
      char buf[128];
      sprintf(buf, "%s/stat/%s", _rootTopicPrefix, subtopic);
      rVal = (esp_mqtt_client_publish(_mqtt_client_handle, buf, value, strlen(value), 1, retained) != -1);
      if (!rVal) {
         loge("**** Failed to publish MQTT message");
      }
   }
   return rVal;
}

boolean IOT::Publish(const char *topic, float value, boolean retained) {
   char buf[256];
   snprintf_P(buf, sizeof(buf), "%.1f", value);
   return Publish(topic, buf, retained);
}

boolean IOT::PublishMessage(const char *topic, JsonDocument &payload, boolean retained) {
   boolean rVal = false;
   if (_mqtt_client_handle != 0) {
      String s;
      serializeJson(payload, s);
      rVal = (esp_mqtt_client_publish(_mqtt_client_handle, topic, s.c_str(), s.length(), 0, retained) != -1);
      if (!rVal) {
         loge("**** Configuration payload exceeds MAX MQTT Packet Size, %d [%s] topic: %s", s.length(), s.c_str(), topic);
      }
   }
   return rVal;
}

void IOT::StopMQTT() {
   if (_mqtt_client_handle != 0) {
      esp_mqtt_client_publish(_mqtt_client_handle, _willTopic, "Offline", 0, 1, 0);
      esp_mqtt_client_stop(_mqtt_client_handle);
   }
   return;
}

String IOT::getRootTopicPrefix() {
   String s(_rootTopicPrefix);
   return s;
};

#endif

} // namespace CLASSICDIY
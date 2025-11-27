#include "Log.h"
#include <math.h>
#include <ThreadController.h>
#include <Thread.h>
#include "WebLog.h"
#include "HelperFunctions.h"
#include "IOT.h"
#include "Tracker.h"
#include "Anemometer.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "style.htm"
#include "Tracker.htm"

namespace CLASSICDIY {

static AsyncWebServer _asyncServer(ASYNC_WEBSERVER_PORT);
static AsyncWebSocket _webSocket("/ws_home");
IOT _iot = IOT();
Thread *_workerThread = new Thread();
Anemometer _anemometer(AnemometerPin);

Tracker::Tracker() {
   _errorState = TrackerError_Ok;
   _waitingForMorning = false;
   _trackerState = TrackerState_Off;
}

Tracker::~Tracker() {
   logd("Tracker destructor");
   delete _sun;
   delete _azimuth;
   delete _elevation;
}

void Tracker::Setup(ThreadController *controller) {
   Init();
   _iot.Init(this, &_asyncServer);
   setState(TrackerState_Initializing);
   _lastWindEvent = 0;
   _sun = new Sun(_config.getLat(), _config.getLon());
   _cycleTime = getTime();
   _sun->calcSun(&_cycleTime);
   _azimuth = new LinearActuatorNoPot("Horizontal", ENABLE_H, PWMa_H, PWMb_H);
   controller->add(_azimuth);
   _elevation = new LinearActuatorNoPot("Vertical", ENABLE_V, PWMa_V, PWMb_V);
   controller->add(_elevation);
   controller->add(this);
   InitializeActuators();
   _asyncServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      String page = home_html;
      page.replace("{style}", style);
      page.replace("{n}", _iot.getThingName().c_str());
      page.replace("{v}", APP_VERSION);
      request->send(200, "text/html", page);
   });
   _asyncServer.on("/appsettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
      JsonDocument payload;
      _config.Save(payload);
      JsonDocument app = payload["tracker"];
      String s;
      serializeJson(app, s);
      logd("/appsettings: %s", s.c_str());
      request->send(200, "text/html", s);
   });
   _asyncServer.addHandler(&_webSocket).addMiddleware([this](AsyncWebServerRequest *request, ArMiddlewareNext next) {
      // ws.count() is the current count of WS clients: this one is trying to upgrade its HTTP connection
      if (_webSocket.count() > 1) {
         // if we have 2 clients or more, prevent the next one to connect
         request->send(503, "text/plain", "Server is busy");
      } else {
         // process next middleware and at the end the handler
         next();
      }
   });
   _webSocket.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
      (void)len;
      if (type == WS_EVT_CONNECT) {
         _lastMessagePublished.clear(); // force a broadcast
         client->setCloseClientOnQueueFull(false);
         client->ping();
      } else if (type == WS_EVT_DISCONNECT) {
         // logi("Home Page Disconnected!");
      } else if (type == WS_EVT_ERROR) {
         loge("ws error");
         // } else if (type == WS_EVT_PONG) {
         // 	logd("ws pong");
      }
   });
   logd("Initialize done!");
}

void Tracker::onSaveSetting(JsonDocument &doc) { _config.Save(doc); }

void Tracker::onLoadSetting(JsonDocument &doc) { _config.Load(doc); }

void Tracker::addApplicationConfigs(String &page) {
   String appFields = app_config;
   page += appFields;
   String appScript = app_script;
   appScript.replace("<script>", "");
   appScript.replace("</script>", "");
   page.replace("{script}", appScript);
   appScript = onload_script;
   page.replace("{onload}", appScript);
}

void Tracker::onSubmitForm(AsyncWebServerRequest *request) {}

void Tracker::Process() {
   _iot.Run();
   Run(); // base class
#ifdef HasMQTT
   _iot.Publish("readings", s.c_str(), false);
#endif
   return;
}

void Tracker::onNetworkState(NetworkState state) { _networkState = state; }

void Tracker::Move(Direction dir) {
   switch (dir) {
   case Direction_East:
      _azimuth->MoveIn();
      break;
   case Direction_West:
      _azimuth->MoveOut();
      break;
   case Direction_Up: // vertical actuator is wired in reverse due to mechanical setup
      _elevation->MoveOut();
      break;
   case Direction_Down:
      _elevation->MoveIn();
      break;
   default:
      break;
   }
}

void Tracker::Stop() {
   _azimuth->Stop();
   _elevation->Stop();
}

void Tracker::Cycle() {
   cycleHour = 0;
   enabled = true;
   setInterval(CYCLE_POSITION_UPDATE_INTERVAL);
   setState(TrackerState_Cycling);
   this->run();
}

void Tracker::Track() {
   logd("Tracker track");
   if (_errorState != TrackerError_Ok) {
      // traceln(_stm, F("Error detected, Tracker moving to default position"));
      // move array to face south as a default position when an error exists
      Park(true);
      enabled = false; // don't run worker thread when error exists
   } else if (getState() != TrackerState_Initializing && getState() != TrackerState_Off) {
      if (getState() == TrackerState_Manual || getState() == TrackerState_Cycling || getState() == TrackerState_Parked) {
         // re-initialize actuators if moved
         InitializeActuators();
      }
      enabled = true;
      setInterval(POSITION_UPDATE_INTERVAL);
      setState(TrackerState_Tracking);
      this->run();
   }
}

void Tracker::Resume() {
   if (_sun->ItsDark() == false) {
      Track();
   }
}

void Tracker::Park(bool protect = false) {
   if (getState() != TrackerState_Parked) {
      _azimuth->MoveTo(180);
      if (_config.isDual()) {
         float elevation = 45;
         if (protect) {
            elevation = 90; // max elevation to protect against wind.
         }
         _elevation->MoveTo(elevation);
      }
      setState(TrackerState_Parked);
   }
}

void Tracker::WaitForMorning() {
   if (_waitingForMorning == false) {
      _azimuth->Retract(); // wait for morning, full east, lowest elevation
      if (_config.isDual()) {
         _elevation->Retract();
      }
      _waitingForMorning = true;
      logi("Waiting For Morning");
   }
}

void Tracker::TrackToSun() {
   logi("TrackToSun ");
   _azimuth->MoveTo(_sun->azimuth());
   if (_config.isDual()) {
      _elevation->MoveTo(_sun->elevation());
   }
}

TrackerState Tracker::getState() {
   if (_azimuth->getState() == ActuatorState_Initializing) {
      return TrackerState_Initializing;
   }
   if (_config.isDual() && _elevation->getState() == ActuatorState_Initializing) {
      return TrackerState_Initializing;
   }
   if (_azimuth->getState() == ActuatorState_Error) {
      _errorState = TrackerError_HorizontalActuator;
      return TrackerState_Initializing;
   }
   if (_config.isDual() && _elevation->getState() == ActuatorState_Error) {
      _errorState = TrackerError_VerticalActuator;
      return TrackerState_Initializing;
   }
   return _trackerState;
}

void Tracker::setState(TrackerState state) {
   logd("setState: %d", state);
   if (_trackerState != state) {
      _trackerState = state;
#ifdef HasMQTT
      String mode;
      switch (state) {
      case TrackerState_Off:
         mode = "Off";
         break;
      case TrackerState_Initializing:
         mode = "Initializing";
         break;
      case TrackerState_Standby:
         mode = "Standby";
         break;
      case TrackerState_Manual:
         mode = "Manual";
         break;
      case TrackerState_Cycling:
         mode = "Cycling";
         break;
      case TrackerState_Tracking:
         mode = "Tracking";
         break;
      case TrackerState_Parked:
         mode = "Parked";
         break;
      }
      _iot->Publish("mode", mode.c_str(), false);
#endif
   }
   return;
}

void Tracker::InitializeActuators() {
   _azimuth->Initialize(_config.getEastAzimuth(), _config.getWestAzimuth(), _config.getHorizontalLength(), _config.getHorizontalSpeed(),
                        _config.getLat() < 0);
   if (_config.isDual()) {
      _elevation->Initialize(_config.getMinimumElevation(), _config.getMaximumElevation(), _config.getVerticalLength(),
                             _config.getVerticalSpeed());
   }
   setInterval(POSITION_UPDATE_INTERVAL);
   setState(TrackerState_Standby);
}

void Tracker::run() {
   runned();
   TrackerState state = getState();
   _cycleTime = getTime();
   switch (state) {
   case TrackerState_Cycling:
      if (_azimuth->getState() == ActuatorState_Stopped && _elevation->getState() == ActuatorState_Stopped) {
         cycleHour++;
         cycleHour %= 24;
         struct tm *ptm = gmtime(&_cycleTime);
         ptm->tm_hour = cycleHour;
         ptm->tm_min = 0;
         ptm->tm_sec = 0;
         _cycleTime = mktime(ptm);
         _sun->calcSun(&_cycleTime);
      }
      break;
   case TrackerState_Tracking:
      _sun->calcSun(&_cycleTime);
      break;
   default:
      return;
   }
   if (_sun->ItsDark()) {
      WaitForMorning();
   } else {
      _waitingForMorning = false;
      TrackToSun();
   }
   if (_config.hasAnemometer()) {
      float windSpeed = _anemometer.WindSpeed();
      if (_recordedWindSpeedAtLastEvent < windSpeed) {
         _lastWindEvent = getTime();
         _recordedWindSpeedAtLastEvent = windSpeed;
      }
      if (getState() == TrackerState_Tracking) {
         if (windSpeed > (AnemometerWindSpeedProtection / 3.6)) // wind speed greater than 28.8 km/hour? (8 M/S *3600 S)
         {
            _lastWindEvent = getTime();
            _recordedWindSpeedAtLastEvent = windSpeed;
            Park(true);
            _protectCountdown = 300; // 10 minute countdown to resume tracking
         }
      }
      if (getState() == TrackerState_Parked && --_protectCountdown <= 0) {
         _protectCountdown = 0;
         Resume();
      }
   }
};

/// <summary>
///  Process commands received from android app
/// </summary>
/// <param name="input"></param>
void Tracker::ProcessCommand(const char *input) {
   boolean afterDelimiter = false;
   char command[32];
   char data[64];
   unsigned int commandIndex = 0;
   unsigned int dataIndex = 0;
   JsonDocument doc;
   for (unsigned int i = 0; i < strlen(input); i++) {
      if (input[i] == '|') {
         afterDelimiter = true;
         continue;
      }
      if (input[i] == '\r' || input[i] == '\n') {
         continue;
      }
      if (afterDelimiter == false) {
         command[commandIndex++] = input[i];
         if (commandIndex >= sizeof(command)) {
            loge("Command overflow!");
         }
      } else {
         data[dataIndex++] = input[i];
         if (dataIndex >= sizeof(data)) {
            loge("data overflow!");
         }
      }
   }
   command[commandIndex++] = 0;
   data[dataIndex++] = 0;
   logi("%s|%s", command, data);
   if (strcmp(command, c_Track) == 0) {
      Track();
   } else if (strcmp(command, c_Cycle) == 0) {
      Cycle();
   } else if (strcmp(command, c_Park) == 0) {
      Park(false);
   } else if (strcmp(command, c_Protect) == 0) {
      Park(true);
   } else if (strcmp(command, c_Stop) == 0) {
      setState(TrackerState_Manual);
      Stop();
   } else if (strcmp(command, c_SetC) == 0) // set configuration
   {
      DeserializationError error = deserializeJson(doc, data);
      if (!error) {
         _config.SetLocation(doc["a"], doc["o"]);
         setInterval(PENDING_RESET);
      }
   } else if (strcmp(command, c_SetA) == 0) // set actuator size/speed
   {
      DeserializationError error = deserializeJson(doc, data);
      if (!error) {
         _config.SetActuatorParameters(doc["lh"], doc["lv"], doc["sh"], doc["sv"]);
         setInterval(PENDING_RESET);
      }
   } else if (strcmp(command, c_SetL) == 0) // set limits
   {
      DeserializationError error = deserializeJson(doc, data);
      if (!error) {
         _config.SetLimits(doc["e"], doc["w"], doc["n"], doc["x"]);
         setInterval(PENDING_RESET);
      }
   } else if (strcmp(command, c_SetO) == 0) // set options
   {
      DeserializationError error = deserializeJson(doc, data);
      if (!error) {
         _config.setDual(doc["d"]);
         _config.setHasAnemometer(doc["an"]);
         setInterval(PENDING_RESET);
      }
   } else if (strcmp(command, c_SetDateTime) == 0) {
      timeval tv;
      tv.tv_sec = atol(data);
      tv.tv_usec = 0;
      settimeofday(&tv, NULL);
      _cycleTime = getTime();
      printLocalTime();
   } else if (strcmp(command, c_MoveTo) == 0) {
      MoveTo(data);
   }
}

void Tracker::MoveTo(char *arg) {
   setState(TrackerState_Manual);
   if (strcmp(arg, c_East) == 0) {
      Move(Direction_East);
   } else if (strcmp(arg, c_West) == 0) {
      Move(Direction_West);
   } else if (strcmp(arg, c_Up) == 0) {
      Move(Direction_Up);
   } else if (strcmp(arg, c_Down) == 0) {
      Move(Direction_Down);
   }
}

#ifdef HasMQTT

void PLC::onMqttConnect() {
   if (!_discoveryPublished) {
      for (int i = 0; i < _digitalInputDiscretes.coils(); i++) {
         std::stringstream ss;
         ss << "DI" << i;
         if (PublishDiscoverySub(DigitalInputs, ss.str().c_str(), nullptr, "mdi:switch") == false) {
            return; // try later
         }
      }
      for (int i = 0; i < _digitalOutputCoils.coils(); i++) {
         std::stringstream ss;
         ss << "DO" << i;
         if (PublishDiscoverySub(DigitalOutputs, ss.str().c_str(), nullptr, "mdi:valve") == false) {
            return; // try later
         }
      }
      for (int i = 0; i < _analogInputRegisters.size(); i++) {
         std::stringstream ss;
         ss << "AI" << i;
         if (PublishDiscoverySub(AnalogInputs, ss.str().c_str(), "%", "mdi:lightning-bolt") == false) {
            return; // try later
         }
      }
      for (int i = 0; i < _analogOutputRegisters.size(); i++) {
         std::stringstream ss;
         ss << "AO" << i;
         if (PublishDiscoverySub(AnalogOutputs, ss.str().c_str(), nullptr, nullptr) == false) {
            return; // try later
         }
      }
      _discoveryPublished = true;
   }
}

boolean PLC::PublishDiscoverySub(IOTypes type, const char *entityName, const char *unit_of_meas, const char *icon) {
   String topic = HOME_ASSISTANT_PREFIX;
   switch (type) {
   case DigitalOutputs:
      topic += "/switch/";
      break;
   case AnalogOutputs:
      topic += "/number/";
      break;
   case DigitalInputs:
      topic += "/sensor/";
      break;
   case AnalogInputs:
      topic += "/sensor/";
      break;
   }
   topic += String(_iot.getUniqueId());
   topic += "/";
   topic += entityName;
   topic += "/config";

   JsonDocument payload;
   payload["platform"] = "mqtt";
   payload["name"] = entityName;
   payload["unique_id"] = String(_iot.getUniqueId()) + "_" + String(entityName);
   payload["value_template"] = ("{{ value_json." + String(entityName) + " }}").c_str();
   payload["state_topic"] = _iot.getRootTopicPrefix().c_str() + String("/stat/readings");
   if (type == DigitalOutputs) {
      payload["command_topic"] = _iot.getRootTopicPrefix().c_str() + String("/set/") + String(entityName);
      payload["state_on"] = "On";
      payload["state_off"] = "Off";
   } else if (type == AnalogOutputs) {
      payload["command_topic"] = _iot.getRootTopicPrefix().c_str() + String("/set/") + String(entityName);
      payload["min"] = 0;
      payload["max"] = 65535;
      payload["step"] = 1;
   } else if (type == DigitalInputs) {
      payload["payload_off"] = "Low";
      payload["payload_on"] = "High";
   }
   payload["availability_topic"] = _iot.getRootTopicPrefix().c_str() + String("/tele/LWT");
   payload["payload_available"] = "Online";
   payload["payload_not_available"] = "Offline";
   if (unit_of_meas) {
      payload["unit_of_measurement"] = unit_of_meas;
   }
   if (icon) {
      payload["icon"] = icon;
   }

   char buffer[STR_LEN];
   JsonObject device = payload["device"].to<JsonObject>();
   device["name"] = _iot.getThingName();
   device["sw_version"] = APP_VERSION;
   device["manufacturer"] = "ClassicDIY";
   sprintf(buffer, "%s (%X)", TAG, _iot.getUniqueId());
   device["model"] = buffer;
   JsonArray identifiers = device["identifiers"].to<JsonArray>();
   sprintf(buffer, "%X", _iot.getUniqueId());
   identifiers.add(buffer);

   logd("Discovery => topic: %s", topic.c_str());
   return _iot.PublishMessage(topic.c_str(), payload, true);
}

void PLC::onMqttMessage(char *topic, char *payload) {
   logd("onMqttMessage [%s] %s", topic, payload);
   std::string cmnd = _iot.getRootTopicPrefix() + "/set/";
   std::string fullPath = topic;
   if (strncmp(topic, cmnd.c_str(), cmnd.length()) == 0) {
      // Handle set commands
      size_t lastSlash = fullPath.find_last_of('/');
      std::string dout;
      if (lastSlash != std::string::npos) {
         dout = fullPath.substr(lastSlash + 1);
         if (dout[0] == 'D') // coils?
         {
            logd("coil: %s: ", dout.c_str());
            for (int i = 0; i < _digitalOutputCoils.coils(); i++) {
               std::stringstream ss;
               ss << "DO" << i;
               if (dout == ss.str()) {
                  String input = payload;
                  input.toLowerCase();
                  if (input == "on" || input == "high" || input == "1") {
                     _digitalOutputCoils.set(i, true);
                     logi("Write Coil %d HIGH", i);
                  } else if (input == "off" || input == "low" || input == "0") {
                     _digitalOutputCoils.set(i, false);
                     logi("Write Coil %d LOW", i);
                  } else {
                     logw("Write Coil %d invalid state: %s", i, input.c_str());
                  }
                  break;
               }
            }
#if DO_PINS > 0
            // set native DO pins
            for (int j = 0; j < DO_PINS; j++) {
               SetRelay(j, _digitalOutputCoils[j] ? HIGH : LOW);
            }
#endif
         } else if (dout[0] == 'A') // registers?
         {
            logd("gerister: %s: ", dout.c_str());
            for (int i = 0; i < _analogOutputRegisters.size(); i++) {
               std::stringstream ss;
               ss << "AO" << i;
               if (dout == ss.str()) {
                  String input = payload;
                  input.toLowerCase();
                  logd("Analog output value: %s", input.c_str());
                  _analogOutputRegisters.set(i, atoi(input.c_str()));
                  break;
               }
            }
#if AO_PINS > 0
            // set native AO pins
            for (int i = 0; i < AO_PINS; i++) {
               _PWMOutputs[i].SetDutyCycle(_analogOutputRegisters[i]);
            }
#endif
         }
      }
   }
}
#endif

} // namespace CLASSICDIY
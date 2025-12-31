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
#include "Tracker.htm"
#include "app_script.js"

namespace CLASSICDIY {

const char *TrackerModeStrings[] = {"Manual", "Cycle", "Track", "Park", "Protect"};
const char *TrackerStateStrings[] = {"Off", "Initializing", "Standby", "Tracking"};
static AsyncWebServer _asyncServer(ASYNC_WEBSERVER_PORT);
static AsyncWebSocket _webSocket("/ws_home");
IOT _iot = IOT();
Thread *_workerThread = new Thread();
Anemometer _anemometer(AnemometerPin);

Tracker::Tracker() {
   _errorState = TrackerError_Ok;
   _waitingForMorning = false;
   _config = Configuration();
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
   _lastWindEvent = 0;
   _sun = new Sun(_config.getLat(), _config.getLon());
   _cycleTime = getTime();
   _sun->calcSun(&_cycleTime);
#ifdef Lilygo_Relay_6CH
   _azimuth = new LinearActuatorNoPot("Horizontal", 0, 1, _reg);
   _elevation = new LinearActuatorNoPot("Vertical", 2, 3, _reg);
#else
   _azimuth = new LinearActuatorNoPot("Horizontal", ENABLE_H, PWMa_H, PWMb_H);
   _elevation = new LinearActuatorNoPot("Vertical", ENABLE_V, PWMa_V, PWMb_V);
#endif
   controller->add(_azimuth);
   controller->add(_elevation);
   controller->add(this);
   _asyncServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      request->send(200, "text/html", home_html, [this](const String &var) { return appTemplateProcessor(var); });
   });
   _asyncServer.on("/appsettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
      JsonDocument app;
      _config.Save(app);
      String s;
      serializeJson(app, s);
      logd("/appsettings: %s", s.c_str());
      request->send(200, "text/html", s);
   });
   _asyncServer.on(
       "/app_fields", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          // Called after all chunks are received
          logv("Full body received: %s", _bodyBuffer.c_str());
          // Parse JSON safely
          JsonDocument doc; // adjust size to expected payload
          DeserializationError err = deserializeJson(doc, _bodyBuffer);
          if (err) {
             logd("JSON parse failed: %s", err.c_str());
          } else {
             logd("HTTP_POST /app_fields: %s", formattedJson(doc).c_str());
             onLoadSetting(doc);
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
   _asyncServer.on(
       "/control", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          // This callback is called after the body is processed
          request->send(200, "application/json", "{\"status\":\"ok\"}");
       },
       NULL, // file upload handler (not used here)
       [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
          JsonDocument doc;
          DeserializationError err = deserializeJson(doc, data, len);
          if (err) {
             loge("JSON parse failed!");
             return;
          }
          String jsonString;
          serializeJson(doc, jsonString);

          String command = doc["command"];
          logd("/control: %s => %s", jsonString.c_str(), command);
          if (command == "up") {
             Move(Direction_Up);
          } else if (command == "down") {
             Move(Direction_Down);
          } else if (command == "left") {
             Move(Direction_East);
          } else if (command == "right") {
             Move(Direction_West);
          } else if (command == "stop") {
             Stop();
          } else if (command == "track") {
             Track();
          } else if (command == "cycle") {
             Cycle();
          } else if (command == "park") {
             Park(false);
          } else if (command == "protect") {
             Park(true);
          }
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
         client->setCloseClientOnQueueFull(false);
         client->ping();
      } else if (type == WS_EVT_DISCONNECT) {
         logi("Home Page Disconnected!");
      } else if (type == WS_EVT_ERROR) {
         loge("ws error");
      } else if (type == WS_EVT_PONG) {
         logd("ws pong");
         _lastMessagePublished.clear(); // force a broadcast
      }
   });
   logd("Setup done!");
}

void Tracker::onSaveSetting(JsonDocument &doc) { 
   logd("Saving: %s", formattedJson(doc).c_str());
   _config.Save(doc); 
}

void Tracker::onLoadSetting(JsonDocument &doc) {
   logd("Loading: %s", formattedJson(doc).c_str());
   _config.Load(doc);
   if (doc["_date"].isNull() == false && doc["_time"].isNull() == false) {
      struct tm tm{};
      sscanf(doc["_date"], "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
      sscanf(doc["_time"], "%d:%d", &tm.tm_hour, &tm.tm_min);
      tm.tm_year -= 1900; // struct tm years since 1900
      tm.tm_mon -= 1;     // struct tm months 0-11
      SetRTC(&tm);
      time_t t = mktime(&tm);
      struct timeval tv = {.tv_sec = t, .tv_usec = 0};
      settimeofday(&tv, nullptr);
      printLocalTime();
   }
}

String Tracker::appTemplateProcessor(const String &var) {
   if (var == "title") {
      return String(_iot.getThingName().c_str());
   }
   if (var == "version") {
      return String(APP_VERSION);
   }
   if (var == "app_fields") {
      return String(app_config);
   }
   if (var == "onload") {
      return String(onLoadScript);
   }
   if (var == "validateInputs") {
      return String(validate_script);
   }
   if (var == "app_script_js") {
      return String(app_script_js);
   }
   logd("Did not find app template for: %s", var.c_str());
   return String("");
}

void Tracker::Process() {
   _iot.Run();
   Run(); // base class
   if (_trackerState == Off) {
      InitializeActuators();
   }
   uint32_t now = millis();
   if ((now - _lastPublishTimeStamp) > PUBLISH_RATE_LIMIT) {
      _lastPublishTimeStamp = now;
      JsonDocument doc;
      doc["mode"] = TrackerModeStrings[_trackerMode];
      if (_sun->ItsDark()) {
         doc["state"] = "Waiting for Morning";
      } else {
         doc["state"] = TrackerStateStrings[getState()];
      }
      char buf[64];
      buf[0] = 0;
      strftime(buf, 64, "%A, %B %d %Y %H:%M", gmtime(&_cycleTime));
      doc["date_time_utc"] = buf;
      doc["azimuth"] = _sun->azimuth();
      doc["elevation"] = _sun->elevation();
      doc["horizontal_extent"] = std::round(_azimuth->CurrentPosition() * 10.0) / 10.0;
      doc["horizontal_angle"] = std::round(_azimuth->CurrentAngle() * 10.0) / 10.0;
      doc["vertical_extent"] = std::round(_elevation->CurrentPosition() * 10.0) / 10.0;
      doc["vertical_angle"] = std::round(_elevation->CurrentAngle() * 10.0) / 10.0;
      if (_config.hasAnemometer()) {
         doc["wind_speed"] = _anemometer.WindSpeed();
      }
      String s;
      serializeJson(doc, s);
      if (_lastMessagePublished != s) {
         _lastMessagePublished = s;
         if (_webSocket.count() > 0) { // any clients?
            _webSocket.textAll(s);
            logv("_webSocket Sent %s", s.c_str());
         }
#ifdef Has_TFT
         _tft.Update(doc);
#endif
#ifdef HasMQTT
         if (_discoveryPublished) { // wait until MQTT is connected and discovery is published
            String topic = _iot.getRootTopicPrefix() + "/state";
            _iot.PublishMessage(topic.c_str(), s.c_str(), false);
         }
#endif
      }
   }
   return;
}

void Tracker::onNetworkState(NetworkState state) {
   _networkState = state;
   if (_networkState == OnLine) {
      _config.GeoLocate(); // try to find current location if not set by user
      time_t now = time(nullptr);
      if (now > 1600000000) { // sanity check: valid epoch (>2020)
         // Write back to RTC
         SetRTC(localtime(&now));
      }
   }
}

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
   setState(TrackerState::Stopped);
}

void Tracker::Stop() {
   setMode(Manual);
   _azimuth->Stop();
   _elevation->Stop();
   setState(TrackerState::Stopped);
}

void Tracker::Cycle() {
   cycleHour = 0;
   enabled = true;
   setInterval(CYCLE_POSITION_UPDATE_INTERVAL);
   setMode(TrackerMode::Cycle);
   this->run();
   setState(TrackerState::Stopped);
}

void Tracker::Track() {
   logd("Tracker track");
   if (_errorState != TrackerError_Ok) {
      logd("Tracker error state: %s", describeTrackerError(_errorState));
      // traceln(_stm, F("Error detected, Tracker moving to default position"));
      // move array to face south as a default position when an error exists
      Park(true);
      enabled = false; // don't run worker thread when error exists
   } else if (getState() != TrackerState::Initializing && getState() != TrackerState::Off) {
      if (_trackerMode != TrackerMode::Track) {
         // re-initialize actuators if moved
         InitializeActuators();
      }
      enabled = true;
      setInterval(POSITION_UPDATE_INTERVAL);
      setMode(TrackerMode::Track);
      this->run();
   }
}

void Tracker::Resume() {
   if (_sun->ItsDark() == false) {
      Track();
   }
}

void Tracker::Park(bool protect = false) {
   if (_trackerMode != TrackerMode::Park && _trackerMode != TrackerMode::Protect) {
      _azimuth->MoveTo(180);
      if (_config.isDual()) {
         float elevation = 45;
         if (protect) {
            elevation = 90; // max elevation to protect against wind.
         }
         _elevation->MoveTo(elevation);
      }
      setMode(protect ? TrackerMode::Protect : TrackerMode::Park);
      setState(TrackerState::Stopped);
   }
}

void Tracker::WaitForMorning() {
   if (_waitingForMorning == false) {
      _azimuth->Retract(); // wait for morning, full east, lowest elevation
      if (_config.isDual()) {
         _elevation->Retract();
      }
      _waitingForMorning = true;
      setState(TrackerState::Tracking);
      logi("Waiting For Morning");
   }
}

void Tracker::TrackToSun() {
   logi("TrackToSun ");
   setState(TrackerState::Tracking);
   _azimuth->MoveTo(_sun->azimuth());
   if (_config.isDual()) {
      _elevation->MoveTo(_sun->elevation());
   }
}

TrackerState Tracker::getState() {
   // return initializing while actuators are initializing
   if (_azimuth->getState() == ActuatorState_Initializing) {
      return TrackerState::Initializing;
   }
   if (_config.isDual() && _elevation->getState() == ActuatorState_Initializing) {
      _trackerState = TrackerState::Initializing;
   }
   if (_azimuth->getState() == ActuatorState_Error) {
      _errorState = TrackerError_HorizontalActuator;
      return TrackerState::Initializing;
   }
   if (_config.isDual() && _elevation->getState() == ActuatorState_Error) {
      _errorState = TrackerError_VerticalActuator;
      return TrackerState::Initializing;
   }
   return _trackerState;
}

void Tracker::setState(TrackerState state) {
   if (_trackerState != state) {
      logd("setState: %d", state);
      _trackerState = state;
   }
   return;
}

void Tracker::setMode(TrackerMode mode) {
   logd("setMode: %d", mode);
   if (_trackerMode != mode) {
      _trackerMode = mode;
   }
   return;
}

void Tracker::InitializeActuators() {
   logd("InitializeActuators");
   _azimuth->Initialize(_config.getEastAzimuth(), _config.getWestAzimuth(), _config.getHorizontalLength(), _config.getHorizontalSpeed(),
                        _config.getLat() < 0);
   if (_config.isDual()) {
      _elevation->Initialize(_config.getMinimumElevation(), _config.getMaximumElevation(), _config.getVerticalLength(),
                             _config.getVerticalSpeed());
   }
   setInterval(POSITION_UPDATE_INTERVAL);
   setState(TrackerState::Standby);
}

void Tracker::run() {
   runned();
   _cycleTime = getTime();
   switch (_trackerMode) {
   case TrackerMode::Cycle:
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
   case TrackerMode::Track:
      _sun->calcSun(&_cycleTime);
      break;
   default:
      break;
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
      if (_trackerMode == TrackerMode::Track) {
         if (windSpeed > (AnemometerWindSpeedProtection / 3.6)) // wind speed greater than 28.8 km/hour? (8 M/S *3600 S)
         {
            _lastWindEvent = getTime();
            _recordedWindSpeedAtLastEvent = windSpeed;
            Park(true);
            _protectCountdown = 300; // 10 minute countdown to resume tracking
         }
      }
      if (_trackerMode == TrackerMode::Protect && --_protectCountdown <= 0) {
         _protectCountdown = 0;
         Resume();
      }
   }
};

#ifdef HasMQTT

void Tracker::onMqttConnect(esp_mqtt_client_handle_t &client) {
   // Subscribe to command topics
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/mode/set").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Up").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Down").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Left").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Right").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Stop").c_str(), 0);

   if (!_discoveryPublished) {
      
      if (PublishSensorDiscoverySub("Azimuth", "°", "azimuth", "mdi:sun-compass") == false) { return; } // try later
      if (PublishSensorDiscoverySub("Elevation", "°", "elevation", "mdi:sun-compass") == false) { return; }
      if (PublishSensorDiscoverySub("Horizontal_extent", "\"", "horizontal_extent", "mdi:ruler") == false) { return; }
      if (PublishSensorDiscoverySub("Horizontal_angle", "°", "horizontal_angle", "mdi:sun-angle") == false) { return; }
      if (PublishSensorDiscoverySub("Vertical_extent", "\"", "vertical_extent", "mdi:ruler") == false) { return; }
      if (PublishSensorDiscoverySub("Vertical_angle", "°", "vertical_angle", "mdi:sun-angle") == false) { return; }
      if (PublishSensorDiscoverySub("Wind_speed", "km/hr", "wind_speed", "mdi:weather-windy") == false) { return; }
      if (PublishSensorDiscoverySub("State", "", "state", "mdi:state-machine") == false) { return; }
      if (PublishSensorDiscoverySub("Mode", "", "mode", "mdi:wrench-cog-outline") == false) { return; }

      // Discovery payload for tracker mode (select entity)
      String modeConfigTopic = HOME_ASSISTANT_PREFIX;
      modeConfigTopic += "/select/";
      modeConfigTopic += _iot.getRootTopicPrefix();
      modeConfigTopic += "/mode/config";
      JsonDocument payload;
      payload["name"] = "Mode";
      payload["unique_id"] = String(_iot.getUniqueId()) + "_" + String("Tracker_Mode");
      payload["command_topic"] = _iot.getRootTopicPrefix() + String("/mode/set");
      payload["state_topic"] = _iot.getRootTopicPrefix() + String("/mode");
      JsonArray options = payload["options"].to<JsonArray>();
      options.add("Manual");
      options.add("Cycle");
      options.add("Track");
      options.add("Park");
      options.add("Protect");
      if (PublishDiscoverySub(modeConfigTopic, payload) == false) {
         return; // try later
      }
      // Discovery payloads for buttons
      const char *buttons[5] = {"Up", "Down", "Left", "Right", "Stop"};
      for (int i = 0; i < 5; i++) {
         payload.clear();
         String buttonTopic = HOME_ASSISTANT_PREFIX;
         buttonTopic += "/button/";
         buttonTopic += _iot.getRootTopicPrefix();
         buttonTopic += "/";
         buttonTopic += String(buttons[i]);
         buttonTopic += "/config";
         payload["name"] = String(buttons[i]);
         payload["unique_id"] = String("Tracker_") + String(buttons[i]);
         payload["command_topic"] = _iot.getRootTopicPrefix() + String("/button/") + String(buttons[i]);
         if (PublishDiscoverySub(buttonTopic, payload) == false) {
            return; // try later
         }
      }
      _discoveryPublished = true;
      _lastMessagePublished.clear(); // publish state after discovery
   }
}

boolean Tracker::PublishSensorDiscoverySub(const char* name, const char* unitOfMeasure, const char* field, const char* icon) {
   String stateConfigTopic = HOME_ASSISTANT_PREFIX;
   stateConfigTopic += "/sensor/";
   stateConfigTopic += _iot.getRootTopicPrefix();
   stateConfigTopic += "/";
   stateConfigTopic += name;
   stateConfigTopic += "/config";
   JsonDocument payload;
   String text = name;
   text.replace('_', ' ');
   payload["name"] = text;
   payload["unique_id"] = String(_iot.getUniqueId()) + "_" + name;
   payload["state_topic"] = _iot.getRootTopicPrefix() + "/state";
   payload["unit_of_measurement"] = unitOfMeasure;
   payload["value_template"] = String("{{ value_json.") + field + String(" }}");
   payload["icon"] = icon;
   return PublishDiscoverySub(stateConfigTopic, payload);
}

boolean Tracker::PublishDiscoverySub(String &topic, JsonDocument &payload) {
   payload["availability_topic"] = _iot.getRootTopicPrefix() + String("/tele/LWT");
   payload["payload_available"] = "Online";
   payload["payload_not_available"] = "Offline";
   char buffer[STR_LEN];
   JsonObject device = payload["device"].to<JsonObject>();
   device["name"] = _iot.getThingName().c_str();
   device["sw_version"] = APP_VERSION;
   device["manufacturer"] = "ClassicDIY";
   sprintf(buffer, "%s (%X)", TAG, _iot.getUniqueId());
   device["model"] = buffer;
   JsonArray identifiers = device["identifiers"].to<JsonArray>();
   sprintf(buffer, "%X", _iot.getUniqueId());
   identifiers.add(buffer);
   return _iot.PublishMessage(topic.c_str(), payload, true);
}

void Tracker::onMqttMessage(char *topic, char *payload) {
   // Handle state select
   if (String(topic) == (_iot.getRootTopicPrefix() + "/mode/set")) {
      logd("Mode command received: %s", payload);
      String cmd(payload);
      if (cmd == "Track") {
         Track();
      }
      if (cmd == "Park") {
         Park(false);
      }
      if (cmd == "Protect") {
         Park(true);
      }
      if (cmd == "Cycle") {
         Cycle();
      }
   }

   // Handle buttons
   if (String(topic).startsWith(_iot.getRootTopicPrefix() + "/button/")) {
      String button = String(topic).substring(strlen((_iot.getRootTopicPrefix() + "/button/").c_str()));
      Serial.print("Button pressed: ");
      Serial.println(button);
      if (button == "Up") {
         Move(Direction_Up);
      } else if (button == "Down") {
         Move(Direction_Down);
      } else if (button == "Left") {
         Move(Direction_East);
      } else if (button == "Right") {
         Move(Direction_West);
      } else if (button == "Stop") {
         Stop();
      }
   }
}

#endif

} // namespace CLASSICDIY
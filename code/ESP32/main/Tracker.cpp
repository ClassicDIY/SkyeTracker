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

const char *TrackerStateStrings[] = {"Off", "Initializing", "Standby", "Manual", "Cycling", "Tracking", "Parked"};
static AsyncWebServer _asyncServer(ASYNC_WEBSERVER_PORT);
static AsyncWebSocket _webSocket("/ws_home");
IOT _iot = IOT();
Thread *_workerThread = new Thread();
Anemometer _anemometer(AnemometerPin);

Tracker::Tracker() {
   _errorState = TrackerError_Ok;
   _waitingForMorning = false;
   _trackerState = TrackerState_Off;
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
   _trackerState = TrackerState_Off;
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
             setState(TrackerState_Manual);
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

void Tracker::onSaveSetting(JsonDocument &doc) { _config.Save(doc); }

void Tracker::onLoadSetting(JsonDocument &doc) {
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
   if (_trackerState == TrackerState_Off) {
      InitializeActuators();
   }
   String s;
   serializeJson(_ws_home_doc, s);
   if (_lastMessagePublished != s) {
      _lastMessagePublished = s;
#ifdef HasMQTT
      String topic = _iot.getRootTopicPrefix() + "/state";
      String state = _ws_home_doc["state"];
      _iot.PublishMessage(topic.c_str(), state.c_str(), false);
#endif
      if (_webSocket.count() > 0) { // any clients?
         _webSocket.textAll(s);
         logd("_webSocket Sent %s", s.c_str());
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
      logd("Tracker error state: %s", describeTrackerError(_errorState));
      // traceln(_stm, F("Error detected, Tracker moving to default position"));
      // move array to face south as a default position when an error exists
      Park(true);
      enabled = false; // don't run worker thread when error exists
   } else if (getState() != TrackerState_Initializing && getState() != TrackerState_Off) {
      if (getState() == TrackerState_Manual || getState() == TrackerState_Cycling || getState() == TrackerState_Parked || getState() == TrackerState_Protect) {
         // re-initialize actuators if moved
         InitializeActuators();
      }
      enabled = true;
      setInterval(POSITION_UPDATE_INTERVAL);
      logd("Track setState");
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
      setState(protect ? TrackerState_Protect : TrackerState_Parked);
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
         mode = "Cycle";
         break;
      case TrackerState_Tracking:
         mode = "Tracking";
         break;
      case TrackerState_Parked:
         mode = "Park";
         break;
      case TrackerState_Protect:
         mode = "Protect";
         break;
      }
      String topic = _iot.getRootTopicPrefix() + "/mode";
      _iot.PublishMessage(topic.c_str(), mode.c_str(), false);
#endif
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
   setState(TrackerState_Standby);
   _ws_home_doc["state"] = "Initializing Actuators";
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
      _ws_home_doc["state"] = "Cycling";
      break;
   case TrackerState_Tracking:
      _sun->calcSun(&_cycleTime);
      _ws_home_doc["state"] = "Tracking";
      break;
   default:
      break;
   }
   if (_sun->ItsDark()) {
      WaitForMorning();
      _ws_home_doc["state"] = "Waiting for Morning";
   } else {
      _waitingForMorning = false;
      TrackToSun();
      _ws_home_doc["state"] = "Track To Sun";
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

            _ws_home_doc["state"] = "High winds detected";
         }
      }
      if (getState() == TrackerState_Protect && --_protectCountdown <= 0) {
         _protectCountdown = 0;
         Resume();
      }
   }
#ifdef Has_TFT
   _tft.Update(TrackerStateStrings[getState()], _sun, _azimuth, _elevation);
#endif
};

#ifdef HasMQTT

void Tracker::onMqttConnect(esp_mqtt_client_handle_t &client) {
   // Subscribe to command topics
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/state/set").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Up").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Down").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Left").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Right").c_str(), 0);
   esp_mqtt_client_subscribe(client, (_iot.getRootTopicPrefix() + "/button/Stop").c_str(), 0);

   if (!_discoveryPublished) {
      String stateConfigTopic = HOME_ASSISTANT_PREFIX;
      stateConfigTopic += "/sensor/";
      stateConfigTopic += _iot.getRootTopicPrefix();
      stateConfigTopic += "/state/config";
      JsonDocument payload;
      payload["name"] = "State";
      payload["unique_id"] = String(_iot.getUniqueId()) + "_" + String("Tracker_State");
      payload["state_topic"] = _iot.getRootTopicPrefix() + String("/state");
      payload["icon"] = "mdi:solar-power";
      if (PublishDiscoverySub(stateConfigTopic, payload) == false) {
         return; // try later
      }
      // Discovery payload for tracker mode (select entity)
      String modeConfigTopic = HOME_ASSISTANT_PREFIX;
      modeConfigTopic += "/select/";
      modeConfigTopic += _iot.getRootTopicPrefix();
      modeConfigTopic += "/mode/config";
      payload.clear();
      payload["name"] = "Mode";
      payload["unique_id"] = String(_iot.getUniqueId()) + "_" + String("Tracker_Mode");
      payload["command_topic"] = _iot.getRootTopicPrefix() + String("/mode/set");
      payload["state_topic"] = _iot.getRootTopicPrefix() + String("/mode");
      JsonArray options = payload["options"].to<JsonArray>();
      options.add("Off");
      options.add("Manual");
      options.add("Tracking");
      options.add("Park");
      options.add("Protect");
      options.add("Cycle");
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
   }
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
   logd("onMqttMessage [%s] %s", topic, payload);
   // Handle state select
   if (String(topic) == (_iot.getRootTopicPrefix() + "/mode/set")) {
      logd("Mode command received: %s", payload);
      if (payload == "Tracking") {
         TrackToSun();
      }
      if (payload == "Park") {
         Park(false);
      }
      if (payload == "Protect") {
         Park(true);
      }
      if (payload == "Cycle") {
         Cycle();
      }
      _iot.PublishMessage((_iot.getRootTopicPrefix() + String("/mode")).c_str(), payload, true);
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
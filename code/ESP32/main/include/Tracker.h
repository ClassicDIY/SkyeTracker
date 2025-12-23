#pragma once

#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include "mqtt_client.h"
#include "Defines.h"
#include "Device.h"
#include "Sun.h"
#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"
#include "LinearActuatorNoPot.h"
#include "IOTCallbackInterface.h"
#include "IDisplayServiceInterface.h"
#include "Enumerations.h"

extern CLASSICDIY::Configuration _config;

// Rate (in ms) at which the tracker will update to the sun's position
#define POSITION_UPDATE_INTERVAL 60000
// Configuration changed, reset countdown. Changes in the config will
// delay so that the actuator doesn't change direction while it's already moving.
#define PENDING_RESET 3000
// Used for testing the tracker. Sweep through the day one hour at this interval.
#define CYCLE_POSITION_UPDATE_INTERVAL 5000

namespace CLASSICDIY {
class Tracker : public Device, public Thread, public IOTCallbackInterface {
 public:
   Tracker();
   ~Tracker();

   TrackerError _errorState;
   void Setup(ThreadController *controller);
   void Process();
   void Track();
   void Resume();
   void Park(bool protect);
   TrackerState getState();

   // IOTCallbackInterface
   void onNetworkState(NetworkState state);
   void onSaveSetting(JsonDocument &doc);
   void onLoadSetting(JsonDocument &doc);
   String appTemplateProcessor(const String &var);
#ifdef HasMQTT
   void onMqttConnect(esp_mqtt_client_handle_t &client);
   void onMqttMessage(char *topic, char *payload);
#endif
#ifdef HasModbus
   bool onModbusMessage(ModbusMessage &msg);
#endif
#ifdef Has_OLED
   IDisplayServiceInterface &getDisplayInterface() override { return _oled; };
#endif
#ifdef Has_TFT
   IDisplayServiceInterface &getDisplayInterface() override { return _tft; };
#endif
 protected:
#ifdef HasMQTT
   boolean PublishDiscoverySub(String &topic, JsonDocument &payload);
#endif

 private:
   void InitializeActuators();
   void Move(Direction dir);
   void Stop();
   void Cycle();
   void run();

   void setState(TrackerState state);
   void setMode(TrackerMode mode);
   TrackerMode _trackerMode = TrackerMode::Park;
   TrackerState _trackerState = TrackerState::Off;

   Configuration _config;
   boolean _discoveryPublished = false;
   String _lastMessagePublished;
   unsigned long _lastPublishTimeStamp = 0;
   time_t _cycleTime; // used for cycle test
   LinearActuatorNoPot *_azimuth;
   LinearActuatorNoPot *_elevation;
   Sun *_sun;

   bool _waitingForMorning;
   int cycleHour;
   void WaitForMorning();
   void TrackToSun();
   int _protectCountdown = 0;
   float _recordedWindSpeedAtLastEvent = 0;
   time_t _lastWindEvent;
   String _bodyBuffer;
};

} // namespace CLASSICDIY
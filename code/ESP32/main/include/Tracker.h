#pragma once

#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include "Defines.h"
#include "Device.h"
#include "Sun.h"
#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"
#include "LinearActuatorNoPot.h"
#include "IOTCallbackInterface.h"
#include "Enumerations.h"

extern CLASSICDIY::Configuration _config;

// Rate (in ms) at which the tracker will update to the sun's position
#define POSITION_UPDATE_INTERVAL 60000
// Configuration changed, reset countdown. Changes in the config will
// delay so that the actuator doesn't change direction while it's already moving.
#define PENDING_RESET 3000
// Used for testing the tracker. Sweep through the day one hour at this interval.
#define CYCLE_POSITION_UPDATE_INTERVAL 5000

const char c_Track[] = "Track";
const char c_Cycle[] = "Cycle";
const char c_Stop[] = "Stop";
const char c_Park[] = "Park";
const char c_Protect[] = "Protect";
const char c_SetC[] = "SetC";
const char c_SetL[] = "SetL";
const char c_SetA[] = "SetA";
const char c_SetO[] = "SetO";
const char c_SetDateTime[] = "SetDateTime";
const char c_MoveTo[] = "MoveTo";
const char c_East[] = "East";
const char c_West[] = "West";
const char c_Up[] = "Up";
const char c_Down[] = "Down";

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
   void setState(TrackerState state);
   void ProcessCommand(const char *input);

   // IOTCallbackInterface
   void onNetworkState(NetworkState state);
   void addApplicationConfigs(String &page);
   void onSubmitForm(JsonDocument &doc);
   void onSaveSetting(JsonDocument &doc);
   void onLoadSetting(JsonDocument &doc);
#ifdef HasMQTT
   void onMqttConnect();
   void onMqttMessage(char *topic, char *payload);
#endif
#ifdef HasModbus
   bool onModbusMessage(ModbusMessage &msg);
#endif
 protected:
   boolean PublishDiscoverySub(const char *component, const char *entityName, const char *jsonElement, const char *device_class,
                               const char *unit_of_meas, const char *icon = "");
   bool ReadyToPublish() { return (!_discoveryPublished); }
   void UpdateState(String &s);

 private:
   void InitializeActuators();
   void Move(Direction dir);
   void Stop();
   void Cycle();
   void run();

   Configuration _config;
   boolean _discoveryPublished = false;
   String _lastMessagePublished;
   unsigned long _lastPublishTimeStamp = 0;
   time_t _cycleTime; // used for cycle test
   LinearActuatorNoPot *_azimuth;
   LinearActuatorNoPot *_elevation;
   Sun *_sun;
   TrackerState _trackerState;
   bool _waitingForMorning;
   int cycleHour;
   void WaitForMorning();
   void TrackToSun();
   void MoveTo(char *arg);
   int _protectCountdown = 0;
   float _recordedWindSpeedAtLastEvent = 0;
   time_t _lastWindEvent;
};

} // namespace CLASSICDIY
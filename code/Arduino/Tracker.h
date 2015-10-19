#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include "RTClib.h"
#include "sun.h"

#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"

#if defined(NOPOT)
#include "LinearActuatorNoPot.h";
#else
#include "LinearActuatorWithPotentiometer.h"
#endif

#define POSITION_UPDATE_INTERVAL 60000 // Check tracker every minute
#define PENDING_RESET 3000 // Configuration changed, reset countdown
#define CYCLE_POSITION_UPDATE_INTERVAL 10000 // Check tracker every 5 seconds
#define POSITIONINTERVAL 5 // Move array when sun moves 5 degrees past current position


const char c_Track[] = "Track";
const char c_Cycle[] = "Cycle";
const char c_Stop[] = "Stop";
const char c_GetConfiguration[] = "GetConfiguration";
const char c_GetDateTime[] = "GetDateTime";
const char c_BroadcastPosition[] = "BroadcastPosition";
const char c_StopBroadcast[] = "StopBroadcast";
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


namespace SkyeTracker
{
	class Tracker : public Thread
	{
	public:
		Tracker(Configuration* config, RTC_DS1307* rtc);
		~Tracker();

		TrackerError _errorState;
		void Initialize(ThreadController* controller);
		void Track();
		TrackerState getState();
		void setState(TrackerState state);
		void BroadcastPosition();
		void ProcessCommand(const char* input);

	private:

		void InitializeActuators();
		void Move(Direction dir);
		void Stop();
		void Cycle();
		void sendTrackerPosition()
		{
			Serial.print("\"aZ\":"); // array azimuth
			Serial.print(_azimuth->CurrentAngle());
			Serial.print(",\"hP\":"); // horizontal position
			Serial.print(_azimuth->CurrentPosition());
			
			if (_config->isDual())
			{
				Serial.print(",\"aE\":"); // array elevation
				Serial.print(_elevation->CurrentAngle());
				Serial.print(",\"vP\":"); // vertical position
				Serial.print(_elevation->CurrentPosition());
			}
			else
			{
				Serial.print(",\"aE\":0,\"vP\":0"); // no elevation
			}
		};
		void sendSunsPosition()
		{
			Serial.print("\"dk\":"); // isDark
			Serial.print(_sun->ItsDark() ? "true" : "false");
			Serial.print(",\"sZ\":"); // sun azimuth
			Serial.print(_sun->azimuth());
			Serial.print(",\"sE\":"); // sun elevation
			Serial.print(_sun->elevation());
		};
		void sendDateTime()
		{
			Serial.print("Dt|{");
			Serial.print("\"sT\":"); // seconds in unixtime
			Serial.print(_rtc->now().unixtime());
			Serial.println("}");
		};
		void sendState()
		{
			Serial.print("\"tS\":"); // tracker state
			Serial.print(getState());
			Serial.print(",\"tE\":"); // tracker status
			Serial.print(_errorState);
		};

		void run();

		Configuration* _config;
#if defined(NOPOT)
		LinearActuatorNoPot* _azimuth;
		LinearActuatorNoPot* _elevation;
#else
		LinearActuatorWithPotentiometer* _azimuth;
		LinearActuatorWithPotentiometer* _elevation;
#endif

		RTC_DS1307* _rtc;
		Sun* _sun;
		bool _broadcastPosition;
		TrackerState _trackerState = TrackerState_Off;
		bool _waitingForMorning;
		int cycleHour;
		void WaitForMorning();
		void TrackToSun();
		void MoveTo(char* arg);
	};

}
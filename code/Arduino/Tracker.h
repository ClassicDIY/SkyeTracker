#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include "RTClib.h"
#include "sun.h"
#include "LinearActuator.h"
#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"
#include"ConfigTransfer.h"

#define POSITION_UPDATE_INTERVAL 60000 // Check tracker every minute
#define CYCLE_POSITION_UPDATE_INTERVAL 5000 // Check tracker every 5 seconds
#define POSITIONINTERVAL 5 // Move array when sun moves 5 degrees past current position


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
		void BroadcastPosition();
		void ProcessCommand(const char* input);

	private:
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
		LinearActuator* _azimuth;
		LinearActuator* _elevation;
		RTC_DS1307* _rtc;
		Sun* _sun;
		bool _broadcastPosition;
		TrackerState _trackerState = TrackerState_Off;
		int cycleHour;
		void WaitForMorning();
		void TrackToSun();
		void MoveTo(char* arg);
	};

}
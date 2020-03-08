#pragma once

#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include "Sun.h"
#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"
#include "LinearActuatorNoPot.h"
#include "BluetoothSerial.h"
#include "IOT.h"

extern BluetoothSerial ESP_BT;
extern SkyeTracker::Configuration _config;
extern SkyeTracker::IOT _iot;

const char c_Track[] = "Track";
const char c_Cycle[] = "Cycle";
const char c_Stop[] = "Stop";
const char c_Park[] = "Park";
const char c_Protect[] = "Protect";
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
		Tracker();
		~Tracker();

		TrackerError _errorState;
		void Initialize(ThreadController* controller);
		void Track();
		void Resume();
		void Park(bool protect);
		TrackerState getState();
		void setState(TrackerState state);
		bool BroadcastPosition();
		void ProcessCommand(const char* input);

	private:

		void InitializeActuators();
		void Move(Direction dir);
		void Stop();
		void Cycle();
		void sendTrackerPosition()
		{
			ESP_BT.print("\"aZ\":"); // array azimuth
			ESP_BT.print(_azimuth->CurrentAngle());
			ESP_BT.print(",\"hP\":"); // horizontal position
			ESP_BT.print(_azimuth->CurrentPosition());
			
			if (_config.isDual())
			{
				ESP_BT.print(",\"aE\":"); // array elevation
				ESP_BT.print(_elevation->CurrentAngle());
				ESP_BT.print(",\"vP\":"); // vertical position
				ESP_BT.print(_elevation->CurrentPosition());
			}
			else
			{
				ESP_BT.print(",\"aE\":0,\"vP\":0"); // no elevation
			}
		};
		void sendSunsPosition()
		{
			ESP_BT.print("\"dk\":"); // isDark
			ESP_BT.print(_sun->ItsDark() ? "true" : "false");
			ESP_BT.print(",\"sZ\":"); // sun azimuth
			ESP_BT.print(_sun->azimuth());
			ESP_BT.print(",\"sE\":"); // sun elevation
			ESP_BT.print(_sun->elevation());
		};
		void sendDateTime()
		{
			ESP_BT.print("Dt|{");
			ESP_BT.print("\"sT\":"); // seconds in unixtime
			ESP_BT.print(_cycleTime);
			ESP_BT.println("}");
		};
		void sendState()
		{
			ESP_BT.print("\"tS\":"); // tracker state
			ESP_BT.print(getState());
			ESP_BT.print(",\"tE\":"); // tracker status
			ESP_BT.print(_errorState);
		};

		void run();

		time_t _cycleTime; // used for cycle test
		LinearActuatorNoPot* _azimuth;
		LinearActuatorNoPot* _elevation;
		Sun* _sun;
		bool _broadcastPosition;
		TrackerState _trackerState;
		bool _waitingForMorning;
		int cycleHour;
		void WaitForMorning();
		void TrackToSun();
		void MoveTo(char* arg);
	};

}
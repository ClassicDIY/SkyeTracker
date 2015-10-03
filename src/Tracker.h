#pragma once

#include "Arduino.h"
#include <ThreadController.h>
#include <Thread.h>
#include "RTClib.h"
#include "sun.h"
#include "LinearActuator.h"
#include "Configuration.h"
#include "Enumerations.h"
#include "Position.h"
#include "PositionTransfer.h"
#include"ConfigTransfer.h"

#define POSITION_UPDATE_INTERVAL 60000 // Check tracker every minute
#define POSITIONINTERVAL 5 // Move array when sun moves 5 degrees past current position


namespace SkyeTracker
{
	class Tracker : public Thread
	{
	public:
		Tracker(Configuration* config, RTC_DS1307* rtc);
		~Tracker();

		void Initialize(ThreadController* controller);
		void Move(Direction dir);
		void Stop();
		void Track();
		Position getTrackerOrientation()
		{
			Position p;
			p.Azimuth = _azimuth->CurrentAngle();
			p.Elevation = 90 - _elevation->CurrentAngle();
			return p;
		};
		SunsPosition getSunsPosition()
		{
			SunsPosition p;
			p.Azimuth = _sun->azimuth();
			p.Elevation = _sun->elevation();
			p.Dark = _sun->ItsDark();
			return p;
		};
		TrackerState getState();
		void ProcessCommand(String cmd);
		void ProcessArgument(String arg);

	protected:
		void run();

	private:

		Configuration* _config;
		LinearActuator* _azimuth;
		LinearActuator* _elevation;
		RTC_DS1307* _rtc;
		Sun* _sun;

		TrackerState _trackerState = Off;

		void WaitForMorning();
		void TrackToSun();
	};

}
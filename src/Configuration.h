#pragma once

#include "Arduino.h"
#include "RTClib.h"
#include "ConfigTransfer.h"

namespace SkyeTracker
{
	class Configuration
	{
	public:
		Configuration(RTC_DS1307* rtc);
		~Configuration();

	private:
		RTC_DS1307* _rtc;
		bool _dualAxis;
		float _latitude;
		float _longitude;
		float _eastAzimuth;
		float _westAzimuth;
		float _minimumElevation;
		float _maximumElevation;
		int _timeZoneOffsetToUTC;

	public:
		bool isDual() { return _dualAxis; }
		float getLat() { return _latitude; }
		float getLon() { return _longitude; }
		float getEastAzimuth() { return _eastAzimuth; }
		float getWestAzimuth() { return _westAzimuth; }
		float getMinimumElevation() { return _minimumElevation; }
		float getMaximumElevation() { return _maximumElevation; }
		int getTimeZoneOffsetToUTC() { return _timeZoneOffsetToUTC; }
		void setDual(bool val);
		void SetLocation(float lat, float lon);
		void SetLimits(float minAzimuth, float maxAzimuth, float minElevation, float maxElevation);
		void SetUTCOffset(int val);
		void Load();
		void Save();
		void PrintJson();
	};

}
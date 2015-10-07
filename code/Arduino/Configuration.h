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
		int _eastAzimuth;
		int _westAzimuth;
		int _minimumElevation;
		int _maximumElevation;
		signed char _timeZoneOffsetToUTC;
		void LoadFactoryDefault();

	public:
		bool isDual() { return _dualAxis; }
		double getLat() { return _latitude; }
		double getLon() { return _longitude; }
		int getEastAzimuth() { return _eastAzimuth; }
		int getWestAzimuth() { return _westAzimuth; }
		int getMinimumElevation() { return _minimumElevation; }
		int getMaximumElevation() { return _maximumElevation; }
		signed char getTimeZoneOffsetToUTC() { return _timeZoneOffsetToUTC; }
		void setDual(bool val);
		void SetLocation(double lat, double lon);
		void SetLimits(int minAzimuth, int maxAzimuth, int minElevation, int maxElevation);
		void SetUTCOffset(signed char val);
		void Load();
		void Save();
		void SendConfiguration();
	};

}
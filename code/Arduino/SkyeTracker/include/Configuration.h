#pragma once

#include <Arduino.h>
#include "RTClib.h"

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
		bool _hasAnemometer;
		float _latitude;
		float _longitude;
		int _eastAzimuth;
		int _westAzimuth;
		int _minimumElevation;
		int _maximumElevation;
		int _horizontalLength;
		int _verticalLength;
		int _horizontalSpeed;
		int _verticalSpeed;
		void LoadFactoryDefault();
		byte CalcChecksum(byte _buffer[]);
		bool _isDirty;

	public:
		bool isDirty() { return _isDirty; }
		bool isDual() { return _dualAxis; }
		bool hasAnemometer() { return _hasAnemometer; }
		double getLat() { return _latitude; }
		double getLon() { return _longitude; }
		int getEastAzimuth() { return _eastAzimuth; }
		int getWestAzimuth() { return _westAzimuth; }
		int getMinimumElevation() { return _minimumElevation; }
		int getMaximumElevation() { return _maximumElevation; }
		int getHorizontalLength() { return _horizontalLength; }
		int getVerticalLength() { return _verticalLength; }
		int getHorizontalSpeed() { return _horizontalSpeed; }
		int getVerticalSpeed() { return _verticalSpeed; }
		void setDual(bool val);
		void setHasAnemometer(bool val);
		void SetLocation(double lat, double lon);
		void SetLimits(int minAzimuth, int maxAzimuth, int minElevation, int maxElevation);
		void SetActuatorParameters(int horizontalLength, int verticalLength, int horizontalSpeed, int verticalSpeed);
		void Load();
		void Save();
		void SendConfiguration();
	};

}
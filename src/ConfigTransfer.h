#pragma once

#include "Arduino.h"
#include "ArduinoJson.h"


namespace SkyeTracker
{
	class ConfigTransfer {
	public:
		ConfigTransfer();
		~ConfigTransfer();

		boolean _dual;
		float _lat;
		float _long;
		float _eastAz;
		float _westAz;
		float _minElevation;
		float _maxElevation;
		int _offsetToUTC;

		void PrintJson();
	};
}
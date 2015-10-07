#pragma once

#include "Arduino.h"

namespace SkyeTracker
{
	class ConfigTransfer {
	public:
		ConfigTransfer();
		~ConfigTransfer();

		boolean _dual;
		double _lat;
		double _long;
		float _eastAz;
		float _westAz;
		float _minElevation;
		float _maxElevation;
		int _offsetToUTC;
	};
}
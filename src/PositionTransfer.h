#pragma once

#include "Arduino.h"
#include "ArduinoJson.h"

namespace SkyeTracker
{
	class PositionTransfer {
	public:
		PositionTransfer();
		~PositionTransfer();

		float _arrayAz;
		float _arrayEl;
		float _sunAz;
		float _sunEl;
		boolean _isDark;

		void PrintJson();
	};
}
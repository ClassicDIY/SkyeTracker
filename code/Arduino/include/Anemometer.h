#pragma once
#include <Arduino.h>

namespace SkyeTracker
{
	class Anemometer
	{
		int _sensorPin; //Defines the pin that the anemometer output is connected to
	#define voltageConversionConstant .004882814 //This constant maps the value provided from the analog read function, which ranges from 0 to 1023, to actual voltage, which ranges from 0V to 5V

	public:
		Anemometer(int sensorPin);
		~Anemometer();
		float WindSpeed();
	};
}

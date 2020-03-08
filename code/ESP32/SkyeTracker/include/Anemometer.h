#pragma once

namespace SkyeTracker
{
	class Anemometer
	{
		int _sensorPin; // Defines the pin that the anemometer output is connected to
		#define ADC_Resolution 4095.0
	public:
		Anemometer(int sensorPin);
		~Anemometer();
		float WindSpeed();
	};
}

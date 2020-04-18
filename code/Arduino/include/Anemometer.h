#pragma once
#include <Arduino.h>

namespace SkyeTracker
{
	class Anemometer
	{
		int _sensorPin; //Defines the pin that the anemometer output is connected to
	#define voltageConversionConstant .004882814 //This constant maps the value provided from the analog read function, which ranges from 0 to 1023, to actual voltage, which ranges from 0V to 5V

	//Anemometer Technical Variables
	//The following variables correspond to the anemometer sold by Adafruit, but could be modified to fit other anemometers.
	#define voltageMin .4 // Mininum output voltage from anemometer in mV.
	#define windSpeedMin  0 // Wind speed in meters/sec corresponding to minimum voltage
	#define voltageMax 2.0 // Maximum output voltage from anemometer in mV.
	#define windSpeedMax 32 // Wind speed in meters/sec corresponding to maximum voltage

	public:
		Anemometer(int sensorPin);
		~Anemometer();
		float WindSpeed();
	};
}

#pragma once

namespace SkyeTracker
{
	class Anemometer
	{
		int _sensorPin; //Defines the pin that the anemometer output is connected to
		#define ADC_Resolution 4095.0
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

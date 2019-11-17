#include "Anemometer.h"
#include <Arduino.h>

namespace SkyeTracker
{
	Anemometer::Anemometer(int sensorPin)
	{
		_sensorPin = sensorPin;
	}

	Anemometer::~Anemometer()
	{
	}

	// wind speed in meters per second
	float Anemometer::WindSpeed()
	{
		double reading = analogRead(_sensorPin); //Get a value between 0 and 1023 from the analog pin connected to the anemometer
		if (reading < 1 || reading > ADC_Resolution)
		{
			return 0;
		}
		double sensorVoltage = -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
		//Convert voltage value to wind speed using range of max and min voltages and wind speed for the anemometer
		if (sensorVoltage <= voltageMin) {
			return 0; //Check if voltage is below minimum value. If so, set wind speed to zero.
		}
		else {
			return (sensorVoltage - voltageMin)*windSpeedMax / (voltageMax - voltageMin); //For voltages above minimum value, use the linear relationship to calculate wind speed.
		}
	}
}

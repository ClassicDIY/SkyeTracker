
#pragma once

#include "GPIO_pins.h"

#define PUBLISH_RATE_LIMIT 200 // delay between MQTT publishes

// The following variables correspond to the anemometer sold by Adafruit, but could be modified to fit other anemometers.
// Mininum output voltage from anemometer in mV.
#define AnemometerVoltageMin .5
// Maximum output voltage from anemometer in mV.
#define AnemometerVoltageMax 2.0
// Wind speed in meters/sec corresponding to maximum voltage of anemometer.
#define AnemometerWindSpeedMax 32
// Wind speed in km/hour where the array will move to a horizontal position to avoid wind damage.
#define AnemometerWindSpeedProtection 28.8 // in km / h, = > 8 meters per second
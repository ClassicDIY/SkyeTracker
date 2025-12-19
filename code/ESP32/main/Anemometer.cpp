#include "Anemometer.h"
#include <Arduino.h>
#include "defines.h"
#include "Log.h"
#include "GPIO_pins.h"

namespace CLASSICDIY {
Anemometer::Anemometer(int sensorPin)
{
	_sensorPin = sensorPin;
}

Anemometer::~Anemometer()
{
}
float Anemometer::WindSpeed()
{
    float rVal = 0;
    // --- Oversampled ADC read ---
    double reading = ReadOversampledADC();
    if (reading >= 1 && reading <= ADC_Resolution)
    {
        // --- ADC correction polynomial ---
        double sensorVoltage =
            -0.000000000000016 * pow(reading, 4) +
             0.000000000118171 * pow(reading, 3) -
             0.000000301211691 * pow(reading, 2) +
             0.001109019271794 * reading +
             0.034143524634089;
        if (sensorVoltage > AnemometerVoltageMin)
        {
            rVal = (sensorVoltage - AnemometerVoltageMin) *
                   AnemometerWindSpeedMax /
                   (AnemometerVoltageMax - AnemometerVoltageMin);
        }
    }
    // --- Filtering chain ---
    float median = FilterMedian(rVal);
    float filtered = FilterEMA(median);
    // --- Convert to km/h and round to 0.1 ---
    filtered = roundf(filtered * 3.6f * 10.0f);
    return filtered / 10.0f;
}

float Anemometer::ReadOversampledADC()
{
    const int samples = 32;
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++)
        sum += analogRead(_sensorPin);
    return (float)sum / samples;
}

float Anemometer::FilterEMA(float val)
{
    const float alpha = 0.12f;   // tune 0.05–0.2 depending on responsiveness
    _emaFiltered = (alpha * val) + ((1.0f - alpha) * _emaFiltered);
    return _emaFiltered;
}

float Anemometer::FilterMedian(float val)
{
    _medianBuf[_medianIndex] = val;
    _medianIndex = (_medianIndex + 1) % 3;
    float a = _medianBuf[0];
    float b = _medianBuf[1];
    float c = _medianBuf[2];
    // median of 3
    return max(min(a, b), min(max(a, b), c));
}

} // namespace CLASSICDIY

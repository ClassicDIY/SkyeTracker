#pragma once

class Anemometer {
   int _sensorPin; // Defines the pin that the anemometer output is connected to

 public:
   Anemometer(int sensorPin);
   ~Anemometer();
   float WindSpeed();
   	private:
    float ReadOversampledADC();
    float FilterEMA(float val);
    float FilterMedian(float val);
    // --- Filtering state ---
    float _emaFiltered = 0.0;
    float _medianBuf[3] = {0, 0, 0};
    int _medianIndex = 0;
};


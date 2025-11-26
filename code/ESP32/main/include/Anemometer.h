#pragma once

namespace CLASSICDIY {
class Anemometer {
   int _sensorPin; // Defines the pin that the anemometer output is connected to

 public:
   Anemometer(int sensorPin);
   ~Anemometer();
   float WindSpeed();
};
} // namespace CLASSICDIY

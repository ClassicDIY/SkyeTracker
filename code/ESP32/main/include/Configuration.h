#pragma once

#include <ArduinoJson.h>

// Factory default settings
#define DualAxisEnabled true
#define AnemometerEnabled true
#define AzimuthMin 90
#define AzimuthMax 270
#define ElevationMin 0
#define ElevationMax 90
// Location
#define LocationLatitude 45.936527
#define LocationLongitude -75.091259
// Actuators
#define ActuatorHorizontalLength 12
#define ActuatorVerticalLength 8
#define ActuatorHorizontalSpeed 31
#define ActuatorVerticalSpeed 31

namespace CLASSICDIY {
class Configuration {
 public:
   Configuration();
   ~Configuration();

 private:
   bool _dualAxis = false;
   bool _hasAnemometer = false;
   float _latitude;
   float _longitude;
   int _eastAzimuth;
   int _westAzimuth;
   int _minimumElevation;
   int _maximumElevation;
   int _horizontalLength;
   int _verticalLength;
   int _horizontalSpeed;
   int _verticalSpeed;

 public:
   bool isDual() { return _dualAxis; }
   bool hasAnemometer() { return _hasAnemometer; }
   double getLat() { return _latitude; }
   double getLon() { return _longitude; }
   int getEastAzimuth() { return _eastAzimuth; }
   int getWestAzimuth() { return _westAzimuth; }
   int getMinimumElevation() { return _minimumElevation; }
   int getMaximumElevation() { return _maximumElevation; }
   int getHorizontalLength() { return _horizontalLength; }
   int getVerticalLength() { return _verticalLength; }
   int getHorizontalSpeed() { return _horizontalSpeed; }
   int getVerticalSpeed() { return _verticalSpeed; }
   void setDual(bool val);
   void setHasAnemometer(bool val);
   void SetLocation(float lat, float lon);
   void SetLimits(int minAzimuth, int maxAzimuth, int minElevation, int maxElevation);
   void SetActuatorParameters(int horizontalLength, int verticalLength, int horizontalSpeed, int verticalSpeed);
   void Load(JsonDocument &doc);
   void Save(JsonDocument &doc);
};

} // namespace CLASSICDIY
#include "Configuration.h"
#include "defines.h"
#include "GPIO_pins.h"
#include "Log.h"

namespace CLASSICDIY {
Configuration::Configuration() {
   _dualAxis = DualAxisEnabled;
   _hasAnemometer = AnemometerEnabled;
   _horizontalLength = ActuatorHorizontalLength;
   _verticalLength = ActuatorVerticalLength;
   _horizontalSpeed = ActuatorHorizontalSpeed;
   _verticalSpeed = ActuatorVerticalSpeed;
   _latitude = LocationLatitude;
   _longitude = LocationLongitude;
   SetLimits(AzimuthMin, AzimuthMax, ElevationMin, ElevationMax);
}

Configuration::~Configuration() {}

void Configuration::setDual(bool val) { _dualAxis = val; }

void Configuration::setHasAnemometer(bool val) { _hasAnemometer = val; }

void Configuration::SetLocation(float lat, float lon) {
   _latitude = lat;
   _longitude = lon;
}

void Configuration::SetLimits(int minAzimuth, int maxAzimuth, int minElevation, int maxElevation) {
   _eastAzimuth = minAzimuth;
   _westAzimuth = maxAzimuth;
   _minimumElevation = minElevation;
   _maximumElevation = maxElevation;
   // verify limits
   if (_maximumElevation > 90) {
      _maximumElevation = 90;
   }
   if (_maximumElevation < 45) {
      _maximumElevation = 45;
   }

   if (_minimumElevation > 44) {
      _minimumElevation = 44;
   }
   if (_minimumElevation < 0) {
      _minimumElevation = 0;
   }

   if (_eastAzimuth < 0) {
      _eastAzimuth = 0;
   }
   if (_eastAzimuth > 180) {
      _eastAzimuth = 180;
   }

   if (_westAzimuth < 182) {
      _westAzimuth = 182;
   }
   if (_westAzimuth > 359) {
      _westAzimuth = 359;
   }
}

void Configuration::SetActuatorParameters(int horizontalLength, int verticalLength, int horizontalSpeed, int verticalSpeed) {
   _horizontalLength = horizontalLength;
   _verticalLength = verticalLength;
   _horizontalSpeed = horizontalSpeed;
   _verticalSpeed = verticalSpeed;
}

void Configuration::Load(JsonDocument &doc) {
   JsonObject trk = doc["plc"].as<JsonObject>();
   _dualAxis = trk["dualAxis"].isNull() ? 0 : trk["dualAxis"].as<bool>();
   _eastAzimuth = trk["eastAzimuth"].isNull() ? 0 : trk["eastAzimuth"].as<int>();
   _westAzimuth = trk["westAzimuth"].isNull() ? 0 : trk["westAzimuth"].as<int>();
   _minimumElevation = trk["minimumElevation"].isNull() ? 0 : trk["minimumElevation"].as<int>();
   _maximumElevation = trk["maximumElevation"].isNull() ? 0 : trk["maximumElevation"].as<int>();
   _latitude = trk["latitude"].isNull() ? 0 : trk["latitude"].as<int>();
   _longitude = trk["longitude"].isNull() ? 0 : trk["longitude"].as<int>();
   _horizontalLength = trk["horizontalLength"].isNull() ? 0 : trk["horizontalLength"].as<int>();
   _verticalLength = trk["verticalLength"].isNull() ? 0 : trk["verticalLength"].as<int>();
   _horizontalSpeed = trk["horizontalSpeed"].isNull() ? 0 : trk["horizontalSpeed"].as<int>();
   _verticalSpeed = trk["verticalSpeed"].isNull() ? 0 : trk["verticalSpeed"].as<int>();
   _hasAnemometer = trk["hasAnemometer"].isNull() ? 0 : trk["hasAnemometer"].as<int>();
}

void Configuration::Save(JsonDocument &doc) {
   JsonObject trk = doc["tracker"].to<JsonObject>();
   trk["dualAxis"] = _dualAxis;
   trk["eastAzimuth"] = _eastAzimuth;
   trk["westAzimuth"] = _westAzimuth;
   trk["minimumElevation"] = _minimumElevation;
   trk["maximumElevation"] = _maximumElevation;
   trk["latitude"] = _latitude;
   trk["longitude"] = _longitude;
   trk["horizontalLength"] = _horizontalLength;
   trk["verticalLength"] = _verticalLength;
   trk["horizontalSpeed"] = _horizontalSpeed;
   trk["verticalSpeed"] = _verticalSpeed;
   trk["hasAnemometer"] = _hasAnemometer;
   logi("Saved settings");
}

/*
        Configuration|{"_dual":true,"_lat":45.936527,"_long":75.091255,"_eastAz":90.0,"_westAz":270.0,"_minElevation":0.0,"_maxElevation":90.0,"_secondsTime":1444033308}
        */

} // namespace CLASSICDIY
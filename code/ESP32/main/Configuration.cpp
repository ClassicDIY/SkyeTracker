#include <HTTPClient.h>
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

void Configuration::GeoLocate() {
   if (_geolocated == false) {
      HTTPClient http;
      http.begin("http://ip-api.com/json/"); // Free IP geolocation API
      int httpCode = http.GET();

      if (httpCode > 0) {
         String payload = http.getString();
         logd("Response: %s", payload.c_str());

         // Parse JSON
         JsonDocument doc;
         DeserializationError error = deserializeJson(doc, payload);
         if (!error) {
            _latitude = doc["lat"];
            _longitude = doc["lon"];
            _geolocated = true;
            const char *city = doc["city"];
            const char *country = doc["country"];
            logd("Latitude: %f", _latitude);
            logd("Longitude: %f", _longitude);
            logd("City: %s", city);
            logd("Country: %s", country);
         } else {
            logd("JSON parsing failed!");
         }
      } else {
         logd("HTTP request failed, code: %d\n", httpCode);
      }
      http.end();
   }
}

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
   JsonObject trk = doc["tracker"].as<JsonObject>();
   _dualAxis = trk["_dualAxis"].isNull() ? 0 : trk["_dualAxis"].as<bool>();
   _eastAzimuth = trk["_eastAzimuth"].isNull() ? 0 : trk["_eastAzimuth"].as<int>();
   _westAzimuth = trk["_westAzimuth"].isNull() ? 0 : trk["_westAzimuth"].as<int>();
   _minimumElevation = trk["_minimumElevation"].isNull() ? 0 : trk["_minimumElevation"].as<int>();
   _maximumElevation = trk["_maximumElevation"].isNull() ? 0 : trk["_maximumElevation"].as<int>();
   _latitude = trk["_latitude"].isNull() ? 0 : trk["_latitude"].as<float>();
   _longitude = trk["_longitude"].isNull() ? 0 : trk["_longitude"].as<float>();
   _horizontalLength = trk["_horizontalLength"].isNull() ? 0 : trk["_horizontalLength"].as<int>();
   _verticalLength = trk["_verticalLength"].isNull() ? 0 : trk["_verticalLength"].as<int>();
   _horizontalSpeed = trk["_horizontalSpeed"].isNull() ? 0 : trk["_horizontalSpeed"].as<int>();
   _verticalSpeed = trk["_verticalSpeed"].isNull() ? 0 : trk["_verticalSpeed"].as<int>();
   _hasAnemometer = trk["_hasAnemometer"].isNull() ? 0 : trk["_hasAnemometer"].as<bool>();
   _geolocated = trk["_geolocated"].isNull() ? 0 : trk["_geolocated"].as<bool>();
}

void Configuration::Save(JsonDocument &doc) {
   JsonObject trk = doc["tracker"].to<JsonObject>();
   trk["_dualAxis"] = _dualAxis;
   trk["_eastAzimuth"] = _eastAzimuth;
   trk["_westAzimuth"] = _westAzimuth;
   trk["_minimumElevation"] = _minimumElevation;
   trk["_maximumElevation"] = _maximumElevation;
   trk["_latitude"] = _latitude;
   trk["_longitude"] = _longitude;
   trk["_horizontalLength"] = _horizontalLength;
   trk["_verticalLength"] = _verticalLength;
   trk["_horizontalSpeed"] = _horizontalSpeed;
   trk["_verticalSpeed"] = _verticalSpeed;
   trk["_hasAnemometer"] = _hasAnemometer;
   trk["_geolocated"] = _geolocated;
   logi("Saved settings");
}

/*
        Configuration|{"_dual":true,"_lat":45.936527,"_long":75.091255,"_eastAz":90.0,"_westAz":270.0,"_minElevation":0.0,"_maxElevation":90.0,"_secondsTime":1444033308}
        */

} // namespace CLASSICDIY
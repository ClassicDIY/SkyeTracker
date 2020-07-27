#include "Configuration.h"
#include "Log.h"

#define STORAGE_SIZE 30

Preferences _preferences;

namespace SkyeTracker
{
Configuration::Configuration()
{
	_isDirty = false;
}

Configuration::~Configuration()
{
}

void Configuration::setDual(bool val)
{
	_dualAxis = val;
	_isDirty = true;
}

void Configuration::setHasAnemometer(bool val)
{
	_hasAnemometer = val;
	_isDirty = true;
}

void Configuration::SetLocation(float lat, float lon)
{
	_latitude = lat;
	_longitude = lon;
	_isDirty = true;
}

void Configuration::SetLimits(int minAzimuth, int maxAzimuth, int minElevation, int maxElevation)
{
	_eastAzimuth = minAzimuth;
	_westAzimuth = maxAzimuth;
	_minimumElevation = minElevation;
	_maximumElevation = maxElevation;
	// verify limits
	if (_maximumElevation > 90)
	{
		_maximumElevation = 90;
	}
	if (_maximumElevation < 45)
	{
		_maximumElevation = 45;
	}

	if (_minimumElevation > 44)
	{
		_minimumElevation = 44;
	}
	if (_minimumElevation < 0)
	{
		_minimumElevation = 0;
	}

	if (_eastAzimuth < 0)
	{
		_eastAzimuth = 0;
	}
	if (_eastAzimuth > 180)
	{
		_eastAzimuth = 180;
	}

	if (_westAzimuth < 182)
	{
		_westAzimuth = 182;
	}
	if (_westAzimuth > 359)
	{
		_westAzimuth = 359;
	}
	_isDirty = true;
}

void Configuration::SetActuatorParameters(int horizontalLength, int verticalLength, int horizontalSpeed, int verticalSpeed)
{
	_horizontalLength = horizontalLength;
	_verticalLength = verticalLength;
	_horizontalSpeed = horizontalSpeed;
	_verticalSpeed = verticalSpeed;
	_isDirty = true;
}

float deserialFloat(byte *buffer)
{
	float f = 0;
	byte *b = (byte *)&f;
	b[0] = buffer[0];
	b[1] = buffer[1];
	b[2] = buffer[2];
	b[3] = buffer[3];
	return f;
}

int deserialInt(byte *buffer)
{
	int i = 0;
	byte *b = (byte *)&i;
	b[0] = buffer[0];
	b[1] = buffer[1];
	return i;
}

void Configuration::Load()
{

	if (_preferences.begin("SkyeTracker", false))
	{
		if (digitalRead(FACTORY_RESET_PIN) == LOW)
		{
			logi("FACTORY_RESET_PIN LOW, loading default settings");
			LoadFactoryDefault();
			_preferences.clear();
			Save();
		}
		byte _buffer[STORAGE_SIZE + 1];
		_preferences.getBytes("Configuration", _buffer, STORAGE_SIZE);
		if (CalcChecksum(_buffer) == 0)
		{
			logi("Checksum passed, loading saved settings");
			if (_buffer[0] == 1)
				_dualAxis = true;
			else
				_dualAxis = false;
			_eastAzimuth = deserialInt(&(_buffer[2]));
			_westAzimuth = deserialInt(&(_buffer[4]));
			_minimumElevation = deserialInt(&(_buffer[6]));
			_maximumElevation = deserialInt(&(_buffer[8]));
			_latitude = deserialFloat(&(_buffer[10]));
			_longitude = deserialFloat(&(_buffer[14]));
			_horizontalLength = deserialInt(&(_buffer[18]));
			_verticalLength = deserialInt(&(_buffer[20]));
			_horizontalSpeed = deserialInt(&(_buffer[22]));
			_verticalSpeed = deserialInt(&(_buffer[24]));
			if (_buffer[26] == 1)
				_hasAnemometer = true;
			else
				_hasAnemometer = false;
			_isDirty = false;
		}
		else
		{
			logi("Checksum failed, loading default settings");
			LoadFactoryDefault();
		}
	}
	else
	{
		logi("Could not initialize preferences, loading default settings");
		LoadFactoryDefault();
	}
}

void Configuration::LoadFactoryDefault()
{
	logi("loading factory default settings");
	setDual(DualAxisEnabled);
	setHasAnemometer(AnemometerEnabled);
	SetLimits(AzimuthMin, AzimuthMax, ElevationMin, ElevationMax);
	SetActuatorParameters(ActuatorHorizontalLength, ActuatorVerticalLength,
    ActuatorHorizontalSpeed, ActuatorVerticalSpeed);
	SetLocation(LocationLatitude, LocationLongitude);
}

void serialFloat(byte *buffer, float f)
{
	byte *b = (byte *)&f;
	buffer[0] = b[0];
	buffer[1] = b[1];
	buffer[2] = b[2];
	buffer[3] = b[3];
}

void serialInt(byte *buffer, int f)
{
	byte *b = (byte *)&f;
	buffer[0] = b[0];
	buffer[1] = b[1];
}

void Configuration::Save()
{
	byte _buffer[STORAGE_SIZE + 1];
	for (int i = 0; i < STORAGE_SIZE; i++)
	{
		_buffer[i] = 0x00;
	}
	if (_dualAxis)
		_buffer[0] = 1;
	else
		_buffer[0] = 0;

	serialInt(&(_buffer[2]), _eastAzimuth);
	serialInt(&(_buffer[4]), _westAzimuth);
	serialInt(&(_buffer[6]), _minimumElevation);
	serialInt(&(_buffer[8]), _maximumElevation);
	serialFloat(&(_buffer[10]), _latitude); // arduino nano double is the same as float
	serialFloat(&(_buffer[14]), _longitude);

	serialInt(&(_buffer[18]), _horizontalLength);
	serialInt(&(_buffer[20]), _verticalLength);
	serialInt(&(_buffer[22]), _horizontalSpeed);
	serialInt(&(_buffer[24]), _verticalSpeed);
	if (_hasAnemometer)
		_buffer[26] = 1;
	else
		_buffer[26] = 0;
	_buffer[28] = CalcChecksum(_buffer);
	_preferences.putBytes("Configuration", _buffer, STORAGE_SIZE);
	_isDirty = false;
	logi("Saved settings");
}

byte Configuration::CalcChecksum(byte _buffer[])
{
	byte crc = 0;
	for (int i = 0; i < STORAGE_SIZE; i++)
		crc += _buffer[i];
	crc = -crc;
	logi("Checksum cal: %x", crc);
	return crc;
}

/*
	Configuration|{"_dual":true,"_lat":45.936527,"_long":75.091255,"_eastAz":90.0,"_westAz":270.0,"_minElevation":0.0,"_maxElevation":90.0,"_secondsTime":1444033308}
	*/

void Configuration::SendConfiguration()
{
	ESP_BT.print(F("Cf|{"));
	ESP_BT.print(F("\"d\":"));
	ESP_BT.print(isDual() ? F("true") : F("false"));
	ESP_BT.print(F(",\"a\":"));
	ESP_BT.print(getLat(), 6);
	ESP_BT.print(F(",\"o\":"));
	ESP_BT.print(getLon(), 6);
	ESP_BT.print(F(",\"e\":"));
	ESP_BT.print(getEastAzimuth());
	ESP_BT.print(F(",\"w\":"));
	ESP_BT.print(getWestAzimuth());
	ESP_BT.print(F(",\"n\":"));
	ESP_BT.print(getMinimumElevation());
	ESP_BT.print(F(",\"x\":"));
	ESP_BT.print(getMaximumElevation());
	ESP_BT.print(F(",\"lh\":"));
	ESP_BT.print(getHorizontalLength());
	ESP_BT.print(F(",\"lv\":"));
	ESP_BT.print(getVerticalLength());
	ESP_BT.print(F(",\"sh\":"));
	ESP_BT.print(getHorizontalSpeed());
	ESP_BT.print(F(",\"sv\":"));
	ESP_BT.print(getVerticalSpeed());
	ESP_BT.print(F(",\"an\":"));
	ESP_BT.print(hasAnemometer() ? F("true") : F("false"));
	ESP_BT.println(F("}"));
}


time_t GetSystemTime()
{
	uint32_t start = millis();
	time_t now;
	while ((millis() - start) <= 100)
	{
		time(&now);
		if (now > 1573996930)
		{
			return now;
		}
		delay(10);
	}
	return (time_t)0;
}

// try to get valid time from system, last resort use last saved time from watchdog reset
time_t Configuration::GetTime()
{
	time_t now = GetSystemTime();
	if (now == 0)
	{
		return (time_t)_preferences.getLong("time");
	}
	return now;
}

void Configuration::SaveTime()
{
	time_t now = GetSystemTime();
	if (now != 0)
	{
		_preferences.putLong("time", (int32_t)now);
	}
}

} // namespace SkyeTracker
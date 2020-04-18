#include "Configuration.h"
#include <SoftwareSerial.h>
#include <util/crc16.h>
#include "Trace.h"

extern SoftwareSerial _swSer;

#define DS1307_RAM_SIZE 30

namespace SkyeTracker
{

	Configuration::Configuration(RTC_DS1307* rtc)
	{
		_rtc = rtc;
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

	void Configuration::SetLocation(double lat, double lon)
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

	float deserialFloat(byte* buffer) {
		float f = 0;
		byte * b = (byte *)&f;
		b[0] = buffer[0];
		b[1] = buffer[1];
		b[2] = buffer[2];
		b[3] = buffer[3];
		return f;
	}

	int deserialInt(byte* buffer) {
		int i = 0;
		byte * b = (byte *)&i;
		b[0] = buffer[0];
		b[1] = buffer[1];
		return i;
	}

	void Configuration::Load()
	{
		byte _buffer[DS1307_RAM_SIZE+1];
		_rtc->readnvram(_buffer, DS1307_RAM_SIZE, 0);
		if (CalcChecksum(_buffer) == 0)
		{
			traceln(&_swSer, F("Checksum passed, loading saved settings"));
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
			traceln(&_swSer, F("Checksum failed, loading default settings"));
			LoadFactoryDefault();
		}
	}

	void Configuration::LoadFactoryDefault()
	{
		setDual(true);
		setHasAnemometer(false);
		SetLimits(90, 270, 0, 90);
		SetActuatorParameters(12, 8, 31, 31);
		//45.936527, -75.091259 Lac Simon
		SetLocation(45.936527, -75.091259);
		Save();
	}

	void serialFloat(byte * buffer, float f) {
		byte * b = (byte *)&f;
		buffer[0] = b[0];
		buffer[1] = b[1];
		buffer[2] = b[2];
		buffer[3] = b[3];
	}

	void serialInt(byte * buffer, int f) {
		byte * b = (byte *)&f;
		buffer[0] = b[0];
		buffer[1] = b[1];
	}


	void Configuration::Save()
	{
		byte _buffer[DS1307_RAM_SIZE+1];
		for (int i = 0; i < DS1307_RAM_SIZE; i++)
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
		serialFloat(&(_buffer[10]), _latitude);  // arduino nano double is the same as float
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
		_rtc->writenvram(0, _buffer, DS1307_RAM_SIZE);
		_isDirty = false;
		traceln(&_swSer, F("Saved settings"));
	}

	byte Configuration::CalcChecksum(byte _buffer[])
	{
		byte crc = 0;
		for (int i = 0; i < DS1307_RAM_SIZE; i++)
			crc += _buffer[i];
		crc = -crc;
		return crc;
	}

	/*
	Configuration|{"_dual":true,"_lat":45.936527,"_long":75.091255,"_eastAz":90.0,"_westAz":270.0,"_minElevation":0.0,"_maxElevation":90.0,"_secondsTime":1444033308}
	*/

	void Configuration::SendConfiguration()
	{
		Serial.print(F("Cf|{"));
		Serial.print(F("\"d\":"));
		Serial.print(isDual() ? F("true") : F("false"));
		Serial.print(F(",\"a\":"));
		Serial.print(getLat(), 6);
		Serial.print(F(",\"o\":"));
		Serial.print(getLon(), 6);
		Serial.print(F(",\"e\":"));
		Serial.print(getEastAzimuth());
		Serial.print(F(",\"w\":"));
		Serial.print(getWestAzimuth());
		Serial.print(F(",\"n\":"));
		Serial.print(getMinimumElevation());
		Serial.print(F(",\"x\":"));
		Serial.print(getMaximumElevation());
		Serial.print(F(",\"lh\":"));
		Serial.print(getHorizontalLength());
		Serial.print(F(",\"lv\":"));
		Serial.print(getVerticalLength());
		Serial.print(F(",\"sh\":"));
		Serial.print(getHorizontalSpeed());
		Serial.print(F(",\"sv\":"));
		Serial.print(getVerticalSpeed());
		Serial.print(F(",\"an\":"));
		Serial.print(hasAnemometer() ? F("true") : F("false"));
		Serial.println(F("}"));
	}

}
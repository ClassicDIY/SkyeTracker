#include "ConfigTransfer.h"

namespace SkyeTracker
{
	ConfigTransfer::ConfigTransfer()
	{
	}

	ConfigTransfer::~ConfigTransfer()
	{
	}


	void ConfigTransfer::PrintJson()
	{

		StaticJsonBuffer<140> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root.set("_dual", _dual);
		root.set("_lat", _lat, 6);
		root.set("_long", _long, 6);
		root.set("_eastAz", _eastAz, 1);
		root.set("_westAz", _westAz, 1);
		root.set("_minElevation", _minElevation, 1);
		root.set("_maxElevation", _maxElevation, 1);
		root.set("_offsetToUTC", _offsetToUTC);
		//Serial.println(root.measureLength()); // 138
		if (root.success())
		{
			Serial.print("Configuration|");
			root.printTo(Serial);
			Serial.println();
		}
		else
		{
			Serial.println("invalid root");
		}
	}
}
#include "PositionTransfer.h"

namespace SkyeTracker
{
	PositionTransfer::PositionTransfer()
	{
	}

	PositionTransfer::~PositionTransfer()
	{
	}

	void PositionTransfer::PrintJson()
	{
		StaticJsonBuffer<80> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root.set("_isDark", _isDark);
		root.set("_arrayAz", _arrayAz, 1);
		root.set("_arrayEl", _arrayEl, 1l);
		root.set("_sunAz", _sunAz, 1);
		root.set("_sunEl", _sunEl, 1);
		if (root.success())
		{
			Serial.println(root.measureLength());
			Serial.print("Position|");
			root.printTo(Serial);
			Serial.println();
		}
		else
		{
			Serial.println("invalid root");
		}
	}
}
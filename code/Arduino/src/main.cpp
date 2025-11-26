
#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include "sun.h"
#include "Configuration.h"
#include "Tracker.h"
#include "Trace.h"
#include "Anemometer.h"

using namespace SkyeTracker;

#define RECEIVE_BUFFER 200

RTC_DS1307 _rtc;
Configuration _config(&_rtc);
ThreadController _controller = ThreadController();
Tracker _tracker(&_config, &_rtc);
Anemometer _anemometer(A3);
Thread* _workerThread = new Thread();
SoftwareSerial _swSer(8, 9, false);
char* _receiveBuffer;
int _receiveIndex = 0;
int _protectCountdown = 0;
float _recordedWindSpeedAtLastEvent = 0;
uint32_t _lastWindEvent;

int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void runWorker()
{
	if (_config.hasAnemometer())
	{
		float windSpeed = _anemometer.WindSpeed();
		if (_recordedWindSpeedAtLastEvent < windSpeed)
		{
			_lastWindEvent = _rtc.now().unixtime();
			_recordedWindSpeedAtLastEvent = windSpeed;
		}
		if (_tracker.getState() == TrackerState_Tracking)
		{
			if (windSpeed > (AnemometerWindSpeedProtection / 3.6)) // wind speed greater than 28.8 km/hour? (8 M/S *3600 S)
			{
				_lastWindEvent = _rtc.now().unixtime();
				_recordedWindSpeedAtLastEvent = windSpeed;
				_tracker.Park(true);
				_protectCountdown = 300; // 10 minute countdown to resume tracking
			}
			else if (--_protectCountdown <= 0)
			{
				_protectCountdown = 0;
				_tracker.Resume();
			}
		}
	}
}

void setup()
{
	Serial.begin(9600); // HC-06 default baud rate
	while (!Serial) {
		; // wait for serial port to connect.
	}
	_swSer.begin(115200);
	//freeMem("At start of setup");
	trace(&_swSer, F("RAM At start of setup: "));
	traceln(&_swSer, String(freeRam(), DEC));
	Wire.begin();
	_rtc.begin();
	if (!_rtc.isrunning()) {
		traceln(&_swSer, F("RTC not running!"));
		_rtc.adjust(DateTime(2015, 6, 21, 12, 00, 0));
		_tracker._errorState = TrackerError_FailedToAccessRTC;
	}
	_lastWindEvent = 0;
	String d = "";
	DateTime now = _rtc.now();
	d = d + now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + ":" + now.minute() + ":" + now.second() + "    ";
	traceln(&_swSer, d);
	traceln(&_swSer, F("Loading configuration"));
	_config.Load();
	// Configure main worker thread
	_workerThread->onRun(runWorker);
	_workerThread->setInterval(2000);
	_controller.add(_workerThread);
	traceln(&_swSer, F("Initializing tracker"));
	_tracker.Initialize(&_controller);
	_tracker.Track();
	_receiveBuffer = (char*)malloc(RECEIVE_BUFFER);
	//freeMem("At end of setup");
	traceln(&_swSer, F("RAM Completing setup: ") );
	traceln(&_swSer, String(freeRam(), DEC));
}

void loop()
{
	_controller.run();
	if (_tracker.getState() == TrackerState_Standby)
	{
		_tracker.Track();
	}
}

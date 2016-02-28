
#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include <Wire.h> 
#include <SoftwareSerial.h>

#include "RTClib.h"
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
SoftwareSerial _swSer(12, 13, false);

char* _receiveBuffer;
int _receiveIndex = 0;
int _protectCountdown = 0;
float _recordedWindSpeedAtLastEvent = 0;
uint32_t _lastWindEvent;

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
	_tracker.Initialize(&_controller, &_swSer);
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
	serialEvent();
}

void runWorker()
{
	//freeMem("Run");
	//trace(&_swSer, F("RAM in Run: "));
	//traceln(&_swSer, String(freeRam(), DEC));
	bool broadcasting = _tracker.BroadcastPosition();
	if (_config.hasAnemometer())
	{
		float windSpeed = _anemometer.WindSpeed();
		if (windSpeed > 11) // wind speed greater than 40 km/hour?
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
		if (broadcasting)
		{
			Serial.print("Wind|{");
			Serial.print("\"sT\":"); // seconds in unixtime
			Serial.print(_lastWindEvent);
			Serial.print(",\"sC\":"); // current wind speed
			Serial.print(windSpeed);
			Serial.print(",\"sH\":"); // last high wind speed
			Serial.print(_recordedWindSpeedAtLastEvent);
			Serial.println("}");
		};
	}
}

void serialEvent() {
	
	while (Serial.available()) {
		char inChar = (char)Serial.read();
		_receiveBuffer[_receiveIndex++] = inChar;
		if (_receiveIndex >= RECEIVE_BUFFER)
		{
			Serial.print(F("receive buffer overflow!: "));
			_receiveIndex = 0;
		}
		else if (inChar == '\r') {
			//printHexString(_receiveBuffer);
			_receiveBuffer[_receiveIndex] = 0;
			_tracker.ProcessCommand(_receiveBuffer);
			_receiveIndex = 0;
		}
	}
}

// Debug helper functions
//
//void printHexString(String txt)
//{
//	for (int i = 0; i < txt.length(); i++)
//	{
//		trace(&_swSer, txt[i], HEX);
//	}
//	traceln(&_swSer, "");
//}


//Code to print out the free memory

//struct __freelist {
//	size_t sz;
//	struct __freelist *nx;
//};
//
//extern char * const __brkval;
//extern struct __freelist *__flp;
//
//uint16_t freeMem(uint16_t *biggest)
//{
//	char *brkval;
//	char *cp;
//	unsigned freeSpace;
//	struct __freelist *fp1, *fp2;
//
//	brkval = __brkval;
//	if (brkval == 0) {
//		brkval = __malloc_heap_start;
//	}
//	cp = __malloc_heap_end;
//	if (cp == 0) {
//		cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
//	}
//	if (cp <= brkval) return 0;
//
//	freeSpace = cp - brkval;
//
//	for (*biggest = 0, fp1 = __flp, fp2 = 0;
//	fp1;
//		fp2 = fp1, fp1 = fp1->nx) {
//		if (fp1->sz > *biggest) *biggest = fp1->sz;
//		freeSpace += fp1->sz;
//	}
//
//	return freeSpace;
//}
//
//uint16_t biggest;
//
//void freeMem(char* message) {
//	trace(&_swSer, message);
//	trace(&_swSer, ":\t");
//	traceln(&_swSer, freeMem(&biggest));
//}

int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}



#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include <Wire.h> 
#include "RTClib.h"
#include "sun.h"
#include "Configuration.h"
#include "Tracker.h"

using namespace SkyeTracker;

#define RECEIVE_BUFFER 200

RTC_DS1307 _rtc;
Configuration _config(&_rtc);
ThreadController _controller = ThreadController();
Tracker _tracker(&_config, &_rtc);
Thread* _workerThread = new Thread();
char* _receiveBuffer;
int _receiveIndex = 0;

void setup()
{
	Serial.begin(9600); // HC-06 default baud rate
	while (!Serial) {
		; // wait for serial port to connect.
	}
	//freeMem("At start of setup");
	Serial.print(F("RAM At start of setup: "));
	Serial.println(String(freeRam(), DEC));
	Wire.begin();
	_rtc.begin();
	if (!_rtc.isrunning()) {
		Serial.println(F("RTC not running!"));
		_rtc.adjust(DateTime(2015, 6, 21, 12, 00, 0));
		_tracker._errorState = TrackerError_FailedToAccessRTC;
	}
	String d = "";
	DateTime now = _rtc.now();
	d = d + now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + ":" + now.minute() + ":" + now.second() + "    ";
	Serial.println(d);
	Serial.println(F("Loading configuration"));
	_config.Load();
	// Configure main worker thread
	_workerThread->onRun(runWorker);
	_workerThread->setInterval(2000);
	_controller.add(_workerThread);
	Serial.println(F("Initializing tracker"));
	_tracker.Initialize(&_controller);
	_tracker.Track();
	_receiveBuffer = (char*)malloc(RECEIVE_BUFFER);
	//freeMem("At end of setup");
	Serial.print(F("RAM Completing setup: ") );
	Serial.println(String(freeRam(), DEC));
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
	//Serial.print(F("RAM in Run: "));
	//Serial.println(String(freeRam(), DEC));
	_tracker.BroadcastPosition();
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
			_receiveBuffer[_receiveIndex] = NULL;
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
//		Serial.print(txt[i], HEX);
//	}
//	Serial.println("");
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
//	Serial.print(message);
//	Serial.print(":\t");
//	Serial.println(freeMem(&biggest));
//}

int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}


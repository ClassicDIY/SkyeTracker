

#include <ArduinoJson.h>
#include <ThreadController.h>
#include <Thread.h>
#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "RTClib.h"
#include "sun.h"
#include "LinearActuator.h"
#include "Configuration.h"
#include "Tracker.h"

using namespace SkyeTracker;

RTC_DS1307 _rtc;
Configuration _config(&_rtc);
//LiquidCrystal_I2C _lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
ThreadController _controller = ThreadController();
Tracker _tracker(&_config, &_rtc);
Thread* lcdThread = new Thread();
char _receiveBuffer[32];
int _receiveIndex = 0;

void setup()
{
	Serial.begin(9600);
	delay(1000);
	freeMem("setup1");
	//_lcd.init();
	Wire.begin();
	_rtc.begin();
	// Print a message to the LCD.
	//_lcd.backlight();
	//_lcd.print("SkyeTracker");

	if (!_rtc.isrunning()) {
		Serial.println("RTC not running!");
		_rtc.adjust(DateTime(2015, 9, 30, 9, 12, 0));
	}
	String d = "";
	DateTime now = _rtc.now();
	d = d + now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + ":" + now.minute() + ":" + now.second() + "    ";
	Serial.println(d);

	//_lcd.setCursor(0, 2);
	//_lcd.print("Initializing");
	Serial.println("Loading configuration");
	_config.Load();

	// Configure lcdThread
	lcdThread->onRun(lcdUpdate);
	lcdThread->setInterval(2000);
	_controller.add(lcdThread);
	freeMem("setup4");
	Serial.println("Initializing tracker");
	_tracker.Initialize(&_controller);
	_tracker.Track();
	Serial.println("Completing setup");
	freeMem("setup5");
}

void loadFactoryDefault()
{
	_config.setDual(true);
	_config.SetUTCOffset(5);
	_config.SetLimits(90, 270, 0, 90);
	//45.936527, 75.091259 Lac Simon
	_config.SetLocation(45.936527, 75.091259);
	_config.PrintJson();
	_config.Save();
}

void loop()
{
	_controller.run();
	if (_tracker.getState() == Standby)
	{
		_tracker.Track();
	}
	serialEvent();
}

void lcdUpdate()
{
	freeMem("Run");
	//DateTime now = _rtc.now();
	//_lcd.setCursor(0, 1);
	//String d = "";
	//d = d + now.year() + "/" + now.month() + "/" + now.day() + " " + now.hour() + ":" + now.minute() + ":" + now.second() + "    ";
	//_lcd.print(d);
	//if (_tracker.getState() != Initializing)
	//{
	//	d = "T A: ";
	//	Position p = _tracker.getTrackerOrientation();
	//	d += p.Azimuth;
	//	d += " E: ";
	//	d += p.Elevation;
	//	_lcd.setCursor(0, 2);
	//	_lcd.print(d);
	//	
	//	SunsPosition s = _tracker.getSunsPosition();
	//	if (s.Dark)
	//	{
	//		d = "It's Dark!";
	//	}
	//	else
	//	{
	//		d = "S A: ";
	//		d += s.Azimuth;
	//		d += " E: ";
	//		d += s.Elevation;
	//	}
	//	_lcd.setCursor(0, 3);
	//	_lcd.print(d);
	//}

}

void serialEvent() {
	
	while (Serial.available()) {
		char inChar = (char)Serial.read();
		_receiveBuffer[_receiveIndex++] = inChar;
		if (_receiveIndex >= sizeof(_receiveBuffer))
		{
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

void printHexString(String txt)
{
	for (int i = 0; i < txt.length(); i++)
	{
		Serial.print(txt[i], HEX);
	}
	Serial.println("");
}


//Code to print out the free memory

struct __freelist {
	size_t sz;
	struct __freelist *nx;
};

extern char * const __brkval;
extern struct __freelist *__flp;

uint16_t freeMem(uint16_t *biggest)
{
	char *brkval;
	char *cp;
	unsigned freeSpace;
	struct __freelist *fp1, *fp2;

	brkval = __brkval;
	if (brkval == 0) {
		brkval = __malloc_heap_start;
	}
	cp = __malloc_heap_end;
	if (cp == 0) {
		cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
	}
	if (cp <= brkval) return 0;

	freeSpace = cp - brkval;

	for (*biggest = 0, fp1 = __flp, fp2 = 0;
	fp1;
		fp2 = fp1, fp1 = fp1->nx) {
		if (fp1->sz > *biggest) *biggest = fp1->sz;
		freeSpace += fp1->sz;
	}

	return freeSpace;
}

uint16_t biggest;

void freeMem(char* message) {
	Serial.print(message);
	Serial.print(":\t");
	Serial.println(freeMem(&biggest));
}
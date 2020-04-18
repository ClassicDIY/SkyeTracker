
#include "Arduino.h"
#include <ThreadController.h>
#include <Thread.h>
#include <BluetoothSerial.h>
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include <sys/time.h>
#include "time.h"
#include "Sun.h"
#include "Configuration.h"
#include "IOT.h"
#include "Tracker.h"
#include "Anemometer.h"
#include "Log.h"

using namespace SkyeTracker;

#define RECEIVE_BUFFER 200
#define WATCHDOG_TIMER 60000 //time in ms to trigger the watchdog

char *_receiveBuffer;
int _receiveIndex = 0;
int _protectCountdown = 0;
float _recordedWindSpeedAtLastEvent = 0;
time_t _lastWindEvent;
hw_timer_t *timer = NULL;
Configuration _config = Configuration();
ThreadController _controller = ThreadController();
Tracker _tracker = Tracker();
Anemometer _anemometer(AnemometerPin);
Thread *_workerThread = new Thread();
BluetoothSerial ESP_BT;
IOT _iot = IOT();

void IRAM_ATTR resetModule()
{
	ets_printf("watchdog timer expired - rebooting\n");
	_config.SaveTime(); // save current time during reboot in case we don't have wifi/ntp
	esp_restart();
}

void init_watchdog()
{
	if (timer == NULL)
	{
		timer = timerBegin(0, 80, true);					  //timer 0, div 80
		timerAttachInterrupt(timer, &resetModule, true);	  //attach callback
		timerAlarmWrite(timer, WATCHDOG_TIMER * 1000, false); //set time in us
		timerAlarmEnable(timer);							  //enable interrupt
	}
}

void feed_watchdog()
{
	if (timer != NULL)
	{
		timerWrite(timer, 0); // feed the watchdog
	}
}

bool hasClient = false;
void runWorker()
{
	bool broadcasting = _tracker.BroadcastPosition();
	if (_config.hasAnemometer())
	{
		float windSpeed = _anemometer.WindSpeed();
		if (_recordedWindSpeedAtLastEvent < windSpeed)
		{
			_lastWindEvent = _config.GetTime();
			_recordedWindSpeedAtLastEvent = windSpeed;
		}
		if (windSpeed > (AnemometerWindSpeedProtection / 3.6)) // wind speed greater than 28.8 km/hour? (8 M/S *3600 S)
		{
			_lastWindEvent = _config.GetTime();
			_recordedWindSpeedAtLastEvent = windSpeed;
			_tracker.Park(true);
			_protectCountdown = 300; // 10 minute countdown to resume tracking
		}
		else if (--_protectCountdown <= 0)
		{
			_protectCountdown = 0;
			if (_tracker.getState() == TrackerState_Parked)
			{
				_tracker.Resume();
			}
		}
		if (broadcasting && ESP_BT.hasClient())
		{
			ESP_BT.print("Wind|{");
			ESP_BT.print("\"sT\":"); // seconds in unixtime
			ESP_BT.print(_lastWindEvent);
			ESP_BT.print(",\"sC\":"); // current wind speed
			ESP_BT.print(windSpeed);
			ESP_BT.print(",\"sH\":"); // last high wind speed
			ESP_BT.print(_recordedWindSpeedAtLastEvent);
			ESP_BT.println("}");
		};
	}
	feed_watchdog();
	if (!hasClient && ESP_BT.hasClient())
	{
		hasClient = true;
		logi("BT has client");
	}
}

void bluetoothEvent()
{
	while (ESP_BT.available())
	{
		char inChar = (char)ESP_BT.read();
		_receiveBuffer[_receiveIndex++] = inChar;
		if (_receiveIndex >= RECEIVE_BUFFER)
		{
			loge("receive buffer overflow!: ");
			_receiveIndex = 0;
		}
		else if (inChar == '\r')
		{
			printHexString(_receiveBuffer, _receiveIndex);
			_receiveBuffer[_receiveIndex] = 0;
			_tracker.ProcessCommand(_receiveBuffer);
			_receiveIndex = 0;
		}
	}
}

void setup()
{
	Serial.begin(115200); 
	while (!Serial)
	{
		; // wait for serial port to connect.
	}
	logi("Loading configuration");
	_config.Load();
	_iot.Init();
	_lastWindEvent = 0;
	ESP_BT.begin("HC-06"); // name must be HC-06 to work with Android App
	while (!ESP_BT.isReady())
	{
		;
	}
	logi("Bluetooth Device is Ready to Pair");
	// Configure main worker thread
	_workerThread->onRun(runWorker);
	_workerThread->setInterval(2000);
	_controller.add(_workerThread);
	logi("Initializing tracker");
	_tracker.Initialize(&_controller);
	_tracker.Track();
	_receiveBuffer = (char *)malloc(RECEIVE_BUFFER);
	init_watchdog();
}

void loop()
{
	_iot.Run();
	_controller.run();
	if (_tracker.getState() == TrackerState_Standby)
	{
		_tracker.Track();
	}
	bluetoothEvent();
}

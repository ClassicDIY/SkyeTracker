
#include "Arduino.h"
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ThreadController.h>
#include <Thread.h>
#include <sys/time.h>
#include "time.h"
#include "Log.h"
#include "GPIO_pins.h"
#include "Defines.h"
#include "Configuration.h"
#include "Tracker.h"
#include "GPIO_pins.h"


using namespace CLASSICDIY;

ThreadController _controller = ThreadController();
Tracker _tracker = Tracker();

void setup() {
#ifdef ENABLE_H // disable actuators during initialization
   pinMode(ENABLE_H, OUTPUT);
   digitalWrite(ENABLE_H, false);
   pinMode(ENABLE_V, OUTPUT);
   digitalWrite(ENABLE_V, false);
#endif
   delay(3000);
   // wait for Serial to connect, give up after 5 seconds, USB may not be connected
   unsigned long start = millis();
   Serial.begin(115200);
   while (!Serial) {
      if (5000 < millis() - start) {
         break;
      }
   }

   logd("------------ESP32 specifications ---------------");
   logd("Chip Model: %s", ESP.getChipModel());
   logd("Chip Revision: %d", ESP.getChipRevision());
   logd("Number of CPU Cores: %d", ESP.getChipCores());
   logd("CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
   logd("Flash Memory Size: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
   logd("Flash Frequency: %d MHz", ESP.getFlashChipSpeed() / 1000000);
   logd("Heap Size: %d KB", ESP.getHeapSize() / 1024);
   logd("Free Heap: %d KB", ESP.getFreeHeap() / 1024);
   logd("------------ESP32 specifications ---------------");

   _tracker.Setup(&_controller);
   esp_task_wdt_init(60, true); // 60-second timeout, panic on timeout
   esp_task_wdt_add(NULL);
   logd("Free Heap after setup: %d KB", ESP.getFreeHeap() / 1024);
   logd("------------Setup Done ---------------");
}

void loop() {
   _tracker.Process();
   _controller.run();
   if (_tracker.getState() == TrackerState::Standby) {
      _tracker.Track();
   }
   esp_task_wdt_reset(); // Feed the watchdog
   delay(10);
}

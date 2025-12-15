
#pragma once

#include "GPIO_pins.h"

#define TAG "SkyeTracker"

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define HOME_ASSISTANT_PREFIX "homeassistant" // Home Assistant Auto discovery root topic

#define WATCHDOG_TIMEOUT 10 // time in seconds to trigger the watchdog reset
#define STR_LEN 64
#define EEPROM_SIZE 2048
#define LOG_BUFFER_SIZE 1024
#define AP_BLINK_RATE 600
#define NC_BLINK_RATE 100

// #define AP_TIMEOUT 1000 //set back to 30000 in production
#define AP_TIMEOUT 30000 
#define FLASHER_TIMEOUT 10000
#define GPIO0_FactoryResetCountdown 5000 // do a factory reset if GPIO0 is pressed for GPIO0_FactoryResetCountdown
#define WS_CLIENT_CLEANUP 5000
#define WIFI_CONNECTION_TIMEOUT 120000
#define DEFAULT_AP_PASSWORD "12345678"
#define SAMPLESIZE 5
#define MQTT_PUBLISH_RATE_LIMIT 500 // delay between MQTT publishes
#define MODBUS_POLL_RATE 1000
#define MODBUS_RTU_TIMEOUT 2000
#define MODBUS_RTU_REQUEST_QUEUE_SIZE 64
#define ASYNC_WEBSERVER_PORT 80
#define DNS_PORT 53

// The following variables correspond to the anemometer sold by Adafruit, but could be modified to fit other anemometers.
// Mininum output voltage from anemometer in mV.
#define AnemometerVoltageMin .40
// Maximum output voltage from anemometer in mV.
#define AnemometerVoltageMax 2.0
// Wind speed in meters/sec corresponding to maximum voltage of anemometer.
#define AnemometerWindSpeedMax 32
// Wind speed in km/hour where the array will move to a horizontal position to avoid wind damage.
#define AnemometerWindSpeedProtection 28.8 // in km / h, = > 8 meters per second
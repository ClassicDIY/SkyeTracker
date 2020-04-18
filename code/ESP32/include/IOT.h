#pragma once

#include "WiFi.h"
#include "ArduinoJson.h"
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include "AsyncMqttClient.h"
#include "IotWebConf.h"
#include "Configuration.h"

#define STR_LEN 64                            // general string buffer size
#define CONFIG_LEN 32                         // configuration string buffer size

extern SkyeTracker::Configuration _config;

namespace SkyeTracker
{
    class IOT
    {
    public:
        IOT();
        void Init();
        void Run();
        void publish(const char *subtopic, const char *value, boolean retained = false);

    private:
        bool _clientsConfigured = false;
    };
} // namespace SkyeTracker
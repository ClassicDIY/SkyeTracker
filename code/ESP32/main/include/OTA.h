#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Log.h"
#include "Defines.h"

namespace CLASSICDIY {
class OTA {
 public:
   OTA() {};
   void begin(AsyncWebServer *asyncServer);

 private:
   AsyncWebServer *_pAsyncServer;
};

const char update_firmware_html[] PROGMEM = R"rawliteral(
        <!DOCTYPE html><html lang=\"en\"><head><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
        <title>{n}</title>
        </head><body>
            <div>
                <h2>{n}</h2>
                <div style='font-size: .6em;'>Firmware version '{v}'</div>
                <hr>
            </div>
            <form method='POST' action='/doupdate' enctype='multipart/form-data'>
            <input type='file' name='firmware'>
            <input type='submit' value='Update Firmware'>
            </form>
        </body></html>
        )rawliteral";
} // namespace CLASSICDIY
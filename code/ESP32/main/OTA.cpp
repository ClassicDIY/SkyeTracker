#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Update.h>
#include "Log.h"
#include "OTA.h"

namespace CLASSICDIY {

void OTA::begin(AsyncWebServer *asyncServer) {
   _pAsyncServer = asyncServer;
   _pAsyncServer->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
      String page = update_firmware_html;
      page.replace("{n}", TAG);
      page.replace("{v}", APP_VERSION);
      request->send(200, "text/html", page);
   });
   // Handle firmware upload
   asyncServer->on(
       "/doupdate", HTTP_POST,
       [](AsyncWebServerRequest *request) {
          bool updateSuccess = !Update.hasError();
          request->send(200, "text/plain", updateSuccess ? "Update successful" : "Update failed");
          if (updateSuccess) {
             ESP.restart();
          }
       },
       [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
          if (!index) {
             logd("Update: %s\n", filename.c_str());
             if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
                Update.printError(Serial);
             }
          }
          if (len) {
             if (Update.write(data, len) != len) {
                Update.printError(Serial);
             }
          }
          if (final) {
             if (Update.end(true)) {
                logd("Update Success: %u bytes\n", index + len);
             } else {
                Update.printError(Serial);
             }
          }
       });
}

} // namespace CLASSICDIY
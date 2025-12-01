#include "Log.h"
#include "WebLog.h"

static AsyncWebSocket _webSocket("/ws_log");

int weblog_log_printfv(const char *format, va_list arg) {
   static char loc_buf[LOG_BUFFER_SIZE];
   uint32_t len;
   va_list copy;
   va_copy(copy, arg);
   len = vsnprintf(NULL, 0, format, copy);
   va_end(copy);
   if (len >= (LOG_BUFFER_SIZE - 1)) {
      len = LOG_BUFFER_SIZE - 5; // truncate log msg
      vsnprintf(loc_buf, len, format, arg);
      strcat(loc_buf, "...\n");
   } else {
      vsnprintf(loc_buf, len + 1, format, arg);
   }
   if (_webSocket.count() > 0) {
      _webSocket.textAll(loc_buf);
   }
#ifdef LOG_TO_SERIAL_PORT
   ets_printf("%s\r", loc_buf);
#endif
   return len;
}

int weblog(const char *format, ...) {
   int len;
   va_list arg;
   va_start(arg, format);
   len = weblog_log_printfv(format, arg);
   va_end(arg);
   return len;
}

void WebLog::begin(AsyncWebServer *pwebServer) {
   pwebServer->addHandler(&_webSocket).addMiddleware([](AsyncWebServerRequest *request, ArMiddlewareNext next) {
      // ws.count() is the current count of WS clients: this one is trying to upgrade its HTTP connection
      if (_webSocket.count() > 1) {
         // if we have 2 clients or more, prevent the next one to connect
         request->send(503, "text/plain", "Server is busy");
      } else {
         // process next middleware and at the end the handler
         next();
      }
   });

   _webSocket.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
      (void)len;

      if (type == WS_EVT_CONNECT) {
         logd("new client connected");
         client->setCloseClientOnQueueFull(false);
         client->ping();

      } else if (type == WS_EVT_DISCONNECT) {
         logd("ws client disconnect");

      } else if (type == WS_EVT_ERROR) {
         loge("ws error");

      } else if (type == WS_EVT_PONG) {
         logd("ws pong");

      } else if (type == WS_EVT_DATA) {
         AwsFrameInfo *info = (AwsFrameInfo *)arg;
         logd("index: %" PRIu64 ", len: %" PRIu64 ", final: %" PRIu8 ", opcode: %" PRIu8 "\n", info->index, info->len, info->final,
              info->opcode);
         String msg = "";
         if (info->final && info->index == 0 && info->len == len) {
            if (info->opcode == WS_TEXT) {
               data[len] = 0;
               logd("ws text: %s\n", (char *)data);
            }
         }
      }
   });
   pwebServer->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/html", web_serial_html); });
}

void WebLog::end() { _webSocket.closeAll(); }

void WebLog::process() {
   uint32_t now = millis();
   if (now - _lastHeap >= 2000) {
      _lastHeap = now;
      // cleanup disconnected clients or too many clients
      _webSocket.cleanupClients();
   }
}

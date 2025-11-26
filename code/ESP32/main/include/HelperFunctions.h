#pragma once
#include <Arduino.h>
#include "hal/uart_types.h"
#include "Defines.h"

std::string inline formatDuration(unsigned long milliseconds) {
   const unsigned long MS_PER_SECOND = 1000;
   const unsigned long MS_PER_MINUTE = MS_PER_SECOND * 60;
   const unsigned long MS_PER_HOUR = MS_PER_MINUTE * 60;
   const unsigned long MS_PER_DAY = MS_PER_HOUR * 24;

   unsigned long days = milliseconds / MS_PER_DAY;
   milliseconds %= MS_PER_DAY;
   unsigned long hours = milliseconds / MS_PER_HOUR;
   milliseconds %= MS_PER_HOUR;
   unsigned long minutes = milliseconds / MS_PER_MINUTE;
   milliseconds %= MS_PER_MINUTE;
   unsigned long seconds = milliseconds / MS_PER_SECOND;

   std::string result = std::to_string(days) + " days, " + std::to_string(hours) + " hours, " + std::to_string(minutes) + " minutes, " +
                        std::to_string(seconds) + " seconds";

   return result;
}

time_t inline getTime() {
   time_t now;
   struct tm timeinfo;
   if (!getLocalTime(&timeinfo)) {
      return (0);
   }
   time(&now);
   return now;
}

template <typename T> String htmlConfigEntry(const char *label, T val) {
   String s = "<li>";
   s += label;
   s += ": ";
   s += val;
   s += "</li>";
   return s;
}

void inline light_sleep(uint32_t sec) {
   esp_sleep_enable_timer_wakeup(sec * 1000000ULL);
   esp_light_sleep_start();
}

SerialConfig inline getSerialConfig(uart_parity_t parity, uart_stop_bits_t stopBits) {
   uint32_t config = SERIAL_8N1;

   // Stop bits
   switch (stopBits) {
   case UART_STOP_BITS_1:
      config |= 0x00000000;
      break;
   case UART_STOP_BITS_2:
      config |= 0x00000020;
      break;
   default:
      config |= 0x00000020;
      break;
   }
   // Parity
   switch (parity) {
   case UART_PARITY_DISABLE:
      config |= 0x00000000;
      break;
   case UART_PARITY_EVEN:
      config |= 0x00000002;
      break;
   case UART_PARITY_ODD:
      config |= 0x00000003;
      break;
   }
   return static_cast<SerialConfig>(config);
}
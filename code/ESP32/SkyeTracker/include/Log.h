#pragma once

#include "esp_log.h"
#define TAG "SkyeTracker"


void inline printHexString(char* ptr, int len)
{
#if APP_LOG_LEVEL >= ESP_LOG_DEBUG
	esp_log_level_set(TAG, ESP_LOG_DEBUG);
	esp_log_buffer_hex_internal(TAG, ptr, len, ESP_LOG_DEBUG);
#endif
}

#if APP_LOG_LEVEL >= ESP_LOG_VERBOSE
#define logv(format, ...) log_printf(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define logv(format, ...)
#endif

#if APP_LOG_LEVEL >= ESP_LOG_DEBUG
#define logd(format, ...) log_printf(ARDUHAL_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define logd(format, ...)
#endif

#if APP_LOG_LEVEL >= ESP_LOG_INFO
#define logi(format, ...) log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define logi(format, ...)
#endif

#if APP_LOG_LEVEL >= ESP_LOG_WARN
#define logw(format, ...) log_printf(ARDUHAL_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define logw(format, ...)
#endif

#if APP_LOG_LEVEL >= ESP_LOG_ERROR
#define loge(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define loge(format, ...)
#endif

void inline printLocalTime()
{
#if APP_LOG_LEVEL >= ESP_LOG_DEBUG
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo))
	{
		logi("Failed to obtain time");
		return;
	}
	char buf[64];
	buf[0] = 0;
	strftime(buf, 64, "%A, %B %d %Y %H:%M:%S", &timeinfo);
	logi("Date Time: %s", buf);
#endif
}

#include "IOT.h"
#include <sys/time.h>
#include <EEPROM.h>
#include "time.h"
#include "Log.h"
#include "Tracker.h"
#include <IotWebConfESP32HTTPUpdateServer.h>

extern SkyeTracker::Tracker _tracker;

namespace SkyeTracker
{
AsyncMqttClient _mqttClient;
TimerHandle_t mqttReconnectTimer;
DNSServer _dnsServer;
WebServer _webServer(80);
// Create Update Server
HTTPUpdateServer _httpUpdater;
IotWebConf _iotWebConf(TAG, &_dnsServer, &_webServer, TAG, CONFIG_VERSION);
char _mqttRootTopic[STR_LEN];
char _willTopic[STR_LEN];
char _mqttServer[IOTWEBCONF_WORD_LEN];
char _mqttPort[5];
char _mqttUserName[IOTWEBCONF_WORD_LEN];
char _mqttUserPassword[IOTWEBCONF_WORD_LEN];
u_int32_t _uniqueId = ESP.getEfuseMac() & 0xFFFFFFFF;
iotwebconf::ParameterGroup MQTT_group = iotwebconf::ParameterGroup("MQTT", "MQTT");
iotwebconf::TextParameter mqttServerParam = iotwebconf::TextParameter("MQTT server", "mqttServer", _mqttServer, IOTWEBCONF_WORD_LEN);
iotwebconf::NumberParameter mqttPortParam = iotwebconf::NumberParameter("MQTT port", "mqttSPort", _mqttPort, NUMBER_CONFIG_LEN, "text", NULL, "1883");
iotwebconf::TextParameter mqttUserNameParam = iotwebconf::TextParameter("MQTT user", "mqttUser", _mqttUserName, IOTWEBCONF_WORD_LEN);
iotwebconf::PasswordParameter mqttUserPasswordParam = iotwebconf::PasswordParameter("MQTT password", "mqttPass", _mqttUserPassword, IOTWEBCONF_WORD_LEN, "password");
iotwebconf::TextParameter mqttRootTopicParam = iotwebconf::TextParameter("MQTT Root Topic", "mqttRootTopic", _mqttRootTopic, IOTWEBCONF_WORD_LEN);
const char *ntpServer = "pool.ntp.org";

void publishDiscovery()
{
	char buffer[STR_LEN];
	JsonDocument doc;
	doc["name"] = _iotWebConf.getThingName();
	sprintf(buffer, "%X", _uniqueId);
	doc["unique_id"] = buffer;
	doc["mode_cmd_t"] = "~/cmnd/MODE";
	doc["mode_stat_t"] = "~/stat/MODE";
	doc["avty_t"] = "~/tele/LWT";
	doc["pl_avail"] = "Online";
	doc["pl_not_avail"] = "Offline";
	JsonObject device = doc["device"].to<JsonObject>();
	device["name"] = "SkyeTracker";
	device["sw_version"] = CONFIG_VERSION;
	device["manufacturer"] = "SkyeTracker";
	sprintf(buffer, "ESP32-Bit (%X)", _uniqueId);
	device["model"] = buffer;
	sprintf(buffer, "%X_%s", _uniqueId, "SkyeTracker");
	device["identifiers"] = buffer;
	doc["~"] = _mqttRootTopic;
	String s;
	serializeJson(doc, s);
	char configurationTopic[64];
	sprintf(configurationTopic, "%s/solar/%X/config", HOME_ASSISTANT_PREFIX, _uniqueId);
	if (_mqttClient.publish(configurationTopic, 0, true, s.c_str(), s.length()) == 0)
	{
		loge("**** Configuration payload exceeds MAX MQTT Packet Size");
	}
}

void onMqttConnect(bool sessionPresent)
{
	logd("Connected to MQTT. Session present: %d", sessionPresent);
	char mqttCmndTopic[STR_LEN];
	sprintf(mqttCmndTopic, "%s/cmnd/Mode", _mqttRootTopic);
	uint16_t packetIdSub = _mqttClient.subscribe(mqttCmndTopic, 1);
	sprintf(mqttCmndTopic, "%s/cmnd/MoveTo", _mqttRootTopic);
	packetIdSub = _mqttClient.subscribe(mqttCmndTopic, 1);
	logd("MQTT subscribe, QoS 1, packetId: %d", packetIdSub);
	publishDiscovery();
	_mqttClient.publish(_willTopic, 0, false, "Online");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
	logw("Disconnected from MQTT. Reason: %d", (int8_t)reason);
}

void connectToMqtt()
{
	logd("Connecting to MQTT...");
	if (WiFi.isConnected())
	{
		_mqttClient.connect();
	}
}

void WiFiEvent(WiFiEvent_t event)
{
	logd("[WiFi-event] event: %d", event);
	String s;
	JsonDocument doc;
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
		// logd("WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());
		doc["IP"] = WiFi.localIP();
		doc["ApPassword"] = TAG;
		serializeJson(doc, s);
		s += '\n';
		Serial.printf(s.c_str()); // send json to flash tool
		configTime(0, 0, ntpServer);
		printLocalTime();
		xTimerStart(mqttReconnectTimer, 0);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		logw("WiFi lost connection");
		xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		break;
	default:
		break;
	}
}

void onMqttPublish(uint16_t packetId)
{
	logd("Publish acknowledged.  packetId: %d", packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
	logd("MQTT Message arrived [%s]  qos: %d len: %d index: %d total: %d", topic, properties.qos, len, index, total);
	printHexString(payload, len);
	int l = strlen(_mqttRootTopic) + 6;
	if (l < strlen(topic) && len < 8)
	{
		char *p = &topic[l];
		logd("p: %s", p);
		char cmd[16];
		char pl[16];
		if (strcmp("Mode", p) == 0)
		{
			strncpy(pl, payload, len);
			pl[len] = 0;
			sprintf(cmd, "%s|", pl);
			logd("cmd: %s", cmd);
			_tracker.ProcessCommand(cmd);
		}
		if (strcmp("MoveTo", p) == 0)
		{

			strncpy(pl, payload, len);
			pl[len] = 0;
			sprintf(cmd, "MoveTo|%s", pl);
			logd("cmd: %s", cmd);
			_tracker.ProcessCommand(cmd);
		}
	}
}

IOT::IOT()
{
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
	// -- Let IotWebConf test and handle captive portal requests.
	if (_iotWebConf.handleCaptivePortal())
	{
		logd("Captive portal");
		// -- Captive portal request were already served.
		return;
	}
	logd("handleRoot");
	String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
	s += "<title>";
	s += _iotWebConf.getThingName();
	s += "</title></head><body>";
	s += _iotWebConf.getThingName();
	s += "<ul>";
	s += "<li>MQTT server: ";
	s += _mqttServer;
	s += "</ul>";
	s += "<ul>";
	s += "<li>MQTT port: ";
	s += _mqttPort;
	s += "</ul>";
	s += "<ul>";
	s += "<li>MQTT user: ";
	s += _mqttUserName;
	s += "</ul>";
	s += "<ul>";
	s += "<li>MQTT root topic: ";
	s += _mqttRootTopic;
	s += "</ul>";
	s += "Go to <a href='config'>configure page</a> to change values.";
	s += "</body></html>\n";
	_webServer.send(200, "text/html", s);
}

void IOT::Init()
{
	pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
	_iotWebConf.setStatusPin(WIFI_STATUS_PIN);
	_iotWebConf.setConfigPin(WIFI_AP_PIN);
	if (digitalRead(FACTORY_RESET_PIN) == LOW)
	{
		EEPROM.begin(IOTWEBCONF_CONFIG_START + IOTWEBCONF_CONFIG_VERSION_LENGTH );
		for (byte t = 0; t < IOTWEBCONF_CONFIG_VERSION_LENGTH; t++)
		{
			EEPROM.write(IOTWEBCONF_CONFIG_START + t, 0);
		}
		EEPROM.commit();
		EEPROM.end();
		logw("Factory Reset!");
	}
	mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
	WiFi.onEvent(WiFiEvent);
	MQTT_group.addItem(&mqttServerParam);
	MQTT_group.addItem(&mqttPortParam);
	MQTT_group.addItem(&mqttUserNameParam);
	MQTT_group.addItem(&mqttUserPasswordParam);
	MQTT_group.addItem(&mqttRootTopicParam);
	_iotWebConf.addParameterGroup(&MQTT_group);
	boolean validConfig = _iotWebConf.init();
	if (!validConfig)
	{
		logw("!invalid configuration!");
		_mqttServer[0] = '\0';
		_mqttPort[0] = '\0';
		_mqttUserName[0] = '\0';
		_mqttUserPassword[0] = '\0';
		strcpy(_mqttRootTopic, _iotWebConf.getThingName());
		_iotWebConf.resetWifiAuthInfo();
	}
	else
	{
		_iotWebConf.skipApStartup(); // Set WIFI_AP_PIN to gnd to force AP mode
		if (_mqttServer[0] != '\0') // skip if factory reset
		{
			logd("Valid configuration!");
			_clientsConfigured = true;
			// setup MQTT
			_mqttClient.onConnect(onMqttConnect);
			_mqttClient.onDisconnect(onMqttDisconnect);
			_mqttClient.onMessage(onMqttMessage);
			_mqttClient.onPublish(onMqttPublish);
			IPAddress ip;
			if (ip.fromString(_mqttServer))
			{
				int port = atoi(_mqttPort);
				_mqttClient.setServer(ip, port);
				_mqttClient.setCredentials(_mqttUserName, _mqttUserPassword);
				sprintf(_willTopic, "%s/tele/LWT", _mqttRootTopic);
				_mqttClient.setWill(_willTopic, 0, false, "Offline");
			}
		}
	}
	// Set up required URL handlers on the web server.
	_webServer.on("/", handleRoot);
	_webServer.on("/config", [] { _iotWebConf.handleConfig(); });
	_webServer.onNotFound([]() { _iotWebConf.handleNotFound(); });
}

void IOT::Run()
{
	_iotWebConf.doLoop();
	if (_clientsConfigured && WiFi.isConnected())
	{
		// ToDo MQTT monitoring
		// if (_mqttClient.connected())
		// {
		// 	if (_lastPublishTimeStamp < millis())
		// 	{
		// 		_lastPublishTimeStamp = millis() + _currentPublishRate;
		// 		_publishCount++;
		// 		publishReadings();
		// 		// Serial.printf("%d ", _publishCount);
		// 	}
		// 	if (!_stayAwake && _publishCount >= WAKE_COUNT)
		// 	{
		// 		_publishCount = 0;
		// 		_currentPublishRate = SNOOZE_PUBLISH_RATE;
		// 		logd("Snoozing!");
		// 	}
		// }
	}
	else
	{
		if (Serial.peek() == '{')
		{
			String s = Serial.readStringUntil('}');
			s += "}";
			JsonDocument doc;
			DeserializationError err = deserializeJson(doc, s);
			if (err)
			{
				loge("deserializeJson() failed: %s", err.c_str());
			}
			else
			{
				if (doc.containsKey("ssid") && doc.containsKey("password"))
				{
					iotwebconf::Parameter *p = _iotWebConf.getWifiSsidParameter();
					strcpy(p->valueBuffer, doc["ssid"]);
					logd("Setting ssid: %s", p->valueBuffer);
					p = _iotWebConf.getWifiPasswordParameter();
					strcpy(p->valueBuffer, doc["password"]);
					logd("Setting password: %s", p->valueBuffer);
					p = _iotWebConf.getApPasswordParameter();
					strcpy(p->valueBuffer, TAG); // reset to default AP password
					_iotWebConf.saveConfig();
					esp_restart(); // force reboot
				}
				else
				{
					logw("Received invalid json: %s", s.c_str());
				}
			}
		}
		else
		{
			Serial.read(); // discard data
		}
	}
}

void IOT::publish(const char *subtopic, const char *value, boolean retained)
{
	if (_mqttClient.connected())
	{
		char buf[64];
		sprintf(buf, "%s/stat/%s", _mqttRootTopic, subtopic);
		_mqttClient.publish(buf, 0, retained, value);
	}
}

} // namespace SkyeTracker
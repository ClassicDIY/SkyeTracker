#include "IOT.h"
#include <sys/time.h>
#include "time.h"
#include "Log.h"

namespace SkyeTracker
{
AsyncMqttClient _mqttClient;
TimerHandle_t mqttReconnectTimer;
DNSServer _dnsServer;
WebServer _webServer(80);
HTTPUpdateServer _httpUpdater;
IotWebConf _iotWebConf(TAG, &_dnsServer, &_webServer, TAG, CONFIG_VERSION);
char _mqttRootTopic[STR_LEN];
char _willTopic[STR_LEN];
char _mqttServer[IOTWEBCONF_WORD_LEN];
char _mqttPort[5];
char _mqttUserName[IOTWEBCONF_WORD_LEN];
char _mqttUserPassword[IOTWEBCONF_WORD_LEN];
u_int _uniqueId = 0;
IotWebConfSeparator seperatorParam = IotWebConfSeparator("MQTT");
IotWebConfParameter mqttServerParam = IotWebConfParameter("MQTT server", "mqttServer", _mqttServer, IOTWEBCONF_WORD_LEN);
IotWebConfParameter mqttPortParam = IotWebConfParameter("MQTT port", "mqttSPort", _mqttPort, 5, "text", NULL, "8080");
IotWebConfParameter mqttUserNameParam = IotWebConfParameter("MQTT user", "mqttUser", _mqttUserName, IOTWEBCONF_WORD_LEN);
IotWebConfParameter mqttUserPasswordParam = IotWebConfParameter("MQTT password", "mqttPass", _mqttUserPassword, IOTWEBCONF_WORD_LEN, "password");

const char *ntpServer = "pool.ntp.org";

void publishDiscovery()
{
	char buffer[STR_LEN];
	StaticJsonDocument<1024> doc; // MQTT discovery
	doc["name"] = _iotWebConf.getThingName();
	sprintf(buffer, "%X", _uniqueId);
	doc["unique_id"] = buffer;
	doc["mode_cmd_t"] = "~/cmnd/MODE";
	doc["mode_stat_t"] = "~/stat/MODE";
	doc["avty_t"] = "~/tele/LWT";
	doc["pl_avail"] = "Online";
	doc["pl_not_avail"] = "Offline";
	JsonObject device = doc.createNestedObject("device");
	device["name"] = "SkyeTracker";
	device["sw_version"] = CONFIG_VERSION;
	device["manufacturer"] = "SkyeTracker";
	sprintf(buffer, "ESP32-Bit (%X)", _uniqueId);
	device["model"] = buffer;
	JsonArray identifiers = device.createNestedArray("identifiers");
	identifiers.add(_uniqueId);
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
	logi("Connected to MQTT. Session present: %d", sessionPresent);
	char mqttModeCmndTopic[STR_LEN];
	sprintf(mqttModeCmndTopic, "%s/cmnd/MODE", _mqttRootTopic);
	uint16_t packetIdSub = _mqttClient.subscribe(mqttModeCmndTopic, 1);
	logi("MQTT subscribe, QoS 1, packetId: %d", packetIdSub);
	publishDiscovery();
	_mqttClient.publish(_willTopic, 0, false, "Online");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
	logi("Disconnected from MQTT. Reason: %d", (int8_t)reason);
}

void connectToMqtt()
{
	logi("Connecting to MQTT...");
	if (WiFi.isConnected())
	{
		_mqttClient.connect();
	}
}

void WiFiEvent(WiFiEvent_t event)
{
	logi("[WiFi-event] event: %d", event);
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
		logi("WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());
		configTime(0, 0, ntpServer);
		printLocalTime();
		xTimerStart(mqttReconnectTimer, 0);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		logi("WiFi lost connection");
		xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
		break;
	default:
		break;
	}
}

void onMqttPublish(uint16_t packetId)
{
	logi("Publish acknowledged.  packetId: %d", packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
	logi("MQTT Message arrived [%s]  qos: %d len: %d", topic, properties.qos, len);
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
		logi("Captive portal");
		// -- Captive portal request were already served.
		return;
	}
	logi("handleRoot");
	String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
	s += "<title>SkyeTracker</title></head><body>";
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
	mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
	WiFi.onEvent(WiFiEvent);
	_iotWebConf.setupUpdateServer(&_httpUpdater);
	_iotWebConf.addParameter(&seperatorParam);
	_iotWebConf.addParameter(&mqttServerParam);
	_iotWebConf.addParameter(&mqttPortParam);
	_iotWebConf.addParameter(&mqttUserNameParam);
	_iotWebConf.addParameter(&mqttUserPasswordParam);
	boolean validConfig = _iotWebConf.init();
	if (!validConfig)
	{
		loge("!invalid configuration!");
		_mqttServer[0] = '\0';
		_mqttPort[0] = '\0';
		_mqttUserName[0] = '\0';
		_mqttUserPassword[0] = '\0';
	}
	else
	{
		_iotWebConf.setApTimeoutMs(AP_TIMEOUT);
	}
	// Set up required URL handlers on the web server.
	_webServer.on("/", handleRoot);
	_webServer.on("/config", [] { _iotWebConf.handleConfig(); });
	_webServer.onNotFound([]() { _iotWebConf.handleNotFound(); });
	_mqttClient.onConnect(onMqttConnect);
	_mqttClient.onDisconnect(onMqttDisconnect);
	_mqttClient.onMessage(onMqttMessage);
	_mqttClient.onPublish(onMqttPublish);
	// generate unique id from mac address NIC segment
	uint8_t chipid[6];
	esp_efuse_mac_get_default(chipid);
	_uniqueId = chipid[3] << 16;
	_uniqueId += chipid[4] << 8;
	_uniqueId += chipid[5];
	sprintf(_mqttRootTopic, "%s/%X/solar", _iotWebConf.getThingName(), _uniqueId);
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

void IOT::Run()
{
	_iotWebConf.doLoop();
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
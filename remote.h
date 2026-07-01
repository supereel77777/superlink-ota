#ifndef REMOTE_H
#define REMOTE_H

#include <WiFi.h>

extern PubSubClient client;
extern const char* currentStateText;
extern const char* client_id;
extern const char* mqtt_status_topic;
extern const char* mqtt_cmd_topic;
extern String deviceID;
static unsigned long lastStatusTime = 0;

void sendStatus()
{
    if (millis() - lastStatusTime < 5000)
        return;

    lastStatusTime = millis();


    char msg[512];

  snprintf(msg, sizeof(msg),
    "{"
    "\"device\":\"%s\","
    "\"fw\":\"%s\","
    "\"ip\":\"%s\","
    "\"ssid\":\"%s\","
    "\"state\":\"%s\","
    "\"rssi\":%d,"
    "\"uptime\":%lu,"
    "\"heap\":%u"
    "}",
    deviceID.c_str(),
    FW_VERSION,
    WiFi.localIP().toString().c_str(),
    WiFi.SSID().c_str(),
    currentStateText,
    WiFi.RSSI(),
    millis() / 1000,
    ESP.getFreeHeap()
);

    if (client.connected())
    {
        client.publish(mqtt_status_topic, msg);
    }

    Serial.print("STATUS : ");
    Serial.println(msg);
}

void checkRemoteCommand()
{
    // Reserved
}

#endif
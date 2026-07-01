#ifndef REMOTE_OTA_H
#define REMOTE_OTA_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "version.h"

#define OTA_VERSION_URL  "https://raw.githubusercontent.com/supereel77777/superlink-ota/main/version.txt"
#define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/supereel77777/superlink-ota/main/firmware.bin"

int versionToInt(String ver)
{
    int major = 0;
    int minor = 0;
    int patch = 0;

    sscanf(ver.c_str(), "%d.%d.%d",
           &major,
           &minor,
           &patch);

    return major * 10000 +
           minor * 100 +
           patch;
}

bool checkRemoteVersion()
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    if (!http.begin(client, OTA_VERSION_URL))
    {
        Serial.println("Version URL Open Fail");
        return false;
    }

    int code = http.GET();

    if (code != HTTP_CODE_OK)
    {
        Serial.printf("HTTP Error : %d\n", code);
        http.end();
        return false;
    }

    String remoteVersion = http.getString();
    remoteVersion.trim();

    Serial.println("----------------------");
    Serial.print("Current : ");
    Serial.println(FW_VERSION);

    Serial.print("Remote  : ");
    Serial.println(remoteVersion);

    int current = versionToInt(FW_VERSION);
    int remote  = versionToInt(remoteVersion);

    http.end();

    if(remote > current)
    {
        Serial.println("New Version Found");
        return true;
    }

    Serial.println("Latest Version");
    return false;
}

bool startRemoteOTA()
{
    WiFiClientSecure client;
    client.setInsecure();

    Serial.println("Starting Remote OTA...");

    t_httpUpdate_return result =
        httpUpdate.update(client, OTA_FIRMWARE_URL);

    switch(result)
    {
        case HTTP_UPDATE_FAILED:
            Serial.printf("OTA Failed (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No Update");
            return false;

        case HTTP_UPDATE_OK:
            Serial.println("OTA Success");
            return true;
    }

    return false;
}

#endif
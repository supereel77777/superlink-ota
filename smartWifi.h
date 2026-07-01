#ifndef SMART_WIFI_H
#define SMART_WIFI_H

#include <WiFi.h>
#include <Preferences.h>

#define MAX_WIFI 100

struct WifiItem
{
    String ssid;
    String password;
    String bssid;
};

Preferences smartPrefs;

WifiItem wifiList[MAX_WIFI];
int wifiCount = 0;

// ===== 내부 유틸 =====
bool loadWifiList()
{
    smartPrefs.begin("smartwifi", true);

    wifiCount = smartPrefs.getInt("count", 0);
    if (wifiCount > MAX_WIFI)
        wifiCount = MAX_WIFI;

    for (int i = 0; i < wifiCount; i++)
    {
        String base = "w" + String(i);

        wifiList[i].ssid = smartPrefs.getString((base + "_s").c_str(), "");
        wifiList[i].password = smartPrefs.getString((base + "_p").c_str(), "");
        wifiList[i].bssid = smartPrefs.getString((base + "_b").c_str(), "");
    }

    smartPrefs.end();
    return true;
}

bool saveWifiList()
{
    smartPrefs.begin("smartwifi", false);

    smartPrefs.putInt("count", wifiCount);

    for (int i = 0; i < wifiCount; i++)
    {
        String base = "w" + String(i);

        smartPrefs.putString((base + "_s").c_str(), wifiList[i].ssid);
        smartPrefs.putString((base + "_p").c_str(), wifiList[i].password);
        smartPrefs.putString((base + "_b").c_str(), wifiList[i].bssid);
    }

    smartPrefs.end();
    return true;
}

// ===== AP Scan =====
int scanMatchedIndex()
{
    int n = WiFi.scanNetworks();

    if (n <= 0)
        return -1;

    for (int i = 0; i < n; i++)
    {
        String scanBssid = WiFi.BSSIDstr(i);

        for (int j = 0; j < wifiCount; j++)
        {
            if (wifiList[j].bssid == scanBssid)
            {
                return j;
            }
        }
    }

    return -1;
}

// ===== 연결 =====
bool connectWiFi(int index)
{
    if (index < 0 || index >= wifiCount)
        return false;

    WiFi.begin(wifiList[index].ssid.c_str(),
               wifiList[index].password.c_str());

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);

        if (millis() - start > 10000)
        {
            return false;
        }
    }

    return true;
}
#include <WiFiManager.h>

// ===== 신규 저장 =====
bool addWifi(String ssid, String password, String bssid)
{
    // 이미 존재하면 업데이트
    for (int i = 0; i < wifiCount; i++)
    {
        if (wifiList[i].bssid == bssid)
        {
            wifiList[i].ssid = ssid;
            wifiList[i].password = password;
            saveWifiList();
            return true;
        }
    }

    // 새로 추가
    if (wifiCount < MAX_WIFI)
    {
        wifiList[wifiCount].ssid = ssid;
        wifiList[wifiCount].password = password;
        wifiList[wifiCount].bssid = bssid;

        wifiCount++;

        saveWifiList();
        return true;
    }

    // 꽉 찼으면 FIFO (0 삭제)
    for (int i = 1; i < MAX_WIFI; i++)
    {
        wifiList[i - 1] = wifiList[i];
    }

    wifiList[MAX_WIFI - 1].ssid = ssid;
    wifiList[MAX_WIFI - 1].password = password;
    wifiList[MAX_WIFI - 1].bssid = bssid;

    saveWifiList();
    return true;
}

// ===== WiFiManager fallback =====
bool runWiFiManager(String fallbackName)
{
    WiFiManager wm;

    bool res = wm.autoConnect(fallbackName.c_str());

    if (!res)
        return false;

    return true;
}

// ===== V2.3 → V2.4 마이그레이션 =====
void migrateLegacyWiFi()
{
    Preferences p;
    p.begin("config", true);

    String oldSSID = p.getString("ssid", "");
    String oldPASS = p.getString("pass", "");

    p.end();

    if (oldSSID.length() == 0)
        return;

    String bssid = WiFi.BSSIDstr();

    if (bssid.length() == 0)
        return;

    addWifi(oldSSID, oldPASS, bssid);

    // 삭제 (무시 처리)
    p.begin("config", false);
    p.remove("ssid");
    p.remove("pass");
    p.end();
}

// ===== 핵심 실행 함수 =====
bool startSmartWiFi(String fallbackName)
{
    loadWifiList();

    int index = scanMatchedIndex();

    if (index >= 0)
    {
        if (connectWiFi(index))
        {
            return true;
        }
    }

    // fallback WiFiManager
    if (!runWiFiManager(fallbackName))
    {
        return false;
    }

    // 연결 후 정보 저장
    String ssid = WiFi.SSID();
    String pass = ""; // WiFiManager 내부 저장값 사용 안하면 빈값 처리
    String bssid = WiFi.BSSIDstr();

    addWifi(ssid, pass, bssid);

    migrateLegacyWiFi();

    return true;
}

#endif

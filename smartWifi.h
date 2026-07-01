#pragma once
#include <WiFi.h>
#include <Preferences.h>
#include <WiFiManager.h>

#define MAX_WIFI_PROFILES 10
#define WIFI_CONNECT_TIMEOUT_MS 6000 // 한 와이파이당 최대 대기 시간 (6초)

struct WifiProfile {
    char ssid[33];
    char password[65];
};

// 1. NVS에서 저장된 와이파이 리스트를 불러오는 함수
int loadWifiProfiles(WifiProfile* profiles) {
    Preferences wifiPrefs;
    wifiPrefs.begin("wifi_list", true); // 읽기 전용 모드
    
    int count = 0;
    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        String keySsid = "s" + String(i);
        String keyPw = "p" + String(i);
        
        if (wifiPrefs.isKey(keySsid.c_str())) {
            String s = wifiPrefs.getString(keySsid.c_str(), "");
            String p = wifiPrefs.getString(keyPw.c_str(), "");
            if (s.length() > 0) {
                strncpy(profiles[count].ssid, s.c_str(), sizeof(profiles[count].ssid));
                strncpy(profiles[count].password, p.c_str(), sizeof(profiles[count].password));
                count++;
            }
        }
    }
    wifiPrefs.end();
    return count;
}

// 2. 새 와이파이를 NVS에 중복 체크 후 추가 저장하는 함수
void saveWifiProfile(const char* ssid, const char* password) {
    if (strlen(ssid) == 0) return;

    Preferences wifiPrefs;
    wifiPrefs.begin("wifi_list", false); // 읽기/쓰기 모드

    WifiProfile tempProfiles[MAX_WIFI_PROFILES];
    int count = 0;
    bool alreadyExists = false;

    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        String keySsid = "s" + String(i);
        if (wifiPrefs.isKey(keySsid.c_str())) {
            String s = wifiPrefs.getString(keySsid.c_str(), "");
            if (s == String(ssid)) {
                alreadyExists = true;
                String keyPw = "p" + String(i);
                wifiPrefs.putString(keyPw.c_str(), password); // 비밀번호만 갱신
                Serial.println("[NVS] 기존 와이파이 비밀번호 업데이트 완료.");
                break;
            }
            count++;
        }
    }

    if (!alreadyExists) {
        int targetIndex = (count < MAX_WIFI_PROFILES) ? count : (MAX_WIFI_PROFILES - 1); 
        String keySsid = "s" + String(targetIndex);
        String keyPw = "p" + String(targetIndex);
        
        wifiPrefs.putString(keySsid.c_str(), ssid);
        wifiPrefs.putString(keyPw.c_str(), password);
        Serial.printf("[NVS] 새 와이파이 저장 완료 (슬롯 %d): %s\n", targetIndex, ssid);
    }
    wifiPrefs.end();
}

// 3. 메인 와이파이 제어 함수 (goto 에러 해결 버전)
bool startSmartWiFi(const char* apName, bool forceApMode = false) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    bool connected = false;

    // 💡 변경 포인트: forceApMode가 false일 때만 주변 스캔 및 NVS 검색을 안전하게 수행
    if (!forceApMode) {
        WifiProfile storedProfiles[MAX_WIFI_PROFILES];
        int storedCount = loadWifiProfiles(storedProfiles);
        Serial.printf("[WiFi] NVS에서 로드된 와이파이 개수: %d개\n", storedCount);

        if (storedCount > 0) {
            Serial.println("[WiFi] 주변 와이파이 스캔 중...");
            int n = WiFi.scanNetworks();
            Serial.printf("[WiFi] 스캔 완료. 주변에 %d개의 네트워크 감지됨.\n", n);

            if (n > 0) {
                // RSSI(신호 세기)가 높은 순서대로 차례로 순회
                for (int i = 0; i < n; ++i) {
                    String scannedSSID = WiFi.SSID(i);
                    int32_t rssi = WiFi.RSSI(i);
                    
                    for (int j = 0; j < storedCount; j++) {
                        if (scannedSSID == String(storedProfiles[j].ssid)) {
                            Serial.printf("\n[매칭 성공] %s (%d dBm) 접속 시도 중...", storedProfiles[j].ssid, rssi);
                            
                            WiFi.begin(storedProfiles[j].ssid, storedProfiles[j].password);
                            
                            unsigned long startAttemptTime = millis();
                            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONNECT_TIMEOUT_MS) {
                                delay(200);
                                Serial.print(".");
                            }

                            if (WiFi.status() == WL_CONNECTED) {
                                Serial.println("\n🎉 와이파이 최종 접속 성공!");
                                connected = true;
                                break;
                            } else {
                                Serial.println("\n❌ 접속 실패(인터넷 먹통/오류). 다음 와이파이 검사로 넘어갑니다.");
                                WiFi.disconnect();
                                delay(100);
                            }
                        }
                    }
                    if (connected) break; // 루프 탈출
                }
            }
            WiFi.scanDelete();
        }
    } else {
        Serial.println("\n[WiFi] 강제 설정 모드 발동! 순회 접속을 스킵합니다.");
    }

    // NVS로 접속 성공했다면 바로 true 반환하여 셋업창을 켜지 않음
    if (connected) {
        return true;
    }

    // 접속된 적이 없거나, 다 실패했거나, forceApMode가 true일 때 셋업 웹창 진입
    Serial.println("\n⚠️ 접속 가능한 와이파이가 없으므로 WiFiManager 설정창을 활성화합니다.");
    
    WiFiManager wm;
    wm.setConfigPortalTimeout(180); // 3분 타임아웃 안전장치
    
    if (!wm.startConfigPortal(apName)) {
        Serial.println("[WiFi] 설정창 타임아웃 종료.");
        return false;
    }

    // 새 장소에서 접속 성공 시 기존 10개 리스트를 유지한 채 새 프로필만 추가 저장
    Serial.println("[WiFi] 웹 설정창을 통한 새 와이파이 접속 성공!");
    saveWifiProfile(WiFi.SSID().c_str(), WiFi.psk().c_str());
    
    return true;
}
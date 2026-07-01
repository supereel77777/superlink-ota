#include "remoteOTA.h"
#include "smartWifi.h"
#include "version.h"
#include <Preferences.h>

#define DEVICE 'C'
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include "sound.h" 
#include "remote.h"
#include <DFRobotDFPlayerMini.h>

Preferences prefs;
String deviceID = "";
extern String deviceID;

#define LED_PIN 33
#define TOUCH1_PIN 32   // 측면 버튼: 색상 변경 및 밝기
#define TOUCH2_PIN 27   // 하트 버튼: 호출/응답

// 수정이 필요한 경우: 터치 감도 기준 (안 만졌을 때 값보다 낮아야 함)
// 손을 안 댔는데 리셋 루프가 돈다면 이 값을 30 정도로 낮추세요.
#define TOUCH_THRESHOLD 50 

#define NUMPIXELS 16
#define DF_RX 25   // DFPlayer TX
#define DF_TX 26   // DFPlayer RX

// ===== MQTT 설정 =====
const char* mqtt_server = "9e96c25cb00f48148a6340b5f2d59065.s1.eu.hivemq.cloud";
const char* mqtt_topic = "luvlink_jg_240623/call";
const char* mqtt_status_topic = "luvlink_jg_240623/status";
const char* mqtt_cmd_topic;

const char* client_id;
const char* myCall;
const char* myAck;
const char* wifiName;

uint8_t myR;
uint8_t myG;
uint8_t myB;

void loadDeviceConfig() {
    if(deviceID == "A") {
        client_id = "LuvLink_A_JG_240623";
        mqtt_cmd_topic = "luvlink_jg_240623/cmd/A";
        myCall = "CALL_A";
        myAck  = "ACK_A";
        wifiName = "LuvLink_A_Setup";
        myR = 255; myG = 0; myB = 0;
    }
    else if(deviceID == "B") {
        client_id = "LuvLink_B_JG_240623";
        mqtt_cmd_topic = "luvlink_jg_240623/cmd/B";
        myCall = "CALL_B";
        myAck  = "ACK_B";
        wifiName = "LuvLink_B_Setup";
        myR = 0; myG = 0; myB = 255;
    }
    else {
        client_id = "LuvLink_C_JG_240623";
        mqtt_cmd_topic = "luvlink_jg_240623/cmd/C";
        myCall = "CALL_C";
        myAck  = "ACK_C";
        wifiName = "LuvLink_C_Setup";
        myR = 0; myG = 255; myB = 0;
    }
}

HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;
WiFiClientSecure espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum LampState { IDLE, CALLING, RINGING, CONNECTED };
LampState currentState = IDLE;
const char* currentStateText = "IDLE";

// 상태 변수
bool waitingAck = false;
bool incomingCall = false;
unsigned long lastCallTime = 0;

// 효과 플래그
bool needCallEffect = false;
bool needConnectEffect = false;

// 버튼 상태 및 디바운스
bool lastTouch1 = false;
bool lastTouch2 = false;
unsigned long lastTouch1Time = 0;
unsigned long lastTouch2Time = 0;

// 색상 및 밝기 모드
int mode = 0;
int brightnessLevel = 5;
int brightnessDirection = 1;
bool touch1Pressed = false;
unsigned long touch1StartTime = 0;
unsigned long lastBrightnessTime = 0;

// WiFi 초기화용
bool resetPressed = false;
unsigned long resetStartTime = 0;

// 현재 색 저장
uint8_t saveR = 255; uint8_t saveG = 0; uint8_t saveB = 0;

// 색상 통합 및 로그용
uint32_t callerColor = 0;
uint32_t responderColor = 0;
String lastCaller = "";
String lastResponder = "";

bool effectRunning = false;
bool mqttOfflineEffect = true;

bool touchDetected(int pin);

void setColor(uint8_t r, uint8_t g, uint8_t b) {
    saveR = r; saveG = g; saveB = b;
    for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(r, g, b));
    }
    pixels.show();
}

void serviceTasks() {
    client.loop();
    ArduinoOTA.handle();

    // RINGING 상태에서 터치2(하트)가 감지되면 즉시 응답(ACK) 전송
    if (touchDetected(TOUCH2_PIN) && currentState == RINGING && (millis() - lastTouch2Time > 500)) {
        lastTouch2Time = millis();
        Serial.println("ACK OK (from serviceTasks)");
        client.publish(mqtt_topic, myAck);

        currentState = CONNECTED;
        currentStateText = "CONNECTED";
        incomingCall = false;
        waitingAck = false;
        needCallEffect = false;
        
        playConnected();  
        needConnectEffect = true;
    }
}

void smartDelay(unsigned long ms) {
    unsigned long startTime = millis();
    while (millis() - startTime < ms) {
        serviceTasks();
        delay(1);
    }
}

#include "effects.h"

void playEffect() {
    effectRunning = true;

    flashEffect();          if (needConnectEffect) { return; }
    heartEffect();          if (needConnectEffect) { return; }
    heartEffect();          if (needConnectEffect) { return; }
    connectBurstEffect();   if (needConnectEffect) { return; }
    rainbowBreathing();

    setColor(saveR, saveG, saveB);
    effectRunning = false;
}

void connectedEffect() {
    effectRunning = true;
    connectSuperFlash();
    connectWhiteColorFlash();
    twinkleStarEffect();

    myDFPlayer.stop();
    setColor(saveR, saveG, saveB);
    effectRunning = false;
}

void callback(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    // ===== 원격 명령 처리 =====
    if (String(topic) == mqtt_cmd_topic) {
        Serial.print("CMD : "); Serial.println(msg);
        if (msg == "REBOOT") {
            Serial.println("Remote Reboot");
            delay(500);
            ESP.restart();
        }
        else if (msg == "LEDTEST") {
            Serial.println("Remote LED TEST");
            connectSuperFlash();
            connectWhiteColorFlash();
            twinkleStarEffect();
            setColor(saveR, saveG, saveB);
        }
        else if(msg == "CALLTEST") {
            Serial.println("Remote CALL TEST");
            currentState = RINGING;
            currentStateText = "RINGING";
            incomingCall = true;
            callerColor = pixels.Color(255,0,0);
            playMomCall();
            needCallEffect = true;
        }
        else if(msg == "PLAYTEST") {
            Serial.println("Remote PLAY TEST");
            playConnected();
        }
        else if(msg == "OTA") {
            Serial.println("Remote OTA");
            if(checkRemoteVersion()) { startRemoteOTA(); }
            else { Serial.println("Already Latest"); }
        }
        return;
    }
    
    Serial.print("re msg : "); Serial.println(msg);

    // ===== CALL 수신 =====
    if (msg.startsWith("CALL_") && msg != myCall && (currentState == IDLE || currentState == RINGING)) {
        if (currentState == IDLE) {
            incomingCall = true;
            currentState = RINGING;
            currentStateText = "RINGING";
            lastCaller = msg;
        }

        if (msg == "CALL_A")      { playMomCall(); callerColor = pixels.Color(255,0,0); }
        else if (msg == "CALL_B") { myDFPlayer.playLargeFolder(4,1); callerColor = pixels.Color(0,0,255); }
        else if (msg == "CALL_C") { myDFPlayer.playLargeFolder(4,2); callerColor = pixels.Color(0,255,0); }

        Serial.print("Caller : "); Serial.println(msg);
        needCallEffect = true;
    }

    // ===== ACK 수신 =====
    if (msg.startsWith("ACK_") && msg != myAck) {
        waitingAck = false;
        currentState = CONNECTED;
        currentStateText = "CONNECTED";
        lastResponder = msg;

        if (msg == "ACK_A")      { responderColor = pixels.Color(255,0,0); }
        else if (msg == "ACK_B") { responderColor = pixels.Color(0,0,255); }
        else if (msg == "ACK_C") { responderColor = pixels.Color(0,255,0); }
        
        Serial.print("Responder : "); Serial.println(msg);
        needCallEffect = false;
        needConnectEffect = true;
        playConnected();
    }
}

unsigned long lastReconnect = 0;
void reconnect() {
    if (client.connected()) return;
    if (millis() - lastReconnect < 5000) return;
    lastReconnect = millis();

    Serial.println("MQTT Access...");
    if (client.connect(client_id,"supereel","Wjs1534!")) {
        Serial.println("MQTT OK");
        mqttOfflineEffect = false;
        client.subscribe(mqtt_topic);
        client.subscribe(mqtt_cmd_topic);
        myDFPlayer.playLargeFolder(3,1);
    } else {
        mqttOfflineEffect = true;
        Serial.print("MQTT Fail = "); Serial.println(client.state());
    }
}

bool touchDetected(int pin) {
    for(int i = 0; i < 5; i++) {
        if(touchRead(pin) > TOUCH_THRESHOLD) { return false; }
        delay(5);
    }
    return true;
}

void loadDeviceID() {
    prefs.begin("config", false);
    deviceID = prefs.getString("device", "");
    if (deviceID == "") {
#if DEVICE == 'A'
        deviceID = "A";
#elif DEVICE == 'B'
        deviceID = "B";
#elif DEVICE == 'C'
        deviceID = "C";
#endif
        prefs.putString("device", deviceID);
    }
    prefs.end();
}

void setup() {
    Serial.begin(115200);
    loadDeviceID();
    loadDeviceConfig();
    Serial.print("Saved Device = "); Serial.println(deviceID);

    pixels.begin();
    pixels.setBrightness(50);
    setColor(myR, myG, myB);

    // ===== 부팅 후 5초간 WiFi 초기화 의도 체크 =====
    Serial.println("5sec WiFi reset check...");
    unsigned long startTime = millis();
    bool triggerReset = false;

    while (millis() - startTime < 5000) {
        if (touchDetected(TOUCH1_PIN) && touchDetected(TOUCH2_PIN)) {
            Serial.println("Reset condition detected! Hold for 2 seconds...");
            delay(2000); 
            if (touchDetected(TOUCH1_PIN) && touchDetected(TOUCH2_PIN)) {
                triggerReset = true;
                break; 
            }
        }
        delay(20);
    }

    if (triggerReset) {
        Serial.println("Clearing stored WiFi settings...");
        WiFiManager wm;
        wm.resetSettings(); // 기존 메모리 클리어 후 아래에서 바로 Setup 창을 켜도록 연계 (리부팅 제거)
    }

    mySerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
    if (myDFPlayer.begin(mySerial)) {
        Serial.println("DFPlayer OK");
        myDFPlayer.volume(30);
    } else {
        Serial.println("DFPlayer Fail");
    }

    Serial.println("Smart WiFi Start");
    if (startSmartWiFi(wifiName)) {
        Serial.println("WiFi OK");
        delay(2000);
        Serial.print("IP Ad : "); Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi Fail - stop boot");
        while(true) delay(1000);
    }

    ArduinoOTA.setHostname(client_id);
    ArduinoOTA.begin();
    espClient.setInsecure();
    Serial.println("OTA Ready");

    client.setServer(mqtt_server, 8883);
    client.setKeepAlive(60);
    client.setSocketTimeout(30);
    client.setCallback(callback);
    reconnect();

    currentState = IDLE;
    currentStateText = "IDLE";
}

void loop() {
    ArduinoOTA.handle();
    client.loop();

    // ===== 네트워크 상태 지속 확인 =====
    if (WiFi.status() != WL_CONNECTED || !client.connected()) {
        mqttOfflineEffect = true;
        reconnect();
    } else {
        mqttOfflineEffect = false;
    }

    if (mqttOfflineEffect) {
        mqttOfflineEffectFlash();
        return;
    }

    bool touch1 = touchDetected(TOUCH1_PIN);
    bool touch2 = touchDetected(TOUCH2_PIN);

    // ===== 작동 중 양쪽 터치 5초 지속 시 WiFi 공장초기화 =====
    if (touch1 && touch2) {
        if (!resetPressed) {
            resetPressed = true;
            resetStartTime = millis();
        }
        if (millis() - resetStartTime > 5000) {
            Serial.println("WiFi reset triggered via Loop.");
            for (int i = 0; i < 3; i++) {
                pixels.fill(pixels.Color(255,255,255)); pixels.show(); delay(200);
                pixels.clear(); pixels.show(); delay(200);
            }
            WiFiManager wm;
            wm.resetSettings();
            
            Serial.println("Reset Done. Restarting ESP32 in 2 seconds...");
            delay(2000); // 메모리에 반영 완료될 대기 시간을 준 뒤 리부팅
            ESP.restart();
        }
        return; 
    } else {
        resetPressed = false;
    }

    // ===== TOUCH1: 측면 버튼 제어 =====
    if (touch1 && !touch1Pressed) {
        touch1Pressed = true;
        touch1StartTime = millis();
    }

    if (touch1Pressed && touch1) {
        if (millis() - touch1StartTime > 800) { // 롱 프레스 (밝기 조절)
            if (millis() - lastBrightnessTime > 300) {
                lastBrightnessTime = millis();
                brightnessLevel += brightnessDirection;
                if (brightnessLevel >= 10) { brightnessLevel = 10; brightnessDirection = -1; }
                if (brightnessLevel <= 1)  { brightnessLevel = 1;  brightnessDirection = 1; }
                pixels.setBrightness(brightnessLevel * 10);
                Serial.print("Brightness = "); Serial.println(brightnessLevel);
            }
        }
    }

    if (!touch1 && touch1Pressed) {
        touch1Pressed = false;
        if (millis() - touch1StartTime < 800) { // 숏 프레스 (모드 변경)
            mode++;
            if (mode > 7) { mode = 0; } 
            Serial.print("Mode = "); Serial.println(mode);
        }
    }

    // ===== 평상시 기본 LED 무드등 모드 출력 =====
    if (!effectRunning) {
        switch(mode) {
            case 0: solidColorEffect(); break;
            case 1: breathingEffect(); break;
            case 2: yellowSolidEffect(); break;
            case 3: yellowBreathingEffect(); break;
            case 4: colorAuroraEffect(); break;
            case 5: goldAuroraEffect(); break;
            case 6: purpleAuroraEffect(); break;
            case 7: ledOffEffect(); break;
        }
    }

    // ===== TOUCH2: 하트 버튼 (싱글 에지 트리거 분리) =====
    if (touch2 && !lastTouch2 && (millis() - lastTouch2Time > 1200)) {
        lastTouch2Time = millis();

        if (currentState == IDLE) {
            Serial.println("CALLing 시작...");
            lastCaller = ""; lastResponder = "";
            callerColor = pixels.Color(myR, myG, myB);
            responderColor = pixels.Color(myR, myG, myB);

            client.publish(mqtt_topic, myCall);
            currentState = CALLING;
            currentStateText = "CALLING";
            waitingAck = true;
            lastCallTime = millis();
            needCallEffect = true;
        } 
        else if (currentState == RINGING) {
            Serial.println("ACK 전송 완료");
            client.publish(mqtt_topic, myAck);
            currentState = CONNECTED;
            currentStateText = "CONNECTED";
            incomingCall = false;
            waitingAck = false;
            needCallEffect = false;
            
            playConnected();  
            needConnectEffect = true;
        }
    }

    // ===== 이벤트 효과 제어 시퀀스 =====
    if (needCallEffect && !effectRunning) {
        needCallEffect = false;
        playEffect();
    }

    if (needConnectEffect && !effectRunning) {
        needConnectEffect = false;
        connectedEffect();
        currentState = IDLE;
        currentStateText = "IDLE";
        setColor(saveR, saveG, saveB);
    }

    // ===== 3초 주기 재호출(Recall) 처리 및 무한루프 방지 =====
    if (waitingAck) {
        if (millis() - lastCallTime >= 3000) {
            client.loop(); 
            if (waitingAck) { 
                Serial.println("re-calling...");
                callerColor = pixels.Color(myR, myG, myB);
                client.publish(mqtt_topic, myCall);
                needCallEffect = true;
                lastCallTime = millis();
            }
        }
    }

    lastTouch1 = touch1;
    lastTouch2 = touch2;
    
    checkRemoteCommand();
    sendStatus();
}
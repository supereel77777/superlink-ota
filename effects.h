// ===== Mode 0 : 고유색 =====
void solidColorEffect()
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i,
                             pixels.Color(myR, myG, myB));
    }

    pixels.show();
}
void yellowSolidEffect()
{
    effectRunning = true;

    pixels.setBrightness(brightnessLevel * 10);

    for(int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(255, 255, 0));
    }

    pixels.show();

    effectRunning = false;
}
void yellowBreathingEffect()
{
    effectRunning = true;

    static int brightness = 40;
    static int dir = 1;

    brightness += dir;

    if (brightness >= 255)
        dir = -1;

    if (brightness <= 40)
        dir = 1;

    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(
            i,
            pixels.Color(
                brightness,
                brightness,
                0
            )
        );
    }

    pixels.show();

    delay(10);

    effectRunning = false;
}
// ===== Mode 1 : 고유색 숨쉬기 =====
void breathingEffect()
{
    static int bright = 40;
    static int step = 1;

    bright += step;

    if (bright >= 70)
        step = -1;

    if (bright <= 40)
        step = 1;

    pixels.setBrightness(bright);

    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i,
                             pixels.Color(myR, myG, myB));
    }

    pixels.show();

    smartDelay(50);
}


// ===== Mode 2 : 고유색 + 오로라 =====
void colorAuroraEffect()
{
    static float t = 0;

    t += 0.05;

    for (int i = 0; i < NUMPIXELS; i++)
    {
        float wave1 =
            (sin(i * 0.5 + t) + 1.0) / 2.0;

        float wave2 =
            (sin(i * 0.8 - t) + 1.0) / 2.0;

        int r = myR;
        int g = myG;
        int b = myB;

        if (myR > 0) // 빨강 계열
        {
            g = 80 * wave1;
            b = 120 * wave2;
        }
        else if (myG > 0) // 초록 계열
        {
            b = 120 * wave1;
            r = 60 * wave2;
        }
        else // 파랑 계열
        {
            r = 120 * wave1;
            g = 100 * wave2;
        }

        pixels.setPixelColor(
            i,
            pixels.Color(r, g, b));
    }

    pixels.show();

    smartDelay(50);
}


// ===== Mode 3 : 별빛 =====
void starEffect()
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(
            i,
            pixels.Color(
                myR / 6,
                myG / 6,
                myB / 6));
    }

    for (int k = 0; k < 4; k++)
    {
        int pos = random(NUMPIXELS);

        int type = random(4);

        switch (type)
        {
        case 0:
            pixels.setPixelColor(pos,
                                 pixels.Color(255,255,255));
            break;

        case 1:
            pixels.setPixelColor(pos,
                                 pixels.Color(255,220,100));
            break;

        case 2:
            pixels.setPixelColor(pos,
                                 pixels.Color(180,220,255));
            break;

        case 3:
            pixels.setPixelColor(pos,
                                 pixels.Color(255,150,200));
            break;
        }
    }

    pixels.show();

    smartDelay(120);
}


// ===== Mode 4 : 무지개 오로라 =====
void rainbowAuroraEffect()
{
    static int offset = 0;

    offset += 200;

    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(
            i,
            pixels.gamma32(
                pixels.ColorHSV(
                    offset +
                    i * 65536 / NUMPIXELS)));
    }

    pixels.show();

    smartDelay(50);
}


// ===== Mode 5 : OFF =====
void ledOffEffect()
{
    pixels.clear();

    pixels.show();
}

void flashEffect()
{
    for (int k = 0; k < 3; k++)
    { if (needConnectEffect) return;
        for (int i = 0; i < NUMPIXELS; i++)
{
    pixels.setPixelColor(i, callerColor);
}

        pixels.show();
        smartDelay(80);

        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, pixels.Color(0,0,0));
        }

        pixels.show();
        smartDelay(80);
    }
}

void mqttOfflineEffectFlash()
{
    // 보라
    pixels.fill(pixels.Color(180, 0, 255));
    pixels.show();
    smartDelay(150);

    // 주황
    pixels.fill(pixels.Color(255, 180, 0));
    pixels.show();
    smartDelay(150);

    // 원래 색 복원
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(saveR, saveG, saveB));
    }

    pixels.show();
}
void heartEffect()
{
    for (int k = 0; k < 3; k++)
    {  
        for (int b = 0; b <= 80; b += 5)
        { if (needConnectEffect) return;
            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
    i,
    pixels.Color(
        ((callerColor >> 16) & 0xFF) * b / 80,
        ((callerColor >> 8) & 0xFF) * b / 80,
        (callerColor & 0xFF) * b / 80));
            }

            pixels.show();
            smartDelay(15);
        }

        for (int b = 80; b >= 0; b -= 5)
        {  if (needConnectEffect) return;
            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
    i,
    pixels.Color(
        ((callerColor >> 16) & 0xFF) * b / 80,
        ((callerColor >> 8) & 0xFF) * b / 80,
        (callerColor & 0xFF) * b / 80));
            }

            pixels.show();
            smartDelay(15);
            }

        smartDelay(100);
    }
}
void connectBurstEffect()
{
    uint8_t r = (callerColor >> 16) & 0xFF;
uint8_t g = (callerColor >> 8) & 0xFF;
uint8_t b = callerColor & 0xFF;

    // 퍼지면서 밝아짐
    for (int bright = 0; bright <= 100; bright += 5)
    {  if (needConnectEffect) return;
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(
                    r * bright / 100,
                    g * bright / 100,
                    b * bright / 100));
        }

        pixels.show();

        smartDelay(20);
    }

    smartDelay(200);

    // 천천히 사라짐
    for (int bright = 100; bright >= 0; bright -= 5)
    { if (needConnectEffect) return;
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(
                    r * bright / 100,
                    g * bright / 100,
                    b * bright / 100));
        }

        pixels.show();

smartDelay(20);
    }
}



void connectSpinEffect()
{
    uint8_t r = (responderColor >> 16) & 0xFF;
    uint8_t g = (responderColor >> 8) & 0xFF;
    uint8_t b = responderColor & 0xFF;

    for (int round = 0; round < 3; round++)
    {
        for (int pos = 0; pos < NUMPIXELS; pos++)
        {
            pixels.clear();

            // 머리1 (100%)
            pixels.setPixelColor(
                pos,
                pixels.Color(
                    r,
                    g,
                    b));

            // 머리2 (90%)
            pixels.setPixelColor(
                (pos - 1 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r * 9 / 10,
                    g * 9 / 10,
                    b * 9 / 10));

            // 꼬리1 (80%)
            pixels.setPixelColor(
                (pos - 2 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r * 8 / 10,
                    g * 8 / 10,
                    b * 8 / 10));

            // 꼬리2 (60%)
            pixels.setPixelColor(
                (pos - 3 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r * 6 / 10,
                    g * 6 / 10,
                    b * 6 / 10));

            // 꼬리3 (40%)
            pixels.setPixelColor(
                (pos - 4 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r * 4 / 10,
                    g * 4 / 10,
                    b * 4 / 10));

            // 꼬리4 (20%)
            pixels.setPixelColor(
                (pos - 5 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r * 2 / 10,
                    g * 2 / 10,
                    b * 2 / 10));

            // 꼬리5 (10%)
            pixels.setPixelColor(
                (pos - 6 + NUMPIXELS) % NUMPIXELS,
                pixels.Color(
                    r / 10,
                    g / 10,
                    b / 10));

            pixels.show();
smartDelay(120);
}
    }
}
void connectGlowEffect()
{
    uint8_t r = (responderColor >> 16) & 0xFF;
    uint8_t g = (responderColor >> 8) & 0xFF;
    uint8_t b = responderColor & 0xFF;

    // 천천히 밝아짐
    for (int bright = 0; bright <= 100; bright += 2)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(
                    r * bright / 100,
                    g * bright / 100,
                    b * bright / 100));
        }

        pixels.show();
smartDelay(25);
    }

    // 잠시 유지
    smartDelay(700);

    // 천천히 어두워짐
    for (int bright = 100; bright >= 0; bright -= 2)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(
                    r * bright / 100,
                    g * bright / 100,
                    b * bright / 100));
        }

        pixels.show();
smartDelay(25); 
   }
}

void rainbowBreathing()
{
    for (int cycle = 0; cycle < 2; cycle++)
    {
        for (int bright = 10; bright <= 80; bright += 2)
        {  if (needConnectEffect) return;
            pixels.setBrightness(bright);

            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
                    i,
                    pixels.gamma32(
                        pixels.ColorHSV(
                            (i * 65536 / NUMPIXELS) +
                            millis() * 5)));
            }

            pixels.show();
            smartDelay(40);
        }

        for (int bright = 80; bright >= 10; bright -= 2)
        { if (needConnectEffect) return;
            pixels.setBrightness(bright);

            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
                    i,
                    pixels.gamma32(
                        pixels.ColorHSV(
                            (i * 65536 / NUMPIXELS) +
                            millis() * 5)));
            }

            pixels.show();
     smartDelay(40);
        }
    }

    pixels.setBrightness(50);
}

void connectFlashEffect()
{
    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < NUMPIXELS; i++)
{
    pixels.setPixelColor(i, responderColor);
}

        pixels.show();
        smartDelay(80);

        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i,
                                 pixels.Color(0, 0, 0));
        }

        pixels.show();
        smartDelay(80);
    }
}

void connectSuperFlash()
{
    for (int k = 0; k < 10; k++)
    {
        // 흰색
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, pixels.Color(255,255,255));
        }

        pixels.show();

smartDelay(80);
        // 꺼짐
        pixels.clear();

        pixels.show();
smartDelay(80);
    }
}
void connectWhiteColorFlash()
{
    uint8_t r = (responderColor >> 16) & 0xFF;
    uint8_t g = (responderColor >> 8) & 0xFF;
    uint8_t b = responderColor & 0xFF;

    for (int k = 0; k < 30; k++)
    {
        // 흰색
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(255,255,255));
        }

        pixels.show();

smartDelay(80);

        // 상대방 색
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(r,g,b));
        }

        pixels.show();
smartDelay(80);
    }
}
void auroraEffect()
{
    for (int t = 0; t < 250; t++)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            float wave1 =
                (sin(i * 0.6 + t * 0.12) + 1.0) / 2.0;

            float wave2 =
                (sin(i * 0.9 - t * 0.08) + 1.0) / 2.0;

            float wave3 =
                (sin(i * 1.3 + t * 0.05) + 1.0) / 2.0;

            int r =
                255 * wave3;

            int g =
                100 +
                155 * wave1;

            int b =
                100 +
                155 * wave2;

            pixels.setPixelColor(
                i,
                pixels.Color(r, g, b));
        }

        pixels.show();

        smartDelay(60);
    }
}

void meteorEffect()
{
    for (int round = 0; round < 5; round++)
    {
        for (int pos = 0; pos < NUMPIXELS + 12; pos++)
        {
            pixels.clear();

            for (int tail = 0; tail < 10; tail++)
            {
                int p = pos - tail;

                if (p >= 0 && p < NUMPIXELS)
                {
                    int bright = 255 - tail * 25;

                    if (bright < 0)
                        bright = 0;

                    pixels.setPixelColor(
                        p,
                        pixels.Color(
                            bright,
                            bright,
                            bright));
                }
            }

            pixels.show();

            smartDelay(50);
        }
    }
}
void fireflyEffect()
{
    for (int cycle = 0; cycle < 120; cycle++)
    {
        // 배경 잔상
        for (int i = 0; i < NUMPIXELS; i++)
        {
            uint32_t c = pixels.getPixelColor(i);

            int r = ((c >> 16) & 0xFF) * 85 / 100;
            int g = ((c >> 8) & 0xFF) * 85 / 100;
            int b = (c & 0xFF) * 85 / 100;

            pixels.setPixelColor(
                i,
                pixels.Color(r, g, b));
        }

        // 랜덤 반딧불 생성
        if (random(100) < 45)
        {
            int pos = random(NUMPIXELS);

            int type = random(3);

            switch (type)
            {
            case 0:
                // 노란 반딧불
                pixels.setPixelColor(
                    pos,
                    pixels.Color(
                        255,
                        255,
                        80));
                break;

            case 1:
                // 연두
                pixels.setPixelColor(
                    pos,
                    pixels.Color(
                        180,
                        255,
                        100));
                break;

            case 2:
                // 민트
                pixels.setPixelColor(
                    pos,
                    pixels.Color(
                        100,
                        255,
                        180));
                break;
            }
        }

        pixels.show();

        smartDelay(120);
    }
}
void twinkleStarEffect()
{
    uint8_t r = (responderColor >> 16) & 0xFF;
    uint8_t g = (responderColor >> 8) & 0xFF;
    uint8_t b = responderColor & 0xFF;

    for (int cycle = 0; cycle < 30; cycle++)
    {
        // 배경색 20%
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(
                i,
                pixels.Color(
                    r / 5,
                    g / 5,
                    b / 5));
        }

        // 별 4개 생성
        for (int k = 0; k < 4; k++)
        {
            int pos = random(NUMPIXELS);

            int starType = random(4);

            switch (starType)
            {
            case 0:
                pixels.setPixelColor(pos, pixels.Color(255,255,255));
                break;

            case 1:
                pixels.setPixelColor(pos, pixels.Color(255,220,100));
                break;

            case 2:
                pixels.setPixelColor(pos, pixels.Color(180,220,255));
                break;

            case 3:
                pixels.setPixelColor(pos, pixels.Color(255,150,200));
                break;
            }
        }

        pixels.show();

        smartDelay(120);
    }
}
void connectHeartEffect()
{
    for (int k = 0; k < 3; k++)
    {
        // 밝아짐
        for (int b = 0; b <= 80; b += 5)
        {
            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
    i,
    pixels.Color(
        ((responderColor >> 16) & 0xFF) * b / 80,
        ((responderColor >> 8) & 0xFF) * b / 80,
        (responderColor & 0xFF) * b / 80));
            }

            pixels.show();
smartDelay(15);
        }

        // 어두워짐
        for (int b = 80; b >= 0; b -= 5)
        {
            for (int i = 0; i < NUMPIXELS; i++)
            {
                pixels.setPixelColor(
    i,
    pixels.Color(
        ((responderColor >> 16) & 0xFF) * b / 80,
        ((responderColor >> 8) & 0xFF) * b / 80,
        (responderColor & 0xFF) * b / 80));
            }

            pixels.show();
smartDelay(15);
        }

        delay(100);
    }
}
void otaEffect()
{
    for (int k = 0; k < 50; k++)
    {
        // Gold
        pixels.fill(pixels.Color(255,180,0));
        pixels.show();
        smartDelay(60);

        // Silver
        pixels.fill(pixels.Color(180,180,220));
        pixels.show();
        smartDelay(60);

        // White Flash
        pixels.fill(pixels.Color(255,255,255));
        pixels.show();
        smartDelay(25);

        // OFF
        pixels.clear();
        pixels.show();
        smartDelay(25);
    }
}
void goldAuroraEffect()
{
    static float t = 0;

    t += 0.06;

    for (int i = 0; i < NUMPIXELS; i++)
    {
        float wave1 =
            (sin(i * 0.55 + t) + 1.0) / 2.0;

        float wave2 =
            (sin(i * 0.85 - t) + 1.0) / 2.0;

        int r =
            180 + 75 * wave1;

        int g =
            120 + 70 * wave2;

        int b =
            15 + 35 * wave1;

        pixels.setPixelColor(
            i,
            pixels.Color(r, g, b));
    }

    pixels.show();

    smartDelay(50);
}
void purpleAuroraEffect()
{
    static float t = 0;

    t += 0.06;

    for (int i = 0; i < NUMPIXELS; i++)
    {
        float wave1 =
            (sin(i * 0.55 + t) + 1.0) / 2.0;

        float wave2 =
            (sin(i * 0.85 - t) + 1.0) / 2.0;

        int r =
            110 + 90 * wave1;

        int g =
            10 + 30 * wave2;

        int b =
            180 + 75 * wave1;

        pixels.setPixelColor(
            i,
            pixels.Color(r, g, b));
    }

    pixels.show();

    smartDelay(50);
}
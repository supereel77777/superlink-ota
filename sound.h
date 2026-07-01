#ifndef SOUND_H
#define SOUND_H

#include <DFRobotDFPlayerMini.h>

extern DFRobotDFPlayerMini myDFPlayer;
extern void smartDelay(unsigned long ms);

void initSound();

void playMomCall()
{
    static int momIndex = 1;

    Serial.print("playMomCall : ");
    Serial.println(momIndex);

    myDFPlayer.playLargeFolder(1, momIndex);

    Serial.println("play command sent");

    momIndex++;

    if (momIndex > 4)
    {
        momIndex = 1;
    }
}
void playConnected()
{
    static int connectIndex = 2;

    Serial.println("playConnected 시작");

    myDFPlayer.playLargeFolder(2,1);

    smartDelay(2800);

    myDFPlayer.playLargeFolder(2,2);

    Serial.println("playConnected 종료");
}
#endif
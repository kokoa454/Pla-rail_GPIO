#include "Plarail_GPIO.h"

void stopTrain(int iHndl)
{
    if(lgGpioWrite(iHndl, SIG, LG_LOW) != 0)
    {
        outputLog("列車の停止に失敗しました");
        return;
    }
    outputLog("列車を停止させました");
    return;
}
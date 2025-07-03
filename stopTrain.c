#include "Plarail_GPIO.h"

void stopTrain()
{
    lgGpioWrite(iHndl, SIG, LG_LOW);
    outputLog("列車を停止させました");
    return;
}
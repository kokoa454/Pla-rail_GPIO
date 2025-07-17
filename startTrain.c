#include "Plarail_GPIO.h"

void startTrain(int iHndl)
{
    if(lgGpioWrite(iHndl, SIG, LG_HIGH) != 0)
    {
        outputLog("列車の発車に失敗しました");
        return;
    }
    outputLog("列車を発車させました");
    return;
}
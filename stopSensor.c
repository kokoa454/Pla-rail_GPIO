#include "Plarail_GPIO.h"

bool stopSensor(int iMeasureDistance)
{
    if(lgGpioSetAlertFunc(iHndl, ECHO, NULL, NULL) != 0)
    {
        outputLog("測距センサの停止に失敗しました");
        return false;
    }

    lgThreadStop(iMeasureDistance);
    
    outputLog("測距センサを停止しました");
    return true;
}
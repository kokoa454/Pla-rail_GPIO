#include "Plarail_GPIO.h"

bool startSensor(int *piMeasureDistanceId)
{
    int iUserdata = 0;

    if(lgGpioSetAlertFunc(iHndl, ECHO, catchEcho, NULL) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }
    
    int iMeasureDistanceId = lgThreadStart(measureDistance, &iUserdata);

    if (iMeasureDistanceId != NULL) {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }

    *piMeasureDistanceId = iMeasureDistanceId;

    outputLog("測距センサを起動しました");
    return true;
}
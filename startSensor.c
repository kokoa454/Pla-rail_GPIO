#include "Plarail_GPIO.h"

bool startSensor(int iHndl)
{
    ppMeasureDistanceId = lgThreadStart(measureDistance, &iHndl);

    if (ppMeasureDistanceId == NULL) {
        outputLog("測距センサの起動に失敗しました");
        return FUNC_FAILURE;
    }

    if(lgGpioSetAlertsFunc(iHndl, ECHO, catchEcho, &iHndl) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        stopSensor(iHndl);
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}
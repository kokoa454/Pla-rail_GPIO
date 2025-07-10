#include "Plarail_GPIO.h"

pthread_t *pMeasureDistanceId = NULL;

bool startSensor(int iHndl)
{
    *pMeasureDistanceId = lgThreadStart(measureDistance, &iHndl);

    if (pMeasureDistanceId == NULL) {
        outputLog("測距センサの起動に失敗しました");
        return FUNC_FAILURE;
    }

    if(lgGpioSetAlertsFunc(iHndl, ECHO, catchEcho, &iHndl) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        stopSensor(iHndl, pMeasureDistanceId);
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}
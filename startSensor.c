#include "Plarail_GPIO.h"

pthread_t *pMeasureDistanceId = NULL;

bool startSensor()
{
    pthread_t *pMeasureDistanceId = lgThreadStart(measureDistance, NULL);

    if (pMeasureDistanceId == NULL) {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }

    if(lgGpioSetAlertsFunc(iHndl, ECHO, catchEcho, pMeasureDistanceId) == FUNC_FAILURE)
    {
        outputLog("測距センサの起動に失敗しました");
        stopSensor(pMeasureDistanceId);
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}
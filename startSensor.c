#include "Plarail_GPIO.h"

bool startSensor()
{
    int *piMeasureDistanceId = malloc(sizeof(int));

    *piMeasureDistanceId = lgThreadStart(measureDistance, NULL);

    if (*piMeasureDistanceId != NULL) {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }

    if(lgGpioSetAlertFunc(iHndl, ECHO, catchEcho, piMeasureDistanceId) == FUNC_FAILURE)
    {
        outputLog("測距センサの起動に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}
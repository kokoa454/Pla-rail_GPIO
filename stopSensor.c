#include "Plarail_GPIO.h"

bool stopSensor(int *piMeasureDistanceId)
{
    if(lgGpioSetAlertFunc(iHndl, ECHO, NULL, NULL) == FUNC_FAILURE)
    {
        outputLog("測距センサの停止に失敗しました");
        return FUNC_FAILURE;
    }

    lgThreadStop(*piMeasureDistanceId);
    
    outputLog("測距センサを停止しました");
    return FUNC_SUCCESS;
}
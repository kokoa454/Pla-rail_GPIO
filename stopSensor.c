#include "Plarail_GPIO.h"

bool stopSensor(int iHndl)
{
    if(lgGpioSetAlertsFunc(iHndl, ECHO, NULL, NULL) != 0)
    {
        outputLog("測距センサの停止に失敗しました");
        return FUNC_FAILURE;
    }

    lgThreadStop(pMeasureDistanceId);
    
    outputLog("測距センサを停止しました");
    return FUNC_SUCCESS;
}
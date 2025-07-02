#include "Plarail_GPIO.h"

bool startSensor(int iHndl, pthread_t *pThreadID)
{
    if(lgGpioSetAlertFunc(iHndl, ECHO, catchEcho, NULL) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }
    
    if(pthread_create(pThreadID, NULL, measureDistance, iHndl) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        return false;
    }

    outputLog("測距センサを起動しました");
    return true;
}
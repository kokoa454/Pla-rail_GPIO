#include "Plarail_GPIO.h"

bool stopSensor(int iHndl, pthread_t threadID)
{
    if(lgGpioSetAlertFunc(iHndl, ECHO, NULL, NULL) != 0)
    {
        outputLog("測距センサの停止に失敗しました");
        return false;
    }

    if(pthread_cancel(threadID) != 0)
    {
        outputLog("測距センサの停止に失敗しました");
        return false;
    }
    
    outputLog("測距センサを停止しました");
    return true;
}
#include "Plarail_GPIO.h"

void *measureDistance(void *vpUserdata)
{
    while(1)
    {
        if(lgGpioRead(iHndl, TRIG) == LG_HIGH)
        {
            outputLog("測距センサの異常を検知しました");
            lgGpioWrite(iHndl, TRIG, LG_LOW);
        } 
        else 
        {
            lgGpioWrite(iHndl, TRIG, LG_HIGH);
            usleep(10);
            lgGpioWrite(iHndl, TRIG, LG_LOW);
            usleep(250000);
        }
    }
}
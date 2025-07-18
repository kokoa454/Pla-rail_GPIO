#include "Plarail_GPIO.h"

void *measureDistance(void *vpHndl)
{
    printf("測距センサのスレッドを開始しました\n");
    while(1)
    {
        if(lgGpioRead(*(int*)vpHndl, TRIG) == LG_HIGH)
        {
            outputLog("測距センサの異常を検知しました");
            lgGpioWrite(*(int*)vpHndl, TRIG, LG_LOW);
        } 
        else 
        {
            lgGpioWrite(*(int*)vpHndl, TRIG, LG_HIGH);
            usleep(10);
            lgGpioWrite(*(int*)vpHndl, TRIG, LG_LOW);
            usleep(250000);
        }
    }
}
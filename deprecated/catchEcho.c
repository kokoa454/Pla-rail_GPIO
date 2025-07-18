#include "Plarail_GPIO.h"

#define AEBS_DISTANCE 10.0 //自動停止までの距離

struct timeval start, end;

void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpHndl)
{
    long lUsec = 0;
    float fResult = 0;

    printf("ECHO信号を受信しました\n");
    gettimeofday(&start,NULL);
    while(lgGpioRead(*(int*)vpHndl,ECHO) == LG_HIGH);//ECHOがhighの間
    gettimeofday(&end,NULL);

    lUsec = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    fResult = (lUsec* 0.034) / 2;
    
    printf("距離: %.2f cm\n", fResult);

    if (AEBS_DISTANCE >= fResult)
    {
        stopSensor(*(int*)vpHndl);
        stopTrain(*(int*)vpHndl);
    }
}
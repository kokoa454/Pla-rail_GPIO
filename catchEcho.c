#include <Pla-rail_GPIO.h>
#define DISTANCE_AEBS 10.0 //自動停止までの距離

void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpUserdata)
{
    long lUsec = 0;
    float fResult = 0;
    struct timeval start;
    struct timeval end;

    gettimeofday(&start,NULL);
    while(lgGpioRead(iHundl,ECHO));//ECHOがhighの間
    gettimeofday(&end,NULL);

    lUsec = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    fResult = (lUsec* 0.034) / 2;
    
    if (fResult <= DISTANCE_AEBS)
    {
        stopSensor();
        stopTrain();
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define CHIPSET 0
#define TRIG 23
#define ECHO 24
#define SIG 25
#define FUNC_SUCCESS true
#define FUNC_FAILURE false
#define COMMAND_COMPLETE_MATCH 0
#define TRAIN_STOPPING 0
#define TRAIN_RUNNING 1
#define AEBS_DISTANCE 10.0 //自動停止までの距離

struct timeval start, end;

bool setGpio();
void startTrain();
void stopTrain();
bool startSensor();
bool stopSensor();
void *measureDistance();
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpUserData);
void outputLog(char cMsg[]);

pthread_t *ppMeasureDistanceId = NULL;
int iHndl = 0;
int iIsTrainRunning = TRAIN_STOPPING;

int main(void)
{
    iHndl = lgGpiochipOpen(CHIPSET);
    char cUserInput[16];
    int iTrash = 0;

    if (setGpio() == FUNC_FAILURE)
    {
        goto failure;
    }
    while(1)
    {
        printf("コマンドを入力してください (start/stop/exit): ");
        scanf("%15s", cUserInput);
        while ((iTrash = getchar()) != '\n' && iTrash != EOF);
        if (strcmp(cUserInput,"start") == COMMAND_COMPLETE_MATCH)
        {
            if (iIsTrainRunning != TRAIN_STOPPING)
            {
                outputLog("列車は停車していません");
            }
            else if (startSensor() == FUNC_SUCCESS)
            {
                startTrain();
                iIsTrainRunning = TRAIN_RUNNING;
            }
            else
            {
                goto failure;
            }
        }
        else if (strcmp(cUserInput,"stop") == COMMAND_COMPLETE_MATCH)
        {
            if (iIsTrainRunning != TRAIN_RUNNING)
            {
                outputLog("列車は発車していません");
            }
            else if (stopSensor() == FUNC_SUCCESS)
            {
                stopTrain();
                iIsTrainRunning = TRAIN_STOPPING;
            }
            else
            {
                goto failure;
            }
        }
        else if ((strcmp(cUserInput,"exit") == COMMAND_COMPLETE_MATCH))
        {
            if (iIsTrainRunning != TRAIN_STOPPING)
            {
                outputLog("列車を停車させてください");
            }
            else
            {
                outputLog("プログラムを終了します");
                break;
            }
        }
        else
        {
            printf("コマンド名が違います\n");
        }
    }

    lgGpiochipClose(iHndl);
    return EXIT_SUCCESS;

    failure://失敗
        lgGpiochipClose(iHndl);
        return EXIT_FAILURE;
}

void startTrain()
{
    if(lgGpioWrite(iHndl, SIG, LG_HIGH) != 0)
    {
        outputLog("列車の発車に失敗しました");
        return;
    }
    outputLog("列車を発車させました");
    return;
}

void stopTrain()
{
    if(lgGpioWrite(iHndl, SIG, LG_LOW) != 0)
    {
        outputLog("列車の停止に失敗しました");
        return;
    }
    outputLog("列車を停止させました");
    return;
}

bool startSensor()
{
    ppMeasureDistanceId = lgThreadStart(measureDistance, &iHndl);
    int userData;

    if (ppMeasureDistanceId == NULL) {
        outputLog("測距センサの起動に失敗しました");
        return FUNC_FAILURE;
    }

    if(lgGpioSetAlertsFunc(iHndl, ECHO, catchEcho, &userData) != 0)
    {
        outputLog("測距センサの起動に失敗しました");
        stopSensor();
        return FUNC_FAILURE;
    }

    if(lgGpioClaimAlert(iHndl, LG_SET_PULL_DOWN, LG_RISING_EDGE, ECHO, -1) != 0)
    {
        outputLog("測距センサのアラート設定に失敗しました");
        stopSensor();
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}

bool stopSensor()
{
    if(lgGpioSetAlertsFunc(iHndl, ECHO, NULL, NULL) != 0)
    {
        outputLog("測距センサの停止に失敗しました");
        return FUNC_FAILURE;
    }

    lgThreadStop(ppMeasureDistanceId);
    
    outputLog("測距センサを停止しました");
    return FUNC_SUCCESS;
}

void *measureDistance()
{
    outputLog("測距センサのスレッドを開始しました");
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

void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpUserData)
{
    long lUsec = 0;
    float fResult = 0;

    gettimeofday(&start,NULL);
    while(lgGpioRead(iHndl, ECHO) == LG_HIGH);//ECHOがhighの間
    gettimeofday(&end,NULL);

    lUsec = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    fResult = (lUsec* 0.034) / 2;
    
    if (AEBS_DISTANCE >= fResult)
    {
		printf("\n障害物を検知しました。\n列車までの距離: %.2f cm\n", fResult);
        stopSensor();
        stopTrain();
        iIsTrainRunning = TRAIN_STOPPING;
        printf("コマンドを入力してください (start/stop/exit): ");
        fflush(stdout);
    }
}

void outputLog(char cMsg[]){
    printf("%s\n",cMsg);
}

bool setGpio()
{
    // ラズパイからの出力の設定
    int iFlgOut = 0;
    
    // 測距センサーのTRIGGERの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, TRIG, LG_LOW) != 0)
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }
    
    // リレーのSIGの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, SIG, LG_LOW) != 0)
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("GPIOを設定しました");
    return FUNC_SUCCESS;
}

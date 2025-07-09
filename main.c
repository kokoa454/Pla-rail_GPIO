#include "Plarail_GPIO.h"

#define MAX_NUM_USERINPUT 16
#define COMMAND_COMPLETE_MATCH 0
#define TRAIN_STOPPING 0
#define TRAIN_RUNNING 1

int iHndl = 0;

int main(void)
{
    iHndl = lgGpiochipOpen(CHIPSET);
    int iIsTrainRunning = TRAIN_STOPPING;
    char cUserInput[16];

    if (setGpio() == FUNC_SUCCESS)
    {
        goto failure;
    }
    while(1)
    {
        scanf("%15s", cUserInput);
        rewind(stdin);
        if (strcmp(cUserInput,"start") == COMMAND_COMPLETE_MATCH)
        {
            if (iIsTrainRunning != TRAIN_STOPPING)
            {
                outputLog("列車は停車していません");
                continue;
            }
            if (startSensor() == FUNC_SUCCESS)
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
                continue;
            }
            if (stopSensor(*pMeasureDistanceId) == FUNC_SUCCESS)
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
            outputLog("コマンド名が違います");
        }
    }

    lgGpiochipClose(iHndl);
    return EXIT_SUCCESS;

    failure://失敗
        lgGpiochipClose(iHndl);
        return EXIT_FAILURE;
}
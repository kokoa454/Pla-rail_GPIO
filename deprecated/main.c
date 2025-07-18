#include "Plarail_GPIO.h"

#define COMMAND_COMPLETE_MATCH 0
#define TRAIN_STOPPING 0
#define TRAIN_RUNNING 1

pthread_t *ppMeasureDistanceId = NULL;

int main(void)
{
    int iHndl = lgGpiochipOpen(CHIPSET);
    int iIsTrainRunning = TRAIN_STOPPING;
    char cUserInput[16];
    int iTrash = 0;

    if (setGpio(iHndl) == FUNC_FAILURE)
    {
        goto failure;
    }
    while(1)
    {
        printf("コマンドを入力してください (start/stop/exit): ");
        scanf("%15s", cUserInput);
        while(iTrash = getchar() != '\n' && iTrash != EOF);
        if (strcmp(cUserInput,"start") == COMMAND_COMPLETE_MATCH)
        {
            if (iIsTrainRunning != TRAIN_STOPPING)
            {
                outputLog("列車は停車していません");
            }
            else if (startSensor(iHndl) == FUNC_SUCCESS)
            {
                startTrain(iHndl);
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
            else if (stopSensor(iHndl) == FUNC_SUCCESS)
            {
                stopTrain(iHndl);
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
#include <Pla-rail_GPIO.h>
#define PI5 4
#define MAX_NUM_USERINPUT 16
#define COMPLETE_MATCH 0
#define STOPPING 0
#define RUN 1

//引数を入れてない！！！！

int main(void)
{
    int iIsTrainRunning = 0;
    char cUserInput[16]

    if (setGpio() == true)
    {
        goto failure;
    }
    while(1)
    {
        scanf("%s",cUserInput);
        if (MAX_NUM_USERINPUT >= sizeof(cUserInput))
        {
            if (strcmp(cUserInput,"start") == COMPLETE_MATCH)
            {
                if (iIsTrainRunning != STOPPING)
                {
                    outputLog("列車は停車していません");
                    continue;
                }
                if (startSensor())
                {
                    startTrain()
                    iIsTrainRunning = RUN;
                }
                else
                {
                    goto failure;
                }
            }
            else if (strcmp(cUserInput,"stop") == COMPLETE_MATCH)
            {
                if (iIsTrainRunning != RUN)
                {
                    outputLog("列車は発車していません");
                    continue;
                }
                if (stopSensor())
                {
                    stopTrain();
                    iIsTrainRunning = STOPPING;
                }
                else
                {
                    goto failure;
                }
            }
            else if ((strcmp(cUserInput,"exit") == COMPLETE_MATCH))
            {
                if (iIsTrainRunning != STOPPING)
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
        else
        {
            outputLog("規定のコマンドを入力してください");
        }
    }

    lgGpiochipClose(iHundl);
    return EXIT_SUCCESS;

    failure://失敗
        lgGpiochipClose(iHundl);
        return EXIT_FAILURE;
}
#include <Pla-rail_GPIO.h>
#define MAX_NUM_USERINPUT 16
#define COMPLETE_MATCH 0
#define STOPPING 0
#define RUN 1

//引数を入れてない
//gotoのジャンプ地点を設定してない

int main(void)
{
    int iIsTrainRunning = 0;
    char cUserInput[16]

    int iHndl = lgGpiochipOpen(PI5);
    if (setGpio == true)
    {
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

    end:
        lgGpiochipClose(iHundl);
        return EXIT_FAILURE;
}
#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define CHIPSET 0 //ラズパイのGPIO番号
#define TRIG 23 //測距センサーのTRIGGER
#define ECHO 24 //測距センサーのECHO
#define SIG 25 //リレーのSIG
#define FUNC_SUCCESS true //ユーザ関数の実行結果（成功）
#define FUNC_FAILURE false //ユーザ関数の実行結果（失敗）
#define COMMAND_COMPLETE_MATCH 0 //strcmpの比較結果
#define TRAIN_STOPPING 0 //列車の状態（停車中）
#define TRAIN_RUNNING 1 //列車の状態（動作中）
#define AEBS_DISTANCE 10.0 //自動停止までの距離

//GPIOハンドル、列車の動作状態、測距用スレッドのスレッド番号が格納される構造体
typedef struct {
    int iHndl;
    int iIsTrainRunning;
    pthread_t *ppMeasureDistanceId;
} PLARAIL_DATA;

bool setGpio(PLARAIL_DATA *pdpPlarailData); //GPIOの設定
void startTrain(PLARAIL_DATA *pdpPlarailData); //列車の発車
void stopTrain(PLARAIL_DATA *pdpPlarailData); //列車の停止
bool startSensor(PLARAIL_DATA *pdpPlarailData); //測距センサの起動
bool stopSensor(PLARAIL_DATA *pdpPlarailData); //測距センサの停止
void *measureDistance(void *vpPlarailData); //測距センサのスレッド（TRIG信号の送信）
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpPlarailData); //ECHO信号の受信
void outputLog(char cMsg[]); //ログ出力

int main(void)
{
    //構造体の初期化
    PLARAIL_DATA pdPlarailData = {0,0,NULL};
    pdPlarailData.iHndl = lgGpiochipOpen(CHIPSET);
    pdPlarailData.iIsTrainRunning = TRAIN_STOPPING;

    //変数の宣言
    char cUserInput[16]; //コマンド入力用配列
    int iTrash = 0; //バッファクリア用変数

    //GPIOの設定が失敗したらプログラム強制終了
    if (FUNC_FAILURE == setGpio(&pdPlarailData))
    {
        goto failure;
    }

    //コマンド入力はユーザがexitを入力するまで繰り返す
    while(1)
    {
        printf("コマンドを入力してください (start/stop/exit): ");
        scanf("%15s", cUserInput);
        while ((iTrash = getchar()) != '\n' && iTrash != EOF); //バッファクリア

        //コマンドstartの場合
        if (COMMAND_COMPLETE_MATCH == strcmp(cUserInput,"start"))
        {
            //列車が停車していない場合
            if (TRAIN_STOPPING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車は停車していません");
            }
            //測距センサの起動に成功した場合
            else if (FUNC_SUCCESS == startSensor(&pdPlarailData))
            {
                startTrain(&pdPlarailData);
                pdPlarailData.iIsTrainRunning = TRAIN_RUNNING;
            }
            //測距センサの起動に失敗した場合
            else
            {
                goto failure;
            }
        }
        //コマンドstopの場合
        else if (COMMAND_COMPLETE_MATCH == strcmp(cUserInput,"stop"))
        {
            //列車が発車していない場合
            if (TRAIN_RUNNING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車は発車していません");
            }
            //測距センサの停止に成功した場合
            else if (FUNC_SUCCESS == stopSensor(&pdPlarailData))
            {
                stopTrain(&pdPlarailData);
                pdPlarailData.iIsTrainRunning = TRAIN_STOPPING;
            }
            //測距センサの停止に失敗した場合
            else
            {
                goto failure;
            }
        }
        //コマンドexitの場合
        else if ((COMMAND_COMPLETE_MATCH  ==strcmp(cUserInput,"exit")))
        {
            //列車が停車していない場合
            if (TRAIN_STOPPING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車を停車させてください");
            }
            //列車が停車している場合
            else
            {
                outputLog("プログラムを終了します");
                break;
            }
        }
        //コマンド名がstart、stop、exitと一致しない場合
        else
        {
            printf("コマンド名が違います\n");
        }
    }

    //プログラムがexitコマンドで終了した場合
    lgGpiochipClose(pdPlarailData.iHndl);
    return EXIT_SUCCESS;

    //プログラムが異常終了した場合
    failure:
        lgGpiochipClose(pdPlarailData.iHndl);
        return EXIT_FAILURE;
}

void startTrain(PLARAIL_DATA *pdpPlarailData)
{
    //列車の発車に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioWrite(pdpPlarailData->iHndl, SIG, LG_HIGH))
    {
        outputLog("列車の発車に失敗しました");
        return;
    }
    outputLog("列車を発車させました");
    return;
}

void stopTrain(PLARAIL_DATA *pdpPlarailData)
{
    //列車の停止に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioWrite(pdpPlarailData->iHndl, SIG, LG_LOW))
    {
        outputLog("列車の停止に失敗しました");
        return;
    }
    outputLog("列車を停止させました");
    return;
}

bool startSensor(PLARAIL_DATA *pdpPlarailData)
{
    //ppMeasureDistanceIdに測距用スレッドのスレッド番号を格納
    pdpPlarailData->ppMeasureDistanceId = lgThreadStart(measureDistance, pdpPlarailData);

    //測距用スレッドのスレッド番号がNULLの場合はエラー出力
    if (NULL == pdpPlarailData->ppMeasureDistanceId) {
        outputLog("測距センサの起動に失敗しました");
        return FUNC_FAILURE;
    }

    //測距センサの起動に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioSetAlertsFunc(pdpPlarailData->iHndl, ECHO, catchEcho, pdpPlarailData))
    {
        outputLog("測距センサの起動に失敗しました");
        stopSensor(pdpPlarailData);
        return FUNC_FAILURE;
    }

    //測距センサのアラート設定に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimAlert(pdpPlarailData->iHndl, LG_SET_PULL_DOWN, LG_RISING_EDGE, ECHO, -1))
    {
        outputLog("測距センサのアラート設定に失敗しました");
        stopSensor(pdpPlarailData);
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");
    return FUNC_SUCCESS;
}

bool stopSensor(PLARAIL_DATA *pdpPlarailData)
{
    //測距センサの停止に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioSetAlertsFunc(pdpPlarailData->iHndl, ECHO, NULL, NULL))
    {
        outputLog("測距センサの停止に失敗しました");
        return FUNC_FAILURE;
    }

    //測距用スレッドの停止
    lgThreadStop(pdpPlarailData->ppMeasureDistanceId);
    
    outputLog("測距センサを停止しました");
    return FUNC_SUCCESS;
}

void *measureDistance(void *vpPlarailData)
{
    //PLARAIL_DATA構造体のポインタを格納
    PLARAIL_DATA *pdPlarailData = (PLARAIL_DATA *)vpPlarailData;

    outputLog("測距センサのスレッドを開始しました");

    //測距センサ起動中は0.25秒おきに10マイクロ秒間TRIGをHIGHにする
    while(1)
    {
        //測距センサの異常を検知した場合
        if(LG_HIGH == lgGpioRead(pdPlarailData->iHndl, TRIG))
        {
            outputLog("測距センサの異常を検知しました");
            lgGpioWrite(pdPlarailData->iHndl, TRIG, LG_LOW);
        } 
        //測距センサが正常な場合
        else 
        {
            lgGpioWrite(pdPlarailData->iHndl, TRIG, LG_HIGH);
            usleep(10);
            lgGpioWrite(pdPlarailData->iHndl, TRIG, LG_LOW);
            usleep(250000);
        }
    }
}

void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpPlarailData)
{
    //PLARAIL_DATA構造体のポインタを格納
    PLARAIL_DATA *pdpPlarailData = (PLARAIL_DATA *)vpPlarailData;

    //測距計算用の変数
    struct timeval start, end;
    long lUsec = 0;
    float fResult = 0;

    //ECHOがHIGHの間の時間を計測
    gettimeofday(&start,NULL); //開始時刻
    while(lgGpioRead(pdpPlarailData->iHndl, ECHO) == LG_HIGH); //ECHOがHIGHの間待機
    gettimeofday(&end,NULL); //終了時刻

    //測距計算
    lUsec = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    fResult = (lUsec* 0.034) / 2;
    
    //列車が障害物検知時停止距離以下になった場合
    if (AEBS_DISTANCE >= fResult)
    {
		printf("\n障害物を検知しました。\n列車までの距離: %.2f cm\n", fResult);
        stopSensor(pdpPlarailData);
        stopTrain(pdpPlarailData);
        pdpPlarailData->iIsTrainRunning = TRAIN_STOPPING;

        printf("コマンドを入力してください (start/stop/exit): "); //コマンド入力用printfを再表示
        fflush(stdout); //出力バッファを空にする
    }
}

void outputLog(char cMsg[]){
    printf("%s\n",cMsg);
}

bool setGpio(PLARAIL_DATA *pdpPlarailData)
{
    // ラズパイからの出力の設定
    int iFlgOut = 0;
    int iFlgIn = 0;
    
    // 測距センサーのTRIGGERの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimOutput(pdpPlarailData->iHndl, iFlgOut, TRIG, LG_LOW))
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }
    
    // リレーのSIGの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioSetAlertsFunc(pdpPlarailData->iHndl, iFlgOut, SIG, LG_LOW))
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }

    // 測距センサーのECHOの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimInput(pdpPlarailData->iHndl, iFlgIn, ECHO))
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("GPIOを設定しました");
    return FUNC_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#include <lgpio.h>

#define CHIPSET (0) //ラズパイのGPIO番号
#define TRIG (23) //測距センサーのTRIGGER
#define ECHO (24) //測距センサーのECHO
#define SIG (25) //リレーのSIG
#define MAG (26) //磁気センサのMAG
#define FUNC_SUCCESS (true) //ユーザ関数の実行結果（成功）
#define FUNC_FAILURE (false) //ユーザ関数の実行結果（失敗）
#define COMMAND_COMPLETE_MATCH (0) //strcmpの比較結果
#define TRAIN_STOPPING (0) //列車の状態（停車中）
#define TRAIN_RUNNING (1) //列車の状態（動作中）
#define AEBS_DISTANCE (15.0F) //自動停止までの距離
#define NO_NOTIFY_HANDLE (-1) //通知ハンドルの初期値（lgGpioSetAlertFuncの第5引数）
#define USEC_PER_SEC (1000000L) //1秒のマイクロ秒
#define SONIC_SPEED (0.034) //音速をcm/usに変換
#define HALF_SONIC_SPEED (SONIC_SPEED / 2) //片道分の距離計算
#define WAIT_TIME_FOR_TRIG (10) //TRIG信号を送信し続けるための待ち時間
#define WAIT_TIME_FOR_MEASURE (25000) //測距ループを行うための待ち時間
#define WAIT_TIME_FOR_DEPARTURE (1000000) //駅から離れるまでの待ち時間
#define USER_INPUT_DATA_SIZE (16) //ユーザ入力データのサイズ

typedef struct {
    int iHndl; //GPIOハンドル
    int iIsTrainRunning; //列車の動作状態
    pthread_t *ppMeasureDistanceId; //測距用スレッドのスレッド番号
    pthread_t *ppMagSensorId; //磁気センサ用スレッドのスレッド番号
    float fDistanceCalculationResult; //距離計算結果
} PLARAIL_DATA;

bool setGpio(PLARAIL_DATA *pdpPlarailData);
bool startTrain(PLARAIL_DATA *pdpPlarailData);
bool stopTrain(PLARAIL_DATA *pdpPlarailData);
bool startSensor(PLARAIL_DATA *pdpPlarailData);
bool stopSensor(PLARAIL_DATA *pdpPlarailData);
void *measureDistance(void *vpPlarailData);
void *measureMag(void *vpPlarailData);
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpPlarailData);
void outputLog(char cMsg[]);

int main(void)
{
    //構造体の初期化
    PLARAIL_DATA pdPlarailData = {0, 0, NULL,NULL,0};
    pdPlarailData.iHndl = lgGpiochipOpen(CHIPSET);
    pdPlarailData.iIsTrainRunning = TRAIN_STOPPING;

    //変数の宣言
    char cUserInput[USER_INPUT_DATA_SIZE]; //コマンド入力用配列
    int iTrash = 0; //バッファクリア用変数

    for(int i = 0; i < USER_INPUT_DATA_SIZE; i++)
    {
        cUserInput[i] = '\0';
    }
	
	printf("【電車でGO】");
	
    //GPIOの設定が失敗したらプログラム強制終了
    if (FUNC_FAILURE == setGpio(&pdPlarailData))
    {
        goto FAILURE;
    }

    //コマンド入力はユーザがexitを入力するまで繰り返す
    while(1)
    {
        outputLog("\nコマンドを入力してください (start/stop/exit): ");
        scanf("%15s", cUserInput);
        while (((iTrash = getchar()) != '\n') && (EOF != iTrash)); //バッファクリア

        //コマンドstartの場合
        if (COMMAND_COMPLETE_MATCH == strcmp(cUserInput,"start"))
        {
            //列車が停車していない場合
            if (TRAIN_STOPPING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車は停車していません");
                continue;
            }

            //測距センサの起動に失敗した場合
            if(FUNC_FAILURE == startSensor(&pdPlarailData))
            {
                goto FAILURE;
            }

            //列車の発車に失敗した場合
            if(FUNC_FAILURE == startTrain(&pdPlarailData))
            {
                goto FAILURE;
            }
            
            //列車が停止しておらず、測距センサの起動に成功し、列車の発車に成功した場合
            pdPlarailData.iIsTrainRunning = TRAIN_RUNNING;
        }
        //コマンドstopの場合
        else if (COMMAND_COMPLETE_MATCH == strcmp(cUserInput,"stop"))
        {
            //列車が発車していない場合
            if (TRAIN_RUNNING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車は発車していません");
                continue;
            }

            //測距センサの停止に失敗した場合
            if(FUNC_FAILURE == stopSensor(&pdPlarailData))
            {
                goto FAILURE;
            }
            
            //列車の停止に失敗した場合
            if(FUNC_FAILURE == stopTrain(&pdPlarailData))
            {
                goto FAILURE;
            }
            
            //列車が発車しておらず、測距センサの停止に成功し、列車の停止に成功した場合
            pdPlarailData.iIsTrainRunning = TRAIN_STOPPING;
        }
        //コマンドexitの場合
        else if ((COMMAND_COMPLETE_MATCH == strcmp(cUserInput,"exit")))
        {
            //列車が停車していない場合
            if (TRAIN_STOPPING != pdPlarailData.iIsTrainRunning)
            {
                outputLog("列車を停車させてください");
            }
            //列車が停車している場合
            else
            {
                stopSensor(&pdPlarailData); //センサの停止
                outputLog("プログラムを終了します");
                break;
            }
        }
        //コマンド名がstart、stop、exitと一致しない場合
        else
        {
            outputLog("コマンド名が違います");
        }
    }

    //プログラムがexitコマンドで終了した場合
    lgGpiochipClose(pdPlarailData.iHndl);
    return EXIT_SUCCESS;

//プログラムが異常終了した場合
FAILURE:
    stopSensor(&pdPlarailData); //センサの停止
    lgGpiochipClose(pdPlarailData.iHndl);
    return EXIT_FAILURE;
}

//列車の発車
bool startTrain(PLARAIL_DATA *pdpPlarailData)
{
    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        return FUNC_FAILURE;
    }
    
    //列車の発車に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioWrite(pdpPlarailData->iHndl, SIG, LG_HIGH))
    {
        outputLog("列車の発車に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("列車を発車させました");
    return FUNC_SUCCESS;
}

//列車の停止
bool stopTrain(PLARAIL_DATA *pdpPlarailData)
{
    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        return FUNC_FAILURE;
    }

    //列車の停止に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioWrite(pdpPlarailData->iHndl, SIG, LG_LOW))
    {
        outputLog("列車の停止に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("列車を停止させました");
    return FUNC_SUCCESS;
}

//センサの起動
bool startSensor(PLARAIL_DATA *pdpPlarailData)
{
    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        return FUNC_FAILURE;
    }

    //ppMeasureDistanceIdに測距用スレッドのスレッド番号を格納
    pdpPlarailData->ppMeasureDistanceId = lgThreadStart(measureDistance, pdpPlarailData);

    //測距用スレッドのスレッド番号がNULLの場合はエラー出力
    if (NULL == pdpPlarailData->ppMeasureDistanceId) {
        outputLog("測距用スレッドのスレッド番号設定に失敗しました");
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
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimAlert(pdpPlarailData->iHndl, LG_SET_PULL_DOWN, LG_RISING_EDGE, ECHO, NO_NOTIFY_HANDLE))
    {
        outputLog("測距センサのアラート設定に失敗しました");
        stopSensor(pdpPlarailData);
        return FUNC_FAILURE;
    }

    outputLog("測距センサを起動しました");

    //ppMagSensorIdに磁気センサ用スレッドのスレッド番号を格納
    pdpPlarailData->ppMagSensorId = lgThreadStart(measureMag, pdpPlarailData);

    //磁気センサ用スレッドのスレッド番号がNULLの場合はエラー出力
    if (NULL == pdpPlarailData->ppMagSensorId) {
        outputLog("磁気センサ用スレッドのスレッド番号設定に失敗しました");
        return FUNC_FAILURE;
    }

    outputLog("磁気センサを起動しました");
    return FUNC_SUCCESS;
}

//センサの停止
bool stopSensor(PLARAIL_DATA *pdpPlarailData)
{
    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        return FUNC_FAILURE;
    }

    //測距センサの停止に失敗した場合はエラー出力
    if(COMMAND_COMPLETE_MATCH != lgGpioSetAlertsFunc(pdpPlarailData->iHndl, ECHO, NULL, NULL))
    {
        outputLog("測距センサの停止に失敗しました");
        return FUNC_FAILURE;
    }

    //測距用スレッドの停止
    lgThreadStop(pdpPlarailData->ppMeasureDistanceId);

    //磁気センサ用スレッドの停止
    lgThreadStop(pdpPlarailData->ppMagSensorId);
    
    outputLog("測距センサと磁気センサを停止しました");
    return FUNC_SUCCESS;
}


//測距センサのスレッド（TRIG信号の送信）
void *measureDistance(void *vpPlarailData)
{
    //PLARAIL_DATA構造体のポインタを格納
    PLARAIL_DATA *pdpPlarailData = (PLARAIL_DATA *)vpPlarailData;

    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        lgGpiochipClose(pdpPlarailData->iHndl);
        exit(EXIT_FAILURE);
    }

    outputLog("測距センサのスレッドを開始しました");

    //測距センサ起動中は0.25秒おきに10マイクロ秒間TRIGをHIGHにする
    while(1)
    {
        //測距センサの異常を検知した場合
        if(LG_HIGH == lgGpioRead(pdpPlarailData->iHndl, TRIG))
        {
            outputLog("測距センサの異常を検知しました");
            lgGpioWrite(pdpPlarailData->iHndl, TRIG, LG_LOW);
        } 

        continue;

        //測距センサが正常な場合
        lgGpioWrite(pdpPlarailData->iHndl, TRIG, LG_HIGH);
        usleep(WAIT_TIME_FOR_TRIG);
        lgGpioWrite(pdpPlarailData->iHndl, TRIG, LG_LOW);
        
        //列車が障害物検知時停止距離以下になった場合
        if (AEBS_DISTANCE >= pdpPlarailData->fDistanceCalculationResult)
        {
            if(pdpPlarailData->iIsTrainRunning == TRAIN_RUNNING){
                printf("\n障害物を検知しました。\n列車までの距離: %.2f cm", pdpPlarailData->fDistanceCalculationResult);

                //列車の停止に失敗した場合
                if(FUNC_FAILURE == stopTrain(pdpPlarailData))
                {
                    lgGpiochipClose(pdpPlarailData->iHndl);
                    exit(EXIT_FAILURE);
                }

                //センサの停止に成功し、列車の停止に成功した場合
                pdpPlarailData->iIsTrainRunning = TRAIN_STOPPING;

                outputLog("\nコマンドを入力してください (start/stop/exit): "); //コマンド入力用printfを再表示
                fflush(stdout); //出力バッファを空にする
            }
        }
        else
        {
            if(pdpPlarailData->iIsTrainRunning == TRAIN_STOPPING){

            //列車の発車に失敗した場合
            if(FUNC_FAILURE == startTrain(pdpPlarailData))
            {
                lgGpiochipClose(pdpPlarailData->iHndl);
                exit(EXIT_FAILURE);
            }
            
            pdpPlarailData->iIsTrainRunning = TRAIN_RUNNING;

            outputLog("\nコマンドを入力してください (start/stop/exit): "); //コマンド入力用printfを再表示
            fflush(stdout); //出力バッファを空にする
            }
        }
        usleep(WAIT_TIME_FOR_MEASURE);
    }
}

//磁気センサのスレッド（MAG信号の監視）
void *measureMag(void *vpPlarailData)
{
    //PLARAIL_DATA構造体のポインタを格納
    PLARAIL_DATA *pdpPlarailData = (PLARAIL_DATA *)vpPlarailData;

    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        lgGpiochipClose(pdpPlarailData->iHndl);
        exit(EXIT_FAILURE);
    }

    outputLog("磁気センサのスレッドを開始しました");

    usleep(WAIT_TIME_FOR_DEPARTURE); //駅から離れるまでの待ち時間

    while(1)
    {
        // 磁石に反応した場合
        if(pdpPlarailData->iIsTrainRunning == TRAIN_RUNNING)
        {
            if(LG_LOW == lgGpioRead(pdpPlarailData->iHndl, MAG))
            {
                outputLog("駅に到着しました");

                //列車の停止に失敗した場合
                if(FUNC_FAILURE == stopTrain(pdpPlarailData))
                {
                    lgGpiochipClose(pdpPlarailData->iHndl);
                    exit(EXIT_FAILURE);
                }

                //センサの停止に成功し、列車の停止に成功した場合
                pdpPlarailData->iIsTrainRunning = TRAIN_STOPPING;
                
                outputLog("\nコマンドを入力してください (start/stop/exit): "); //コマンド入力用printfを再表示
                fflush(stdout); //出力バッファを空にする

                //測距センサの停止に失敗した場合
                if(FUNC_FAILURE == stopSensor(pdpPlarailData))
                {
                    lgGpiochipClose(pdpPlarailData->iHndl);
                    exit(EXIT_FAILURE);
                }
            }
        }

        usleep(WAIT_TIME_FOR_MEASURE);
    }
}

//ECHO信号の受信
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpPlarailData)
{
    //PLARAIL_DATA構造体のポインタを格納
    PLARAIL_DATA *pdpPlarailData = (PLARAIL_DATA *)vpPlarailData;

    //測距計算用の変数
    struct timeval start = {0}, end = {0};
    long lUsec = 0;
    //float fResult = 0;

    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        lgGpiochipClose(pdpPlarailData->iHndl);
        exit(EXIT_FAILURE);
    }

    //ECHOがHIGHの間の時間を計測
    gettimeofday(&start, NULL); //開始時刻
    while(LG_HIGH == lgGpioRead(pdpPlarailData->iHndl, ECHO)); //ECHOがHIGHの間待機
    gettimeofday(&end, NULL); //終了時刻

    //測距計算
    lUsec = (end.tv_sec - start.tv_sec) * USEC_PER_SEC + (end.tv_usec - start.tv_usec);
    pdpPlarailData->fDistanceCalculationResult = lUsec * HALF_SONIC_SPEED;
}

//ログ出力
void outputLog(char cMsg[]){
    printf("\n%s",cMsg);
}

//GPIOの設定
bool setGpio(PLARAIL_DATA *pdpPlarailData)
{
    // ラズパイからの出力の設定
    int iFlgOut = 0;
    int iFlgIn = 0;

    if(NULL == pdpPlarailData)
    {
        outputLog("引数pdpPlarailDataの値エラー(NULL)");
        return FUNC_FAILURE;
    }
    
    // 測距センサーのTRIGGERの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimOutput(pdpPlarailData->iHndl, iFlgOut, TRIG, LG_LOW))
    {
        outputLog("GPIOの設定に失敗しました(TRIG)");
        return FUNC_FAILURE;
    }
    
    // リレーのSIGの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimOutput(pdpPlarailData->iHndl, iFlgOut, SIG, LG_LOW))
    {
        outputLog("GPIOの設定に失敗しました(SIG)");
        return FUNC_FAILURE;
    }

    // 測距センサーのECHOの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimInput(pdpPlarailData->iHndl, iFlgIn, ECHO))
    {
        outputLog("GPIOの設定に失敗しました(ECHO)");
        return FUNC_FAILURE;
    }

    // 磁気センサーのMAGの設定
    if(COMMAND_COMPLETE_MATCH != lgGpioClaimInput(pdpPlarailData->iHndl, LG_SET_PULL_UP, MAG))
    {
        outputLog("GPIOの設定に失敗しました(MAG)");
        return FUNC_FAILURE;
    }

    outputLog("GPIOを設定しました");
    return FUNC_SUCCESS;
}

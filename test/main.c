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
#define FUNC_SUCCESS (true) //ユーザ関数の実行結果（成功）
#define FUNC_FAILURE (false) //ユーザ関数の実行結果（失敗）
#define COMMAND_COMPLETE_MATCH (0) //strcmpの比較結果
#define TRAIN_STOPPING (0) //列車の状態（停車中）
#define TRAIN_RUNNING (1) //列車の状態（動作中）
#define AEBS_DISTANCE (10.0F) //自動停止までの距離
#define NO_NOTIFY_HANDLE (-1) //通知ハンドルの初期値（lgGpioSetAlertFuncの第5引数）
#define USEC_PER_SEC (1000000L) //1秒のマイクロ秒
#define SONIC_SPEED (0.034) //音速をcm/usに変換
#define HALF_SONIC_SPEED (SONIC_SPEED / 2) //片道分の距離計算
#define WAIT_TIME_FOR_TRIG (10) //TRIG信号を送信し続けるための待ち時間
#define WAIT_TIME_FOR_MEASURE (250000) //測距ループを行うための待ち時間
#define USER_INPUT_DATA_SIZE (16) //ユーザ入力データのサイズ

typedef struct {
    int iHndl; //GPIOハンドル
    int iIsTrainRunning; //列車の動作状態
    pthread_t *ppMeasureDistanceId; //測距用スレッドのスレッド番号
} PLARAIL_DATA;

bool setGpio(PLARAIL_DATA *pdpPlarailData);
bool startTrain(PLARAIL_DATA *pdpPlarailData);
bool stopTrain(PLARAIL_DATA *pdpPlarailData);
bool startSensor(PLARAIL_DATA *pdpPlarailData);
bool stopSensor(PLARAIL_DATA *pdpPlarailData);
void *measureDistance(void *vpPlarailData);
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpPlarailData);
void outputLog(char cMsg[]);

int main(void)
{
    printf("==== 引数チェックドライバ ====\n\n");

    // setGpio()
    printf("[テスト] setGpio(NULL)\n");
    if (!setGpio(NULL)) {
        printf("NULLチェック成功\n\n");
    } else {
        printf("NULLチェック失敗");
    }

    // startTrain()
    printf("[テスト] startTrain(NULL)\n");
    if (!startTrain(NULL)) {
        printf("NULLチェック成功\n\n");
    } else {
        printf("NULLチェック失敗\n\n");
    }

    // stopTrain()
    printf("[テスト] stopTrain(NULL)\n");
    if (!stopTrain(NULL)) {
        printf("NULLチェック成功\n\n");
    } else {
        printf("NULLチェック失敗\n\n");
    }

    // startSensor()
    printf("[テスト] startSensor(NULL)\n");
    if (!startSensor(NULL)) {
        printf("NULLチェック成功\n\n");
    } else {
        printf("NULLチェック失敗\n\n");
    }

    // stopSensor()
    printf("[テスト] stopSensor(NULL)\n");
    if (!stopSensor(NULL)) {
        printf("NULLチェック成功\n\n");
    } else {
        printf("NULLチェック失敗\n\n");
    }

    printf("[テスト] measureDistance(NULL), catchEcho(...)はスレッド・割り込み処理なためスキップ\n\n");

    printf("==== 引数チェックドライバ終了 ====\n");
    printf("Enterを押すと終了\n");
    getchar(); // �^�[�~�i�����������Ȃ��悤�ɑҋ@

    return 0;
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

//測距センサの起動
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
    return FUNC_SUCCESS;
}

//測距センサの停止
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
    
    outputLog("測距センサを停止しました");
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
        //測距センサが正常な場合
        else 
        {
            lgGpioWrite(pdpPlarailData->iHndl, TRIG, LG_HIGH);
            usleep(WAIT_TIME_FOR_TRIG);
            lgGpioWrite(pdpPlarailData->iHndl, TRIG, LG_LOW);
            usleep(WAIT_TIME_FOR_MEASURE);
        }
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
    float fResult = 0;

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
    fResult = lUsec * HALF_SONIC_SPEED;
    
    //列車が障害物検知時停止距離以下になった場合
    if (AEBS_DISTANCE >= fResult)
    {
		printf("\n障害物を検知しました。\n列車までの距離: %.2f cm\n", fResult);
        
        //測距センサの停止に失敗した場合
        if(FUNC_FAILURE == stopSensor(pdpPlarailData)){
            lgGpiochipClose(pdpPlarailData->iHndl);
            exit(EXIT_FAILURE);
        }

        //列車の停止に失敗した場合
        if(FUNC_FAILURE == stopTrain(pdpPlarailData)){
            lgGpiochipClose(pdpPlarailData->iHndl);
            exit(EXIT_FAILURE);
        }

        //測距センサの停止に成功し、列車の停止に成功した場合
        pdpPlarailData->iIsTrainRunning = TRAIN_STOPPING;

        printf("コマンドを入力してください (start/stop/exit): "); //コマンド入力用printfを再表示
        fflush(stdout); //出力バッファを空にする
    }
}

//ログ出力
void outputLog(char cMsg[]){
    printf("%s\n",cMsg);
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

    outputLog("GPIOを設定しました");
    return FUNC_SUCCESS;
}

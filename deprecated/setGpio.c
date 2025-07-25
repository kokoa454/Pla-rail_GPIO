#include "Plarail_GPIO.h"

bool setGpio(int iHndl)
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
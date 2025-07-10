#include "Plarail_GPIO.h"

bool setGpio(int iHndl)
{
    // ラズパイからの入力・出力の設定
    int iFlgIn = 0;
    int iFlgOut = 0;
    
    // 測距センサーのTRIGGERの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, TRIG, LG_LOW) == LGGPIO_FAILURE)
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }
    
    // リレーのSIGの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, SIG, LG_LOW) == LGGPIO_FAILURE)
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }

    // 測距センサーのECHOの設定
    if(lgGpioClaimInput(iHndl, iFlgIn, ECHO) == LGGPIO_FAILURE)
    {
        outputLog("GPIOの設定に失敗しました");
        return FUNC_FAILURE;
    }
    

    outputLog("GPIOを設定しました");
    return FUNC_SUCCESS;
}
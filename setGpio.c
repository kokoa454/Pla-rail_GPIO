#include "Plarail_GPIO.h"

bool setGpio(int iHndl){
    // ラズパイからの入力・出力の設定
    int iFlgIn = 0;
    int iFlgOut = 0;
    
    // 測距センサーのTRIGGERの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, TRIG, LG_LOW) != 0){
        outputLog("GPIOの設定に失敗しました");
        return false;
    }
    
    // リレーのSIGの設定
    if(lgGpioClaimOutput(iHndl, iFlgOut, SIG, LG_LOW) != 0){
        outputLog("GPIOの設定に失敗しました");
        return false;
    }

    // 測距センサーのECHOの設定
    if(lgGpioClaimInput(iHndl, iFlgIn, ECHO) != 0){
        outputLog("GPIOの設定に失敗しました");
        return false;
    }
    

    outputLog("GPIOを設定しました");
    return true;
}
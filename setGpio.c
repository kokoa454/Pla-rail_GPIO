#include "Plarail_GPIO.h"

bool setGpio(int iHndl){
    int iFlgIn = 0;
    int iFlgOut = 0;
    
    if(lgGpioClaimOutput(iHndl, TRIG, &iFlgIn, &iFlgOut) < 0){
        return false;
    }
    
    if(lgGpioClaimOutput(iHndl, SIG, &iFlgIn, &iFlgOut) < 0){
        return false;
    }

    if(lgGpioClaimInput(iHndl, ECHO, &iFlgIn, &iFlgOut) < 0){
        return false;
    }
    
    return true;
}
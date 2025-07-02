#include "Plarail_GPIO.h"

void startTrain(int iHndl){
    lgGpioWrite(iHndl, SIG, LG_HIGH);
    outputLog("列車を発車させました");
    return;
}
#ifndef PLARAIL_GPIO_H
#define PLARAIL_GPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define CHIPSET 0
#define TRIG 23
#define ECHO 24
#define SIG 25
#define FUNC_SUCCESS true
#define FUNC_FAILURE false

bool setGpio(int iHndl);
void startTrain(int iHndl);
void stopTrain(int iHndl);
bool startSensor(int iHndl);
bool stopSensor(int iHndl);
void *measureDistance(void *vpHndl);
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpHndl);
void outputLog(char cMsg[]);

extern pthread_t *ppMeasureDistanceId;

#endif

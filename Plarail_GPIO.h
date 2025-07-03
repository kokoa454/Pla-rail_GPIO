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

#define CHIPSET 4
#define TRIG 23
#define ECHO 24
#define SIG 25

struct timeval start, end;

bool setGpio();
void startTrain();
void stopTrain();
bool startSensor(int *piMeasureDistance);
bool stopSensor(int iMeadureDistance);
void *measureDistance(void *vpUserdata);
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpUserdata);
void outputLog(char cMsg[]);

extern iHndl;

#endif

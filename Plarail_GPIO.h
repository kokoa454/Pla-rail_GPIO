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
#include <pthread.h>

#define TRIG 23
#define ECHO 24
#define SIG 25

struct timeval start, end;

bool setGpio(int iHndl);
void startTrain(int iHndl);
void stopTrain(int iHndl);
bool startSensor(int iHndl);
bool stopSensor(int iHndl);
void *measureDistance(void *vpArg);
void catchEcho(int iNotification, lgGpioAlert_p lgpGpioinfo, void *vpUserdata);
void outputLog(char cMsg[]);

#endif

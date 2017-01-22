/*
 * BellManager.h
 *
 *  Created on: 06/apr/2015
 *      Author: giovanni
 */

#ifndef BELLMANAGER_H_
#define BELLMANAGER_H_

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#define RUNNING_DIR "/tmp"
#define LOCK_FILE   "/tmp/campanaro.lock"
#define LOG_FILE    "/tmp/campanaro.log"

#define DLY 1
#define WEEKDAY 7
#define MAX_EVENT 4

struct DaySt
{
    int nEvent;
    struct rtc_time messa[MAX_EVENT];
};

struct BellHnd
{
    /* Hw Bell ID linket to GPIO reference*/
    int IDBellHamH;
    int IDBellHamHH;
    int IDBellA;
    int IDBellB;
};

struct BellConfigurationSt
{
    /* functional range */
    int RingTo;
    int RingFrom;

    /* Relè action disable*/
    int DisableBell;

    /* logic status */
    int HoursEnable;
    int HalfEnable;
    int QuarterEnable;

    /* deley between ring*/
    int DelayTime_0; /* delay in on - off  when ring Hours */
    int DelayTime_1; /* delay in on - off  when ring Messa */
    int DelayTime_2; /* delay in on - off  when ring Cenno -15 */
    int DelayTime_3; /* delay in on - off  when ring Cenno -30 */

    /* daily program */
    struct DaySt Day[WEEKDAY];
};

struct rtc_time setNextQuarterPastHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextQuarterToHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextHalfHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextHourAlarm(struct rtc_time rtc_tm);

void BellManager(int, struct BellConfigurationSt,struct BellHnd *Bell);
int  RTCinit(void);
void AlarmInit(void);
void IoInit(struct BellHnd *Bell);

void playBell(int*, int );
void playMessa(int*, int );
void playMessaCenno(int* , int );

int CheckMessa(struct rtc_time *,struct BellConfigurationSt,struct BellHnd *Bell);
void SetAlarm(int);
void UnSetAlarm(int);

void log_message(char*, char*);



#endif /* BELLMANAGER_H_ */

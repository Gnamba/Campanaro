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
#define LOCK_FILE   "campanaro.lock"
#define LOG_FILE    "campanaro.log"

#define DLY 1

struct BellConfigurationSt
{
    int RingTo;
    int RingFrom;

    /* Relè action disable*/
    int DisableBell;

    /* logic staus */
    int HoursEnable;
    int HalfEnable;
    int QuarterEnable;

    struct rtc_time messa;

};

struct rtc_time setNextQuarterPastHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextQuarterToHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextHalfHourAlarm(struct rtc_time rtc_tm);
struct rtc_time setNextHourAlarm(struct rtc_time rtc_tm);


void BellManager(int, struct BellConfigurationSt);
int  RTCinit(void);
void AlarmInit(void);
void IoInit(void);

void playBell(int*, unsigned char);
void SetAlarm(int);
void UnSetAlarm(int);

void log_message(char*, char*);



#endif /* BELLMANAGER_H_ */

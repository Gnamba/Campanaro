/*
 * BellManager.c
 *
 *  Created on: 06/apr/2015
 *      Author: giovanni
 */

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "BellManager.h"
#include "gpio-utils.h"

static const char default_rtc[] = "/dev/rtc0";

struct rtc_time rtc_tm;
struct rtc_time ring_tm;
const char *rtc = default_rtc;

static uint alarm_checked = 0;

/*Global Variable timer rtc */
int i;
int retval;
int irqcount = 0;
unsigned long data;

int  RTCinit(void)
{
    int fd;

    /* apre la comunicazione con il device RTC */
    fd = open(rtc, O_RDONLY);
    if (fd == -1)
    {
        perror(rtc);
        exit(errno);
    }

    /* Read the RTC time/date */
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1)
    {
        perror("RTC_RD_TIME ioctl");
        exit(errno);
    }

    fprintf(stderr, "\n\nCurrent RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    return fd;
}

void AlarmInit(void)
{
    /*initialize alarm*/
    /* allarme ore in punto  o 30 minuti*/
    if (rtc_tm.tm_min < 15)
    {
        ring_tm = setNextQuarterPastHourAlarm(rtc_tm);
    }
    else if (rtc_tm.tm_min < 30)
    {
        ring_tm = setNextHalfHourAlarm(rtc_tm);
    }
    else if (rtc_tm.tm_min < 45)
    {
        ring_tm = setNextQuarterToHourAlarm(rtc_tm);
    }
    else
    {
        ring_tm = setNextHourAlarm(rtc_tm);
    }
}

void BellManager(int fd, struct BellConfigurationSt BellConf, struct BellHnd *Bell)
{
    int JustRing = 0;

    SetAlarm(fd);

    //log_message(LOG_FILE, "SetAlarm executed");

    /* Read the current alarm settings */
    retval = ioctl(fd, RTC_ALM_READ, &ring_tm);
    if (retval == -1)
    {
        log_message(LOG_FILE, "RTC_ALM_READ ioctl error");
        perror("RTC_ALM_READ ioctl");
        exit(errno);
    }

    fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n", ring_tm.tm_hour,
            ring_tm.tm_min, ring_tm.tm_sec);

    fprintf(stderr, "Waiting for alarm...\r\n");
    fflush(stderr);

//    /* Enable alarm interrupts */
//    retval = ioctl(fd, RTC_AIE_ON, 0);
//    if (retval == -1)
//    {
//        log_message(LOG_FILE, "RTC_AIE_ON ioctl error");
//        perror("RTC_AIE_ON ioctl");
//        exit(errno);
//    }

    /* Enable every second interrupts */
    retval = ioctl(fd, RTC_PIE_ON, 0);
    if (retval == -1)
    {
        log_message(LOG_FILE, "RTC_PIE_ON ioctl error");
        perror("RTC_AIE_ON ioctl");
        exit(errno);
    }

    /* This blocks until the alarm ring causes an interrupt */
    retval = read(fd, &data, sizeof(unsigned long));
    if (retval == -1)
    {
        log_message(LOG_FILE, "RTC read error");
        perror("read");
        exit(errno);
    }
    irqcount++;
    //log_message(LOG_FILE, "okay! Alarm rang.");

    /* Read the RTC time/date */
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1)
    {
        log_message(LOG_FILE, "RTC_RD_TIME ioctl error");
        perror("RTC_RD_TIME ioctl");
        exit(errno);
    }
    /* reset alarm only whe the min 00 is passed*/
    if(rtc_tm.tm_min != 00 && rtc_tm.tm_min != 15 && rtc_tm.tm_min != 30 && rtc_tm.tm_min != 45)
    {
        alarm_checked = 0;
    }

    /*check alarm if it set*/
    if (rtc_tm.tm_min == 00 && alarm_checked == 0)
    {
        fprintf(stderr, "It's %02d:%02d:%02d. Ring From:%d to:%d enable:%d \n",
                ring_tm.tm_hour,
                ring_tm.tm_min,
                ring_tm.tm_sec,
                BellConf.RingFrom,BellConf.RingTo,
                BellConf.HoursEnable);

        JustRing = CheckMessa(&rtc_tm,BellConf,Bell);

        /* check the No Sound Zone*/
        if(ring_tm.tm_hour >= BellConf.RingFrom && ring_tm.tm_hour <= BellConf.RingTo &&
                BellConf.HoursEnable && JustRing == 0)
        {
            for (i = 0;i< (ring_tm.tm_hour > 12 ?ring_tm.tm_hour - 12 : ring_tm.tm_hour); i++)
            {
                /*play bells TODO support ring to 00h if necessary*/
                playBell(Bell->IDBellHamH, BellConf.DelayTime_0);
                log_message(LOG_FILE, "Ora piena.");
            }
        }
        /* */
        alarm_checked =1;
        /*set next alarm*/
        ring_tm = setNextQuarterPastHourAlarm(rtc_tm);
    }
    if (rtc_tm.tm_min == 15 && alarm_checked == 0)
    {
        fprintf(stderr, "It's %02d:%02d:%02d. Ring From:%d to:%d enable:%d \n",
                ring_tm.tm_hour,
                ring_tm.tm_min,
                ring_tm.tm_sec,
                BellConf.RingFrom,BellConf.RingTo,
                BellConf.QuarterEnable);

        JustRing = CheckMessa(&rtc_tm,BellConf,Bell);

        /* check the No Sound Zone*/
        if(ring_tm.tm_hour >= BellConf.RingFrom && ring_tm.tm_hour <= BellConf.RingTo &&
                BellConf.QuarterEnable && JustRing == 0)
        {
            /*play bells*/
            playBell(Bell->IDBellHamHH, BellConf.DelayTime_0);
            log_message(LOG_FILE, "Quarto d'ora.");
            /*set next alarm*/
        }
        alarm_checked =1;
        ring_tm = setNextHalfHourAlarm(rtc_tm);
    }
    if (rtc_tm.tm_min == 30 && alarm_checked == 0)
    {
        fprintf(stderr, "It's %02d:%02d:%02d. Ring From:%d to:%d enable:%d \n",
                ring_tm.tm_hour,
                ring_tm.tm_min,
                ring_tm.tm_sec,
                BellConf.RingFrom,BellConf.RingTo,
                BellConf.HalfEnable);

        JustRing = CheckMessa(&rtc_tm,BellConf,Bell);

        /* check the No Sound Zone*/
        if(ring_tm.tm_hour >= BellConf.RingFrom && ring_tm.tm_hour <= BellConf.RingTo &&
                BellConf.HalfEnable && JustRing == 0)
        {
            /*play bells*/
            playBell(Bell->IDBellHamHH, BellConf.DelayTime_0);
            log_message(LOG_FILE, "Ora mezza.");
        }
        alarm_checked =1;
        /*set next alarm*/
        ring_tm = setNextQuarterToHourAlarm(rtc_tm);
    }
    if (rtc_tm.tm_min == 45 && alarm_checked == 0)
    {
        fprintf(stderr, "It's %02d:%02d:%02d. Ring From:%d to:%d enable:%d \n",
                ring_tm.tm_hour,
                ring_tm.tm_min,
                ring_tm.tm_sec,
                BellConf.RingFrom,BellConf.RingTo,
                BellConf.QuarterEnable);

        JustRing = CheckMessa(&rtc_tm,BellConf,Bell);

        /* check the No Sound Zone*/
        if(ring_tm.tm_hour >= BellConf.RingFrom && ring_tm.tm_hour <= BellConf.RingTo &&
                BellConf.QuarterEnable && JustRing == 0)
        {
            /*play bells*/
            playBell(Bell->IDBellHamHH, BellConf.DelayTime_0);
            log_message(LOG_FILE, "Quarto d'ora.");
        }
        alarm_checked =1;
        /*set next alarm*/
        ring_tm = setNextHourAlarm(rtc_tm);
    }

    UnSetAlarm(fd);
}

int CheckMessa(struct rtc_time *Tempo,struct BellConfigurationSt BellConf,struct BellHnd *Bell)
{
    int nday;
    int JustRing;

    nday = Tempo->tm_wday;
    JustRing = 0;

    printf("[checkmessa] giorno:%u -sono le %u:%u:%u \n",nday,Tempo->tm_hour,Tempo->tm_min,Tempo->tm_sec);
    for(i=0; i < BellConf.Day[nday].nEvent; i++)
    {
        // check MESSA - ora messa
        if(BellConf.Day[nday].messa[i].tm_hour == Tempo->tm_hour &&
                BellConf.Day[nday].messa[i].tm_min == Tempo->tm_min)
        {
            playMessa(Bell->IDBellA, BellConf.DelayTime_1);
            log_message(LOG_FILE, "Messa");
            printf("[checkmessa] messa giorno:%u -sono le %u:%u:%u \n",nday,Tempo->tm_hour,Tempo->tm_min,Tempo->tm_sec);
            JustRing = 1;
        }
        // check CENNO - mezza ora prima della messa
        else if((BellConf.Day[nday].messa[i].tm_hour == Tempo->tm_hour) &&
                ((BellConf.Day[nday].messa[i].tm_min - 30) == Tempo->tm_min))
        {
            playMessaCenno(Bell->IDBellA, BellConf.DelayTime_2);
            log_message(LOG_FILE, "Messa in 30 min com messa alle ore mezze");
            printf("[checkmessa] Messa in 30 min - giorno:%u -sono le %u:%u:%u \n",nday,Tempo->tm_hour,Tempo->tm_min,Tempo->tm_sec);
            JustRing = 1;
        }
        else if((BellConf.Day[nday].messa[i].tm_hour - 1) == Tempo->tm_hour &&
                ((BellConf.Day[nday].messa[i].tm_min + 30) == Tempo->tm_min))
        {
            playMessaCenno(Bell->IDBellA, BellConf.DelayTime_2);
            log_message(LOG_FILE, "Messa in 30 min com messa alle ore piene");
            printf("[checkmessa] Messa in 30 min - giorno:%u -sono le %u:%u:%u \n",nday,Tempo->tm_hour,Tempo->tm_min,Tempo->tm_sec);
            JustRing = 1;
        }
        // check DOPPIO - un ora prima della messa
        else if(((BellConf.Day[nday].messa[i].tm_hour - 1) == Tempo->tm_hour) &&
                ((BellConf.Day[nday].messa[i].tm_min) == Tempo->tm_min))
        {
            playMessaDoppio(Bell->IDBellA, Bell->IDBellB,BellConf.DelayTime_3);
            log_message(LOG_FILE, "Messa in 60 min");
            printf("[checkmessa] Messa in 60 min - giorno:%u -sono le %u:%u:%u \n",nday,Tempo->tm_hour,Tempo->tm_min,Tempo->tm_sec);
            JustRing = 1;
        }

    }

    return JustRing;
}

void SetAlarm(int fd)
{
    /*Set the alarm*/
    retval = ioctl(fd, RTC_ALM_SET, &ring_tm);
    if (retval == -1)
    {
        if (errno == ENOTTY)
        {
            log_message(LOG_FILE, "Alarm IRQs not supported.");
            fprintf(stderr, "\n...Alarm IRQs not supported.\n");
        }
        log_message(LOG_FILE, "RTC_ALM_SET ioctl error");
        perror("RTC_ALM_SET ioctl");
        exit(errno);
    }
}

void UnSetAlarm(int fd)
{
    int retval;

    /* Disable alarm interrupts */
    retval = ioctl(fd, RTC_PIE_ON, 0);

    if (retval == -1)
    {
        log_message(LOG_FILE, "RTC_AIE_OFF ioctl error");
        perror("RTC_AIE_OFF ioctl");
        exit(errno);
    }
}


struct rtc_time setNextQuarterPastHourAlarm(struct rtc_time rtc_tm)
{
    struct rtc_time ring_tm;
    ring_tm.tm_hour = rtc_tm.tm_hour;
    ring_tm.tm_min = 15;
    ring_tm.tm_sec = 0;
    return ring_tm;
}

struct rtc_time setNextHalfHourAlarm(struct rtc_time rtc_tm)
{
    struct rtc_time ring_tm;
    ring_tm.tm_hour = rtc_tm.tm_hour;
    ring_tm.tm_min = 30;
    ring_tm.tm_sec = 0;
    return ring_tm;
}

struct rtc_time setNextQuarterToHourAlarm(struct rtc_time rtc_tm)
{
    struct rtc_time ring_tm;
    ring_tm.tm_hour = rtc_tm.tm_hour;
    ring_tm.tm_min = 45;
    ring_tm.tm_sec = 0;
    return ring_tm;
}

struct rtc_time setNextHourAlarm(struct rtc_time rtc_tm)
{
    struct rtc_time ring_tm;
    if (rtc_tm.tm_hour < 23)
        ring_tm.tm_hour = rtc_tm.tm_hour + 1;
    else
        ring_tm.tm_hour = 0;
    ring_tm.tm_min = 00;
    ring_tm.tm_sec = 0;
    return ring_tm;
}

void playBell(int *IDbell, int delay)
{
    printf("on IDbell:%u add_IDbell:%u\n",IDbell,&IDbell);
    setChannelState(IDbell, 1);
    usleep(delay);
    printf("off \n");
    setChannelState(IDbell, 0);
    sleep(2);
}


void playMessa(int *IDbell, int duration)
{
    printf("Messa on IDbell:%u  add_IDbell:%u\n",IDbell,&IDbell);
    printf("on \n");
    setChannelState(IDbell, 1);

    sleep(duration);

    printf("off \n");
    setChannelState(IDbell, 0);

}

void playMessaDoppio(int *IDbell0, int* IDbell1, int duration)
{
    printf("doppio on IDbell0:%u IDbell1:%u  add_IDbell0:%u add_IDbell1:%u\n",IDbell0,IDbell1,&IDbell0,&IDbell1);
    printf("on \n");
    setChannelState(IDbell0, 1);
    setChannelState(IDbell1, 1);

    sleep(duration);

    printf("off \n");
    setChannelState(IDbell0, 0);
    setChannelState(IDbell1, 0);

}

void playMessaCenno(int *IDbell, int duration)
{
    printf("cenno on IDbell:%u  add_IDbell:%u\n",IDbell,&IDbell);
    printf("on \n");
    setChannelState(IDbell, 1);

    sleep(duration);

    printf("off \n");
    setChannelState(IDbell, 0);

}

void IoInit(struct BellHnd *hnd)
{
    /*initialize GPIO Bell*/
    hnd->IDBellHamH = bindOutputChannel(7, 5);       //hummer hours
    hnd->IDBellHamHH = bindOutputChannel(7, 3);      //hummer half hours
    hnd->IDBellA = bindOutputChannel(7, 9);          //bell A
    hnd->IDBellB = bindOutputChannel(7, 7);          //bell B

    fprintf(stderr,"[DEBUG] gpio mapping handler %u - %u - %u - %u \n\r",hnd->IDBellHamH,hnd->IDBellHamHH,hnd->IDBellA,hnd->IDBellB );

    if ((hnd->IDBellHamH == -1) || (hnd->IDBellHamHH == -1) || (hnd->IDBellA == -1) || (hnd->IDBellB == -1))
    {
        fprintf(stderr, "\nError: unable to access gpio resource. Exit.\n");
        exit(-1);
    }
}

void log_message(char* filename, char* message)
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[20];
    FILE *logfile;
    logfile = fopen(filename, "a");
    if (!logfile)
        return;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 20, "%d/%m/%Y %H:%M", timeinfo);

    fprintf(logfile, "%s \t%s\n", buffer, message);
    fclose(logfile);
}

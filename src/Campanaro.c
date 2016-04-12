/*
 8/02/2001
 Giovanni Burchietti
 Stefano Burchietti
 */

/*
 To compile:	gcc -s -Wall -Wstrict-prototypes fraMartino.c -o fraMartino
 To run:		./fraMartino
 To test daemon:	ps -ef|grep fraMartino (or ps -aux on BSD systems)
 To test log:	tail -f /tmp/fraMartino.log
 To test signal:	kill -HUP `cat /tmp/fraMartino.lock`
 To terminate:	kill `cat /tmp/fraMartino.lock`
 */

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <resolv.h>


#include "BellManager.h"
#include "GlueSocket.h"
#include "ToolManager.h"

int checkoperativeMode(const void *data, int len,int fd);

void ConfInit(void);

void daemonize();
void signal_handler(int);

struct BellConfigurationSt BellConf;

/* global gpio hnd*/
struct BellHnd Bell;

static int mode = 1;

/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */

int main(int argc, char **argv)
{
    int dev;
    int mode;
    int length = 0;
    char buf[2000];



    fprintf(stderr, "\n...Campanaro is started and run in background.");

    /*initialize daemonize process*/
    //daemonize();

    dev = RTCinit();

    IoInit(Bell);
    AlarmInit();

    init_tool_socket();

    ConfInit();

    log_message(LOG_FILE, "*Started*");

    /*main loop*/
    while (1)
    {
        mode = checkoperativeMode(buf,length,dev);

        printf("manual mode %u running\r\n",mode);

        switch (mode)
        {
        case 1:
            BellManager(dev,BellConf,Bell);
            break;

        case 2:
            printf("manual mode running\r\n");
            toolmng(buf,length,BellConf,Bell);
            break;

        }
    }

    return 0;
}

int checkoperativeMode(const void *data, int len,int fd)
{
    int received;
    int retval;
    int length = 0;
    char *buf;

    buf = (char*) data;

    received = ListenSocket(buf, length);

    if(received == 1)
    {
        /* start manual key*/
        if(buf[0] == 0x22 && buf[1] == 0x22 && buf[2] == 0x22)
        {
            mode = 2;
            log_message(LOG_FILE, "Manual mode Requested");

            /* Disable periodic interrupt interrupts */
            retval = ioctl(fd, RTC_PIE_OFF, 0);
            if (retval == -1)
            {
                log_message(LOG_FILE, "RTC_PIE_OFF ioctl error");
                perror("RTC_PIE_OFF ioctl");
                exit(errno);
            }

            /* Disable Alarm interrupt interrupts */
            retval = ioctl(fd, RTC_AIE_OFF, 0);
            if (retval == -1)
            {
                log_message(LOG_FILE, "RTC_AIE_OFF ioctl error");
                perror("RTC_AIE_OFF ioctl");
                exit(errno);
            }

        }
        /* stop manual key*/
        else if(buf[0] == 0x33 && buf[1] == 0x33 && buf[2] == 0x33)
        {
            mode = 1;
            log_message(LOG_FILE, "Automatic mode Requested");
        }
        length = len;
    }

    return mode;
}


void ConfInit()
{
    /* transitional function*/

    BellConf.DisableBell = 0;

    BellConf.HalfEnable = 1;
    BellConf.QuarterEnable = 0;
    BellConf.HoursEnable = 1;

    BellConf.RingFrom = 6;
    BellConf.RingTo = 23;

    /* deley between ring*/
    BellConf.DelayTime_0 = 1; /* delay in on - off  when ring Hours */
    BellConf.DelayTime_1 = 30; /* delay in on - off  when ring Messa */
    BellConf.DelayTime_2 = 30;  /* delay in on - off  when ring Cenno -15 */
    BellConf.DelayTime_3 = 30; /* delay in on - off  when ring Cenno -30 */

    /* lunedì */
    BellConf.Day[1].nEvent = 2;

    BellConf.Day[1].messa[0].tm_hour = 14;
    BellConf.Day[1].messa[0].tm_min = 00;
    BellConf.Day[1].messa[0].tm_sec = 00;

    BellConf.Day[1].messa[1].tm_hour = 18;
    BellConf.Day[1].messa[1].tm_min = 00;
    BellConf.Day[1].messa[1].tm_sec = 00;

    /* martedì */
    BellConf.Day[2].nEvent = 2;

    BellConf.Day[2].messa[0].tm_hour = 13;
    BellConf.Day[2].messa[0].tm_min = 00;
    BellConf.Day[2].messa[0].tm_sec = 00;

    BellConf.Day[2].messa[1].tm_hour = 18;
    BellConf.Day[2].messa[1].tm_min = 00;
    BellConf.Day[2].messa[1].tm_sec = 00;

    /* ven */
    BellConf.Day[5].nEvent = 2;

    BellConf.Day[5].messa[0].tm_hour = 13;
    BellConf.Day[5].messa[0].tm_min = 00;
    BellConf.Day[5].messa[0].tm_sec = 00;

    BellConf.Day[5].messa[1].tm_hour = 18;
    BellConf.Day[5].messa[1].tm_min = 00;
    BellConf.Day[5].messa[1].tm_sec = 00;

    /* Sab */
    BellConf.Day[6].nEvent = 2;

    BellConf.Day[6].messa[0].tm_hour = 18;
    BellConf.Day[6].messa[0].tm_min = 00;
    BellConf.Day[6].messa[0].tm_sec = 00;

    BellConf.Day[6].messa[1].tm_hour =19 ;
    BellConf.Day[6].messa[1].tm_min = 00;
    BellConf.Day[6].messa[1].tm_sec = 00;

    /* dom */
    BellConf.Day[7].nEvent = 2;

    BellConf.Day[7].messa[0].tm_hour = 10;
    BellConf.Day[7].messa[0].tm_min = 30;
    BellConf.Day[7].messa[0].tm_sec = 00;

    BellConf.Day[7].messa[1].tm_hour = 17;
    BellConf.Day[7].messa[1].tm_min = 00;
    BellConf.Day[7].messa[1].tm_sec = 00;



}


void daemonize()
{
    int i, lfp;
    char str[10];
    if (getppid() == 1)
        return; /* already a daemon */
    i = fork();
    if (i < 0)
        exit(1); /* fork error */
    if (i > 0)
        exit(0); /* parent exits */
    /* child (daemon) continues */
    setsid(); /* obtain a new process group */
    for (i = getdtablesize(); i >= 0; --i)
        close(i); /* close all descriptors */
    i = open("/dev/null", O_RDWR);
    dup(i);
    dup(i); /* handle standart I/O */
    umask(027); /* set newly created file permissions */
    chdir(RUNNING_DIR); /* change running directory */
    lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
    if (lfp < 0)
        exit(1); /* can not open */
    if (lockf(lfp, F_TLOCK, 0) < 0)
        exit(0); /* can not lock */
    /* first instance continues */
    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str)); /* record pid to lockfile */
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */
}



void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGHUP:
        log_message(LOG_FILE, "*Hangup signal catched*");
        break;
    case SIGTERM:
        log_message(LOG_FILE, "*Terminate signal catched*");
        exit(0);
        break;
    }
}

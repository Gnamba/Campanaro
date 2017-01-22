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

int checkoperativeMode(char *data, int *len,int fd);
void ConfInit(struct BellConfigurationSt *BellConf);
void daemonize();
void signal_handler(int);

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
    int length;
    char buf[2000];

    /* global gpio hnd*/
    struct BellConfigurationSt BellConf;
    struct BellHnd Bell;


    //system("hwclock -w --localtime"); /* sync hw and local clock*/

    fprintf(stderr, "\n...Campanaro is started and run in background.");

    /*initialize daemonize process*/
    daemonize();
    ConfInit(&BellConf);

    dev = RTCinit();
    IoInit(&Bell);

    AlarmInit();

    init_tool_socket();


    log_message(LOG_FILE, "*Started*");

    /*main loop*/
    while (1)
    {
        mode = checkoperativeMode(&buf,&length,dev);

        printf("operative mode %u running\r\n",mode);

        switch (mode)
        {
        case 1:
            BellManager(dev,BellConf,&Bell);
            break;

        case 2:
            printf("manual mode running\r\n");
            printf("length:%d buf[0]:%u \r\n",length,buf[0]);
            toolmng(&buf,length,BellConf,&Bell);
            break;

        }
    }

    return 0;
}

int checkoperativeMode(char *data, int *len,int fd)
{
    int retval;
    int lenght_;
    char *bufffer;

    bufffer = data;

    lenght_ = ListenSocket(bufffer);

    if(lenght_ > 0)
    {
        printf ("received %d byte\r\n",lenght_);
        /* start manual key*/
        if(bufffer[0] == 0x22 && bufffer[1] == 0x22 && bufffer[2] == 0x22)
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
        else if(bufffer[0] == 0x33 && bufffer[1] == 0x33 && bufffer[2] == 0x33)
        {
            mode = 1;
            log_message(LOG_FILE, "Automatic mode Requested");
        }
    }

    printf("[DEBUG] len:%d data[0]:%u \r\n",lenght_,data[0]);

    *len = lenght_;

    return mode;
}


void ConfInit(struct BellConfigurationSt *BellConf)
{
    /* transitional function*/

    BellConf->DisableBell = 0;

    BellConf->HalfEnable = 1;
    BellConf->QuarterEnable = 0;
    BellConf->HoursEnable = 1;

    BellConf->RingFrom = 8;
    BellConf->RingTo = 22;

    /* deley between ring*/
    BellConf->DelayTime_0 = 300000; /* uSec delay in on - off  when ring Hours */
    BellConf->DelayTime_1 = 50; /*  uSec delay in on - off  when ring Messa */
    BellConf->DelayTime_2 = 30;  /* uSec delay in on - off  when ring Cenno -30 */
    BellConf->DelayTime_3 = 30; /*  uSec delay in on - off  when ring Doppio -60 */

    /* dom */
    BellConf->Day[0].nEvent = 1;

    BellConf->Day[0].messa[0].tm_hour = 10;
    BellConf->Day[0].messa[0].tm_min = 30;
    BellConf->Day[0].messa[0].tm_sec = 00;

    /* lunedì */
    BellConf->Day[1].nEvent = 1;

    BellConf->Day[1].messa[0].tm_hour = 18;
    BellConf->Day[1].messa[0].tm_min = 00;
    BellConf->Day[1].messa[0].tm_sec = 00;

    /* martedì */
    BellConf->Day[2].nEvent = 1;

    BellConf->Day[2].messa[0].tm_hour = 18;
    BellConf->Day[2].messa[0].tm_min = 00;
    BellConf->Day[3].messa[0].tm_sec = 00;

    /* mercoledì */
    BellConf->Day[3].nEvent = 1;

    BellConf->Day[3].messa[0].tm_hour = 18;
    BellConf->Day[3].messa[0].tm_min = 00;
    BellConf->Day[3].messa[0].tm_sec = 00;

    /* giovedì */
    BellConf->Day[4].nEvent = 1;

    BellConf->Day[4].messa[0].tm_hour = 18;
    BellConf->Day[4].messa[0].tm_min = 00;
    BellConf->Day[4].messa[0].tm_sec = 00;

    /* ven */
    BellConf->Day[5].nEvent = 1;

    BellConf->Day[5].messa[0].tm_hour = 18;
    BellConf->Day[5].messa[0].tm_min = 00;
    BellConf->Day[5].messa[0].tm_sec = 00;

    /* Sab */
    BellConf->Day[6].nEvent = 1;

    BellConf->Day[6].messa[0].tm_hour = 18;
    BellConf->Day[6].messa[0].tm_min = 00;
    BellConf->Day[6].messa[0].tm_sec = 00;


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

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

#include "BellManager.h"

int checkoperativeMode(void);
void ManualManager(void);
void ConfInit(void);

void daemonize();
void signal_handler(int);

struct BellConfigurationSt BellConf;

/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */

int main(int argc, char **argv)
{
    int dev;
    int mode;

    fprintf(stderr, "\n...Campanaro is started and run in background.");

    /*initialize daemonize process*/
    daemonize();

    dev = RTCinit();

    IoInit();
    AlarmInit();

    ConfInit();

    log_message(LOG_FILE, "*Started*");

    /*main loop*/
    while (1)
    {
        /* check on local file (different from conf.ini) written by external tool*/
        /* il problema è che da qui ci passo solo ogni 1/4 d'ora mmmumblemumbele*/
        mode = checkoperativeMode();

        switch (mode)
        {
        case 1:
            BellManager(dev,BellConf);
            break;

        case 2:
            ManualManager();
            break;

        }
    }

    return 0;
}

int checkoperativeMode()
{
    /* transitional function*/
    return 1;
}

void ManualManager()
{
    /* transitional function*/
}

void ConfInit()
{
    /* transitional function*/

    BellConf.DisableBell = 0;

    BellConf.HalfEnable = 1;
    BellConf.QuarterEnable = 1;
    BellConf.HoursEnable = 1;

    BellConf.RingFrom = 6;
    BellConf.RingTo = 23;

    BellConf.messa.tm_hour = 18;
    BellConf.messa.tm_min = 00;
    BellConf.messa.tm_sec = 00;

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

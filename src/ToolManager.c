/*
 * ToolManager.c
 *
 *  Created on: 02 dic 2015
 *      Author: casa
 */
#include <unistd.h>
#include <resolv.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include    "GlueSocket.h"
#include    "BellManager.h"

void toolmng(const void *data, int len, struct BellConfigurationSt BellConf, struct BellHnd Bell)
{
    int i = 0;
    int index,msglen;
    char* buf;
    char outmsg[200];
    int little_Bell = 0;
    int Big_Bell = 0;
    int little_Hummer = 0;
    int Big_Hummer = 0;

    buf = (char*) data;

    printf("received %u byte message\r\n",len);

    /* listen on Output Command response with I/O status*/
    /* send diag board software version cmd request: 0x0004*/
    if((len == 3) && (buf[0] == 0x00) && (buf[1] == 0x04))
    {
        index = 3;      /* start from offset 2 (payload offset init)*/

        /*header*/
        outmsg[0]       = 0x65;
        outmsg[1]       = 0x63;

        /*payload*/
        outmsg[index] = 0x63;
        index++;
        outmsg[index] = 0x6F;
        index++;
        outmsg[index] = 0x6C;

        /*len UDP*/
        msglen = index +1;
        /*len tool MSG*/
        outmsg[2]       = (msglen - 3);

        /* send message */
        (void) sendto (tool.fd, outmsg, msglen, 0, (struct sockaddr *) &tool.dst, sizeof (tool.dst));
    }

    else if((len == 6) && (buf[0] == 0x00) && (buf[1] == 0x05))
    {
        printf("Bell command received \r\n");
        /* received output command*/

        little_Bell     = buf[2];
        Big_Bell        = buf[3];
        little_Hummer   = buf[4];
        Big_Hummer      = buf[5];


        if((little_Hummer == 1 || Big_Hummer == 1 ) && (little_Bell != 1 || Big_Bell != 1 ))
        {
            playBell(&Bell.IDBellHamHH, BellConf.DelayTime_0);
            playBell(&Bell.IDBellHamH, BellConf.DelayTime_0);
        }
        if((little_Bell == 1 || Big_Bell == 1 ) && (little_Hummer != 1 || Big_Hummer != 1 ))
        {
            playMessaCenno(&Bell.IDBellA, BellConf.DelayTime_3);
            playMessaCenno(&Bell.IDBellB, BellConf.DelayTime_3);
        }
        /* prepare&send status output */
        index = 3;      /* start from offset 2 (payload offset init)*/
        /*header*/
        outmsg[0]       = 0x65;
        outmsg[1]       = 0x63;

        /*payload*/
        outmsg[index] = little_Bell;
        index++;
        outmsg[index] = Big_Bell;
        index++;
        outmsg[index] = little_Hummer;
        index++;
        outmsg[index] = Big_Hummer;

        /*len UDP*/
        msglen = index +1;
        /*len tool MSG*/
        outmsg[2]       = (msglen - 3);

        /* send message */
        (void) sendto (tool.fd, outmsg, msglen, 0, (struct sockaddr *) &tool.dst, sizeof (tool.dst));
    }
}

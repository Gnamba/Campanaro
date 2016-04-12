/*
 * GlueServer.c
 *
 *  Created on: 02 nov 2015
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

#define LOCAL_IP      "192.168.0.2"
#define TOOL_PORT     10000

void init_tool_socket(void)
{
    int reuse = 1;

    struct sockaddr_in ip_bound_tool;
    memset (&tool, 0, sizeof (tool));

    /* socket environment initialization for udp connection */
    tool.dst.sin_family = AF_INET;
    inet_aton (LOCAL_IP, &tool.dst.sin_addr);
    tool.dst.sin_port = htons (TOOL_PORT);

    /* init tool socket */
    tool.fd = socket (AF_INET, SOCK_DGRAM, 0);

    if (tool.fd == -1)
    {
        perror ("socket UDP connection()");
        exit (1);
    }

    if (setsockopt(tool.fd,SOL_SOCKET,SO_REUSEADDR, &reuse,sizeof(int)) == -1)
    {
        perror ("setsockopt()");
        exit (1);
    }

    ip_bound_tool = tool.dst;
    ip_bound_tool.sin_addr.s_addr = INADDR_ANY;
    ip_bound_tool.sin_port = htons (TOOL_PORT);
    tool.addr_len = sizeof(struct sockaddr_in);

    if (bind (tool.fd, (struct sockaddr *) &ip_bound_tool, sizeof (ip_bound_tool)) == -1)
    {
        perror ("bind()");
        exit (1);
    }

    fprintf(stderr,"Socket initialization ok \n");
}

int ListenSocket(const void *data, int len)
{
    int nsocks;
    int res;
    struct timeval tv;
    fd_set readfds;

    char *buf;
    buf = (char*) data;

    res = 0;

    FD_ZERO (&readfds);
    FD_SET (tool.fd, &readfds);

    /* don't care about writefds and exceptfds: */
    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    nsocks = tool.fd + 1;

    /* setup select on interface */
    if ((select (nsocks, &readfds, NULL, NULL, &tv) == -1))
    {
        perror ("select()");
        exit (1);
    }

    /* listen on udp socket for tool connection*/
    if (FD_ISSET (tool.fd, &readfds))
    {
        int length = recvfrom (tool.fd, buf, sizeof (buf), 0, (struct sockaddr *)&tool.dst, &tool.addr_len);

        if (length > 0)
        {
            fprintf(stderr, "\n...received tool message.");
            res = 1;
        }

    }

    return res;
}

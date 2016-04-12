/*
 * GlueSocket.h
 *
 *  Created on: 02 nov 2015
 *      Author: casa
 */

#ifndef SRC_SOCKETGLUE_GLUESOCKET_H_
#define SRC_SOCKETGLUE_GLUESOCKET_H_

void init_tool_socket(void);
int ListenSocket(const void *data, int len);;

struct {

    int fd;
    struct sockaddr_in dst;
    uint32_t addr_len;

}tool;


#endif /* SRC_SOCKETGLUE_GLUESOCKET_H_ */

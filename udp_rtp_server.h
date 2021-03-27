/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: udp_rtp_server.h
Description: rtsp头文件 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0  
Modification History:
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/select.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include"public.h"

#ifndef UDP_RTP_SERVER_H
#define UDP_RTP_SERVER_H



//void *UDP_RTP_Server(struct sockaddr_in *client_addr);
void *UDP_RTP_Server(void *the_client_addr);

#endif

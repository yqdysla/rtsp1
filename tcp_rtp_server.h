/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: tcp_rtp_server.h
Description: rtsp头文件 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0  
Modification History:
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/

#include"public.h"

#ifndef TCP_RTP_SERVER_H
#define TCP_RTP_SERVER_H

#pragma pack(1)
typedef struct Tcp_Rtp_Packet
    {
      u_int8  Dollar;
      u_int8  Channel_id;
      u_int16 Length;
   //   Fragment_Payload payload;
   //   RTP_HEAD TCP_RTP_HEAD;
      Fragment_RTPpacket Rtp_packet; 
  }TCP_RTP_PACKET;

#pragma pack()
void *TCP_RTP_Server(void *Thread_Para);

#endif

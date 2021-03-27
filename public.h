/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: public.h
Description: 全局头文件 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0 
Modification History:
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/

#ifndef PUBLIC_H
#define PUBLIC_H

#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/time.h> 
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>
#include<pthread.h>
#include <semaphore.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/select.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <netinet/tcp.h>


#define PROTOCAL_UDP 0
#define PROTOCAL_TCP 1 

typedef unsigned char u_int8;
typedef unsigned short u_int16;
typedef unsigned int u_int32;
typedef char int8;
typedef short int16;
typedef int int32;

#pragma pack(1)
	typedef struct RTPInfo{
	//RTP协议固定12字节头长度
	u_int8 RTP_CC  :4; //字节中高位BIT在前
	u_int8 RTP_PX  :2; 
	u_int8 RTP_VER  :2; 
	u_int8 RTP_PT :7;
	u_int8 RTP_MARK :1;
	u_int16 RTP_SEQUENCE;
	u_int32 RTP_TIMESTAMP;
	u_int32 RTP_SSRC; 
	//AU Header Section  固定8字节头长度
	u_int16 AU_H_Length; //数据单元头长度
	u_int32 AU_Size; //数据单元长度
	u_int8 Stream_State_1 :5;
	u_int8 RAP_Flag :1;//TRUE-关键帧 FALSE-非关键帧,如果RTP包是关键帧的分片，则第一个分片是TRUE，其他分片是FALSE
	u_int8 DTS_Flag :1;//默认为false
	u_int8 CTS_Flag :1;//默认为false
	u_int8 AU_Padding :5;
	u_int8 Stream_State_2 :3;
	}SConnectRTPInfo;

	typedef struct my_fu_indicator{
	unsigned int Type:5;
	unsigned int NRI_second_bit:1;
	unsigned int NRI_first_bit:1;
	unsigned int f:1;
	}FU_indicator;

	typedef struct my_fu_header{
	unsigned int Type:5;
	unsigned int r:1;
	unsigned int e:1;
	unsigned int s:1;
	}FU_header;
   /* 
    typedef struct my_payload{
    FU_indicator payload_indicator;
    FU_header payload_header;  
	char payload_data[1400]; 
	}Payload;
    */
    
    typedef struct my_payload{
    FU_indicator payload_indicator;
    FU_header payload_header;  
	char payload_data[1400]; 
	}Fragment_Payload;

	typedef struct FileHeader{
 		u_int8   FileMarker[4];  //"NVRM"
 		u_int32  Version; 
 		u_int32  Indexflag;    //0 : Index not exist 1 :  Index exist
 		u_int32  DataSize;  //127M
		u_int32  IndexSize;   //1M   
 		u_int32  Reserverd1;     
 		u_int32  Reserverd2;  
	}FILEHEADER;

	typedef struct VideoInfo{
        char       TerminalName[20];
        u_int32    ATimeStamp;
        u_int32    VTimeStamp; 
        u_int32    AudioType;
        u_int32    VideoType;
        u_int32    FrameWidth;
        u_int32    FrameHeight; 
        u_int32    FrameRate;
        u_int32    ChannelID;//
        u_int32    RecordType;//
	}VIDEOINFO;

   typedef struct RTP_Head{
	    unsigned int cc:4; /* CSRC count */
	    unsigned int x:1; /* header extension flag */
	    unsigned int p:1; /* padding flag */
       unsigned int version:2;    /*protocol version*/
       unsigned int pt:7; /* payload type */
       unsigned int m:1; /* marker bit */
       short int seq; /* sequence number */
       unsigned int ts; /* timestamp */
       unsigned int ssrc; /* synchronization source */
    }RTP_HEAD;


	typedef struct RTPpacket{
	    unsigned int cc:4; /* CSRC count */
	    unsigned int x:1; /* header extension flag */
	    unsigned int p:1; /* padding flag */
       unsigned int version:2;    /*protocol version*/
       unsigned int pt:7; /* payload type */
       unsigned int m:1; /* marker bit */
       short int seq; /* sequence number */
       unsigned int ts; /* timestamp */
       unsigned int ssrc; /* synchronization source */
       Fragment_Payload payload;
     }Fragment_RTPpacket;
    
   	#pragma pack()

   typedef struct Thread_Para {
      struct sockaddr_in *client_addr;   
      sem_t *sem_ptr;
      int socketfd;
   }Rtp_Server_Thread_Para;

#endif /*PUBLIC_H*/
 

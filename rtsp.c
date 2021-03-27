/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: rtsp.cpp
Description: rtsp协议函数部分 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0 
Modification History: 2011.06.01将sem_wait(sem_ptr)函数从
treat_TEARDOWN()函数的末尾移到开头，从而保证，一旦受到来自客户端的
关闭命令就先停止数据的发送，用来防止在TCP模式下媒体数据与relay数据父进程和
子线程同时些一个套接子的冲突。
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/
#include"public.h"
#include"rtsp.h"
#include"udp_rtp_server.h"
char *SrtingInf[]=
    {
        "RTSP/1.0 200 OK\r\n",
        "CSeq: ",
	    "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n",
        "Date: ",
        "Content-Base: ",
        "Content-Type: ",
        "Content-Length: ",
        "Transport: ",
        "Session: ",
        "RTP-Info: ",
        "Range: "
    };     
    
char medianameinfor[100]={0};

int Client_Port_len=0;

char Client_Port_Infor[50]={0};

int send_flag=0;
		
void getDate(char *tim_str){
    time_t sdctime;
	struct tm *nowtime;
	time(&sdctime);
	nowtime=localtime(&sdctime);
    strncat(tim_str,asctime(nowtime),strlen(asctime(nowtime))-1);//asctime在所产生的字符串后加换行符因此比预计的字符串长度多1个 
    strcat(tim_str," GMT");
    }
    
int getNTP(void){
    time_t nsdctime;
	time(&nsdctime);
	return nsdctime;
    }    
void creat_SessionId(char *SessionIdInfo){
    long SessionId=0;
	long Randmax=999999999;
    srand((unsigned)time(NULL));
	SessionId=10000000+rand()%Randmax;
	sprintf(SessionIdInfo,"%ld",SessionId);
}  
int get_Message_content(char *rtspinfor,char *Message_Name,char *Message_content,int  *Message_content_len){
    char *Message_position=NULL;
    char *CRLF_position=NULL;
	Message_position=strstr(rtspinfor,Message_Name);
	if(0==Message_position){
	printf("strstr fail 1:%s\n",strerror(errno));
	return 0;
	}             
	CRLF_position=strstr(Message_position,"\r\n");
	if(0==CRLF_position){
	printf("strstr fail 2:%s\n",strerror(errno));
	return 0;
	}      
   	*Message_content_len=CRLF_position-Message_position-strlen(Message_Name);
   	//printf("%d %d %d %d\n",CRLF_position,Message_position,strlen(Message_Name),*Message_content_len);
   	strncpy(Message_content,Message_position+strlen(Message_Name),*Message_content_len);
   	return 1;
}

int get_Client_Port(char *rtspinfor,char *Message_Name,char *Message_content,int  *Message_content_len){
    char *Message_position=NULL;
    char *CRLF_position=NULL;
    char *Semicolon_position=NULL;
    char *Finish_position=NULL;
    char *Dash_position=NULL;
	Message_position=strstr(rtspinfor,Message_Name);
	if(0==Message_position){
	printf("get_Client_Port strstr fail 1:%s\n",strerror(errno));
	return 0;
	}             
	CRLF_position=strstr(Message_position,"\r\n");
	if(0==CRLF_position){
	printf("get_Client_Port strstr fail 2:%s\n",strerror(errno));
	return 0;
	}      
	Dash_position=strstr(Message_position,"-");
	if(0==Dash_position){
	printf("get_Client_Port strstr fail 3:%s\n",strerror(errno));
	return 0;
	}  
	Semicolon_position=strstr(Message_position,";");
	
	if(0==Semicolon_position)
	Finish_position=CRLF_position;
	else Finish_position=Semicolon_position;
	
   	*Message_content_len=Finish_position-Message_position-strlen(Message_Name);
   	Client_Port_len=Dash_position-Message_position-strlen(Message_Name);
   	//printf("%d %d %d %d\n",CRLF_position,Message_position,strlen(Message_Name),*Message_content_len);
   	strncpy(Message_content,Message_position+strlen(Message_Name),*Message_content_len);  //得到rtp和rtcp的端口号 
   	strncpy(Client_Port_Infor,Message_position+strlen(Message_Name),Client_Port_len);     //得到rtp的端口号 
   	return 1;
}

int get_Client_Ip(char *rtspinfor,char *Client_IP){
    char *double_slash_position=NULL;
    char *slash_position=NULL;
    int  IP_len;

	double_slash_position=strstr(rtspinfor,"//");
	if(0==double_slash_position){
	printf("strstr fail 1:%s\n",strerror(errno));
	return 0;
	}             
	slash_position=strstr(double_slash_position+2,"/");       //跳过"//"中的"/"
	if(0==slash_position){
	printf("strstr fail 2:%s\n",strerror(errno));
	return 0;
	}      
   	IP_len=slash_position-double_slash_position-2;
   	//printf("%d %d %d %d\n",CRLF_position,Message_position,strlen(Message_Name),*Message_content_len);
   	strncpy(Client_IP,double_slash_position+2,IP_len);

   	return 1;
}

int treat_OPTIONS(char *rtspinforma ,int rtspsocket){	
	char relay[2048]={0};
	char Message_cont[100]={0};
	int Message_cont_len=0;
	get_Message_content(rtspinforma,"CSeq: ",Message_cont,&Message_cont_len);
	sprintf(relay,"%.17s%.6s",SrtingInf[Status_Code],SrtingInf[CSeq]);
	strncat(relay,Message_cont,Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[OPTIONS_RESP]);
	if(send(rtspsocket,relay,strlen(relay),0)<0){
	printf("send fail !\n");
	return 0;
	}
	printf("%s\n",relay);
	return 1;
	}

int treat_DESCRIBE(char *rtspinforma ,int rtspsocket){	
	char relay[2048]={0};
	char CSeq_Message_cont[100]={0};
	int CSeq_Message_cont_len=0;
	char DESCRIBE_Message_cont[100]={0};
	int DESCRIBE_Message_cont_len=0;
	char Now_Date[80]={0};
	char SDP_Infor[1024]={0};
	char Client_Ip_Addr[100]={0};
	char SDP_Infor_Len[50]={0};
	get_Message_content(rtspinforma,"CSeq: ",CSeq_Message_cont,&CSeq_Message_cont_len);
	getDate(Now_Date);

	get_Message_content(rtspinforma,"DESCRIBE ",DESCRIBE_Message_cont,&DESCRIBE_Message_cont_len);


	get_Client_Ip(rtspinforma,Client_Ip_Addr);

	sprintf(relay,"%.17s%.6s",SrtingInf[Status_Code],SrtingInf[CSeq]);

	strncat(relay,CSeq_Message_cont,CSeq_Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[DATE]);

	strcat(relay,Now_Date);

	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Content_Base]);


	strncat(relay,DESCRIBE_Message_cont,DESCRIBE_Message_cont_len-9);//减9将DESCRIBE信息最后的“ RTSP/1.0”去掉 

	strcat(relay,"/");
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Content_Type]);
	strcat(relay,"application/sdp");
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Content_Length]);
	
	sprintf(SDP_Infor,"v=0\r\no=NVRStream %d %d IN IP4 %s\r\ns=H.264 video streamed by NVR\r\nt=0 0\r\na=control:*\r\na=range:npt=now-\r\nm=video 0 RTP/AVP 98\r\n",getNTP(),getNTP(),Client_Ip_Addr);
	strcat(SDP_Infor,"a=rtpmap:98 H264/90000\r\n");
	strcat(SDP_Infor,"a=fmtp:98 profile-level-id=42A01E;packetization-mode=1\r\n");
	strcat(SDP_Infor,"a=control:streamid=0\r\n");
	
	sprintf(SDP_Infor_Len,"%d",strlen(SDP_Infor));
	strcat(relay,SDP_Infor_Len);
	strcat(relay,"\r\n");
	strcat(relay,"\r\n");
	strcat(relay,SDP_Infor);
	if(send(rtspsocket,relay,strlen(relay),0)<0){
	printf("send fail :%s\n",strerror(errno));
	return 0;
	}
	printf("%s\n",relay);
	return 1;
	}

int treat_SETUP(char *rtspinforma ,int rtspsocket ,int *protocaltype){
    char relay[2048]={0};
	char CSeq_Message_cont[100]={0};
	int CSeq_Message_cont_len=0;
	char Transport_Message_cont[100]={0};
	int Transport_Message_cont_len=0;
	char Port_Message_cont[100]={0};
	int Port_Message_cont_len=0;
	char Now_Date[80]={0};
	char Session_Id[12]={0};
	char SETUP_Message_cont[100]={0};
	int SETUP_Message_len=0;
	get_Message_content(rtspinforma,"SETUP ",SETUP_Message_cont,&SETUP_Message_len);
	strncat(medianameinfor,SETUP_Message_cont,SETUP_Message_len-9);
	
	get_Message_content(rtspinforma,"CSeq: ",CSeq_Message_cont,&CSeq_Message_cont_len);
	get_Message_content(rtspinforma,"Transport: ",Transport_Message_cont,&Transport_Message_cont_len);
       
   if(0 == strstr(Transport_Message_cont,"TCP"))
	get_Client_Port(rtspinforma,"client_port=",Port_Message_cont,&Port_Message_cont_len);

	getDate(Now_Date);
	
	sprintf(relay,"%.17s%.6s",SrtingInf[Status_Code],SrtingInf[CSeq]);
	strncat(relay,CSeq_Message_cont,CSeq_Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[DATE]);
	strcat(relay,Now_Date);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Transport]);
	
	if(0!=strstr(Transport_Message_cont,"TCP")){
	strcat(relay,"RTP/AVP/TCP;unicast;interleaved=0-1");
	//strcat(relay,"RTP/AVP/TCP;unicast");
	*protocaltype=PROTOCAL_TCP;
	}
	else{
	strcat(relay,"RTP/AVP;unicast;client_port=");
	*protocaltype=PROTOCAL_UDP;
	//}
	strncat(relay,Port_Message_cont,Port_Message_cont_len);
	strcat(relay,";");
	strcat(relay,"server_port=6970-6971");
        
        }
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Session]);
	creat_SessionId(Session_Id);
	strcat(relay,Session_Id);
	strcat(relay,"\r\n");
	strcat(relay,"\r\n");
	
	if(send(rtspsocket,relay,strlen(relay),0)<0){
	printf("send fail :%s\n",strerror(errno));
	return 0;
	}
	printf("%s\n",relay);
	return 1;
	}

int treat_PLAY(char *rtspinforma ,int rtspsocket,sem_t *sem_ptr){
    char relay[2048]={0};
	char CSeq_Message_cont[100]={0};
	int CSeq_Message_cont_len=0;
	char Range_Message_cont[100]={0};
	int Range_Message_cont_len=0;
	char Now_Date[80]={0};
	char Session_Id[12]={0};
	int Session_Id_len=0;
	char RTP_SEandTI[100]={0};
	SConnectRTPInfo  rtp_head;
	int read_size=0;
	int media_file_fd=0;

	get_Message_content(rtspinforma,"CSeq: ",CSeq_Message_cont,&CSeq_Message_cont_len);

	get_Message_content(rtspinforma,"Range: ",Range_Message_cont,&Range_Message_cont_len);

	get_Message_content(rtspinforma,"Session: ",Session_Id,&Session_Id_len);
	getDate(Now_Date);

	sprintf(relay,"%.17s%.6s",SrtingInf[Status_Code],SrtingInf[CSeq]);
	strncat(relay,CSeq_Message_cont,CSeq_Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[DATE]);
	strcat(relay,Now_Date);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Range]);
	strncat(relay,Range_Message_cont,Range_Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Session]);
	strncat(relay,Session_Id,Session_Id_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[RTP_Info]);
	strcat(relay,medianameinfor);
	strcat(relay,";");
	
    media_file_fd=open("play.dat",O_RDONLY);
    if(0==media_file_fd)
	  {
	   printf("open file fail!");
       exit(1);
      }
      lseek(media_file_fd,84,SEEK_CUR);
      
    while(1)
		{
	    read_size=read(media_file_fd,(char *)&rtp_head,20);
	    if(20!=read_size)
			{
			printf("read rtp head fail :%s\n",strerror(errno));
		    close(media_file_fd);	
            exit(1);
			}
		if(98==rtp_head.RTP_PT)
          {
           sprintf(RTP_SEandTI,"seq=%d;rtptime=%d",rtp_head.RTP_SEQUENCE,rtp_head.RTP_TIMESTAMP);
           close(media_file_fd);
           break;
          }
          
        else if(80==rtp_head.RTP_PT)
        lseek(media_file_fd,rtp_head.AU_Size,SEEK_CUR);
        
		else 
			{
		    printf("read rtp infor erro!\n");
		    close(media_file_fd);
            exit(1);
			}
	   }//while
	
	//sprintf(RTP_SEandTI,";seq=%d;rtptime=%d",);     //填入rtp 时间戳和序列号
	strcat(relay,RTP_SEandTI); 
	strcat(relay,"\r\n");
	strcat(relay,"\r\n");
	
	if(strlen(relay) != send(rtspsocket,relay,strlen(relay),0)){
	printf("send fail :%s\n",strerror(errno));
	return 0;
	}
	printf("%s\n",relay);
	
   sem_post(sem_ptr);              //send_flag=1;  //open the send switch  
	
	return 1;
	}

int treat_TEARDOWN(char *rtspinforma ,int rtspsocket,sem_t *sem_ptr){
	char relay[2048]={0};
	char CSeq_Message_cont[100]={0};
	int CSeq_Message_cont_len=0;
	char Now_Date[80]={0};
	char Session_Id[12]={0};
	int Session_Id_len=0;

   sem_wait(sem_ptr);    //close the send switch 

	get_Message_content(rtspinforma,"CSeq: ",CSeq_Message_cont,&CSeq_Message_cont_len);
	get_Message_content(rtspinforma,"Session: ",Session_Id,&Session_Id_len);
	getDate(Now_Date);
    
	sprintf(relay,"%.17s%.6s",SrtingInf[Status_Code],SrtingInf[CSeq]);
	strncat(relay,CSeq_Message_cont,CSeq_Message_cont_len);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[DATE]);
	strcat(relay,Now_Date);
	strcat(relay,"\r\n");
	strcat(relay,SrtingInf[Session]);
	strncat(relay,Session_Id,Session_Id_len);
	strcat(relay,"\r\n");
	strcat(relay,"\r\n");

	if(send(rtspsocket,relay,strlen(relay),0)<0){
	printf("send fail :%s\n",strerror(errno));
	return 0;
	}
	printf("%s\n",relay);
	
   //sem_wait(sem_ptr); //send_flag=0;                      //close the send switch  
	return 1;
	}

int treat_PAUSE(char *rtspinforma ,int rtspsocket){
	time_t sdctime;
	struct tm *nowtime;
	char relay[2048];
	
	return 1;
	}

/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: rtsp.h
Description: rtsp头文件 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0 
Modification History:
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/

#ifndef RTSP_H
#define RTSP_H

enum {
	Status_Code       = 0,
	CSeq,           /* =1 */
	OPTIONS_RESP,   /* =2 */
	DATE,           /* =3 */
	Content_Base,   /* =4 */
	Content_Type,   /* =5 */
	Content_Length, /* =6 */
	Transport,      /* =7 */
	Session,        /* =8 */
	RTP_Info,       /* =9 */
	Range           /* =10 */
};

void getDate(char *tim_str);

int getNTP(void);

void creat_SessionId(char *SessionIdInfo);

int get_Message_content(char *rtspinfor, char *Message_Name, char *Message_content,
                            int  *Message_content_len);    
                            
int get_Client_Port(char *rtspinfor, char *Message_Name, char *Message_content,
                        int *Message_content_len);
                        
int get_Client_Ip(char *rtspinfor, char *Client_IP);

int treat_OPTIONS(char *rtspinforma ,int rtspsocket);

int treat_DESCRIBE(char *rtspinforma ,int rtspsocket);

int treat_SETUP(char *rtspinforma ,int rtspsocket ,int *protocaltype);

int treat_PLAY(char *rtspinforma ,int rtspsocket,sem_t *sem_ptr);

int treat_TEARDOWN(char *rtspinforma ,int rtspsocket,sem_t *sem_ptr);

int treat_PAUSE(char *rtspinforma ,int rtspsocket);

#endif

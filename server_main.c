/**************************************************
Copyright: 20xx-forever firephoenix Co. Ltd.
Filename: main.cpp
Description: 服务器主函数 
Others: 为使用makefile编译，重构代码。 
Author: GaoQiang 
Date: 2011.5.12
Version: 3.0 
Modification History: 2011.5.25系统在收到客户端TEARDOWN命令后
结束对该客户ip的服务进程，并关闭rtsp的端口套接字，加入忽略僵尸进程信号函数。
                                                          2011.06.01在TEARDOWN命令后加入sem_destroy(&sem);
释放poxis信号量。
    GaoQiang 2011.5.12 3.0 build this moudle
**************************************************/

#include"public.h"
#include"rtsp.h"
#include"udp_rtp_server.h"
#include"tcp_rtp_server.h"
int main(){
	
	int listensocketfd = 0, rtspsocketfd = 0, readynum = 0, client_addr_size = 0;
	
	int protocal=-1;
	
	fd_set read_fds;
	
	pid_t pid;
	
	struct timeval timeout;
	
	struct sockaddr_in server_addr,client_addr;
	
	char rtsp_inf_resv[2048]={0};
	
	char mzero=0;
   char bNodelay=1;  
   char bDebug=1;
   int err;
	
	pthread_t udp_rtp_thread;
	pthread_t tcp_rtp_thread;
 
   Rtp_Server_Thread_Para  This_Rtp_Server_Thread_Para;

   sem_t sem;

   signal(SIGCLD, SIG_IGN); 
	
	if((listensocketfd=socket(AF_INET,SOCK_STREAM,0))==-1){
	printf("creat socket fail !\n");
	exit(1);
	}
	//bzero(&server_addr,sizeof(struct sockaddr_in)); 
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(554);
	
	if(setsockopt(listensocketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&mzero, sizeof(int)) < 0)
	 {
      printf("setsockopt reuse fail:%s\n",strerror(errno));
	  close(listensocketfd);
      exit(1);
	 }

   if(setsockopt(listensocketfd,SOL_SOCKET,SO_DEBUG,(char*)&bDebug,sizeof(int))<0)
        {
      printf("setsockopt debug fail:%s\n",strerror(errno));
	   close(listensocketfd);
      exit(1);
        } 

   if(setsockopt(listensocketfd,IPPROTO_TCP,TCP_NODELAY,(char *)&bNodelay,sizeof(int)) <0)
        {
      printf("setsockopt nodely fail:%s\n",strerror(errno));
	   close(listensocketfd);
      exit(1);
        }


	if(bind(listensocketfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1){
	printf("bind fail :%s\n",strerror(errno));
	exit(1);
	}
	if(listen(listensocketfd,5)==-1){
	printf("listen fail !\n");
	exit(1);
	}
	
	while(1){
	timeout.tv_sec=3;
	timeout.tv_usec=0;	
	FD_ZERO(&read_fds);
	FD_SET(listensocketfd,&read_fds);
	readynum=select(listensocketfd+1,&read_fds,NULL,NULL,&timeout);
	
	//readynum=select(listensocketfd+1,&read_fds,NULL,NULL,NULL);
	if(readynum<0){
	printf("select fail :%s\n",strerror(errno));
	exit(1);
	}
	else if(readynum==0){
	printf("no data ready !\n");
	continue;
	}
	else{
		if(FD_ISSET(listensocketfd,&read_fds)){
		client_addr_size=sizeof(struct sockaddr_in); 
		if((rtspsocketfd=accept(listensocketfd,(struct sockaddr *)(&client_addr),(socklen_t*)&client_addr_size))==-1){
            printf("accept fail !\n");
            exit(1); 
	        }
	    pid=fork();	
	    switch(pid){
	           case -1 : printf("fork fail !\n");
	                     break; 
	
               case  0 : close(listensocketfd);

                      sem_init(&sem,0,0);

           				 while(1){
           				 	
	                     memset (rtsp_inf_resv,0,2047);
	                     
			             if(recv(rtspsocketfd,rtsp_inf_resv,2047,0)<0){    //从554端口获得rtsp message 
						 printf("recv() fail !\n");
						 exit(1);
						 }
						 
					     
	                     /*if(0==parse_rtsp_inf(rtsp_inf_resv)){     //解析收到的rtsp Method  DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE
						 printf("parse_rtsp_inf() fail !\n");
						 exit(1);
						 }*/
					 	printf("%s\n",rtsp_inf_resv);
   
                        if(NULL!=strstr(rtsp_inf_resv,"OPTIONS")){                //发送OPTIONS回答
		                  treat_OPTIONS(rtsp_inf_resv ,rtspsocketfd);         
	
                           }
  
                        else if(NULL!=strstr(rtsp_inf_resv,"DESCRIBE")){    
                           treat_DESCRIBE(rtsp_inf_resv ,rtspsocketfd);
	
	                       }

	                    else if(NULL!=strstr(rtsp_inf_resv,"PAUSE")){
                           treat_PAUSE(rtsp_inf_resv ,rtspsocketfd);
	
	                       }
	
	                    else if(NULL!=strstr(rtsp_inf_resv,"PLAY")){
                           treat_PLAY(rtsp_inf_resv ,rtspsocketfd,This_Rtp_Server_Thread_Para.sem_ptr);
	
	                       }
	
	                    else if(NULL!=strstr(rtsp_inf_resv,"SETUP")){
                           treat_SETUP(rtsp_inf_resv ,rtspsocketfd ,&protocal);
	                       
						   if(protocal==PROTOCAL_UDP)
						   { 
                            This_Rtp_Server_Thread_Para.client_addr=&client_addr;
                            This_Rtp_Server_Thread_Para.sem_ptr = &sem;
	                         err=pthread_create(&udp_rtp_thread,NULL,UDP_RTP_Server,(void*)&This_Rtp_Server_Thread_Para);
	                         if(err != 0)
	                           {
	                            printf("create thread fail !\n");
	                            exit(1);
                               }
	                       }//if(protocal==PROTOCAL_UDP)
	                       
	                       else if(protocal==PROTOCAL_TCP)
						   {                      
                            This_Rtp_Server_Thread_Para.sem_ptr = &sem;
                            This_Rtp_Server_Thread_Para.socketfd=rtspsocketfd;
	                         err=pthread_create(&tcp_rtp_thread,NULL,TCP_RTP_Server,(void*)&This_Rtp_Server_Thread_Para);
	                         if(err != 0)
	                           {
	                            printf("create thread fail !\n");
	                            exit(1);
                               }
	                       }//else if(protocal==PROTOCAL_TCP)
	                     
	                       else
                            {
						     printf("protocal== erro\n");
						     exit(1);
                            }
	                       
	                       }//else if(NULL!=strstr(rtsp_inf_resv,"SETUP"))

	                    else if(NULL!=strstr(rtsp_inf_resv,"TEARDOWN")){
                           treat_TEARDOWN(rtsp_inf_resv ,rtspsocketfd,&sem);
                           close(rtspsocketfd);
                           sem_destroy(&sem);
                           exit(0);                     /*退出对该IP客户端的服务进程*/
	                      //break;
	                       }
						
						else {
                               printf("rec an unknow message!!\n");
		                     }
                                                
						 
						 }//while
						 
						if(protocal==PROTOCAL_UDP){ 
                        err=pthread_join(udp_rtp_thread,NULL);
                        if(err != 0)
                          {
                           printf("wait thread fail !\n");
                           exit(1);
                          }
						}
						
						else if(protocal==PROTOCAL_TCP){
                         err=pthread_join(tcp_rtp_thread,NULL);
                        if(err != 0)
                          {
                           printf("wait thread fail !\n");
                           exit(1);
                          }
						}
						
			             close(rtspsocketfd);
			             exit(0);
			             //break;
                          
               default : close(rtspsocketfd);
                         waitpid(-1,NULL,WNOHANG);
                         break;
	                }//switch
		}//if(FD_ISSET(listensocketfd,&read_fds))
		}//else 
	}//while(1)
	
}//main
	

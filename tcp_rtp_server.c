
#include"tcp_rtp_server.h"
#include"public.h"
extern char Client_Port_Infor[50];

extern int send_flag;

void *TCP_RTP_Server(void *Thread_Para){
    Rtp_Server_Thread_Para *Tcp_Rtp_Server_Thread_Para = (Rtp_Server_Thread_Para *)Thread_Para;
//     struct sockaddr_in *client_addr=(struct sockaddr_in *)the_client_addr;
      int rtpsockfd=0;
     int media_file_fd=0;
	 unsigned int feed_back=0;
	 int i=0,read_leave=0;
     int accumulate=0;
	 Fragment_RTPpacket rtppacket;
	 SConnectRTPInfo  rtp_head;
	 Fragment_Payload t_payload;
    TCP_RTP_PACKET Inteleaved_Packet;
	 char source_fream_buf[65535]={0};
	 unsigned int fream_step=0;
	 unsigned int read_size=0;
	 int nzero=1;
	 
	 media_file_fd=open("play.dat",O_RDONLY);
	 if(0==media_file_fd){
	 printf("open file fail!");
	 exit(1);
	 }
	 
 	 if(-1==lseek(media_file_fd,84,SEEK_CUR))					//rtp封包过程 
	  {
	   printf("lseek fail :%s\n",strerror(errno));
	   exit(1);
	  }
	 
	 
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
             accumulate=rtp_head.RTP_SEQUENCE;
             break;
			}
        else if(80==rtp_head.RTP_PT)
                lseek(media_file_fd,rtp_head.AU_Size,SEEK_CUR);
	
		}//while
		
	 lseek(media_file_fd,0,SEEK_SET);
	 if(-1==lseek(media_file_fd,84,SEEK_CUR))					
	  {
	   printf("lseek fail :%s\n",strerror(errno));
	   exit(1);
	  }

	 
 	 while(1)
       {

      
		
		sem_wait((*Tcp_Rtp_Server_Thread_Para).sem_ptr);         //while(1!=send_flag){}          //the switch of send

      sem_post((*Tcp_Rtp_Server_Thread_Para).sem_ptr);

		fream_step=0;
		
	    feed_back=read(media_file_fd,(char *)&rtp_head,20);
	    if(0==feed_back)
		{
		printf("read rtp head fail:%s\n",strerror(errno));
		close(media_file_fd);
        break;
		}
        
		if(98==rtp_head.RTP_PT)
          {
          	
          feed_back=read(media_file_fd,source_fream_buf,rtp_head.AU_Size);
          if(feed_back!=rtp_head.AU_Size)
		  	{
             printf("read source fream fail :%s\n",strerror(errno));
             close(media_file_fd);
             break;
			 //exit(1);
		    }
		  read_leave=rtp_head.AU_Size%1400;  
	      for(i=0;i<rtp_head.AU_Size/1400;i++)
		  	{
		  
             t_payload.payload_indicator.f=0;                  //pack FU indicator
             t_payload.payload_indicator.NRI_first_bit=0;
             t_payload.payload_indicator.NRI_second_bit=1;
             t_payload.payload_indicator.Type=28;
             
             if(0==i)                                            //pack FU header
             t_payload.payload_header.s=1;
             else
             t_payload.payload_header.s=0;
	         
             if((0==read_leave)&&((rtp_head.AU_Size/1400-1)==i))                    //if the last one                                  
             t_payload.payload_header.e=1;
             else
             t_payload.payload_header.e=0;
             
             t_payload.payload_header.r=0;
             
             t_payload.payload_header.Type=3;
             
             memcpy(t_payload.payload_data,source_fream_buf+fream_step,1400);
             fream_step=fream_step+1400;
			 //udp_payload.payload_data=fragment_fream_buf;
		     
		     rtppacket.version=rtp_head.RTP_VER;
	         rtppacket.p=0;
	         rtppacket.x=0;
	         rtppacket.cc=rtp_head.RTP_CC;
	         rtppacket.m=rtp_head.RTP_MARK;
	         rtppacket.pt=rtp_head.RTP_PT;
	         
	         rtppacket.seq=htons(accumulate);
	         accumulate=accumulate+1;
	         
	         rtppacket.ts=htonl(rtp_head.RTP_TIMESTAMP);
	         rtppacket.ssrc=rtp_head.RTP_SSRC;
	         rtppacket.payload=t_payload;

            Inteleaved_Packet.Dollar = '$';
            Inteleaved_Packet.Channel_id = 0;
            Inteleaved_Packet.Length = htons(sizeof(rtppacket));
            memcpy(&Inteleaved_Packet.Rtp_packet,&rtppacket,sizeof(rtppacket));



            feed_back=send((*Tcp_Rtp_Server_Thread_Para).socketfd,(char *)&Inteleaved_Packet,sizeof(Inteleaved_Packet),0);
            if(feed_back!=sizeof(Inteleaved_Packet))
               printf("----------\nfeed_back:%dsizeof(Inteleaved_Packet):%d\n------------\n",feed_back,sizeof(Inteleaved_Packet)); 
                                
		
	        }//for
		    
		    if(0==read_leave)
				{
			    usleep(50000);
			    continue;
		        }
	         t_payload.payload_indicator.f=0;                  //pack FU indicator
             t_payload.payload_indicator.NRI_first_bit=0;
             t_payload.payload_indicator.NRI_second_bit=1;
             t_payload.payload_indicator.Type=28;
             
             t_payload.payload_header.s=0;
	                                          
             t_payload.payload_header.e=1;
             
             t_payload.payload_header.r=0;
             
             t_payload.payload_header.Type=3;
             
             memcpy(t_payload.payload_data,source_fream_buf+fream_step,read_leave);
             fream_step=fream_step+read_leave;
             //udp_payload.payload_data=fragment_fream_buf;
		     
		     rtppacket.version=rtp_head.RTP_VER;
	         rtppacket.p=0;
	         rtppacket.x=0;
	         rtppacket.cc=rtp_head.RTP_CC;
	         rtppacket.m=rtp_head.RTP_MARK;
	         rtppacket.pt=rtp_head.RTP_PT;
	         
	         rtppacket.seq=htons(accumulate);
	         accumulate=accumulate+1;
	         
	         rtppacket.ts=htonl(rtp_head.RTP_TIMESTAMP);
	         rtppacket.ssrc=rtp_head.RTP_SSRC;
	         rtppacket.payload=t_payload;

            Inteleaved_Packet.Dollar = '$';
            Inteleaved_Packet.Channel_id = 0;
            Inteleaved_Packet.Length = htons(sizeof(RTP_HEAD)+sizeof(FU_indicator)+sizeof(FU_header)+read_leave);
            memcpy(&Inteleaved_Packet.Rtp_packet,&rtppacket,sizeof(rtppacket));


		     
            feed_back=send((*Tcp_Rtp_Server_Thread_Para).socketfd,(char *)&Inteleaved_Packet,sizeof(Inteleaved_Packet)-1400+read_leave,0);
            if(feed_back!=sizeof(Inteleaved_Packet)-1400+read_leave)
                    printf("----------\nfeed_back:%dsizeof(Inteleaved_Packet)-1400+read_leave:%d\n------------\n",feed_back,sizeof(Inteleaved_Packet)-1400+read_leave);  


			 usleep(50000);   
          //continue;
          }//if(98==rtp_head.RTP_PT)
          
        else if(80==rtp_head.RTP_PT)
		       {
                 feed_back=lseek(media_file_fd,rtp_head.AU_Size,SEEK_CUR);
                 if(-1==feed_back)
		           {
		           printf("lseek fail :%s\n",strerror(errno));
	               exit(1);
		           }
               }
		else 
		{
		 printf("file data wrong !\n");
		 exit(1);
		}
	
	   }//while(1)	
	 
 	close(rtpsockfd);
 	printf("send rtp packet though udp over!\n");
    return NULL;
     
}

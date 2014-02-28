#include "Irdm_main.h"

interface arg;
extern int MsgID[MSG_Queue_NO];//defined in njkk_interface.h

// THread 1:  OpenPort & Modem state check
void Pthread_OpenPort_Modem_Check()
{
	while(1)
	{
		Th_channel_check_start++;
		printf("Pthread OpenPort & Modem state check!!\n");
		OpenPort_Modem_state_check(MsgIDIrdm, MsgIDPC104,&arg.static_conf,&arg.dynamic_conf);
		sleep(2);
	}
} 

// Thread 2: recieve SBD command////////////
void Pthread_SBD_Check()
{
	int rt;
	int pc104_state;
	while(1)
	{
		Th_sbd_check_start++;
		printf("Pthread : SBD check!!\n");

		if (Comm_Channel == OPENPORT)
		{
			rt = SBDCmd_Read_OpenPort(&arg.static_conf,&arg.dynamic_conf);	//get commands from command files
			if (rt < 0) // have cmd file to check
			{
				usleep(Busy_loop_Time);
				continue;
			}
		}
		else if (Comm_Channel == MODEM)
		{
			if (sbd_rcv_flag == 1 || sbd_snd_flag == 1)
				return;
			else if ( (sbd_rcv_flag==0) && (sbd_snd_flag==0) && (ftp_deliver_flag == 1))//(Trans_Mode == FTPSTART)
			{
				KillWvdial();
				ftp_deliver_flag = 0;
				SBDCmd_Read_Modem(&arg.static_conf,&arg.dynamic_conf);		//get commands from Modem Serial Port Buffers
			}
			else if ((sbd_rcv_flag==0) && (sbd_snd_flag==0) && (ftp_deliver_flag == 0))
			{
				SBDCmd_Read_Modem(&arg.static_conf,&arg.dynamic_conf);
			}
		}

		sleep(Idle_loop_Time);		
	}
}

// Thread 3: msg recv from other modules:PLC etc
void Pthread_msg_Irdm()
{
	int msg_type;
	int recv_result;
	static int TotalMsg=0;
	int tempCount;
	char rcv_buf[MSG_BUF_SIZE] = {0}; //MSG_BUF_SIZE
	char sbd_buf[MSG_BUF_SIZE+50] = {0};
	char file_name[MAX_SBDNAME_LEN] = {0};
	unsigned char CmdNo;
	
	int tCount;	
	FILE *fp;
	int file_size=0;
	MsgUp msg_up;
	MsgUp_Post msg_post;
	MsgDown msg_down;
	int pc104_state=0;
	int last_state;
	
	while(1)
	{
		Th_irdm_msg_start++;	
	
		memset(rcv_buf,0,sizeof(rcv_buf));
		printf("\n Irdm is waiting for msgs:\n");
		recv_result = recv_msg(MsgIDIrdm,&msg_type,rcv_buf,MSG_BUF_SIZE); 

		printf("msg_type=%d   msg_len=%d \n",msg_type,recv_result);
		tCount=0;	
		if (recv_result < 0)
		{
			printf("***Msg queue is empty,sleep 1 seconds!\n");
			sleep(Idle_loop_Time);
			continue;
		}

		char SBD_Priority = 'M';
		char FTP_Priority = 'M';

		switch(msg_type)
		{
////////////////////////////////    SBD   ///////////////////////////////////////////			
///  SBD upload
	// 'M'		// PLC ; priority:SBD 1
			case 2: 			//PLC control start/stop command
			case 4:				//PLC control parament comamnd
			case 6:				//generator priority
			case 8:				//realtime analog inquiry
			case 10:			// realtime digital inquiry
			case 12:
			case 18:    // get monitor parament page
			//PC104: priority:SBD 1
			case 76:   // set temperature up/down limit	
			//NAS: priority:SBD 1
			case 92:
			case 94:
			case 96:
			
			// Irdm: priority:SBD 1
			case 102:      // upload file reult
			case 104:      // download result:  
			case 110:      // file route inquiry		
			case 112:			//  file cp/rm/tar
			case 114:
			case 116:
			case 118:			
			case 120:      // linux system command result:  unsigned short CmdNo; char *result(length<1841); 
					{//msg format:priority(int)+msgtype(int)+filename(str[])				
						while (SBD_Classify(SBD_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}					
						break;
					}
	//  'L'	
			case 41:    //  timed monitor page data upload result	
					{
						if (PC104_No == PC104_2)
							break;
						char SBD_Priority = 'L';
						while (SBD_Classify(SBD_Priority,(msg_type),rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}						
						break;
					}							
	//'H'
			case 132:			//  system repair: restart
			case 134:
			case 136:
			case 142:
			case 144:
			case 146:
			case 148:
			case 150:
			
			case 202:			// alarm feedback automaticlly
			case 204:
			case 206:
					{
						char SBD_Priority = 'H';
						while (SBD_Classify(SBD_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								printf("$$$$$$$$$$$$$$$$$$$$$$  classify error ,send the message back\n");
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}					
						break;	
					}
			case 42:      		// alarm feedback
			case 181:
			case 183:
			case 185:
			case 187:
					{
						if (msg_type == 42 && PC104_No == PC104_2)
							break;
						char SBD_Priority = 'H';
						while (SBD_Classify(SBD_Priority,(msg_type),rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								printf("$$$$$$$$$$$$$$$$$$$$$$  classify error ,send the message back\n");
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}					
						break;	
					}			
			case 138:    // modem reboot
					{
						Modem_Reboot_Time++;
						char SBD_Priority = 'H';
						while (SBD_Classify(SBD_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}
						
						break;	
					}
			case 140:	 // openport reboot				
					{
						OpenPort_Reboot_Time++;
						char SBD_Priority = 'H';
						while (SBD_Classify(SBD_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}
						
						break;	
					}
					
/// FTP download					
	//'none'
			case 103:		// download FTP file
			case 205:
					{
						while (DL_FTP_Classify(rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf)!= 0)
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);
						}
						break;			
					}					
/////////////////////////////////  FTP   /////////////////////////////////////					
/// FTP upload
	//'?'
			case 101:		// upload file 
					{
						memcpy(&FTP_Priority, rcv_buf, 1);
						while(FTP_Classify(MsgIDPLC,FTP_Priority,msg_type,(rcv_buf+1),recv_result,&arg.static_conf,&arg.dynamic_conf) != 0 )
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}	
							sleep(1);				
						}
				
						break;
					}
	// 'H'
			case 46:
					{
						if (PC104_No == PC104_2)
							break;
						FTP_Priority = 'H';
						printf("rcv_buf = %s\n",rcv_buf);
						while(FTP_Classify(MsgIDPLC,FTP_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0 )
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								printf("classify error!\n");
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}	
							sleep(1);				
						}			
						break;
					}	
	// 'M'
			case 14:    //  history analog inquiry result
			case 16:   //  history digital inquiry result
			case 74:   // upload photograph			
					{
						FTP_Priority = 'M';
						printf("rcv_buf=%s\n",rcv_buf);
						while(FTP_Classify(MsgIDPLC,FTP_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0 )
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}
							sleep(1);					
						}
				
						break;
					}			
	// 'L' 
			case 195:  // timed upload photograph
					{
						FTP_Priority = 'L';
						while(FTP_Classify(MsgIDPLC,FTP_Priority,msg_type,rcv_buf,recv_result,&arg.static_conf,&arg.dynamic_conf) != 0 )
						{
							tCount++;
							if (tCount == MAX_WAIT_TIME)
							{
								send_msg(MsgIDIrdm,msg_type,rcv_buf,recv_result);
								break;
							}	
							sleep(1);				
						}
				
						break;
					}					
			default: 
				break;	
		}
	
	sleep(Idle_loop_Time);
	}
}

// Thread 4: SBDupload the files
void Pthread_SBDUpload()
{
	int rt;
	int pc104_state;
	while(1)
	{
		Th_sbd_upload_start++;		
		printf("\n Thread : Pthread_SBDUpload is running!\n");	
	
		if (HSBD_Num + MSBD_Num + LSBD_Num > 0)
		{
			sbd_send_flag = 1; // sbd send is beagining, kill lftp to dead;\
			
			if (Comm_Channel == OPENPORT)
			{
				if (ftp_send_flag == 1)
				{
					rt=system("/root/njkk/killftp");
					if (rt == 0)
					{
						ftp_send_flag = 0;
					}
				}
				SBD_OpenPort_Upload(&arg.static_conf,&arg.dynamic_conf, MsgIDProc);
			}
			else if ((Comm_Channel == MODEM) && (sbd_rcv_flag == 0)) //(Trans_Mode == SBDSTART)
			{					
				if (ftp_deliver_flag == 1)//Trans_Mode == FTPSTART
				{
					KillWvdial();
					ftp_deliver_flag = 0;
				}
				printf("/////////// Iridium Modem begin to upload SBD!////////////\n");	
				SBD_Modem_Upload(&arg.static_conf,&arg.dynamic_conf);
			}
			sbd_send_flag = 0;
			
			usleep(Busy_loop_Time);
			continue;			
		}	
		
		sleep(1);
	}
}

// Thread 5: FTP download and upload the files
void Pthread_FTPTrans()
{
	int pc104_state;
	while(1)
	{
		Th_ftp_upload_start++;	
		printf("\n Thread : Pthread_FTP download & upload  is running!\n");
		
		if((HSBD_Num + MSBD_Num + LSBD_Num == 0) && (DL_FTP_Num > 0))
		{	
			if ( (Comm_Channel == OPENPORT) && (sbd_send_flag == 0))
			{
				printf("^^^^^^^^^^^^^OpenPort:	beagin to download ftp!\n");
				FTP_Downloadftp(pc104_state,&arg.static_conf,&arg.dynamic_conf);
			}
			else if ((Comm_Channel == MODEM) && (sbd_rcv_flag == 0) && (sbd_snd_flag == 0))
			{
				printf("^^^^^^^^^^^^^Modem:	beagin to download ftp!\n");
				FTP_Downloadftp(pc104_state,&arg.static_conf,&arg.dynamic_conf);
			}
			sleep(Idle_loop_Time);
			continue;
		}
		else if((HSBD_Num + MSBD_Num +LSBD_Num == 0) && (DL_FTP_Num == 0) && (HFTP_Num+MFTP_Num+LFTP_Num > 0) )
		{			
			if (Comm_Channel == OPENPORT)
			{
				printf("vvvvvvvvvvvv OepnPort: begin to Upload FTP!\n");				
				FTP_Uploadftp(pc104_state,&arg.static_conf,&arg.dynamic_conf);
			}
			else if ((Comm_Channel == MODEM) && (sbd_rcv_flag ==0) && (sbd_snd_flag == 0))
			{
				printf("vvvvvvvvvvvv Modem: begin to Upload FTP!\n");	
				FTP_Uploadftp(pc104_state,&arg.static_conf,&arg.dynamic_conf);
			}
			sleep(Idle_loop_Time);
			continue;
		}

		sleep(1);		
	}
}

////Thread 6:   UDP client  .send heart
void Thread_Heart_Send()
{
	int rt;
	while (1)
	{
		printf("Thread Heart package send is running!\n");
		printf("====TCP: send starts!\n ");
		int count=0;
		while ((rt=tcp_send_heart())<0)
		{
			count++;
			if (3 == count)
			    break;
			usleep(5000);
		}
/// check whether send heart package throuh CAN network
		if (rt < 0)
		{
			printf("====CAN: send starts!\n ");
			CAN_send_heart();
			/// send heart package through CAN 
		}
		sleep(1);
	}
}

//// Thread 7 :UDP server,recieve heart
void Thread_Heart_Recv()
{	
	int rt;
	while (1)
	{
		printf("Thread Heart package recieve is running!\n");
		printf("====TCP: recieve starts!\n ");
		rt = tcp_recv_heart();				
		if (rt<0)
		{
			printf("====CAN: recieve starts!\n ");
			CAN_recv_heart();
		}
		sleep(1);
	}
}

int main()
{
///Initlizing, check file and directory,etc
	Irdm_Init(&arg.static_conf,&arg.dynamic_conf);
	printf("Iridium initial !!\n");
///create msg_queues
	Install_Msg_Queue();
	
	MsgIDPLC = MsgID[MSGIDPLC];
	MsgIDIrdm = MsgID[MSGIDIrdm];
	MsgIDPC104 = MsgID[MSGIDPC104];
	MsgIDNAS = MsgID[MSGIDNAS];
	MsgIDCamera = MsgID[MSGIDCAMERA];
	MsgIDProc = MsgID[MSGIDPROC];
	printf("MsgIDPLC=%d\tMsgIDIrdm=%d\n",MsgIDPLC,MsgIDIrdm);
			
///create threads
	Thread_Param  Thread_OpenPort_Modem_check,Thread_SBDCheck,Thread_msgcen,Thread_SBDupload,Thread_FTPTrans,heart_send_th,heart_recv_th;

	Install_Thread(&Thread_OpenPort_Modem_check, (void *)Pthread_OpenPort_Modem_Check, 0, NULL);
	Install_Thread(&Thread_SBDCheck, (void *)Pthread_SBD_Check, 0, NULL);	
	Install_Thread(&Thread_msgcen, (void *)Pthread_msg_Irdm, 0, NULL);
	Install_Thread(&Thread_SBDupload, (void *)Pthread_SBDUpload, 0, NULL);
	Install_Thread(&Thread_FTPTrans, (void *)Pthread_FTPTrans, 0, NULL);
	Install_Thread(&heart_send_th, (void *)Thread_Heart_Send, 0, NULL);
	Install_Thread(&heart_recv_th, (void *)Thread_Heart_Recv, 0, NULL);
///main thread is running
	while(1)
	{	
		printf("main thread is running!\n");
/*
		if (Th_channel_check_start>0 && Th_sbd_check_start>0 && Th_sbd_upload_start>0 && Th_ftp_upload_start>0)
		{
			Th_channel_check_start = 0;
			Th_sbd_check_start = 0;
			Th_irdm_msg_start = 0;
			Th_sbd_upload_start = 0;
			Th_ftp_upload_start = 0;
			send_msg(MsgIDProc,151,NULL,0);
		}
		else
		{
			send_msg(MsgIDProc,153,NULL,0); 
		}
*/		
		sleep(10);
	}

/// release resources
	Irdm_Destory();
	return 0;
}

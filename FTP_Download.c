#include "FTP_Download.h"

int DL_FTP_List_Adjust(char *list_path,static_config *jtpz,dynamic_config *dtpz)
{
	FILE *fp;
	char DL_FTP_buf[MAX_DLFTP_NUM][MAX_NAME_LEN];
	memset(DL_FTP_buf, 0, sizeof(DL_FTP_buf));
	char file_list[PATHLENGTH] = {0};
	char delete[100]={0};
	int i = 0;
	int j = 0;
	
	fp = fopen(list_path,"r");
	if (NULL == fp)
	{
		WriteLog("DL_FTP_List file don't exist\n");
		return -1;
	}
	fclose(fp);
	
	fp = fopen(file_list,"r");
	if (NULL == fp)
	{
		WriteLog("LFTP_List num >1,but file don't exist\n");
		return -1;
	}
				
	while (fgets(DL_FTP_buf[i],MAX_NAME_LEN,fp) != NULL)
	{
		i++;
	}
	fclose(fp);
	remove(file_list);
	if (i < 2)
	{
		return -1;
	}				
	fp = fopen(file_list,"w");
	if (NULL == fp)
	{
		WriteLog("Adjust DL_FTP_List.dat,create file error\n");
		return -1;
	}
	for (j=1; j<i; j++)
	{
		fputs(DL_FTP_buf[j],fp);
	}
	fflush(fp);	
	fsync(fileno(fp));
	fclose(fp);

	return 0;		
}

int FTP_Downloadftp(int pc104_state,static_config *jtpz,dynamic_config *dtpz)
{
	printf("FTP_Downloadftp() is called!\n");
	
	FILE *fd_DL_FTPList;
	FILE *fp;
	char Feedback[MAX_SBDNAME_LEN]={0};
	char msg[MAX_SBDNAME_LEN] = {0};	
	char download_dest_src[MAX_SBDNAME_LEN] = {0};
	char down_filename[MAX_SBDNAME_LEN] = {0};
	
	int rtn;
	int len;
	unsigned char CmdType;
	unsigned short CmdNo;
	download_cmd cmd;
	memset(&cmd, 0,sizeof(cmd));
	
	execute_result rt;
	memset(&rt,0,sizeof(rt));
	char file_list[PATHLENGTH] = {0};	
	int count=0;
	int dialup_flag;
	//FTPDownload files processing
	if( (DL_FTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&DL_FTP_mutex,&DL_FTP_Occupy_Sign)==0) )	
	{
		//printf("In FTP_Downloadftp():Download files in DL_FTP list!\n");
		DL_FTP_Occupy_Sign=Occupied;
		if (pc104_state==PC104_Slave)
			sprintf(file_list,"%sDL_FTP_List.dat",jtpz->dM2S_filelist);	
		else
			sprintf(file_list,"%sDL_FTP_List.dat",jtpz->dFilelist);		
		fd_DL_FTPList=fopen(file_list, "r");
		if (fd_DL_FTPList == NULL)	
		{
			fd_DL_FTPList=fopen(file_list, "w");
			fclose(fd_DL_FTPList);	

			DL_FTP_Num = 0;
			DL_FTP_Occupy_Sign = NotOccupied;
			return -1;		
		}
		else
		{
			if (fgets(Feedback,MAX_SBDNAME_LEN,fd_DL_FTPList) == NULL)
			{
				fclose(fd_DL_FTPList);
				DL_FTP_Num = 0;
				DL_FTP_Occupy_Sign = NotOccupied;
				WriteLog("DL_FTP_Num > 0,but DL_FTP_List.dat is empty!!\n");				
				return -1;
			}
			fclose(fd_DL_FTPList);
			
			printf("+++++++++++++++++++downlaod file addr = %s",Feedback);
			
			len = strlen(Feedback);
			if (Feedback[len-1] == '\n')
				Feedback[len-1] = '\0';
				
			fp =fopen(Feedback,"r");
			if (fp == NULL)
			{
				rt.result = 'F';		
				WriteLog("download FTP files include download info not exist\n");
// file list adjust,ignore this file				
				rtn = List_Adjust(file_list,jtpz,dtpz);
				if (rtn < 0)
				{
					WriteLog("DL_FTP_List_Adjust error\n");
					DL_FTP_Num = 0;
					DL_FTP_Occupy_Sign = NotOccupied;
					return -1;
				}			
				DL_FTP_Num -= 1;
				DL_FTP_Occupy_Sign = NotOccupied;
				send_msg(MsgIDIrdm, (CmdType+1),(char *) &rt, sizeof(rt));			
				return;
			}
			fseek(fp,0,SEEK_END);
			len = ftell(fp);
			rewind(fp);
			fread(msg,len,1,fp);
			
			memcpy(&CmdType,msg,sizeof(CmdType));
			memcpy(&CmdNo,(msg+1), sizeof(CmdNo));
			memcpy(download_dest_src,(msg+sizeof(CmdType)+sizeof(CmdNo)),MAX_NAME_LEN);
			
			printf("CmdType = %d\n",CmdType);
			printf("CmdNo = %d\n",CmdNo);
			printf("download_dest_src = %s\n",download_dest_src);
					
// call ftp download script				
			printf("download file & destination = %s\n",download_dest_src);
			len = strlen(download_dest_src);
			if (download_dest_src[len-1] == '\n')
				download_dest_src[len-1] = '\0';
				
			time_t now;
			time(&now);
				
			if (Comm_Channel == OPENPORT)
			{	
				if (sbd_send_flag == 1)
				{
					ftp_send_flag = 0;
					DL_FTP_Occupy_Sign = NotOccupied;
					return  -1;
				}			
				while(downloadftp((download_dest_src)) != 0)
				{
					if (sbd_send_flag == 1)
					{
						system("/root/njkk/killftp");
						ftp_send_flag = 0;
						DL_FTP_Occupy_Sign = NotOccupied;
						return  -1;
					}
					ftp_send_flag = 1;
					count++;				
					if(count == DL_FTP_Wait_Time)
					{
						rtn = system("/root/njkk/killftp");
						if (rtn != 0)
						{
							WriteLog("kill lftp fail\n");
							system("/root/njkk/killftp");
						}	

						rt.result = 'F';								 
						WriteLog("OpenPort lftp download FTP files error\n");
						
						ftp_send_flag = 0;						
						DL_FTP_Occupy_Sign = NotOccupied;
						Comm_Channel = MODEM;
						OpenPort_last_time = now;
						
						send_msg(MsgIDPC104,139,NULL,0);  // restart OpenPort;
					//	send_msg(MsgIDIrdm, (CmdType+1),(char *) &rt, sizeof(rt));						
						return -1;
					}
				}	
				ftp_send_flag = 0;
				OpenPort_last_time = now;
			}
			else if (Comm_Channel == MODEM)
			{
				if ((sbd_rcv_flag == 1) || (sbd_snd_flag == 1))
				{
					ftp_deliver_flag=0;
					DL_FTP_Occupy_Sign = NotOccupied;
					return -1;
				}		
				
				system("cp /etc/ppp/resolv.conf /etc/resolv.conf");
				system("/root/njkk/initwvdial&");
				sleep(50);									
				dialup_flag = GainSystem("ifconfig ppp0 2>&1");//2010-6-28
				printf("flag1=%d\n", dialup_flag);
				if (dialup_flag != 1)
				{
					printf("linkdown, now kill wvdial and restart iridiumlink\n");
					KillWvdial();
					sleep(10);
					system("/root/njkk/initwvdial&");
					sleep(50);						
					dialup_flag = GainSystem("ifconfig ppp0 2>&1");//2010-6-28			
					printf("flag1=%d\n", dialup_flag);		
				}
				if (dialup_flag == 1)
				{
					while ( downloadftp(download_dest_src) != 0)
					{
						if ((sbd_rcv_flag == 1) || (sbd_snd_flag == 1))
						{
							system("/root/njkk/killftp");
							KillWvdial();
							ftp_deliver_flag = 0;
							DL_FTP_Occupy_Sign = NotOccupied;
							return  -1;
						}	
						//Trans_Mode = FTPSTART;
/// added ftp download flag;
						ftp_deliver_flag = 1;
						count++;
						if(count == DL_FTP_Wait_Time)
						{
							rtn = system("/root/njkk/killftp");
							if (rtn != 0)
							{
								WriteLog("kill lftp fail\n");
								system("/root/njkk/killftp");
							}	
							KillWvdial();
							sleep(10);
							rt.result = 'F';
							WriteLog("lftp download FTP files error\n");
							
							ftp_deliver_flag = 0;
							DL_FTP_Occupy_Sign = NotOccupied;
							Comm_Channel = OPENPORT;	
							Modem_last_time = now;
								
							send_msg(MsgIDPC104,137,NULL,0);  // restart modem;
							send_msg(MsgIDIrdm, (cmd.CmdType+1),(char *) &rt, sizeof(rt));									
							return -1;
						}
					}
					
					KillWvdial();	
					sleep(10);		
					ftp_deliver_flag = 0;		
					Modem_last_time = now;

			    }	
				else
				{
					KillWvdial();
					sleep(10);
					WriteLog("lftp download FTP files error\n");					
					rt.result = 'F';				

					ftp_deliver_flag = 0;
					Comm_Channel = OPENPORT;
					Modem_last_time= now;				
					DL_FTP_Occupy_Sign = NotOccupied;
				
					send_msg(MsgIDPC104,134,NULL,0);  // restart modem;
					send_msg(MsgIDIrdm, (cmd.CmdType+1),(char *) &rt, sizeof(rt));					
								
					return -1;
				}
			}
			Comm_Channel = OPENPORT;
// list adjust		
			rtn = List_Adjust(file_list,jtpz,dtpz);
			if (rtn < 0)
			{
				DL_FTP_Num = 0;
				DL_FTP_Occupy_Sign = NotOccupied;
				WriteLog("DL_FTP_List_Adjust error\n");				
				return -1;
			}			
			DL_FTP_Num -= 1;
			DL_FTP_Occupy_Sign = NotOccupied;	
					
//	delete sbd file
			remove(Feedback);
// slave finish then execute result is finished 
			if (PC104_state == PC104_Slave)	
				Slave_exe_result = FINISH;		
// upload sucessfully
			rt.result = 'S';
			send_msg(MsgIDIrdm, (CmdType+1),(char *) &rt, sizeof(rt));	
		}	
			
	}
	return 0;
}

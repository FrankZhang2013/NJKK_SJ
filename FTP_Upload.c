#include "FTP_Upload.h"

int FTP_Modem_OpenPort(char *fileaddr, int compress_flag,dynamic_config *dtpz,static_config *jtpz)
{
	FILE *fp;
	fp = fopen(fileaddr,"r");
	if (NULL == fp)
	{
		WriteLog("the upload file dosen't exist\n");
		return 1;
	}
	fclose(fp);
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	
	unsigned char CmdType;
	char *cmd_type;
	char *cmd_no;
	char *delim = "_";
	char str[MAX_NAME_LEN] = {0};
	char copy_cmd[CMDLENGTH] = {0};  // copy to /tmp/FTP
	
	int rt;
	int count = 0;
	DIR *dir;
	char current_dir[100] = {0};
	char compress_cmd[MAX_NAME_LEN] = {0};
	char uncompress_cmd[MAX_NAME_LEN] = {0};
	char compress_name[MAX_NAME_LEN] = {0};
	char filepath[MAX_NAME_LEN]={0};
	char filename[MAX_NAME_LEN]={0};
	char copy_filepath[MAX_NAME_LEN]={0};
	int dialup_flag;
	
	char upload_cmd_str[100] = {0};
	
	char newname[MAX_NAME_LEN] = {0};
	int file_size;
	int rename_rt;
	char file_type[10]={0};
	/// files won't compress:jpeg and *.gz  changed:20121123
	file_size = FILESIZE(fileaddr);	
	rt = getfile_type(fileaddr,file_type);
	if (rt < 0)  // don't include ".",as file size decide to compress or not
	{
		if (file_size < MIN_COMPRESS_FTP_SIZE)
			compress_flag=0;
	}
	else
	{
		char *p = strstr(Non_Compress_Type,file_type);
		if (NULL != p)
			compress_flag=0;
		else
		{
			if (file_size < MIN_COMPRESS_FTP_SIZE)
				compress_flag=0;			
		}
	}

	sprintf(copy_cmd,"cp %s %s",fileaddr,jtpz->dFTP);
	system(copy_cmd);
	
/// split direction and filename	
	sepFileDir(fileaddr,filepath,filename);
/// get filename: CmdType + CmdNo + ...	
	strcpy(str,filename);
	sprintf(copy_filepath,"%s%s",jtpz->dFTP,filename);
	
	getcwd(current_dir, sizeof(current_dir));
	dir = opendir(jtpz->dFTP);
	if (NULL == dir)
	{
		mkdir(filepath,755);
		perror("filepath doesn't exsit");
		return 1;   // delete list
	}

//// read nFTP 
	Read_dynamic_Config(jtpz,dtpz);
	unsigned int tmp_nFTP  = dtpz->nFTP ; 
	char str_nFTP[LongInt_LEN] = {0};
		
	chdir(jtpz->dFTP);	 	
	if (compress_flag == 1)
	{	
///////////////////////	sprintf(compress_name,"%s%s%d.gz",jtpz->dFTP,FTP_File_Header,tmp_nFTP);
		sprintf(compress_name,"%s%s%s.gz",jtpz->dFTP,FTP_File_Header,filename);
		sprintf(compress_cmd,"gzip -c %s>%s",filename,compress_name);
		
		rt = system(compress_cmd);
		if (rt != 0)
		{
			WriteLog("execute compress FTP file cmd fail\n");
			chdir(current_dir);
			return -1;
		}
		remove(copy_filepath);
		file_size = FILESIZE(compress_name);
		
		tmp_nFTP += 1; 	
		sprintf(str_nFTP,"%d",tmp_nFTP);	
		printf("#########################nFTP = %d\n",tmp_nFTP);	
		
		sprintf(newname,"%s%s%s.gz_%d",jtpz->dFTP,FTP_File_Header,filename,file_size);
		
		rename_rt = rename(compress_name, newname);
		if (rename_rt<0)
		{
			WriteLog("rename jpg error\n");
			chdir(current_dir);
			return -1;
		}		
		printf("uploadftp file:%s\n",newname);
	}
	else if (compress_flag == 0)
	{
		file_size = FILESIZE(fileaddr);
		
		tmp_nFTP += 1; 	
		sprintf(str_nFTP,"%d",tmp_nFTP);
		printf("#########################nFTP = %d\n",tmp_nFTP);		
			
		sprintf(newname,"%s%s%s_%d",jtpz->dFTP,FTP_File_Header,filename,file_size);
		rename_rt = rename(copy_filepath, newname);
		if (rename_rt<0)
		{
			WriteLog("rename jpg error\n");
			chdir(current_dir);
			return -1;
		}
	}
	sprintf(upload_cmd_str,"%s %s",FTP_Upload_DIR,newname);
	
	chdir(current_dir);
	
	time_t now;
	time(&now);
				
	if (Comm_Channel == OPENPORT)
	{
		///upload file			
		while ( uploadftp(upload_cmd_str) != 0)
		{
			if (sbd_send_flag == 1)
			{
				ftp_send_flag = 0;
				return -1;
			}
			ftp_send_flag = 1;
			count++;
			if (count == FTP_Wait_Time)
			{
				rt = system("/root/njkk/killftp");
			/*
				if (rt != 0)
				{
					ftp_send_flag = 0;					
					Comm_Channel = MODEM;				

					send_msg(MsgIDPC104,139,NULL,0);  // restart OpenPort;	
													
					WriteLog("uload FTP by OpenPort, try ** time fail, kill lftp fail\n");
					return -1;
				}	
			*/	
				ftp_send_flag = 0;										
				Comm_Channel = MODEM;
				OpenPort_last_time = now;				
				
				send_msg(MsgIDPC104,139,NULL,0);  // restart OpenPort;
					
				WriteLog("OpenPort:lftp upload FTP files too long time ,kill\n");							
				return -1;
			}
			usleep(5000);
		}
//		Change_Config(dynamic_configfile_path,"nFTP",str_nFTP);	
		ftp_send_flag = 0;
		Comm_Channel = OPENPORT;
		OpenPort_last_time = now;
	}
	else if (Comm_Channel == MODEM)
	{
		// dial up
		system("/root/njkk/initwvdial&");
		sleep(50);	
		// check whether dial up correctly		
		dialup_flag=GainSystem("ifconfig ppp0 2>&1");//2010-6-28
		printf("flag1=%d\n", dialup_flag);
		if (dialup_flag!=1)
		{
			printf("linkdown, now kill wvdial and restart iridiumlink\n");
			KillWvdial();
			sleep(10);
			system("/root/njkk/initwvdial&");
			sleep(50);	
			dialup_flag=GainSystem("ifconfig ppp0 2>&1");//2010-6-28			
		}
		if (dialup_flag == 1)
		{
			// dial up 
			system("/root/njkk/initwvdial&");
			sleep(50);			
			while ( uploadftp(upload_cmd_str) != 0)
			{
				if (sbd_send_flag == 1)
				{
					ftp_send_flag = 0;
					return -1;
				}	
/// added ftp download flag;
				ftp_deliver_flag = 1;
				count++;
				if (count == FTP_Wait_Time)
				{
					rt = system("/root/njkk/killftp");
					if (rt != 0)
					{
						KillWvdial();	
						ftp_deliver_flag = 0;																	
						Comm_Channel = OPENPORT;	
						Modem_last_time = now;							
						WriteLog("upload FTP by Modem , try ** time fail,kill lftp fail\n");
						send_msg(MsgIDPC104,137,NULL,0);  // restart modem;											
						return -1;
					}
					
					KillWvdial();
				//	rt.result = 'F';
				//	send_msg(MsgIDIrdm, (cmd.CmdType+1),(char *) &rt, sizeof(rt));
					ftp_deliver_flag = 0;																	
					Modem_last_time = now;
					WriteLog("Modem :lftp upload FTP files too long time ,kill\n");					

					send_msg(MsgIDPC104,137,NULL,0);  // restart modem;						
					Comm_Channel = OPENPORT;	
			
					return -1;
				}
				usleep(5000);
			 }		 		
			KillWvdial();
//			Change_Config(dynamic_configfile_path,"nFTP",str_nFTP);
			ftp_deliver_flag = 0;	
			Modem_last_time = now;				
		}
		else
		{
			ftp_deliver_flag = 0;
			Comm_Channel = OPENPORT;	
			Modem_last_time = now;	
			send_msg(MsgIDPC104,137,NULL,0);  // restart modem;			
			return -1;
		}	 	 
	}
	else  // all channel is bad 
	{
		ftp_send_flag = 0;
		ftp_deliver_flag = 0;	
		Comm_Channel = OPENPORT;	
		printf("OpenPort & Modem are bad all!,,Irdm_main  exit \n");
	}
///upload file successfully and delete compressed file 
		
	rt = remove(newname);	  // dlete file error, classify list, ignore  this error
	if (rt == EOF)
	{
		WriteLog("delete .gz file fail\n");
		return 1;
	}
	
	printf("---------CmdType = %d  msg_feedback.CmdNo=%d		msg_feedback.result = %c\n",CmdType,msg_feedback.CmdNo,msg_feedback.result);

	return 0;	
}

int  FTP_Uploadftp(int pc104_state,static_config *jtpz,dynamic_config *dtpz)
{
	printf("FTP_Uploadftp() is called!\n");
	int Total_FTP_Num;
	int tmp_num;
	Total_FTP_Num = HFTP_Num + MFTP_Num + LFTP_Num;	
	FILE *fd_FTP_List;

	char Feedback[MAX_NAME_LEN]={0};
	char FTP_level;
	char *str;
	
	int compress_flag = 1; //compress flag default value;
	
	int result;
	char filename[MAX_NAME_LEN] = {0};
	int len=0;
	char file_list[PATHLENGTH]={0};
	
	// read  HFTP
		if (HFTP_Num > 0)
		{
			if( (HFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&HFTP_Occupy_Sign)==0) )		
			{
				FTP_level = 'H';
				str = "H";
				HFTP_Occupy_Sign = Occupied;
				tmp_num = HFTP_Num;
				sprintf(file_list,"%s%sFTP_List.dat",jtpz->dFilelist,str);
				fd_FTP_List = fopen(file_list, "r");
				if (NULL == fd_FTP_List)	
				{
					fd_FTP_List = fopen(file_list, "w");
					fclose(fd_FTP_List);	
					WriteLog("HFTP_Num > 0,but can't find the file:HFTP_List.dat\n");
					HFTP_Num = 0;
					HFTP_Occupy_Sign = NotOccupied;
					return -1;		
				}
				else
				{
					if (fgets(Feedback,MAX_NAME_LEN,fd_FTP_List) == NULL)
					{
						fclose(fd_FTP_List);
						HFTP_Num = 0;
						HFTP_Occupy_Sign = NotOccupied;
						WriteLog("HFTP_Num > 0,but HFTP_List.dat is empty!!\n");					
						return -1;
					}
					fclose(fd_FTP_List);
					
					len = strlen(Feedback);
					if (Feedback[len-1] == '\n')
						Feedback[len-1] = '\0';
					
					/// Upload HFTP file
					printf("In FTP_Uploadftp():HFTPUpload:%s!\n",Feedback);
					result = FTP_Modem_OpenPort(Feedback, compress_flag,dtpz,jtpz);	
					if (result < 0)
					{
						if (PC104_state == PC104_Slave)
						{
							List_Adjust(file_list,jtpz,dtpz);
							HFTP_Num -= 1;								
							rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
							remove(M2S_cmd);
							memset(M2S_cmd,0,sizeof(M2S_cmd));
							Slave_exe_result = FINISH;									
						}						
						HFTP_Occupy_Sign = NotOccupied;
						WriteLog("Upload HFTP fail!!\n");  						
						return -1;
					}
					
					result = List_Adjust(file_list,jtpz,dtpz);
					if (result < 0)
					{
						HFTP_Num = 0;
						HFTP_Occupy_Sign = NotOccupied;
						WriteLog("HFTP_List_Adjust error\n");						
						return -1;
					}

					HFTP_Num -= 1;
	
					HFTP_Occupy_Sign = NotOccupied;
		// slave finish then execute result is finished 
					if (PC104_state == PC104_Slave)	
						Slave_exe_result = FINISH;					
					return 0;
				}
			}
		}
	// read  MFTP
		else if (MFTP_Num > 0)
		{
				if( (MFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&MFTP_Occupy_Sign)==0) )		
				{
					FTP_level = 'M';
					str="M";
					MFTP_Occupy_Sign = Occupied;
					tmp_num = MFTP_Num;
					sprintf(file_list,"%s%sFTP_List.dat",jtpz->dFilelist,str);
					fd_FTP_List = fopen(file_list, "r");
					if (fd_FTP_List == NULL)	
					{
						fd_FTP_List=fopen(file_list, "w");
						fclose(fd_FTP_List);	
						MFTP_Num = 0;
						MFTP_Occupy_Sign = NotOccupied;
						return -1;		
					}
					else
					{
						if (fgets(Feedback,MAX_NAME_LEN,fd_FTP_List) == NULL)
						{
							fclose(fd_FTP_List);
							MFTP_Num = 0;
							MFTP_Occupy_Sign = NotOccupied;
							WriteLog("MFTP_Num > 0,but MFTP_List.dat is empty!!\n");							
							return -1;
						}
						fclose(fd_FTP_List);
						
						len = strlen(Feedback);
						if (Feedback[len-1] == '\n')
							Feedback[len-1] = '\0';
						
						/// Upload MFTP file
						printf("In FTP_Uploadftp():MFTPUpload:%s!\n",Feedback);
						result = FTP_Modem_OpenPort(Feedback, compress_flag,dtpz,jtpz);	
						if (result < 0)
						{
							if (PC104_state == PC104_Slave)
							{
								List_Adjust(file_list,jtpz,dtpz);
								MFTP_Num -= 1;								
								rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
								remove(M2S_cmd);
								memset(M2S_cmd,0,sizeof(M2S_cmd));
								Slave_exe_result = FINISH;									
							}
							MFTP_Occupy_Sign = NotOccupied;							
							WriteLog("Upload fail!!\n");
							return -1;
						}
						
						result = List_Adjust(file_list,jtpz,dtpz);
						if (result < 0)
						{
							MFTP_Num = 0;
							MFTP_Occupy_Sign = NotOccupied;
							WriteLog("MFTP_List_Adjust error\n");							
							return -1;
						}
						MFTP_Num -= 1;
						MFTP_Occupy_Sign = NotOccupied;
					// slave finish then execute result is finished 
						if (PC104_state == PC104_Slave)	
							Slave_exe_result = FINISH;					
						return 0;
					}
				}
		}
	// read  LFTP
		else if (LFTP_Num > 0)
		{
			if( (LFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&LFTP_Occupy_Sign)==0) )		
			{
				FTP_level = 'L';	
				str = "L";
				LFTP_Occupy_Sign = Occupied;
				tmp_num = LFTP_Num;
				sprintf(file_list,"%s%sFTP_List.dat",jtpz->dFilelist,str);
				fd_FTP_List = fopen(file_list, "r");
				if (fd_FTP_List==NULL)	
				{
					fd_FTP_List=fopen(file_list, "w");
					fclose(fd_FTP_List);
					LFTP_Num = 0;
					LFTP_Occupy_Sign = NotOccupied;	
					return -1;		
				}
				else
				{
					if (fgets(Feedback,MAX_NAME_LEN,fd_FTP_List) == NULL)
					{
						fclose(fd_FTP_List);
						LFTP_Num = 0;
						LFTP_Occupy_Sign = NotOccupied;
						WriteLog("LFTP_List.dat don't have file to upload\n");
						return -1;
					}
					fclose(fd_FTP_List);
					
					len = strlen(Feedback);
					if (Feedback[len-1] == '\n')
						Feedback[len-1] = '\0';
					
					/// Upload LFTP file
					printf("In FTP_Uploadftp():LFTPUpload:%s!\n",Feedback);
					result = FTP_Modem_OpenPort(Feedback, compress_flag,dtpz,jtpz);	
					if (result < 0)
					{
						if (PC104_state == PC104_Slave)
						{
							List_Adjust(file_list,jtpz,dtpz);
							LSBD_Num -= 1;								
							rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
							remove(M2S_cmd);
							memset(M2S_cmd,0,sizeof(M2S_cmd));
							Slave_exe_result = FINISH;									
						}
						LFTP_Occupy_Sign = NotOccupied;						
						WriteLog("Upload fail!!\n");
						return -1;
					}
						
					result = List_Adjust(file_list,jtpz,dtpz);
					if (result < 0)
					{
						LFTP_Num = 0;
						LFTP_Occupy_Sign = NotOccupied;
						WriteLog("LFTP_List_Adjust error\n");						
						return -1;
					}
					LFTP_Num -= 1;
					LFTP_Occupy_Sign = NotOccupied;
				// slave finish then execute result is finished 
					if (PC104_state == PC104_Slave)	
						Slave_exe_result = FINISH;
					return 0;
				}
			  }		
		  }
}

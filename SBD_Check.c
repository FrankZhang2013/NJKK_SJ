#include "SBD_Check.h"
										   //	MsgIDPLC,  MsgIDIrdm, MsgIDPC104, MsgIDNAS, MsgIDCamera
int SBD_Check(unsigned char *str,int length,int msqid,int msqid0,int msqid1,int msqid2,int msqid3,static_config *jtpz,dynamic_config *dtpz)
{
	unsigned char *str1;
	unsigned char SBDType;
	//int msqid;
	
	if (length>max_RcvSBD )//|| length != strlen(str))
	{
		WriteLog("check SBD command whether it's error\n");
		return -1;
	}
	Log_cmd(str,length);
	memcpy(&SBDType,str,sizeof(SBDType));
	str1 = str+sizeof(SBDType);
	
	printf("SBDType = %d\n",SBDType);
	printf("SBDtype -->%s\n",str1);	
	
	switch(SBDType)
	{
	//   PLC control/inquiry Cmd   					--->PLC
	 	case 1:  //PLC start/stop
	 	case 3:  //PLC runnig
	 	case 5:  //generator priority 
	 	case 7:  //realtime inquiry analog
	 	case 9:  //realtime inquiry digital
		case 11: //get realtime monitor data
	 	case 13: //history inquiry analog
	 	case 15: //history inquiry digital
	 	case 17: //inquiry one running state 
	 	
				   WriteLog("enter PLC control/inquiry  command!");
				   send_msg(msqid,SBDType,(char *)str1,(length-sizeof(SBDType)));  
				   WriteLog("send msg sucessfully!\n");	
	
			    break;
		case 73:  // upload camera picture to nanjing    ---> Video
				send_msg(msqid3,SBDType,(char *)str1,(length-sizeof(SBDType))); 
				break;
	
	// 	NAS control command								----->  NAS
		case 91:
		case 92:
		case 93:
		case 94:
				send_msg(msqid2,SBDType,(char *)str1,(length-sizeof(SBDType))); 
				break;
				
	//  Irdm File process command
		case 101:
				file_upload_cmd(str1, SBDType,(length-sizeof(SBDType)), msqid0, jtpz, dtpz);
				break;
		case 103:
				file_download_cmd(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;
		case 109:
				search_direction_cmd(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;
		case 111:
				file_copy_cmd(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;
		case 113:
				file_delete_cmd(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;
		case 115:
				file_compress_cmd(str1, SBDType, msqid0, jtpz, dtpz);
				break;
		case 117:
				file_decompress_cmd(str1, SBDType, msqid0, jtpz, dtpz);
				break;
		case 119:
				system_cmd(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;
				
	//  system repaire & remaintance command					-----> PC104				    
	 	case 75: // temperature up and down limit value   ---> PC104	
		case 131:
		case 133:
		case 135:
		case 137:
		case 139:
		case 141:
		case 143:
		case 145:
		case 147:
		case 149:
				send_msg(msqid1,SBDType,(char *)str1,(length-sizeof(SBDType))); 
				break;
	//  software update command 			
		case 201:	//check version
				check_software_version(str1, SBDType, msqid0, jtpz, dtpz);
				break;
		case 203:   // check bakup file
				check_backup(str1, SBDType, msqid0, jtpz, dtpz);
				break;
		case 205:   // update software version
				update_software(str1, SBDType, (length-sizeof(SBDType)),msqid0, jtpz, dtpz);
				break;

		default:
				WriteLog("ERROR:There's no the SBD Cmd!!1\n");
				break;	
	 }	 
	 return 0;
}

void SBDCmd_Read_Modem(static_config *jtpz,dynamic_config *dtpz)
{
	printf("SBDCmd_Read_Modem() is called!\n");
	unsigned char msglen;
	int length=0;
	unsigned char sbd_cmd[max_RcvSBD] = {0};
	int rt;
/// add sbd rcv flag
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++beagin to modem read cmd\n");
	sbd_rcv_flag = 1;
	rt = RCVmainfun();
	sbd_rcv_flag = 0;
	printf("------------------------------------------------- end  of  modem read cmd!\n");
	if (rt < 0)
		return;
		
	memcpy(&msglen,(const char*)SBDbuffer,sizeof(msglen));
	while (msglen > 0)
	{
		length += sizeof(msglen);			
		memcpy(sbd_cmd,(const char*)(SBDbuffer+length),msglen);			
		SBD_Check(sbd_cmd,msglen,MsgIDPLC,MsgIDIrdm,MsgIDPC104,MsgIDNAS,MsgIDCamera,jtpz,dtpz);
		length += msglen;
		memcpy(&msglen,(const char*)(SBDbuffer+length),sizeof(msglen));
		memset(sbd_cmd,0,max_RcvSBD);
	}	
	return;
}

int parser_cmd(char *filename,static_config *jtpz,dynamic_config *dtpz)
{
	FILE *fp;
	unsigned char buffer[max_RcvSBD];
	unsigned char sbd_cmd[max_RcvSBD] = {0};
	unsigned short msglen; //  original unsigned char
	unsigned char msgtype;
	int length=0;
	int cmd_len=0;
	int rt;
	if (NULL == filename)
		return -1;
	fp=fopen(filename,"rb");
	if(NULL==fp)
	{
		fprintf(stdout,"open %s error",filename);
		return 0;
	}
	
	fseek(fp,0,SEEK_END);
	cmd_len = ftell(fp);
	if (cmd_len <= 0 &&  cmd_len > max_RcvSBD)
	{
		return 0;
	}
	
	rewind(fp);

	fread(buffer,sizeof(char),cmd_len,fp);
	fclose(fp);
	
	rt = ReadMTBuf(buffer,OPENPORT,cmd_len);
	if (rt < 0)
	{
		return 0;
	}
	
	memcpy(&msglen,SBDbuffer,sizeof(msglen));
	printf("first command length = %d\n",msglen);
	while (msglen > 0)
	{
		length += sizeof(msglen);			
		memcpy(sbd_cmd,(SBDbuffer+length),msglen);			
		SBD_Check(sbd_cmd,msglen,MsgIDPLC,MsgIDIrdm,MsgIDPC104,MsgIDNAS,MsgIDCamera,jtpz,dtpz);
		length += msglen;
		memcpy(&msglen,(SBDbuffer+length),sizeof(msglen));
		memset(sbd_cmd,0,max_RcvSBD);
	}
	return 1;
}

int SBDCmd_Read_OpenPort(static_config *jtpz,dynamic_config *dtpz)
{
	printf("SBDCmd_Read_OpenPort() is called!\n");
	char filename[CMDFILES_MAX][INT2STR_LEN];
	char filesize[CMDFILES_MAX][INT2STR_LEN];
	int i;
	int num = 0;
	unsigned char buffer[max_RcvSBD];
	unsigned char buf[MAX_CMD_LEN]={0};

	memset(buffer,0,sizeof(buffer));
	unsigned char sbd_cmd[max_RcvSBD] = {0};

	char file_rsync[PATHLENGTH]={0};

	int err;
	char *zzz_file;
	
	int rt;

	//travel through the src cmd file directory
	num = trave_dir(jtpz->dSrc_Cmd,filename);
	if (num > 0)
	{
		BubbleSortStr(filename,num);
		printf("After sorting：\n");
		for(i = 0; i < num; i++)
		{
			printf("filename[%d]:%s\n",i,filename[i]);
		}
		for(i=0; i<num; i++)
		{		
			memset(file_rsync,0,sizeof(file_rsync));
			sprintf(file_rsync,"%s%s",jtpz->dSrc_Cmd,filename[i]);			
			if(checkFileGood(jtpz->dSrc_Cmd,filename[i])==0)
			{//file is good,begin processing this cmdfile
				printf("File '%s' is good,begin processing!\n",filename[i]);
				if (PC104_state == PC104_Slave)
				{
					rt = rsync_file(file_rsync,"src_cmd",jtpz,dtpz);
					printf("---2222---slave rsync cmd file to src-cmd!\n");
					if (0 == rt)
					{
						remove(file_rsync);
					}
					
				}
				else
				{
					copy_file(file_rsync,jtpz->dCmd);
					remove(file_rsync);
				}
			}
			else
			{
				if (i == num-1)
				{
					break;			/// 最后一个文件大小不一致，有可能没传输完成
				}
				else
				{
					remove(file_rsync);   /// 不是最后一个上传文件则已经上传完成但大小出错删除。
					continue;
				}
			}
		}
	}
	num=trave_dir(jtpz->dCmd,filename);
	if (num > 0)
	{
		BubbleSortStr(filename,num);
		printf("After sorting：\n");
		for(i = 0; i < num; i++)
		{
			printf("filename[%d]:%s\n",i,filename[i]);
		}
		for(i=0; i<num; i++)
		{	
			memset(file_rsync,0,sizeof(file_rsync));
			sprintf(file_rsync,"%s%s",jtpz->dCmd,filename[i]);					
			if(checkFileGood(jtpz->dCmd,filename[i])==0)
			{//file is good,begin processing this cmdfile
				printf("File '%s' is good,begin processing!\n",filename[i]);
				if (PC104_state == PC104_Slave || PC104_state == PC104_Single_Master)
				{
					Slave_exe_result = UNFINISH;
					printf("----2222---beagin to parse cmd !\n");						
					rt = parser_cmd(file_rsync,jtpz,dtpz);
///删除处理完的命令文件	
					if (rt>=0 && PC104_state == PC104_Single_Master)
					{
						remove(file_rsync);		
					}	
					if (rt == 0 && PC104_state == PC104_Slave)
					{
						remove(file_rsync);
						Slave_exe_result = FINISH;
					}
					strcpy(M2S_cmd,file_rsync);
					return 0;
				}
				else
				{
					// parse and rsync
					if ( (OpenPort_state == OK && opposite_openport_state==OK) || (OpenPort_state == Error && opposite_openport_state == Error && Modem_state == OK && opposite_modem_state == OK))
					{
					
						if (i==1 && Slave_exe_result == FINISH)
						{
							rt = rsync_file(file_rsync,"cmd",jtpz,dtpz);
							printf("----2222---master rsync cmd file to slave for heavy task!\n");
							if (0 == rt)
							{
								Slave_exe_result = UNFINISH;							
								remove(file_rsync);
							}
							return 0;
						}
						rt = parser_cmd(file_rsync,jtpz,dtpz);
						if (0 == rt)
							remove(file_rsync);					
					}
					// rsync only
					else if ( (OpenPort_state==Error && opposite_openport_state == OK) || (OpenPort_state==Error && opposite_openport_state==Error && Modem_state==Error && opposite_modem_state==OK))
					{
						rt = rsync_file(file_rsync,"cmd",jtpz,dtpz);
						printf("----2222---master rsync all file to slave!\n ");
						if (0 == rt)
						{
							remove(file_rsync);
						}
					}
					// not rsync
					else
					{
						rt = parser_cmd(file_rsync,jtpz,dtpz);
	///删除处理完的命令文件 					
						if (rt==0)
							remove(file_rsync);	
					}
				}
			}
			else
			{
				remove(file_rsync);
			}
		}
	}
	return 0;	
}

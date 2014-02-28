#include "SBD_Upload.h"

int Modem_SBD_send_mainfun(unsigned char SENDFILE[100])
{
	time_t now;
	time(&now);
	 printf("The filename is  %s\n",SENDFILE);
	 if(SENDFILE==NULL)
	 {
		 printf("There is no file..exit\n");
		 return 1;
	 }

	 unsigned char buffer1[max_RcvSBD];
	 unsigned char Sendmsg[max_SendSBD];
	 char sbdwbcmd[128];
	 int retv;
	 int rt;
	 FILE *fp;
	 int len;
	 int checksum;                                          /*checksum 2bytes*/

	 memset(sbdwbcmd,'\0',128); 
	 memset(Sendmsg,'\0',max_SendSBD);
	 memset(buffer1,'\0',max_RcvSBD);

	 len=FILESIZE(SENDFILE);
	 printf("The file length=%d\n",len);
	 if(len>1958)
	 {
		printf("The file is too long!\n");
		return -1;
	 }
	 if(len==0)
	 {
		printf("The empty SBD file!\n");
		return -1;
	 }

	 fp=fopen((const char*)SENDFILE,"rb");
	 if (fp == NULL)
	{
		printf("The SBD file does not exist!\n");
		return -1;
	}         
	 fread(Sendmsg,1,len,fp);
	 fclose(fp);
//==============  CRC for modem =======================================
	 sprintf(sbdwbcmd,"at+sbdwb=%d\r",len);
/*
	 printf("sbdwblen=%c%c%c%c\n",sbdwbcmd[9],sbdwbcmd[10],sbdwbcmd[11],sbdwbcmd[12]);
	 checksum=Checksum(Sendmsg,len);                  
	 Sendmsg[len]=(checksum>>8)&0x000000FF;           //Add checksum 2bytes/
	 Sendmsg[len+1]=checksum&0x000000FF;
	 printf("Checksum=%x %x\n",Sendmsg[len],Sendmsg[len+1]);
*/
///added sbd snd flag;
	sbd_snd_flag = 1;

	 open_serial(0);

	 struct termios newtio;
	 struct termios oldtio;
	 bzero(&newtio,sizeof(newtio));
	 tcgetattr(fd_serial,&oldtio);
	 cfsetispeed(&newtio,B19200);      /*波特率设置为19200bps*/
	 cfsetospeed(&newtio,B19200);
	 newtio.c_cflag&=~PARENB;           /*无校验位*/
	 newtio.c_cflag&=~CSTOPB;          /*1位停止位*/ 
	 newtio.c_cflag&=~CSIZE;
	 newtio.c_cflag|=CS8;              /*8位数据位*/
	 newtio.c_cflag|=(CLOCAL|CREAD);
	 newtio.c_lflag&=~(ICANON|ECHO|ECHOE|ISIG);
	 newtio.c_oflag&=~OPOST;
	 newtio.c_cc[VMIN]=1;
	 newtio.c_cc[VTIME]=0;
	 tcsetattr(fd_serial,TCSANOW,&newtio);
	 sleep(1);

	 printf("ready for sending data...\n");

	 sleep(1);

	 init_iridium(fd_serial,"+++\r");//2010-7-28
	 init_iridium(fd_serial,"ATH0\r");//2010-7-28

	 rt = init_iridium(fd_serial,"atz\r");
	 if(rt<0)
	 {
		 Modem_last_time = now;
		 Comm_Channel = OPENPORT;
		return -1;
	 }
	 init_iridium(fd_serial,"ate0q1\r");
	 init_iridium(fd_serial,"at+sbdd2\r");
	 retv=init_iridium(fd_serial,sbdwbcmd);
	 if(retv==3)
	 {
		printf("Preparing to write the MOmessage!\n");
	    if (write(fd_serial,Sendmsg,len) < 0)
	    {
			WriteLog("write to file error\n");
			sbd_snd_flag = 0;
			
			return -1;
		}

	 	sleep(1);
		SERIAL_RX1(buffer1);
	 	printf("buffer1=%s\n",buffer1);

		if(strncmp((const char*)buffer1,"\r\n0\r\n",5)==0)
			printf("Write MOmessage Success!\n\n");
	 }

	 printf("Prepare To Initial SBD sension!\n");
	 rt = init_iridium(fd_serial,"at+sbdi\r");
	 if(rt<0)
	 {
		 Modem_last_time = now;
		 Comm_Channel = OPENPORT;
		return -1;
	 }
	 if (MT == 1)
	 {
		 rt = init_iridium(fd_serial,"at+sbdrt\r");
		 if(rt<0)
		 {
			 Modem_last_time = now;
			 Comm_Channel = OPENPORT;
			return -1;
		 }
	 }
	 sleep(1);

	 tcsetattr(fd_serial,TCSANOW,&oldtio);
	 printf("tcsetattr-oldtio=%d\n",tcsetattr(fd_serial,TCSANOW,&oldtio));
	 printf("Restored PortConfig\n");

	 flag_close =close(fd_serial);  
	 if(flag_close==-1)
	    printf("Close the Device fail\n");


///end sbd snd flag
	sbd_snd_flag = 0;
	Comm_Channel = OPENPORT;
	Modem_last_time = now;
	if (MO ==1)
		return 0;
	else
		return -1;
}

///\//////////////////////////////SBDUpload:Modem Mode/////////////////////////////////

int SBD_Modem_Upload(static_config *jtpz,dynamic_config *dtpz)
{
	FILE *fd_HSBD_List,*fd_MSBD_List,*fd_LSBD_List;
	int  SBD_compress_num;
	unsigned char SBD_send[MAX_SBDNAME_LEN] = {0};
	char *SBD_level=NULL;
	int result;
	char file_list[PATHLENGTH]={0};
// read  HSBD
	if (HSBD_Num > 0)
	{
 	 	if( (HSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&HSBD_Occupy_Sign) == 0) )		
		{
			HSBD_Occupy_Sign = Occupied;
			SBD_level = "H";			
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);
//			SBD_compress_num = Adjust_IrdmFile("/home/njkk/download/HSBD_List.dat", "/home/njkk/download/");
			SBD_compress_num = SBD_OpenPort_Compress(file_list, HSBD_Num,jtpz,dtpz);
			if (SBD_compress_num <0)
				return -1;
			else if (SBD_compress_num > 0)
			{
				HSBD_Num = SBD_compress_num;
			}

			fd_HSBD_List = fopen(file_list,"r");
			if (fgets((char*)SBD_send,MAX_SBDNAME_LEN,fd_HSBD_List) == NULL)
			{
				WriteLog("read /home/njkk/download/HSBD_List.dat fail\n");
				return -1;
			}

			result = Modem_SBD_send_mainfun(SBD_send);

			if (result >0 )
			{
				List_Adjust(file_list,jtpz,dtpz);			
				HSBD_Num -= 1;
				HSBD_Occupy_Sign = NotOccupied;					
			}
			else
			{
				HSBD_Occupy_Sign = NotOccupied;	
				return -1;
			}		
			return 0;
		}
	}
	else if(MSBD_Num > 0)
	{
		if( (MSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&MSBD_Occupy_Sign) == 0) )		
		{	
			MSBD_Occupy_Sign = Occupied;	
			SBD_level = "M";			
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);
//			SBD_compress_num = Adjust_IrdmFile("/home/njkk/download/MSBD_List.dat", "/home/njkk/download/");
			SBD_compress_num = SBD_OpenPort_Compress(file_list, MSBD_Num,jtpz,dtpz);
			if (SBD_compress_num <0)
				return -1;
			else if (SBD_compress_num > 0)
			{
				MSBD_Num = SBD_compress_num;
			}

			fd_MSBD_List = fopen(file_list,"r");
			if (fgets((char*)SBD_send,MAX_SBDNAME_LEN,fd_MSBD_List) == NULL)
			{
				WriteLog("read /home/njkk/download/MSBD_List.dat fail\n");
				return -1;
			}
			result = Modem_SBD_send_mainfun(SBD_send);
			
			if (result >0)
			{
				List_Adjust(file_list,jtpz,dtpz);		
				MSBD_Num -= 1;
				MSBD_Occupy_Sign = NotOccupied;											
			}
			else
			{
				MSBD_Occupy_Sign = NotOccupied;
				return -1;					
			}			

			return 0;
		}
	}
	else if (LSBD_Num > 0)
	{
		if ( (LSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&LSBD_Occupy_Sign)==0) )		
		{
			LSBD_Occupy_Sign = Occupied;
			SBD_level = "L";			
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);			
//			SBD_compress_num = Adjust_IrdmFile("/home/njkk/download/LSBD_List.dat", "/home/njkk/download/");
			SBD_compress_num = SBD_OpenPort_Compress(file_list, LSBD_Num,jtpz,dtpz);
			if (SBD_compress_num <0)
				return -1;
			else if (SBD_compress_num > 0)
			{
				LSBD_Num = SBD_compress_num;
			}

			fd_LSBD_List = fopen(file_list,"r");
			if (fgets((char*)SBD_send,MAX_SBDNAME_LEN,fd_LSBD_List) == NULL)
			{
				WriteLog("read /home/njkk/download/LSBD_List.dat fail\n");
				return -1;		
			}
			result = Modem_SBD_send_mainfun(SBD_send);
			if (result > 0)
			{
				List_Adjust(file_list,jtpz,dtpz);		
				LSBD_Num -= 1;
				LSBD_Occupy_Sign = NotOccupied;					
			}
			else
			{
				LSBD_Occupy_Sign = NotOccupied;
				return -1;			
			}
			return 0;
		}		
	}
	return -1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///\////////////////// Compress SBD files and upload them using Openport mode//////////////

int SBD_OpenPort_Compress(char *SBD_Name,int num,static_config *jtpz,dynamic_config *dtpz)
{
	int rtn=-7;

	char zSBDPath[MAX_SBDNAME_LEN] = {0};
	char nzSBDPath[MAX_SBDNAME_LEN] = {0};
	char SBD_new_name[MAX_SBDNAME_LEN] = {0};
	
	FILE *tFSBDList,*tFpR,*tFpW;
	char tStr[MAX_SBDNAME_LEN] = {0};

	char tSBD[max_SendSBD*2+2*sizeof(unsigned short)] = {0};
	char tBuff[max_SendSBD*2+2*sizeof(unsigned short)] = {0};
	char dest_compress[max_SendSBD*2+2*sizeof(unsigned short)] = {0};
	char dest_sbd[max_SendSBD*2+2*sizeof(unsigned short)]={0};
	char tmp_buf[max_SendSBD] = {0};
	int OpenPort_MAX_SBD_LEN = max_SendSBD;

	char SBD_Name_List[MAX_SBD_NO][MAX_SBDNAME_LEN]; 
	memset(SBD_Name_List,0,sizeof(SBD_Name_List));
	int err;
	
	unsigned char tFlag;
	unsigned char compress_flag;
	long int tFSize=0;
	long int tFSize1;
	
	int compressed_no = 0;
	
	int ti;
	int len_SBD_Name = 0;
	tFSBDList=fopen(SBD_Name,"r");

	rtn++;
	if (tFSBDList == NULL)
	{
		return rtn;
	}

	rtn++;
	if ( fgets(tStr,MAX_SBDNAME_LEN,tFSBDList) == NULL )
	{
		fclose(tFSBDList);
		return rtn;
	}
		
// read nSBD from dynamic.xml
	 Read_dynamic_Config(jtpz,dtpz);
	 unsigned int tmp_nSBD = dtpz->nSBD;
	 tmp_nSBD += 1;
	 
	sprintf(zSBDPath,"%s%s%d.zzz",jtpz->dSBD,SBD_File_Header,tmp_nSBD);	
	sprintf(nzSBDPath,"%s%s%d",jtpz->dSBD,SBD_File_Header,tmp_nSBD);
	//bzero(tSBD,max_SendSBD+4);		// 
	
	char str_nSBD[LongInt_LEN] = {0};
	sprintf(str_nSBD,"%d",tmp_nSBD);
	Change_Config(dynamic_configfile_path,"nSBD",str_nSBD,jtpz,dtpz);
	
	tFSize=0;
	char *p;
	unsigned short len_sbd = 0;
////////  compress SBD	
	do
	{		
		p = strstr(tStr,".zzz");
		if ( p==NULL )
		{		
// delete the '\n'
			len_SBD_Name = strlen(tStr);
			if (tStr[len_SBD_Name-1] == '\n')
					tStr[len_SBD_Name-1] = '\0';			
			
			tFpR=fopen(tStr,"rb");
			if (tFpR!=NULL)
			{
				fseek(tFpR,0,SEEK_END);
				len_sbd = ftell(tFpR);
				if (len_sbd <= 0)
				{
					fclose(tFpR);
					continue;
				}
				rewind(tFpR);
				memset(tmp_buf,0,sizeof(tmp_buf));	
				ti = fread(tmp_buf,sizeof(char),len_sbd,tFpR);			
//				ti=fread( (tSBD+tFSize+sizeof(unsigned short)),sizeof(char),len_sbd,tFpR);
				
				if (ti != len_sbd)
				{
					fclose(tFpR);
					continue;				
				}
				fclose(tFpR);			
				ti+=sizeof(len_sbd);

				if ( (ti == OpenPort_MAX_SBD_LEN)||(ti<=0) )
				{
					continue;
				}

				if (ti+tFSize >= OpenPort_MAX_SBD_LEN)
				{
					tFlag=1;
					break;
				}
				
				compressed_no++;
			
				memcpy((tSBD+tFSize),&len_sbd,sizeof(len_sbd));
				memcpy( (tSBD+tFSize+sizeof(len_sbd)),tmp_buf,len_sbd);	
				tFSize+=ti;
			}
//			remove(tStr);
		}
		else
		{
			return num;
		}
	}while ( fgets(tStr, MAX_SBDNAME_LEN, tFSBDList) != NULL );
	
	fclose(tFSBDList);
	
	int file_size;
	int i;
	
	rtn++;
	if (tFSize == 0)
		return rtn;
		
	if  (tFSize > 0)
	{		
		if (tFSize < MIN_COMPRESS_SBD_LEN)   // not compress
		{
			compress_flag = 0;
			ti=0;	
//new 			// delete all SBD files
			tFSBDList=fopen(SBD_Name,"r");		
			while ( fgets(tStr, MAX_SBDNAME_LEN, tFSBDList) != NULL )
			{
				// delete the '\n'
				len_SBD_Name = strlen(tStr);
				if (tStr[len_SBD_Name-1] == '\n')
					tStr[len_SBD_Name-1] = '\0';	
				remove(tStr);
			}
			fclose(tFSBDList);
			
			tFpW=fopen(nzSBDPath,"wb+");
			rtn++;
			if (tFpW!=NULL)
			{
				memcpy(dest_sbd,&compress_flag,sizeof(compress_flag));
				memcpy((dest_sbd+sizeof(compress_flag)),tSBD,tFSize);
				fwrite(dest_sbd,sizeof(char),(tFSize+sizeof(compress_flag)),tFpW);
				fflush(tFpW);	
				fsync(fileno(tFpW));
				fclose(tFpW);
			}
			else
			{
				fclose(tFpW);
				return  rtn;
			}

			file_size = FILESIZE(nzSBDPath);
			sprintf(SBD_new_name,"%s%s%d_%d",jtpz->dSBD,SBD_File_Header,tmp_nSBD,file_size);	
			rename(nzSBDPath,	SBD_new_name);		
							
			memcpy(SBD_Name_List[ti],SBD_new_name,strlen(SBD_new_name));
//			fclose(tFSBDList);
			
			rtn++;
			tFSBDList=fopen(SBD_Name,"w");
			if (tFSBDList!=NULL)
			{
				fputs(SBD_Name_List[ti],tFSBDList);
				fflush(tFSBDList);	
				fsync(fileno(tFSBDList));
				fclose(tFSBDList);
				usleep(5000);
			}
			else
			{
				return rtn;
			}
		}
		else if (tFSize >= MIN_COMPRESS_SBD_LEN)  // length is enough ,need to compress
		{
			compress_flag = 1;			
			ti=0;
			tFSize1=sizeof(tBuff);
			err = compress((Bytef *)tBuff, &tFSize1, (const Bytef *)tSBD, (uLong)tFSize);
			if (err == Z_BUF_ERROR || err == Z_MEM_ERROR)
				return -1;
			printf("compress %d to %d\n",(int)tFSize,(int)tFSize1);
// don't consider compress become bigger
			tFpW=fopen(zSBDPath,"wb+");

			rtn++;
			if (tFpW!=NULL)
			{
				memcpy(dest_sbd,&compress_flag,sizeof(compress_flag));
				memcpy((dest_sbd+sizeof(compress_flag)),tBuff,tFSize1);
				fwrite(dest_sbd,sizeof(char),(tFSize1+sizeof(compress_flag)),tFpW);
				fflush(tFpW);	
				fsync(fileno(tFpW));
				fclose(tFpW);
			}
			else
			{
				fclose(tFpW);
				return  rtn;
			}
			file_size = FILESIZE(zSBDPath);
			len_SBD_Name = sprintf(SBD_new_name,"%s%s%d_%d",jtpz->dSBD,SBD_File_Header,tmp_nSBD,file_size);
			rename(zSBDPath, SBD_new_name);
			SBD_new_name[len_SBD_Name] = '\n';			
			memcpy(SBD_Name_List[ti],SBD_new_name,strlen(SBD_new_name));
						
			if (tFlag == 1)
			{
			// detele compressed  sbd source files
				i = 0;	
				tFSBDList=fopen(SBD_Name,"r");		
				while ( fgets(tStr, MAX_SBDNAME_LEN, tFSBDList) != NULL )
				{
				// delete the '\n'
					if (i == compressed_no)
					{
						break;								
					}								
					len_SBD_Name = strlen(tStr);
					if (tStr[len_SBD_Name-1] == '\n')
						tStr[len_SBD_Name-1] = '\0';	
					remove(tStr);				
					i++;				
				}
			
				ti++;
				len_SBD_Name = strlen(tStr);
				tStr[len_SBD_Name] = '\n';
				strcpy(SBD_Name_List[ti], tStr);
				while (fgets(tStr, MAX_SBDNAME_LEN, tFSBDList) != NULL)
				{
					ti++;
					strcpy(SBD_Name_List[ti], tStr);
				}
				
				fclose(tFSBDList);				
			}
			else
			{
//new 			// delete all SBD files
				tFSBDList=fopen(SBD_Name,"r");		
				while ( fgets(tStr, MAX_SBDNAME_LEN, tFSBDList) != NULL )
				{
					// delete the '\n'
					len_SBD_Name = strlen(tStr);
					if (tStr[len_SBD_Name-1] == '\n')
						tStr[len_SBD_Name-1] = '\0';	
					remove(tStr);
				}
				fclose(tFSBDList);			
			}
			
			rtn++;
			// update new list
			tFSBDList=fopen(SBD_Name,"w");
			if (tFSBDList!=NULL)
			{
				if (ti == 0)
					fputs(SBD_Name_List[ti],tFSBDList);
				else
				{
					for (i=0;i<ti;i++)
					{
						fputs(SBD_Name_List[ti],tFSBDList);
					}
				}
				fflush(tFSBDList);	
				fsync(fileno(tFSBDList));
				fclose(tFSBDList);
				usleep(5000);
			}
			else
			{
				fclose(tFSBDList);
				return rtn;
			}
		}
	}
	return (ti+1);
}

////////////////////////SBD OpenPort upload
int OpenPort_SBD_send_mainfun(char *SBD_List_Name,int msqid)
{
	FILE* fp, *fp_upload;
	char SBD_Name[MAX_SBDNAME_LEN] = {0};
	int count=0;
	int rt = 0;
	int len_SBD_Name = 0;
//	
	char result[100]={0};
	
	fp = fopen(SBD_List_Name,"r");
	if (NULL == fp)
	{
		WriteLog("oepn /home/njkk/download/?SBD_List.dat fail\n");
		return -1;		
	}
	if (fgets(SBD_Name,MAX_SBDNAME_LEN,fp) == NULL)
	{
		WriteLog("read /home/njkk/download/?SBD_List.dat fail\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);
// delete the '\n'
	len_SBD_Name = strlen(SBD_Name);
	if (SBD_Name[len_SBD_Name-1] == '\n')
			SBD_Name[len_SBD_Name-1] = '\0';
// file exist??	
	fp_upload = fopen(SBD_Name,"r");
	if (NULL == fp_upload)
	{
		return 1;
	}
	fclose(fp_upload);
	
	time_t now;	
	
	char upload_cmd_str[100] = {0};	
	sprintf(upload_cmd_str,"%s %s",SBD_Upload_DIR,SBD_Name);		
	printf("upload_cmd_str = %s\n",upload_cmd_str);		
	while ( uploadftp(upload_cmd_str) != 0)
	{
		count++;
		if (count == SBD_Wait_Time)
		{
			rt = system("/root/njkk/killftp");
			usleep(5000);
			if (rt != 0)
			{
				time(&now);
				sprintf(result,"upload sbd %s by OpenPort,try ** time fail, kill lftp fail\n",SBD_Name);
				system("/root/njkk/killftp");
				usleep(5000);
				Comm_Channel = MODEM;
				OpenPort_last_time = now;
				send_msg(msqid,139,NULL,0);     // restart OpenPort
				WriteLog(result);
				return -1;
			}
/// change to modem channel
			time(&now);
			Comm_Channel = MODEM;
			OpenPort_last_time = now;
			send_msg(msqid,139,NULL,0);     // restart OpenPort
			WriteLog("lftp upload FTP files once too long time, not connect -> kill\n");
			return -1;
		}
	}
	
	time(&now);
	OpenPort_last_time = now;
	WriteLog("lftp upload FTP files sucessfully\n");
/// upload finish and remove file
	remove(SBD_Name);	
	return 0;
}
///\//////////////////////////////SBDUpload:Openport Mode/////////////////////////////////
int SBD_OpenPort_Upload(static_config *jtpz,dynamic_config *dtpz,int msqid)
{
	FILE *fd_HSBD_List,*fd_MSBD_List,*fd_LSBD_List;

	char SBD_Name[MAX_SBDNAME_LEN] = {0};
	char Feedback[MAX_SBDNAME_LEN] = {0};
	char delete[100]={0};
	int SBD_compress_num = 0;
	int len_SBD_Name = 0;
	int upload_rt = 0;
	int rt=0;
	char *SBD_level=NULL;
	char file_list[PATHLENGTH]={0};
// read  HSBD
	if (HSBD_Num > 0)
	{
 	 	if( (HSBD_Occupy_Sign == NotOccupied) && (Global_OpLock(&SBD_mutex,&HSBD_Occupy_Sign) == 0) )		
		{
			HSBD_Occupy_Sign = Occupied;
			SBD_level = "H";
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);
			fd_HSBD_List = fopen(file_list, "r");
			if (NULL == fd_HSBD_List)	
			{
				fd_HSBD_List=fopen(file_list, "w");
				fclose(fd_HSBD_List);
				HSBD_Num = 0;
				HSBD_Occupy_Sign = NotOccupied;
				return -1;
			}
			else
			{
				fclose(fd_HSBD_List);
		///compress files and upload compressed files			
				SBD_compress_num = SBD_OpenPort_Compress(file_list,HSBD_Num,jtpz,dtpz);
				if (SBD_compress_num <= 0)
				{
					remove(file_list);
					HSBD_Num = 0;
					HSBD_Occupy_Sign = NotOccupied;
					return -1;
				}
				else
				{
					HSBD_Num = SBD_compress_num;
				}			
			//// upload compressed files
				upload_rt = OpenPort_SBD_send_mainfun(file_list,msqid);
				if (upload_rt < 0)
				{
					if (PC104_state == PC104_Slave)
					{
						List_Adjust(file_list,jtpz,dtpz);
						HSBD_Num -= 1;								
						rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
						remove(M2S_cmd);
						memset(M2S_cmd,0,sizeof(M2S_cmd));
						Slave_exe_result = FINISH;									
					}
					HSBD_Occupy_Sign = NotOccupied;	
					return;					
				}

			//// adjust file list
				rt = List_Adjust(file_list,jtpz,dtpz);
				if (rt < 0)
				{
					HSBD_Num = 0;
					HSBD_Occupy_Sign = NotOccupied;	
					return;
				}		
				HSBD_Num -= 1;
				HSBD_Occupy_Sign = NotOccupied;
			// slave finish then execute result is finished 
				if (PC104_state == PC104_Slave)	
					Slave_exe_result = FINISH;	
				return;		
		   	}
		}
	}
	else if(MSBD_Num > 0)
	{
		if( (MSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&MSBD_Occupy_Sign) == 0) )		
		{
			MSBD_Occupy_Sign = Occupied;	
			SBD_level = "M";
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);			
			fd_MSBD_List = fopen(file_list, "r");
			if (NULL == fd_MSBD_List)	
			{
				fd_MSBD_List = fopen(file_list, "w");
				fclose(fd_MSBD_List);	
				MSBD_Num = 0;
				MSBD_Occupy_Sign = NotOccupied;
				return -1;
			}
			else
			{
				fclose(fd_MSBD_List);				
		///compress files and upload compressed files			
				SBD_compress_num = SBD_OpenPort_Compress(file_list,MSBD_Num,jtpz,dtpz);
				if (SBD_compress_num <= 0)
				{
					remove(file_list);
					MSBD_Num = 0;
					MSBD_Occupy_Sign = NotOccupied;
					return -1;
				}
				else if (SBD_compress_num > 0)
				{
					MSBD_Num = SBD_compress_num;
				}
				
				//// upload compressed files
				upload_rt = OpenPort_SBD_send_mainfun(file_list,msqid);
				if (upload_rt < 0)
				{	if (PC104_state == PC104_Slave)
					{
						List_Adjust(file_list,jtpz,dtpz);
						MSBD_Num -= 1;								
						rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
						remove(M2S_cmd);
						memset(M2S_cmd,0,sizeof(M2S_cmd));
						Slave_exe_result = FINISH;									
					}
					MSBD_Occupy_Sign = NotOccupied;	
					return;					
				}				

				rt = List_Adjust(file_list,jtpz,dtpz);	
				if (rt < 0)
				{
					MSBD_Num = 0;
					MSBD_Occupy_Sign = NotOccupied;	
					return;					
				}	
				MSBD_Num -= 1;
				MSBD_Occupy_Sign = NotOccupied;
			// slave finish then execute result is finished 
				if (PC104_state == PC104_Slave)	
					Slave_exe_result = FINISH;	
	
				return;		
			}
		}
	}
	else if (LSBD_Num > 0)
	{

		if( (LSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&LSBD_Occupy_Sign)==0) )		
		{		
			LSBD_Occupy_Sign = Occupied;
			SBD_level = "L";
			sprintf(file_list,"%s%sSBD_List.dat",jtpz->dFilelist,SBD_level);				
			fd_LSBD_List = fopen(file_list, "r");
			if (NULL == fd_LSBD_List)	
			{
				fd_LSBD_List=fopen(file_list, "w");
				fclose(fd_LSBD_List);	
				LSBD_Num = 0;
				LSBD_Occupy_Sign = NotOccupied;
				return -1;
			}
			else
			{
				fclose(fd_LSBD_List);				
		///compress files and upload compressed files			
				SBD_compress_num = SBD_OpenPort_Compress(file_list,LSBD_Num,jtpz,dtpz);
				if (SBD_compress_num <= 0)
				{
					remove(file_list);
					LSBD_Num = 0;
					LSBD_Occupy_Sign = NotOccupied;
					return -1;
				}
				else if (SBD_compress_num > 0)
				{
					LSBD_Num = SBD_compress_num;
				}
				
				//// upload compressed files
				 upload_rt = OpenPort_SBD_send_mainfun(file_list,msqid);
				if (upload_rt < 0)
				{		
				// if it's slave
					if (PC104_state == PC104_Slave)
					{
						List_Adjust(file_list,jtpz,dtpz);
						LSBD_Num -= 1;								
						rsync_file(M2S_cmd,"src_cmd",jtpz,dtpz);
						remove(M2S_cmd);
						memset(M2S_cmd,0,sizeof(M2S_cmd));
						Slave_exe_result = FINISH;									
					}					
					LSBD_Occupy_Sign = NotOccupied;	
					return;					
				}	
				rt = List_Adjust(file_list,jtpz,dtpz);	
				if (rt <0)
				{

					LSBD_Num = 0;
					LSBD_Occupy_Sign = NotOccupied;	
					return;						
				}	
				LSBD_Num -= 1;
				LSBD_Occupy_Sign = NotOccupied;	
			// slave finish then execute result is finished 
				if (PC104_state == PC104_Slave)	
					Slave_exe_result = FINISH;	

				return;		
			}
		}
	}

}

#include "Irdm_Interface.h"

////////////////////////////////Implementing functions////////////////////////////////

int Global_OpLock(pthread_mutex_t *OpLock, int *GlobalVar)
{
#define NOTOCCUPIED 0
#define OCCUPIED 1
	int rtn=-1;
	if(NULL == GlobalVar)
	{
		perror("GlobalVar is NULL");
		return rtn;
	}
	pthread_mutex_lock(OpLock);
	if (*GlobalVar==NOTOCCUPIED)
	{
		*GlobalVar=OCCUPIED;
		rtn=0;
	};
	pthread_mutex_unlock(OpLock);
	return rtn;
}

///\ get file numbers from filelist
int get_filenum(char *fileaddr)
{
	#define Max_Name_Len 100
	int num = 0;
	FILE *fp;
	char buf[Max_Name_Len]={0};
	fp = fopen(fileaddr,"r");

	if (NULL == fp)
	{
		num = 0;
		fp = fopen(fileaddr,"w");
		fclose(fp);
	}
	else
	{
		while (fgets(buf, Max_Name_Len, fp) != NULL)
			num++;
		
		fclose(fp);
	}
	
	return num;
}

///  call lftp script to upload files
int uploadftp(char *file)
{	
	int result;

	char str1[100]="/root/njkk/uploadftp ";
// kill lftp	
	system("/root/njkk/killftp");
//	usleep(5000);	
	
	strncat(str1,file,strlen(file));
	result = system(str1);
	
	if (result != 0)
		return -1;  // zf mod
	else
		return 0;
}

///  call lftp script to download files
int downloadftp(char *file)
{	
	int result;

	char str1[100]="/root/njkk/downloadftp ";
	strncat(str1,file,strlen(file));
	result = system(str1);
	
	if (result != 0)
		return -1;  // zf mod
	else
		return 0;
}

int List_Adjust(char *file_list,static_config *jtpz,dynamic_config *dtpz)
{
	printf("FTP_List_Adjust() is called!\n");
	FILE *fp;
	char FTP_buf[MAX_FTP_NUM][MAX_NAME_LEN];
	memset(FTP_buf, 0, sizeof(FTP_buf));
	char delete[MAX_NAME_LEN]={0};
	int i = 0;
	int j = 0;

	fp = fopen(file_list,"r");
	if (NULL == fp)
	{
		WriteLog("?FTP_List num >1,but file don't exist\n");
		return -1;
	}							
	while (fgets(FTP_buf[i],MAX_NAME_LEN,fp) != NULL)
	{
		i++;
	}
	fclose(fp);
	remove(file_list);
	
	if (i < 1)
	{
		return -1;
	}
	fp = fopen(file_list,"w");
	if (NULL == fp)
	{						
		WriteLog("Adjust HFTP_List.dat,create file error\n");
		return -1;
	}
	for (j=1; j<i; j++)
	{
		fputs(FTP_buf[j],fp);
	}
	fflush(fp);	
	fsync(fileno(fp));
	fclose(fp);
	
	return 0;
}

int rsync_file(char *file_rsync,char *path,static_config *jtpz,dynamic_config *dtpz)
{
	char cmd[CMDLENGTH]={0};
	int rt;
	sprintf(cmd,"/root/njkk/rsync %s %s",file_rsync,path);
	rt = system(cmd);
	if (rt != 0)
	{
		printf("rsync error !\n");
		
	}
	return 0;
}

///seperate fileDir to filepath and filename
void sepFileDir(const char *filesource,char *path,char *name)       //获取原文件路径和文件名
{
	char *ptr;
	char delim='/';
	if(NULL==filesource)
	{
		perror("seperate FileDir error");
		return;
	}		
	ptr=strrchr(filesource,delim);
	memcpy(path,filesource,ptr-filesource+1);
	strcpy(name,ptr+1);
}

/////////////////////// functions for OpenPort Command File Processing ////////////////////////
int trave_dir(const char* path,char filename[CMDFILES_MAX][INT2STR_LEN])
{
	int num = 0;
    DIR *d;
    struct dirent *file;     //readdir函数的返回值就存放在这个结构体中
    struct stat sb;
	char temp[CMDFILENAME_LEN_MAX]={0};
	int len;
	d = opendir(path);
    if (d == NULL)
    {
		printf("error:%s may not exist!\n",path);
		closedir(d);
		return -1;
    }
    len=strlen(path);
	memcpy(temp,path,len);
	
	//cmd filename : Id_filesize
	char *delim = "_";
	char *result_check_filename;
	
    while((file = readdir(d)) != NULL)
    {
		memset(temp+strlen(path),0,CMDFILENAME_LEN_MAX-len);
		//排除.和..文件，并判断文件是否为压缩文件，不是则排除
		if((strncmp(file->d_name, ".", 1) == 0))// || (strstr(file->d_name,".zzz") == NULL))
			continue;
		memcpy(temp+len,file->d_name,strlen(file->d_name));
		//Judge if it's a directory，yes:continue
		//printf("stat of %s:%d\n",temp,stat(temp, &sb));
		if((stat(temp, &sb)!=-1) && S_ISDIR(sb.st_mode))
        {
			//printf("\t%s is a directory!\n",file->d_name);
			continue;
        }
        // check filename is right??
        result_check_filename = strstr(temp,delim);
        if (result_check_filename == NULL)
			remove(temp);
		else
			strcpy(filename[num++], file->d_name);    //save the compressed cmd filename  
     }
     closedir(d);
     return num;
}

//compare string like int numbers
int strIntcmp(const char *str1,const char *str2)
{
	int len1,len2;
	int i=0;
	if(NULL==str1 || NULL==str2)
	{
		perror("strIntcmp():NULL==str1 || NULL==str2\n");
		return -2;
	}
	len1=strlen(str1);
	len2=strlen(str2);
	if(len1 < len2)
		return -1;
	else if(len1 >len2)
		return 1;
	else if(len1 == len2)
	{
		for(i=0;i<len1;i++)
		{
			if(*(str1+i) == *(str2+i))
				continue;
			else if(*(str1+i) < *(str2+i))
				return -1;
			else if(*(str1+i) > *(str2+i))
				return 1;
		}	
	}
	return 0;
}

void BubbleSortStr(char filename[CMDFILES_MAX][INT2STR_LEN],int n)
{
	int i,j;
	char temp[INT2STR_LEN];
	int exchange;	//交换标记
		
	for (i=0;i<n;i++)
	{
		exchange=0;	//开始本次排序前，设置交换标记为假
		for (j=0;j<n-i-1;j++)
		{
			if (strIntcmp(filename[j],filename[j+1])>0)
			{
				strcpy(temp,filename[j]);
				strcpy(filename[j],filename[j+1]);
				strcpy(filename[j+1],temp);
				exchange=1;
			}
		}
	//	printf("exchange=%d\n",exchange);
		if (!exchange)	//本次排序未发生交换，提前终止算法
			return;
	}
}

unsigned long getFileSize(const char *path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	//printf("In getFileSize():path:%s\n",path);
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

int checkFileGood(const char* path,const char *filename)
{
	unsigned long filesize1;
	unsigned long filesize2;
	char *start,*end;
	char fsize[INT2STR_LEN]={0};
	char fullname[CMDFILENAME_LEN_MAX]="";
	
	strcpy(fullname,path);
	strcat(fullname,filename);
	//printf("In checkFileGood():fullname=%s\n",fullname);
	filesize1=getFileSize(fullname);
	if(filesize1 < 0)
	{
		printf("In checkFileGood():get filesize1 fail!\n");
		return -1;
	}
	if(strstr(filename,".zzz"))
	{
		start=strchr(filename,'_');
		end=strchr(filename,'.');
		memcpy(fsize,start+1,end-start);	
	}else{
		start=strchr(filename,'_');
		strcpy(fsize,start+1);
	}
	filesize2=atoi(fsize);
	//printf("filesize2=%u fsize=%s\n",filesize2,fsize);
	if(filesize1 == filesize2)
		return 0;
	else 
		return -1;
}

///\///////////////////uncompress command files/////////////////////////////////
int uncomp_File(const char *path,const char *filename,char *uncomp_filename)
{
	char file_path[PATHLENGTH]={0};
	char compressed_buf[max_RcvSBD] = {0};
	char decompress_buf[max_RcvSBD] = {0};
	uLong length;
	uLong decompress_len;
	int tmp_len;
	int err;
	FILE* fp;
	sprintf(file_path,"%s%s",path,filename);
	fp = fopen(file_path,"rb+");
	if (NULL == NULL)
	{
		fclose(fp);
		return -1;
	}
	else
	{
		fseek(fp,0,SEEK_END);
		length = ftell(fp);
		rewind(fp);
		tmp_len = fread(compressed_buf,length,sizeof(char),fp);
		if (tmp_len != length)
		{
			fclose(fp);
			return -1;
		}
		err = uncompress((Bytef *)decompress_buf,&decompress_len,(Bytef *)compressed_buf,length);//zlib  uncompress()
		if (err != Z_OK)
		{
			WriteLog("decompress cmd error!!\n");
			fclose(fp);
			return -1;
		}
		fclose(fp);
		
		printf("length after decompress = %ld\n",decompress_len);
		
		fp = fopen(file_path,"wb+");
		tmp_len = fwrite(decompress_buf,decompress_len,sizeof(unsigned char),fp);
		if (tmp_len != decompress_len)
		{
			WriteLog("decompress cmd and write decompress cmds to file error: length is not right!!\n");
			fclose(fp);
			return -1;	
		}
		fclose(fp);
		strcpy(uncomp_filename,filename);
	}
	return 0;
}

/////////////////////// end of functions for OpenPort Command File Processing ////////////////////////

///////////////////////////////////////**********end*********//////////////////////////////////////////////

////////////////////  Iridium Modem operation ////////////////////////////////
int decompress_mem(unsigned char compress_flag,unsigned char buffer[max_RcvSBD],int length)
{
	int decompress_len = 1904;  //(max_RcvSBD+12)*100.1% 
	unsigned char decommpress[decompress_len];
	memset(decommpress,0,decompress_len);
	uLong dest_len = sizeof(decommpress);
	int err;

	if (compress_flag == 0)
	{
		memset(SBDbuffer,0,max_RcvSBD);
		memcpy(SBDbuffer,buffer,length);
		return 0;
	}
	else if (compress_flag == 1)
	{
		err = uncompress((Bytef *)decommpress,&dest_len,(const Bytef *)buffer,(uLong)length);
		if (err == Z_OK)
		{
			printf("decompress SBD cmd from Modem SBD sucess,cmd length = %d\n",(int)dest_len);
			memset(SBDbuffer,0,max_RcvSBD);			
			memcpy(SBDbuffer,decommpress,dest_len);
			return 0;
		}
		else if (err == Z_MEM_ERROR)
		{
			printf("decompress error:no enough memory\n");
		}
		else if (err == Z_DATA_ERROR)
		{
			printf("decompress error:data is error,Z_DATA_ERROR\n");
		}
	}	
	return -1;	
}

int ReadMTBuf(unsigned char buffer[max_RcvSBD],unsigned char channel,int length)                                
{
   	int i=0; 
   	unsigned short check_sum;   
   	unsigned char rcv_check_sum[5]={0}; 
   	int sum=0;
   	int rt_check_sum;              
	unsigned char compress_flag;
	unsigned char tmp_buf[max_RcvSBD];
	memset(tmp_buf,0,max_RcvSBD);
	
	int rt_decompress;
	if (channel == OPENPORT || channel == MODEM)
	{
		memcpy(&compress_flag,(const char*)buffer,sizeof(compress_flag));
		memcpy(tmp_buf,(const char*)(buffer+sizeof(compress_flag)),(length-sizeof(compress_flag)));
		rt_decompress = decompress_mem(compress_flag,tmp_buf,(length-sizeof(compress_flag)));
		return rt_decompress;			
	}
	else
	{
/*
		memcpy(&check_sum,(const char*)buffer,sizeof(unsigned short));
		memcpy(&compress_flag,(const char*)(buffer+sizeof(unsigned short)),sizeof(unsigned char));
		memcpy(tmp_buf,(const char*)(buffer+sizeof(unsigned short)+sizeof(unsigned char)),(length-sizeof(unsigned short)-sizeof(unsigned char)));
		sum = Checksum(tmp_buf,(length-sizeof(unsigned short)-sizeof(unsigned char))); 
		rcv_check_sum[0]= (sum>>8)&0x000000FF;
		rcv_check_sum[1]= sum&0x000000FF;        
		rt_check_sum = memcmp(rcv_check_sum,&check_sum,2);
		if (rt_check_sum != 0)
			return -1;
			
		rt_decompress = decompress_mem(compress_flag,tmp_buf,(max_RcvSBD-sizeof(unsigned short)-sizeof(unsigned char)));
		return rt_decompress;
*/		
	}
	return -1;	
}

int Checksum(unsigned char sbdmsg[max_SendSBD],int len)                //写入MObuffer时校验和计算
{ 
  	int i;
  	int acc=0;
  	for(i=0;i<len;i++)
      	acc+=sbdmsg[i];
  	return(acc);
}

int SERIAL_RX1(unsigned char buffer[max_RcvSBD])
{
	int bytes,i;
	int len_sbd= 0; // read to the end is 0;
	i=0;
	while(1)
	{
		ioctl(fd_serial,FIONREAD,&bytes);
		sleep(1);
		if(bytes>0)
		{
			len_sbd = read(fd_serial,buffer,1890);
			break;
		}
		i++;
		if(i==5)
			return  -1;
	}
	return len_sbd;
}

int SERIAL_RX2(unsigned char buffer[max_RcvSBD])
{
	int bytes, CycleNum;
	CycleNum=0;
	while(1)
	{
		ioctl(fd_serial,FIONREAD,&bytes);
		
		if(bytes>1)
		{
	 		sleep(1);
			read(fd_serial,buffer,1890);
			//printf("Readret=%d\n",ret);
			break;
		}
//======== Start of the new added program =========
		else
		{
	 		sleep(1);
		}

		CycleNum++;
		if(CycleNum>35){ break;}
//========  End of the new added program  =========
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int open_serial(int k)
{
	printf("~~~~~~~~ prepare to open serial ~~~~~~~~~\n");//2010-8-8
	if(k==0)
	{
		fd_serial = open("/dev/ttyS0",O_RDWR|O_NOCTTY|O_NDELAY);
		fcntl(fd_serial,F_SETFL,0);
		perror("open /dev/ttyS0");
	} 
	else
	{
		fd_serial = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NDELAY);
		fcntl(fd_serial,F_SETFL,0);
		perror("open /dev/ttyS1");
	}

	if(fd_serial == -1) 
		perror("open /dev/ttyS0 failed!");
	else 
		return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int init_iridium(int fd_serial,char atcommand[40])  
{
	unsigned char buffer[1890]={0};
	int rt;
	int len_sbd= 0;
	//FILE *fp;
	int length=0;
	int i=0;
	memset(buffer, '\0', 1890);//2010-8-1

	for(length=0;length<128;length++)                      //获得at命令长度
	{
	     	if(atcommand[length]=='\r')
		break;
	}
	write(fd_serial,atcommand,length+1);                          //向串口写at命令

	sleep(1);

	if((strncmp(atcommand,"atz\r",4)==0))
	{
		char *ptr;
		SERIAL_RX1(buffer);
		ptr=strstr((const char*)buffer, "OK");
		if ( ptr != NULL)
		{
			//Time for the modem state should be read;
			Modem_state=1;
			rt = 0;
		}
		else
		{
			Modem_state=0;
			rt = -1;
		}
		printf("Modem_state= %d\n", Modem_state);

		if(strlen((const char*)buffer)==0) return -1;
		printf("ATZ=%s\n",buffer);
		return rt;
	}
	if((strncmp(atcommand,"+++\r",4)==0))
	{
		sleep(2);//2010-8-1
		SERIAL_RX1(buffer);
		if(strlen((const char*)buffer)==0) 
			return -1;
		printf("%s\n",buffer);
	}//2010-7-28
	if((strncmp(atcommand,"ATH0\r",5)==0))
	{
		SERIAL_RX1(buffer);
		if(strlen((const char*)buffer)==0) 
			return -1;
		printf("%s\n",buffer);
	}//2010-7-28

	if((strncmp(atcommand,"ate0q1\r",7)==0)) 
	{
		SERIAL_RX1(buffer);
		printf("buffer=%s\n",buffer);
	}

	if((strncmp(atcommand,"at+sbdrt\r",9)==0))
	{
		len_sbd = SERIAL_RX1(buffer);
		printf("******************** read sbd from modem length = %d\n",len_sbd);
		if (len_sbd <= 0)
			return -1;
		printf("buffer=%s\n",buffer);
		rt = ReadMTBuf(buffer,MODEM,len_sbd);
		return rt;
	}

	if((strncmp(atcommand,"at+sbdd2\r",9)==0))
	{  
	SERIAL_RX1(buffer);
	printf("buffer=%s\n",buffer);
	if((strncmp((const char*)buffer,"\r\n0",3)==0))
		printf("MO MT buffer cleaned successfully!\n");
	else 
		return 0;
	}

	if((strncmp(atcommand,"at+sbdwb=",9)==0))
	{
	SERIAL_RX1(buffer);
	printf("buffer=%s\n",buffer);
	if((strncmp(buffer,"\r\nREADY",7)==0))
		return(3);
	}

	if(strncmp(atcommand,"at+sbdi\r",8)==0)
	{
		char *del=" ,";
		char *p,*t;
		char msgbuffer[1890];
		int cunt[6];
		int len = 0;
		memset(msgbuffer,0,1890);
//Start: 2010-8-7
		SERIAL_RX2(buffer); 
		len = strlen((const char*)buffer);
		printf("buffer-len=%d\n",len);
		if(len!=0)
		{
			printf("Initial SBD sension success!\n");
			printf("buffer=%s\n",buffer);
			return 0;
		}
		else 
			return -1;
			
//End: 2010-8-7
//  Strart: 2010-8-31
		if ( strlen(buffer)<20 )
		{
			MO=0;
			printf("Nothing to Send!\n");
			MT=0;
			printf("Nothing to Receive!\n");
			return -1;
		}
//  End: 2010-8-31

		for(i=0;i<strlen((const char*)buffer)-10;i++)
			msgbuffer[i]=buffer[i+8];
		t=strtok(msgbuffer,del);
		cunt[0]=atoi(t);
		printf("cunt0=%d\n",cunt[0]);
		for(i=1;i<6;i++)
		{
				p=strtok(NULL,del);
				cunt[i]=atoi(p);
				printf("cunt%d=%d\n",i,atoi(p));
		}
		MO=cunt[0];
		MOMSN=cunt[1];
		MT=cunt[2];
		MTMSN=cunt[3];
		MTLENGTH=cunt[4];
		MTqueued=cunt[5];
		if (MO==1)printf("Initial SBD sension success!\n");
		else if(MO==0)printf("Nothing to send!\n");
		else printf("An error occurred or no signal while attempting to send!\n");
		if (MT==0)printf("Nothing to receive!\n");
		if (MT==1)
		{
	     	printf("One SBD message is successfully received!\n");
			printf("MTMSN is %d!\n",MTMSN);
		}
		else printf("No SBD message received!\n");

	} 

//*********************** 2010-6-18: new added program ***************************************
	if((strncmp(atcommand,"at&f0\r",6)==0))
	{
		SERIAL_RX1(buffer);
		printf("buffer=%s\n",buffer);
	}    
//*********************** 2010-6-18: End of the new added program ****************************                              

}

// recieve SBD command from Iridium Modem 

int RCVmainfun()       
{
/*******************************************************************/
	 open_serial(0);
/*******************************************************************/
	 struct termios newtio;
	 struct termios oldtio;
	 memset(&newtio,0,sizeof(newtio));
	 tcgetattr(fd_serial,&oldtio);
/*****************************************************************/
	 cfsetispeed(&newtio,B9600);      /*娉㈢壒鐜囪缃负19200bps*/
	 cfsetospeed(&newtio,B9600);
	 newtio.c_cflag&=~PARENB;           /*鏃犳牎楠屼綅*/
	 newtio.c_cflag&=~CSTOPB;          /*1浣嶅仠姝綅*/ 
	 newtio.c_cflag&=~CSIZE;
	 newtio.c_cflag|=CS8;              /*8浣嶆暟鎹綅*/
	 newtio.c_cflag|=(CLOCAL|CREAD);
	 newtio.c_lflag&=~(ICANON|ECHO|ECHOE|ISIG);
	 newtio.c_oflag&=~OPOST;
	 newtio.c_cc[VMIN]=1;
	 newtio.c_cc[VTIME]=0;
	 tcsetattr(fd_serial,TCSANOW,&newtio);
/*******************************************************************/
	 
	 sleep(1);
	 printf("ready for sending data...\n"); 

	 init_iridium(fd_serial,"+++\r");//2010-7-28
	 init_iridium(fd_serial,"ATH0\r");//2010-7-28

	 init_iridium(fd_serial,"atz\r");
	 init_iridium(fd_serial,"ate0q1\r");
	 init_iridium(fd_serial,"at+sbdd2\r"); 
	 printf("SBDIing!\n");
	 int rt = init_iridium(fd_serial,"at+sbdi\r");

	 sleep(1);
	 if(MT==1)	                      
	    rt = init_iridium(fd_serial,"at+sbdrt\r");

	 sleep(1);
	 tcsetattr(fd_serial,TCSANOW,&oldtio);

	 flag_close=close(fd_serial);  
	 if(flag_close==-1)               
	    printf("Close the Irdm failure\n");
	
	return rt;
}


int GainSystem(char systemcommand[80])
{
	FILE *fp;
	char *ptr, *ptr1;//2010-7-27
	char buffer[800];
	memset(buffer, '\0', 800); //2010-8-17
	fp=popen(systemcommand,"r");
	if(fp!=NULL)
	{
		fread(buffer,sizeof(buffer),1,fp);
		printf("buffer=%s\n",buffer);
		//ptr=strstr(buffer,"addr:");
		ptr=strstr(buffer,"255.255.");
		ptr1=strstr(buffer,"ppp0: error fetching interface information");//2010-7-27

		if ((ptr!=NULL) && (ptr1 == NULL))//2010-7-27
			return 1;
		else 
			return 0;
	}
	pclose(fp);
	//memset(buffer, '\0', 800);//2010-6-20
}

void KillWvdial()
{
		system("killall -HUP -f wvdial|pppd 2>/dev/null");
		sleep (5);
		system("killall -9 -f wvdial|pppd 2>/dev/null");
		sleep(5);
		system("killall -9 -f wvdial|pppd 2>/dev/null");
		sleep (3);
}

//Irdm initial:check all needed files
void CheckIrdmAllFile(static_config *jtpz,dynamic_config *dtpz)
{
	#define IRDM_FILE_NO 7
	int i;
	char HSBD_List[PATHLENGTH]={0};
	sprintf(HSBD_List,"%sHSBD_List.dat",jtpz->dFilelist);
	char MSBD_List[PATHLENGTH]={0};
	sprintf(MSBD_List,"%sMSBD_List.dat",jtpz->dFilelist);
	char LSBD_List[PATHLENGTH]={0};
	sprintf(LSBD_List,"%sLSBD_List.dat",jtpz->dFilelist);	
	
	char HFTP_List[PATHLENGTH]={0};
	sprintf(HFTP_List,"%sHFTP_List.dat",jtpz->dFilelist);
	char MFTP_List[PATHLENGTH]={0};
	sprintf(MFTP_List,"%sMFTP_List.dat",jtpz->dFilelist);
	char LFTP_List[PATHLENGTH]={0};
	sprintf(LFTP_List,"%sLFTP_List.dat",jtpz->dFilelist);	
	
	char DL_FTP_List[PATHLENGTH]={0};
	sprintf(DL_FTP_List,"%sDL_FTP_List.dat",jtpz->dFilelist);		
	char *IrdmListFName[IRDM_FILE_NO]={HSBD_List,MSBD_List,LSBD_List,HFTP_List,MFTP_List,LFTP_List,DL_FTP_List};

	for (i=0;i<IRDM_FILE_NO;i++)
	{
		Check_IrdmFile(IrdmListFName[i]);
	}
}
void check_irdm_path(static_config *jtpz,dynamic_config *dtpz)
{
	CheckPath(jtpz->dCmd);		
	CheckPath(jtpz->dDownload);	
	CheckPath(jtpz->dLog);
	CheckPath(jtpz->dTmp);
	CheckPath(jtpz->dCatlog);			
	CheckPath(jtpz->dSBD);
	CheckPath(jtpz->dFTP);
	CheckPath(jtpz->dUpload);
	CheckPath(jtpz->dUpdate);
	CheckPath(jtpz->dFilelist);	
}
void init_occupy_sign()
{
	HSBD_Occupy_Sign = NotOccupied;
	MSBD_Occupy_Sign = NotOccupied;
	LSBD_Occupy_Sign = NotOccupied;

	HFTP_Occupy_Sign = NotOccupied;
	MFTP_Occupy_Sign = NotOccupied;
	LFTP_Occupy_Sign = NotOccupied;
	
	DL_FTP_Occupy_Sign = NotOccupied;
	
	Config_Occupy_Sign = NotOccupied;	
	
	///Initalize transfer state : OpenPort & Modem 
	sbd_send_flag=0;
	ftp_send_flag=0;

/// Modem transport state
	sbd_rcv_flag=0;
	sbd_snd_flag=0;
	ftp_deliver_flag=0;
}
void init_BSD_FTP_num(static_config *jtpz,dynamic_config *dtpz)
{
	char HSBD_List[PATHLENGTH]={0};
	sprintf(HSBD_List,"%sHSBD_List.dat",jtpz->dFilelist);
	char MSBD_List[PATHLENGTH]={0};
	sprintf(MSBD_List,"%sMSBD_List.dat",jtpz->dFilelist);
	char LSBD_List[PATHLENGTH]={0};
	sprintf(LSBD_List,"%sLSBD_List.dat",jtpz->dFilelist);	
	
	char HFTP_List[PATHLENGTH]={0};
	sprintf(HFTP_List,"%sHFTP_List.dat",jtpz->dFilelist);
	char MFTP_List[PATHLENGTH]={0};
	sprintf(MFTP_List,"%sMFTP_List.dat",jtpz->dFilelist);
	char LFTP_List[PATHLENGTH]={0};
	sprintf(LFTP_List,"%sLFTP_List.dat",jtpz->dFilelist);	
	
	char DL_FTP_List[PATHLENGTH]={0};
	sprintf(DL_FTP_List,"%sDL_FTP_List.dat",jtpz->dFilelist);	
	HSBD_Num = get_filenum(HSBD_List);
	MSBD_Num = get_filenum(MSBD_List);
	LSBD_Num = get_filenum(LSBD_List);
	if (HSBD_Num > MAX_SBD_NO)//
	{
		remove(HSBD_List);
		HSBD_Num=0;
	}
	if (MSBD_Num > MAX_SBD_NO)//
	{
		remove(MSBD_List);
		MSBD_Num =0;
	}
	if (LSBD_Num > MAX_SBD_NO)//
	{
		remove(LSBD_List);
		LSBD_Num =0;
	}
	
	HFTP_Num = get_filenum(HFTP_List);
	MFTP_Num = get_filenum(MFTP_List);
	LFTP_Num = get_filenum(LFTP_List);
	DL_FTP_Num =get_filenum(DL_FTP_List);
	if (HFTP_Num > MAX_FTP_NUM)
	{
		remove(HFTP_List);
		HFTP_Num=0;
	}
	if (MFTP_Num > MAX_FTP_NUM)
	{
		remove(MFTP_List);
		MFTP_Num=0;
	}
	if (LFTP_Num > MAX_FTP_NUM)
	{
		remove(LFTP_List);
		LFTP_Num=0;
	}
	if (DL_FTP_Num > MAX_DLFTP_NUM)
	{
		remove(DL_FTP_List);
		DL_FTP_Num=0;
	}
	printf("HSBD_Num=%d	MSBD_Num=%d	LSBD_Num=%d	HFTP_Num=%d	MFTP_Num=%d	LFTP_Num=%d	DL_FTP_Num=%d\n",HSBD_Num,MSBD_Num,LSBD_Num,HFTP_Num,MFTP_Num,LFTP_Num,DL_FTP_Num);	
}

void init_mutex()
{
	pthread_mutex_init(&SBD_mutex,NULL);
	pthread_mutex_init(&FTP_mutex,NULL);
	pthread_mutex_init(&DL_FTP_mutex,NULL);	
	pthread_mutex_init(&Config_mutex,NULL);	
}
void Irdm_Destory()
{
	pthread_mutex_destroy(&SBD_mutex);
	pthread_mutex_destroy(&FTP_mutex);
	pthread_mutex_destroy(&DL_FTP_mutex);
	pthread_mutex_destroy(&Config_mutex);
}
int Ethernet_state_check(static_config *jtpz,dynamic_config *dtpz)
{
	char IP_addr[20]={0};
	int result_ping;
	get_ip(IP_addr);
	char *p;
	p = strstr(IP_addr,IP_PC104_1);
	if (NULL != p)
	{
		strcpy(IP_addr,IP_PC104_2);	
		PC104_No = PC104_1;	
	}
	else
	{
		strcpy(IP_addr,IP_PC104_1);	
		PC104_No = PC104_2;		
	}
	result_ping = ping_test(IP_addr);
	if (result_ping == 0)
		Ethernet_state = OK;
	else
	{
		Ethernet_state = Error;
		printf(" !!! Ethernet is error ,quit\n");
		//exit(0);
	}
				
	return Ethernet_state;
}
void init_SJ(static_config *jtpz,dynamic_config *dtpz)
{
	PC104_state = PC104_Single_Master;
	Slave_exe_result = FINISH;	
	Ethernet_state_check(jtpz,dtpz);
}
void Irdm_Init(static_config *jtpz,dynamic_config *dtpz)
{
/// check config file ; if not exsit,copy from /home/njkk/backup/static.cfg  & dynamic.cfg
	Check_Config_File(static_configfile_path,dynamic_configfile_path);	
///  read static & config figure
	Read_Static_Config(jtpz,dtpz);
	Read_dynamic_Config(jtpz,dtpz);
/// check direction /home/njkk/backup - cmd - download - log - tmp/SBD FTP - upload
	check_irdm_path(jtpz,dtpz);
/// check script file:/home/njkk/backup/downloadftp .uploadftp .initwvdial .Iridium-config IrdmConfig
	check_script_file(jtpz,dtpz);
/// H/M/L SBD-FTP state initial
	init_occupy_sign();
///check  H/M/LSBD_List.dat  H/M/LFTP_List.dat
	CheckIrdmAllFile(jtpz, dtpz);  
///\ get the number from filelist	
	init_BSD_FTP_num(jtpz,dtpz);
///Initalize communicate channel
	poweron_channel_check(jtpz,dtpz);
/// openport modem state check initial
	OpenPort_Reboot_Time = 0;
	Modem_Reboot_Time = 0;	
/// SJ check
	init_SJ(jtpz,dtpz);	
/// Initialize mutex variable
	init_mutex();
}

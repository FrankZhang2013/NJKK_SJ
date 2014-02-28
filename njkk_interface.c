#include "njkk_interface.h"

void verify_success(int status,const char *message_p)
{
		if(status == -1)
		{
				perror(message_p);
				exit(EXIT_FAILURE);
		}
}

int Install_Thread(Thread_P ThreadParam, void *ThreadFun, int ThreadPrio, sem_t *PSem)
{
    int status;
	if (PSem!=NULL)
    {
        sem_init(PSem,0,0);
    }
	if (ThreadPrio>0)
	{
		pthread_attr_init(&ThreadParam->attr);
    	pthread_attr_getschedparam(&ThreadParam->attr, &ThreadParam->param);
		pthread_attr_setschedpolicy(&ThreadParam->attr, SCHED_RR);
    	ThreadParam->param.sched_priority=ThreadPrio;
    	pthread_attr_setschedparam(&ThreadParam->attr, &ThreadParam->param);
		pthread_attr_setinheritsched(&ThreadParam->attr,PTHREAD_EXPLICIT_SCHED);
    	status=pthread_create(&ThreadParam->id, &ThreadParam->attr, (void *)ThreadFun, NULL);
	}
	else
	{
		status=pthread_create(&ThreadParam->id, NULL, (void *)ThreadFun, NULL);
	}
    verify_success(status, "Create thread FAILED\n");   
    return status; 
}
///------------------- File\Direction process --------------------//
int	 CreateDir(const char *PathName)   
{   
	#define MAX_DIR_LEN 70
	#define  ERROR_MSG_LEN 140
    char DirName[MAX_DIR_LEN]; 
    char ErrorMsg[ERROR_MSG_LEN];
    int   i,tLen,status;

    tLen= strlen(PathName);
    strcpy(DirName, PathName);
    if(DirName[tLen-1]!='/')
    {   
       strcat(DirName,"/");
    }     
    tLen=strlen(DirName); 
    
    for(i=1; i<tLen; i++)   
    {   
        if(DirName[i]=='/')   
	    {   
            DirName[i]=0;   
            if( access(DirName, 0)!=0)   
		    {   
		 	   status=mkdir(DirName, 0777);
		 	   sprintf(ErrorMsg, "Creat dir %s",PathName);
		 	   verify_success(status,ErrorMsg);
		    }   
         DirName[i]='/';   
	    }   
    }      
    return   0;   
} 
void CheckPath(const char *dirpath)
{
	DIR *fd;
	fd=opendir(dirpath);
	if(NULL==fd)
	{
		mkdir(dirpath,0777);
		printf("create dir:%s OK!\n",dirpath);
	}
	else
	{
		closedir(fd);
		printf("check dir:%s exist!\n",dirpath);
	}
}

 
int Check_IrdmFile(const char *SBDListName)
{
	FILE *tFSBDList;

	tFSBDList=fopen(SBDListName,"r");
	if (tFSBDList==NULL)
	{
		tFSBDList = fopen(SBDListName, "w");
	}
	fclose(tFSBDList);

	return 0;
}
///////   check script file: Iridium-csq.out \initwvdial\ uploadftp \downloadftp
void check_script_file(static_config *jtpz,dynamic_config *dtpz)
{
	FILE* fp;
///Iridium-csq.out
	char location[PATHLENGTH]={0};
	sprintf(location,"%sIridium-csq.out",jtpz->dShellBackup);
	char dest[PATHLENGTH]="/root/njkk/";
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:Iridium-csq.out!\n");
		exit(0);
	}
	fclose(fp);	
	copy_file(location,dest); 
///	initwvdial
	memset(location,0,sizeof(location));
	sprintf(location,"%sinitwvdial",jtpz->dShellBackup);
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:initwvdial!\n");
		exit(0);	
	}
	fclose(fp);	
	copy_file(location,dest); 			

///	uploadftp
	memset(location,0,sizeof(location));
	sprintf(location,"%suploadftp",jtpz->dShellBackup);
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:uploadftp!!\n");
		exit(0);				
	}
	fclose(fp);	
	copy_file(location,dest); 	

///	downloadftp
	memset(location,0,sizeof(location));
	sprintf(location,"%sdownloadftp",jtpz->dShellBackup);
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:downloadftp!!\n");
		exit(0);		
	}
	fclose(fp);	
	copy_file(location,dest); 

///	testftp
	memset(location,0,sizeof(location));
	sprintf(location,"%stestftp",jtpz->dShellBackup);
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:testftp!!\n");
		exit(0);		
	}
	fclose(fp);	
	copy_file(location,dest);

///killftp	 
	memset(location,0,sizeof(location));
	sprintf(location,"%skill_thread",jtpz->dShellBackup);
	fp = fopen(location,"r");
	if (NULL == fp)
	{
		printf("missing shell:kill_thread!!\n");
		exit(0);		
	}
	fclose(fp);	
	copy_file(location,dest);
}

int FILESIZE(unsigned char SENDFILE[100])
{
	if(access((const char*)SENDFILE,F_OK)!=0) 
		return -1;	
	int FileSize;
	struct stat buf;
	stat((const char*)SENDFILE,&buf);
	FileSize=buf.st_size;
	return FileSize;
}
///get filename from fileDir
void getfilename(const char *filesoure,char *name)       //获取原文件名
{
	char *p=filesoure;
	if(NULL==p)
	{
		perror("getfilename error");
		return;
	}		
	while(*p!='\0')
	{
		p++;
	}
	while(*p!='/')
	{
		p--;
	}
	strcpy(name,p+1);
}
int getfile_type(const char *filename,char *file_type)
{
	char *p = filename;
	char *rt;
	rt = strstr(filename,".");
	if (NULL == rt)
		return -1;
		
	while (*p++ != '.');

	strcpy(file_type,p);
	return 0;

}
/// copy file from souce to destionation ,,all include explict filename
int copy_file(char *source,char *destination)
{
	FILE *fp;
	fp = fopen(source,"r");
	if (NULL == fp)
	{
		printf("backup file don't exit!\n");
		return -1;
	}
	int rt;
	char copy_dynamic_cmd[CMDLENGTH]={0};
	sprintf(copy_dynamic_cmd,"cp -f %s %s",source,destination);
	rt = system(copy_dynamic_cmd);
	if (rt != 0)
		exit(-1);
}
int WriteLog(char *newlog)
{
	extern interface arg;
	time_t   b_time;   
 	struct   tm   *ptm; 
 	b_time=time(NULL); 
 	ptm=localtime(&b_time); 
	char timebuf[30];
	int size;
	size=sprintf(timebuf,"%d%d%d %d:%d:%d ",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);

	char log_path[PATHLENGTH] = {0};
	sprintf(log_path,"%sIrdm_log.txt",arg.static_conf.dLog);
	FILE *fp;
	fp = fopen(log_path,"a");
	if(fp == NULL)
	{
		printf("log file doesn't exist\n");
		return -1;
	}
	int len=size+strlen(newlog)+2;   ////////////////
	char context[len];
	memset(context,0,sizeof(context));
	memcpy(context,timebuf,size);
	memcpy(context+size,newlog,strlen(newlog));
	context[len-2]='\r';        ///fclose(sfp);
	context[len-1]='\n';        ///
	fwrite(context,sizeof(char),len,fp);
	fclose(fp);
	return 0;
}
int log_write(const char* file,char *str)
{
	if (NULL==file || str==NULL)
		return;
	time_t   b_time;   
 	struct   tm   *ptm; 
 	b_time=time(NULL); 
 	ptm=localtime(&b_time); 
	char timebuf[30];
	int size;
	size=sprintf(timebuf,"%d%d%d %d:%d:%d ",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);

	FILE *fp;
	fp = fopen(file,"a");
	if(fp == NULL)
	{
		printf("log file doesn't exist\n");
		return -1;
	}
	int len=size+strlen(newlog)+2;   ////////////////
	char context[len];
	memset(context,0,sizeof(context));
	memcpy(context,timebuf,size);
	memcpy(context+size,str,strlen(str));
	context[len-2]='\0';        ///fclose(sfp);
	context[len-1]='\n';        ///
	fwrite(context,sizeof(char),len,fp);
	fflush(fp);
	fclose(fp);
	return 0;
}
///--------------------- Message queue process --------------------////
int Create_Msg_Queue(const char *MsgPath)
{
    key_t key;
    int msqid;
    char ErrorMsg[70];

    key=ftok(MsgPath,2);
    if(key==-1)
    {
        sprintf(ErrorMsg,"ftok %s", MsgPath);
        verify_success(key,ErrorMsg);
        return -1;
    }
    msqid=msgget(key,IPC_CREAT | 0660);
    if(msqid==-1)
    {
        sprintf(ErrorMsg,"creat sem %s", MsgPath);
        verify_success(msqid, ErrorMsg);
    }
    return msqid;
}

int Install_Msg_Queue()
{
	int i,msg_type;
	int recv_result;
	char rcv_buf[MSG_BUF_SIZE];
	char *MsgPath[MSG_Queue_NO]={"/home/PLC","/home/Irdm","/home/PC104","/home/NAS","/home/Camera"};

    int rtn=0;
	for (i=0;i<MSG_Queue_NO;i++)
	{
		CreateDir(MsgPath[i]);
		MsgID[i]=Create_Msg_Queue(MsgPath[i]);
		if (MsgID[i]==-1)
		{
			rtn=(rtn<<1)|1;
		}
	}
    fprintf(stderr,"creat msg queue: %x\n",rtn);
    return rtn;
}

int send_msg(int msqid,int msgtype,char *msg,int size)
{
	if(show_msg_queue_stat(msqid)>=40)
	{
		printf("msqid=%d\t%s\n",msqid,"too many messages!");
		return -2;
	}
	int status;
	Msg_Info buf;
	char ErrorMsg[70];

	buf.mtype=msgtype;  
	if(size>BUFSIZE)
	{
		memcpy(buf.mtext,msg,BUFSIZE);
		status=msgsnd(msqid,&buf,BUFSIZE,IPC_NOWAIT); 
	}     
	else
	{
		memcpy(buf.mtext,msg,size);
		status=msgsnd(msqid,&buf,size,IPC_NOWAIT); 
	}
    if (status==-1)
	{
		sprintf(ErrorMsg,"send msg %x", msqid);
		verify_success(status, ErrorMsg);
	}
    return status;
}

int recv_msg(int msqid,int *msgtype,char *msg,int size)
{
	int status;
	Msg_Info buffer;
	char ErrorMsg[70];
//fprintf(stderr,"size Msg: %d\n",sizeof(Msg_Info));
	memset(&buffer,0,sizeof(Msg_Info));

	status=msgrcv(msqid,&buffer,size,0,MSG_NOERROR);
	if(status==-1)
	{
		sprintf(ErrorMsg,"recv msg %x", msqid);
		verify_success(status, ErrorMsg);
	}
	else
	{
		memcpy(msg,buffer.mtext,status);
	}
//fprintf(stderr,"size Msg: %d\n",status);

	*msgtype=buffer.mtype;

    return status;
}


//params:1.msqid;2.msgtype;3.msg;4.size;5.prio>=0
//return value:-1,-2: error;
//				 -39: msg queue is empty;
//				  >=0: length of msg.
int recv_msg_NOWAIT_prio(int msqid,int *msgtype,char *msg,int size)
{
	int status;
	Msg_Info buffer;
	char ErrorMsg[70];
//fprintf(stderr,"size Msg: %d\n",sizeof(Msg_Info));
	memset(&buffer,0,sizeof(buffer));
	
	if(msgtype == NULL)
	{
		printf("In recv_msg_prio():msgtype is NULL!\n");
		return -1;
	}
	
	status = msgrcv(msqid,&buffer,size,0,MSG_NOERROR | IPC_NOWAIT);//DON'T block to receive new msg
/*	if(status == -1 && errno == ENOMSG)	//msg queue is empty
	{
		return -1;
	}
*/
//	else if(status == -1)
	if(status == -1)
	{
		sprintf(ErrorMsg,"recv msg %x", msqid);
		verify_success(status, ErrorMsg);
	}
	else
	{
		memcpy(msg,buffer.mtext,status);
		*msgtype=buffer.mtype;		
	}

    return status;
}

int show_msg_queue_stat(int msqid)
{
	struct msqid_ds buffer;
	int status;
	char ErrorMsg[70];

	status=msgctl(msqid,IPC_STAT,&buffer);
    if (status==-1)
	{
		sprintf(ErrorMsg,"show msg %x", msqid);
		verify_success(status, ErrorMsg);
		buffer.msg_qnum=0;
	}
	return buffer.msg_qnum;
}

///---------------------------mutex  lock parameter -----------------------///
//get variable safely in multi-thread
int getVar(pthread_mutex_t *mutex,int *var)
{
	int rtnVal=-1;
	//printf("get_variable():wait for mutex\n");
	pthread_mutex_lock(mutex);
	//printf("get_variable():lock mutex\n");
	rtnVal=*var;
	pthread_mutex_unlock(mutex);
	//printf("get_variable():unlock mutex\n");
	return rtnVal;
}

//set variable safely in multi-thread
int set_variable(pthread_mutex_t *mutex,int *var,int newVal)
{
	int rtn=-1;
	//printf("set_variable():wait for mutex\n");
	pthread_mutex_lock(mutex);
	//printf("set_variable():lock mutex\n");
	*var=newVal;
	rtn =0;
	pthread_mutex_unlock(mutex);
	//printf("set_variable():unlock mutex\n");
	return rtn;
}

//Increase variable by 1 safely in multi-thread
int IncVar(pthread_mutex_t *mutex,int *var)
{
	int rtnVal=-1;
	//printf("get_variable():wait for mutex\n");
	pthread_mutex_lock(mutex);
	//printf("get_variable():lock mutex\n");
	(*var)++;
	rtnVal=*var;
	pthread_mutex_unlock(mutex);
	//printf("get_variable():unlock mutex\n");
	return rtnVal;
}

//Decrease variable by 1 safely in multi-thread
int DecVar(pthread_mutex_t *mutex,int *var)
{
	int rtnVal=-1;
	//printf("get_variable():wait for mutex\n");
	pthread_mutex_lock(mutex);
	//printf("get_variable():lock mutex\n");
	(*var)--;
	rtnVal=*var;
	pthread_mutex_unlock(mutex);
	//printf("get_variable():unlock mutex\n");
	return rtnVal;
}

///-----------------------cofigure file process -------------------//
// record lock 
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;
	
	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	return (fcntl(fd, cmd, &lock));
}
///////////////////  new static config file & Dynamic config file  
int ReadConfig(char *conf_path,char *conf_name,char *config_buff,static_config *jtpz,dynamic_config *dtpz)
{
	char config_linebuf[MAX_LINE_LENGTH] = {0};;
	char exchange_buf[MAX_LINE_LENGTH];	   
	char *config_sign = "=";
	FILE *f;
	int i = 0;
	int j = 0;
	char *p;
	char *q;
	char result[MAX_LINE_LENGTH] = {0};
	int rt;
	int f_no;
	
	char filename[PATHLENGTH]={0};
	char source_file_dir[PATHLENGTH]={0};
		
	if( Config_Occupy_Sign==NotOccupied && Global_OpLock(&Config_mutex,&Config_Occupy_Sign) == 0 )
	{
		Config_Occupy_Sign = Occupied;
		f = fopen(conf_path,"r");
		if(NULL == f)
		{
			getfilename(conf_path,filename);
			sprintf(source_file_dir,"%s%s",jtpz->dConfigBackup,filename);		
			//remove(dynamic_configfile_path);
			copy_file(source_file_dir,conf_path);
			f = fopen(conf_path,"r");			
		}
		f_no = fileno(f);
	// wait read lock
		readw_lock(f_no,0,SEEK_SET,0);
		while(fgets(config_linebuf,MAX_LINE_LENGTH,f) != NULL)
		{
			i++;   
			if(strlen(config_linebuf) < 3) //判断是否是空行
			{
				memset(config_linebuf,0,sizeof(config_linebuf));
				continue;
			}
			if (strstr(config_linebuf,"#"))  //判断是否是注释行：#
			{
				memset(config_linebuf,0,sizeof(config_linebuf));
				continue;
			}
			p = strstr(config_linebuf, conf_name);
			if (NULL != p)
			{
				q = strstr(p, config_sign);		
				if (NULL == q)
				{
					break;
				}
				else
				{	
					strcpy(result,q);
					while (result[++j] == ' ');					
					strcpy(config_buff, (result+j));
					break;
				}
			}
			else
			{
				memset(config_linebuf,0,sizeof(config_linebuf));
				continue;
			}	
		}
		fclose(f);
		if (i == 0)
		{
			getfilename(conf_path,filename);
			sprintf(source_file_dir,"%s%s",jtpz->dConfigBackup,filename);		
			remove(dynamic_configfile_path);
			copy_file(source_file_dir,conf_path);
		}	
		un_lock(f_no,0,SEEK_SET,0);	
		Config_Occupy_Sign = NotOccupied;
	}
	
	return -1;
}
int Change_Config(char *conf_path,char *conf_name,char *config_buff,static_config *jtpz,dynamic_config *dtpz)
{
	printf("!!!!!!!!!!!!!!!!! start to change dynamic.conf       !\n");
	char config_linebuf[MAX_LINE_LENGTH] = {0};
	char *config_sign = "=";
	char *leave_line;
	int  alter_sign = 0;
	int rt;
	char buf[MAX_LINE][MAX_LINE_LENGTH];
	memset(buf,0,sizeof(buf));
	
	char copy_dynamic_cmd[100] = {0};
	int line_no = 0;
	FILE *f,*fp;
	char file_name[PATHLENGTH]={0};
	getfilename(conf_path,file_name);
	char backupfile[PATHLENGTH]={0};	
	int f_no;
		
	if( Global_OpLock(&Config_mutex,&Config_Occupy_Sign) == 0 )
	{
		Config_Occupy_Sign = Occupied;
		f = fopen(conf_path,"r");
		if(f == NULL)
		{
			printf("OPEN CONFIG FALID,file may do not exit\n");
			sprintf(backupfile,"%s%s",jtpz->dConfigBackup,file_name);
			copy_file(backupfile,conf_path);		
			f = fopen(conf_path,"r");
		}
		
		f_no = fileno(f);	
		readw_lock(f_no,0,SEEK_SET,0);	
		while(fgets(config_linebuf,MAX_LINE_LENGTH,f) != NULL)
		{   
			if(strlen(config_linebuf) < 3) //判断是否是空行
			{
				strcpy(buf[line_no],config_linebuf);
				line_no++;
				memset(config_linebuf,0,MAX_LINE_LENGTH);
				continue;
			}
			
			leave_line = strstr(config_linebuf,config_sign);
			if(leave_line == NULL)                            //无"="的情况,则为 注释
			{
				strcpy(buf[line_no],config_linebuf);
				line_no++;
				memset(config_linebuf,0,MAX_LINE_LENGTH);
				continue;
			}
			else
			{
				leave_line = strstr(config_linebuf,conf_name);
				if(leave_line == NULL)                            // 不是需要修改的行
				{
					strcpy(buf[line_no],config_linebuf);
					line_no++;
					memset(config_linebuf,0,MAX_LINE_LENGTH);
					continue;
				}
				else                                        // 正是需要修改的那行
				{
					sprintf(buf[line_no], "%s = %s\n",conf_name,config_buff);
					line_no++;
					memset(config_linebuf,0,MAX_LINE_LENGTH);
					continue;
				}
			}
		}	
		fclose(f);
		if (line_no < 3)			// dynamic.conf file is ruined ,then ,copy from backup.
		{
			printf("dynamic.conf file error:maybe be ruined by software!!\n");
			sprintf(backupfile,"%s%s",jtpz->dConfigBackup,file_name);
			copy_file(backupfile,conf_path);
			
			f = fopen(conf_path,"r");
			if(f == NULL)
			{
				printf("OPEN CONFIG FALID,file may do not exit\n");
				sprintf(backupfile,"%s%s",jtpz->dConfigBackup,file_name);
				copy_file(backupfile,conf_path);
				
				f = fopen(conf_path,"r");
			}

			while(fgets(config_linebuf,MAX_LINE_LENGTH,f) != NULL)
			{   
				if(strlen(config_linebuf) < 3) //判断是否是空行
				{
					strcpy(buf[line_no],config_linebuf);
					line_no++;
					memset(config_linebuf,0,MAX_LINE_LENGTH);
					continue;
				}
				
				leave_line = strstr(config_linebuf,config_sign);
				if(leave_line == NULL)                            //无"="的情况,则为 注释
				{
					strcpy(buf[line_no],config_linebuf);
					line_no++;
					memset(config_linebuf,0,MAX_LINE_LENGTH);
					continue;
				}
				else
				{
					leave_line = strstr(config_linebuf,conf_name);
					if(leave_line == NULL)                            // 不是需要修改的行
					{
						strcpy(buf[line_no],config_linebuf);
						line_no++;
						memset(config_linebuf,0,MAX_LINE_LENGTH);
						continue;
					}
					else                                        // 正是需要修改的那行
					{
						sprintf(buf[line_no], "%s = %s\n",conf_name,config_buff);
						line_no++;
						memset(config_linebuf,0,MAX_LINE_LENGTH);
						continue;
					}
				}
			}	
			fclose(f);
		}	
		un_lock(f_no,0,SEEK_SET,0);
		
		fp = fopen(conf_path,"w");
		if(fp == NULL)
		{
			printf("write -->OPEN CONFIG FALID\n");
			return -1;
		}
		int i;
		int err;
		f_no=fileno(fp);
		writew_lock(f_no,0,SEEK_SET,0);
		for (i=0; i<line_no; i++)
		{
			err = fputs(buf[i],fp);
			if (err == EOF)
			{
				printf("write to dynamic.conf fail!!!!!\n");
				exit(-1);
			}
		}
		
		fflush(fp);	
		fsync(fileno(fp));	
		fclose(fp);
		un_lock(f_no,0,SEEK_SET,0);
		Config_Occupy_Sign = NotOccupied;
	}	
//	usleep(5000);
	WriteLog("@@@@@@@@@@@@@@@@@ change dynamic.conf sucessfully !!@@@@@@@@@@@@@@\n");
	return 0;
}

///////////////////////   read static configure file    ////////////////////////////
void Read_Static_Config(static_config *jtpz,dynamic_config *dtpz)
{
	int value = 0; 
	char str_value[10] = {0};
	
	int len = 0;
	char direction[PATHLENGTH] = {0};
	
	extern interface arg;
//////////	  direction   ////////////
/// dBackup
	ReadConfig(static_configfile_path,"dBackup",arg.static_conf.dBackup,jtpz,dtpz);	
    len = strlen(arg.static_conf.dBackup);
    if (arg.static_conf.dBackup[len-1] == '\n')
		arg.static_conf.dBackup[len-1] = '\0';
	printf("arg.readconf1.dBackup=%s\n",arg.static_conf.dBackup);
/// dConfigBackup
	ReadConfig(static_configfile_path,"dConfigBackup",arg.static_conf.dConfigBackup,jtpz,dtpz);	
    len = strlen(arg.static_conf.dConfigBackup);
    if (arg.static_conf.dConfigBackup[len-1] == '\n')
		arg.static_conf.dConfigBackup[len-1] = '\0';
	printf("arg.static_conf.dConfigBackup=%s\n",arg.static_conf.dConfigBackup);	
/// dShellBackup
	ReadConfig(static_configfile_path,"dShellBackup",arg.static_conf.dShellBackup,jtpz,dtpz);	
    len = strlen(arg.static_conf.dShellBackup);
    if (arg.static_conf.dShellBackup[len-1] == '\n')
		arg.static_conf.dShellBackup[len-1] = '\0';
	printf("arg.static_conf.dShellBackup=%s\n",arg.static_conf.dShellBackup);
///dSrc_Cmd
	ReadConfig(static_configfile_path,"dSrc_Cmd",arg.static_conf.dSrc_Cmd,jtpz,dtpz);	
    len = strlen(arg.static_conf.dSrc_Cmd);
    if (arg.static_conf.dSrc_Cmd[len-1] == '\n')
		arg.static_conf.dSrc_Cmd[len-1] = '\0';
	printf("arg.static_conf.dSrc_Cmd=%s\n",arg.static_conf.dSrc_Cmd);
/// dCmd
	ReadConfig(static_configfile_path,"dCmd",arg.static_conf.dCmd,jtpz,dtpz);	
    len = strlen(arg.static_conf.dCmd);
    if (arg.static_conf.dCmd[len-1] == '\n')
		arg.static_conf.dCmd[len-1] = '\0';
	printf("arg.static_conf.dCmd=%s\n",arg.static_conf.dCmd);
/// dDownload
    ReadConfig(static_configfile_path,"dDownload",arg.static_conf.dDownload,jtpz,dtpz);	
    len = strlen(arg.static_conf.dDownload);
    if (arg.static_conf.dDownload[len-1] == '\n')
		arg.static_conf.dDownload[len-1] = '\0';
	printf("arg.static_conf.dDownload=%s\n",arg.static_conf.dDownload);
/// dConfig
	ReadConfig(static_configfile_path,"dConfig",arg.static_conf.dConfig,jtpz,dtpz);	
    len = strlen(arg.static_conf.dConfig);
    if (arg.static_conf.dConfig[len-1] == '\n')
		arg.static_conf.dConfig[len-1] = '\0';
	printf("arg.static_conf.dConfig=%s\n",arg.static_conf.dConfig);
/// dLog
    ReadConfig(static_configfile_path,"dLog",arg.static_conf.dLog,jtpz,dtpz);	
    len = strlen(arg.static_conf.dLog);
    if (arg.static_conf.dLog[len-1] == '\n')
		arg.static_conf.dLog[len-1] = '\0';
	printf("arg.static_conf.dLog=%s\n",arg.static_conf.dLog);
/// dTmp
    ReadConfig(static_configfile_path,"dTmp",arg.static_conf.dTmp,jtpz,dtpz);	
    len = strlen(arg.static_conf.dTmp);
    if (arg.static_conf.dTmp[len-1] == '\n')
		arg.static_conf.dTmp[len-1] = '\0';
	printf("arg.static_conf.dTmp=%s\n",arg.static_conf.dTmp);	
/// dCatlog
	ReadConfig(static_configfile_path,"dCatlog",arg.static_conf.dCatlog,jtpz,dtpz);	
    len = strlen(arg.static_conf.dCatlog);
    if (arg.static_conf.dCatlog[len-1] == '\n')
		arg.static_conf.dCatlog[len-1] = '\0';
	printf("arg.static_conf.dCatlog=%s\n",arg.static_conf.dCatlog);	
/// dSBD
    ReadConfig(static_configfile_path,"dSBD",arg.static_conf.dSBD,jtpz,dtpz);	
    len = strlen(arg.static_conf.dSBD);
    if (arg.static_conf.dSBD[len-1] == '\n')
		arg.static_conf.dSBD[len-1] = '\0';
	printf("arg.static_conf.dSBD=%s\n",arg.static_conf.dSBD);
/// dFTP
    ReadConfig(static_configfile_path,"dFTP",arg.static_conf.dFTP,jtpz,dtpz);	
    len = strlen(arg.static_conf.dFTP);
    if (arg.static_conf.dFTP[len-1] == '\n')
		arg.static_conf.dFTP[len-1] = '\0';
	printf("arg.static_conf.dFTP=%s\n",arg.static_conf.dFTP);	
/// dUpload
    ReadConfig(static_configfile_path,"dUpload",arg.static_conf.dUpload,jtpz,dtpz);	
    len = strlen(arg.static_conf.dUpload);
    if (arg.static_conf.dUpload[len-1] == '\n')
		arg.static_conf.dUpload[len-1] = '\0';
	printf("arg.static_conf.dUpload=%s\n",arg.static_conf.dUpload);
/// dUpdate
    ReadConfig(static_configfile_path,"dUpdate",arg.static_conf.dUpdate,jtpz,dtpz);	
    len = strlen(arg.static_conf.dUpdate);
    if (arg.static_conf.dUpdate[len-1] == '\n')
		arg.static_conf.dUpdate[len-1] = '\0';
	printf("arg.static_conf.dUpdate=%s\n",arg.static_conf.dUpdate);
/// dFilelist
    ReadConfig(static_configfile_path,"dFilelist",arg.static_conf.dFilelist,jtpz,dtpz);	
    len = strlen(arg.static_conf.dFilelist);
    if (arg.static_conf.dFilelist[len-1] == '\n')
		arg.static_conf.dFilelist[len-1] = '\0';
	printf("arg.static_conf.dFilelist=%s\n",arg.static_conf.dFilelist);
/// dM2S_filelist
    ReadConfig(static_configfile_path,"dM2S_filelist",arg.static_conf.dM2S_filelist,jtpz,dtpz);	
    len = strlen(arg.static_conf.dM2S_filelist);
    if (arg.static_conf.dM2S_filelist[len-1] == '\n')
		arg.static_conf.dM2S_filelist[len-1] = '\0';
	printf("arg.static_conf.dM2S_filelist=%s\n",arg.static_conf.dM2S_filelist);		
	
/// SBD upload nanjing's FTP location
	memset(SBD_Upload_DIR,0,PATHLENGTH);
    ReadConfig(static_configfile_path,"SBD_Upload_DIR",direction,jtpz,dtpz);	
    len = strlen(direction);
    if (direction[len-1] == '\n')
		direction[len-1] = '\0';
	strcpy(SBD_Upload_DIR,direction);
	printf("SBD_Upload_DIR=%s\n",SBD_Upload_DIR);
/// FTP files upload to nanjing's location
	memset(FTP_Upload_DIR,0,PATHLENGTH);
    ReadConfig(static_configfile_path,"FTP_Upload_DIR",direction,jtpz,dtpz);	
    len = strlen(direction);
    if (direction[len-1] == '\n')
		direction[len-1] = '\0';
	strcpy(FTP_Upload_DIR,direction);
	printf("FTP_Upload_DIR=%s\n",FTP_Upload_DIR);
	
///OpenPort IP address
	memset(OpenPort_IP,0,20);
    ReadConfig(static_configfile_path,"OpenPort_IP",direction,jtpz,dtpz);	
    len = strlen(direction);
    if (direction[len-1] == '\n')
		direction[len-1] = '\0';
	strcpy(OpenPort_IP,direction);
	printf("OpenPort_IP=%s\n",OpenPort_IP);

///Non_Compress_Type
	memset(Non_Compress_Type,0,PATHLENGTH);
    ReadConfig(static_configfile_path,"Non_Compress_Type",direction,jtpz,dtpz);	
    len = strlen(direction);
    if (direction[len-1] == '\n')
		direction[len-1] = '\0';
	strcpy(Non_Compress_Type,direction);
	printf("Non_Compress_Type=%s\n",Non_Compress_Type);

/////////  length  /////////////
///MIN_COMPRESS_SBD_LEN     
    ReadConfig(static_configfile_path,"MIN_COMPRESS_SBD_LEN",str_value,jtpz,dtpz);
    MIN_COMPRESS_SBD_LEN = atoi(str_value);
    printf("MIN_COMPRESS_SBD_LEN=%d\n",MIN_COMPRESS_SBD_LEN);     
///MIN_COMPRESS_FTP_SIZE
    ReadConfig(static_configfile_path,"MIN_COMPRESS_FTP_SIZE",str_value,jtpz,dtpz);
    MIN_COMPRESS_FTP_SIZE = atoi(str_value);
    printf("MIN_COMPRESS_FTP_SIZE=%d\n",MIN_COMPRESS_FTP_SIZE);      
///MIN_CMD_RESULT_FTP
    ReadConfig(static_configfile_path,"MIN_CMD_RESULT_FTP",str_value,jtpz,dtpz);
    MIN_CMD_RESULT_FTP = atoi(str_value);
    printf("MIN_CMD_RESULT_FTP=%d\n",MIN_CMD_RESULT_FTP);  
///MAX_CMD_RESULT_LEN
    ReadConfig(static_configfile_path,"MAX_CMD_RESULT_LEN",str_value,jtpz,dtpz);
    MAX_CMD_RESULT_LEN = atoi(str_value);
    printf("MAX_CMD_RESULT_LEN=%d\n",MAX_CMD_RESULT_LEN);
///CMDFILES_MAX  
    ReadConfig(static_configfile_path,"CMDFILES_MAX",str_value,jtpz,dtpz);
    CMDFILES_MAX = atoi(str_value);
    printf("CMDFILES_MAX=%d\n",CMDFILES_MAX);
///MAX_SBD_NO
    ReadConfig(static_configfile_path,"MAX_SBD_NO",str_value,jtpz,dtpz);
    MAX_SBD_NO = atoi(str_value);
    printf("MAX_SBD_NO=%d\n",MAX_SBD_NO);
///MAX_FTP_NUM    
    ReadConfig(static_configfile_path,"MAX_FTP_NUM",str_value,jtpz,dtpz);
    MAX_FTP_NUM = atoi(str_value);
    printf("MAX_FTP_NUM=%d\n",MAX_FTP_NUM);
///MAX_DLFTP_NUM    
    ReadConfig(static_configfile_path,"MAX_DLFTP_NUM",str_value,jtpz,dtpz);
    MAX_DLFTP_NUM = atoi(str_value);
    printf("MAX_DLFTP_NUM=%d\n",MAX_DLFTP_NUM);
 
///MAX_WAIT_TIME   
    ReadConfig(static_configfile_path,"MAX_WAIT_TIME",str_value,jtpz,dtpz);
    MAX_WAIT_TIME = atoi(str_value);
    printf("MAX_WAIT_TIME=%d\n",MAX_WAIT_TIME);    
///SBD_Wait_Time
    ReadConfig(static_configfile_path,"MAX_WAIT_TIME",str_value,jtpz,dtpz);
    SBD_Wait_Time = atoi(str_value);
    printf("MAX_WAIT_TIME=%d\n",SBD_Wait_Time); 
///FTP_Wait_Time
    ReadConfig(static_configfile_path,"MAX_WAIT_TIME",str_value,jtpz,dtpz);
    FTP_Wait_Time = atoi(str_value);
    printf("MAX_WAIT_TIME=%d\n",FTP_Wait_Time);    
///DL_FTP_Wait_Time = 5
    ReadConfig(static_configfile_path,"MAX_WAIT_TIME",str_value,jtpz,dtpz);
    DL_FTP_Wait_Time = atoi(str_value);
    printf("MAX_WAIT_TIME=%d\n",DL_FTP_Wait_Time); 
       
///Idle_loop_Time = 5
    ReadConfig(static_configfile_path,"Idle_loop_Time",str_value,jtpz,dtpz);
    Idle_loop_Time = atoi(str_value);
    printf("Idle_loop_Time=%d\n",Idle_loop_Time);     
///Busy_loop_Time = 500000
    ReadConfig(static_configfile_path,"Busy_loop_Time",str_value,jtpz,dtpz);
    Busy_loop_Time = atoi(str_value);
    printf("Busy_loop_Time=%d\n",Busy_loop_Time);  
      
///OpenPort_Check_Circle;
    ReadConfig(static_configfile_path,"OpenPort_Check_Circle",str_value,jtpz,dtpz);
    OpenPort_Check_Circle = atoi(str_value);
    printf("OpenPort_Check_Circle=%d\n",OpenPort_Check_Circle); 


///MAX_Ping_Time: ping times before quit
    ReadConfig(static_configfile_path,"MAX_Ping_Time",str_value,jtpz,dtpz);
    MAX_Ping_Time = atoi(str_value);
    printf("MAX_Ping_Time=%d\n",MAX_Ping_Time);	
///MAX_NOOP_Time
    ReadConfig(static_configfile_path,"MAX_NOOP_Time",str_value,jtpz,dtpz);
    MAX_NOOP_Time = atoi(str_value);
    printf("MAX_NOOP_Time=%d\n",MAX_NOOP_Time); 
///MAX_OpenPort_Reboot_Time
    ReadConfig(static_configfile_path,"MAX_OpenPort_Reboot_Time",str_value,jtpz,dtpz);
    MAX_OpenPort_Reboot_Time = atoi(str_value);
    printf("MAX_OpenPort_Reboot_Time=%d\n",MAX_OpenPort_Reboot_Time); 
    
///OpenPort_Check_Circle
    ReadConfig(static_configfile_path,"Modem_Check_Circle",str_value,jtpz,dtpz);
    Modem_Check_Circle = atoi(str_value);
    printf("Modem_Check_Circle=%d\n",Modem_Check_Circle); 

///MAX_AT_Time
    ReadConfig(static_configfile_path,"MAX_AT_Time",str_value,jtpz,dtpz);
    MAX_AT_Time = atoi(str_value);
    printf("MAX_AT_Time=%d\n",MAX_AT_Time);  
///MAX_Modem_Reboot_Time
	ReadConfig(static_configfile_path,"MAX_Modem_Reboot_Time",str_value,jtpz,dtpz);
    MAX_Modem_Reboot_Time = atoi(str_value);
    printf("MAX_Modem_Reboot_Time=%d\n",MAX_Modem_Reboot_Time);

    
///PC104-1 IP address
	memset(IP_PC104_1,0,20);
    ReadConfig(static_configfile_path,"IP_PC104_1",IP_PC104_1,jtpz,dtpz);	
    len = strlen(IP_PC104_1);
    if (IP_PC104_1[len-1] == '\n')
		IP_PC104_1[len-1] = '\0';
	printf("IP_PC104_1=%s\n",IP_PC104_1); 
	
///PC104-2 IP address
	memset(IP_PC104_2,0,20);
    ReadConfig(static_configfile_path,"IP_PC104_2",IP_PC104_2,jtpz,dtpz);	
    len = strlen(IP_PC104_2);
    if (IP_PC104_2[len-1] == '\n')
		IP_PC104_2[len-1] = '\0';
	printf("IP_PC104_2=%s\n",IP_PC104_2); 	
}

//////////////////////////   dynamic configure file   //////////////////////////
void Read_dynamic_Config(static_config *jtpz,dynamic_config *dtpz)
{
	int value = 0; 
	char str_value[10] = {0};
	extern interface arg;
/// nSBD	
	ReadConfig(dynamic_configfile_path,"nSBD",str_value,jtpz,dtpz);
    nSBD = arg.dynamic_conf.nSBD = atol(str_value);
    printf("nSBD=%ld\n",nSBD);    
/// nFTP
	ReadConfig(dynamic_configfile_path,"nFTP",str_value,jtpz,dtpz);
    nFTP = arg.dynamic_conf.nFTP = atol(str_value);
    printf("nFTP=%ld\n",nFTP); 	
}

/////////////////////////////////   check configure file  /////////////////////
//////  if not exit ,copy from /home/njkk/backup/**   to /home/njkk/**
void Check_Config_File(const char *static_config_path,const char *dynamic_config_path)
{
	FILE *sfp,*dfp;
	sfp=fopen(static_config_path,"r");
	dfp=fopen(dynamic_config_path,"r");

	char *source1="/home/njkk/backup/static.conf";
	char *source2="/home/njkk/backup/dynamic.conf";
	char temp[PATHLENGTH];

	char comm[PATHLENGTH];
	if(NULL==sfp)
	{
		memset(comm,0,sizeof(comm));
		sprintf(comm,"cp -f %s %s",source1,static_config_path);    	
		system(comm);
	}	
	else
		fclose(sfp);
	
	if(NULL==dfp)
	{
		memset(comm,0,sizeof(comm));
		sprintf(comm,"cp -f %s %s",source2,dynamic_config_path);    
		system(comm);
	}
	else
		fclose(dfp);
}

///--------------------- double PC104 internet -----------------------

int get_ip(char* buffer)
{
	int   sock;
	struct   sockaddr_in   sin;
	struct   ifreq   ifr;
	unsigned char arp[6] ;

	sock = socket(AF_INET,   SOCK_DGRAM,   0);
	if (sock == -1)
	{
		perror("socket");
		return   -1;
	}
	strncpy(ifr.ifr_name,ETH_NAME,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1]   =   0;

	if (ioctl(sock,SIOCGIFADDR,&ifr)   ==  0)  //»ñÈ¡ip
	{
		memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
		strcpy(buffer,inet_ntoa(sin.sin_addr));
		printf("eth3:   %s\n",buffer);
	}		 
	 return   0;
}
int set_modem_openport_No(char* local_ip,static_config *jtpz,dynamic_config *dtpz)
{
	if (NULL == local_ip)
		return -1;
		
	char *p;
	p = strstr(local_ip,IP_PC104_1);
	if (NULL == p)
	{
		PC104_No = PC104_2;
		OpenPort_No = OpenPort_2;
		Modem_No = Modem_2;
	}
	else
	{
		PC104_No = PC104_1;
		OpenPort_No = OpenPort_1;
		Modem_No = Modem_1;		
	}
}

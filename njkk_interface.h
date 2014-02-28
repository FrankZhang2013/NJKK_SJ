#ifndef	_NJKK_INTERFACE_H
#define	_NJKK_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <pthread.h>

#include <semaphore.h>
#include <dirent.h>

#include <fcntl.h>
#include <signal.h>
#include <time.h> 
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

//---about ip protocal
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

// record locking 
#include <fcntl.h>

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);
#define read_lock(fd,offset,whence,len) lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))  // read lock
#define readw_lock(fd,offset,whence,len) lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len)) //wait read lock
#define write_lock(fd,offset,whence,len) lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))  // write lock
#define writew_lock(fd,offset,whence,len) lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len)) //wait write lock
#define un_lock(fd,offset,whence,len) lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))  // unlock

///	------------------------Global variables definition below --------------------
/// Msg Queue
#define MSG_Queue_NO	5      //  msg queue number
int MsgID[MSG_Queue_NO];
#define MSGIDPLC	    0		// msg queue sequence
#define MSGIDIrdm	    1
#define MSGIDPC104    2
#define MSGIDNAS      3
#define MSGIDCAMERA   4
#define MSGIDPROC     5

#define BUFSIZE 256			// msg  infomation max length
#define MSG_BUF_SIZE	2048     // 

/// file: name length;
#define PATHLENGTH 100       // file name length
#define CMDLENGTH 100        // linux command length
/// configure file 
#define static_configfile_path  "/home/njkk/config/static.conf" 
#define dynamic_configfile_path "/home/njkk/config/dynamic.conf"

#define MAX_LINE_LENGTH 256   //   config file max length per line 
#define MAX_LINE  500          //   config file max lines

char SBD_Upload_DIR[PATHLENGTH];   // ./SBD
char FTP_Upload_DIR[PATHLENGTH];   // ./FTP
char Non_Compress_Type[PATHLENGTH];  // need not compress file type

int MS_PC104;
int MAX_CMD_RESULT_LEN;				// max cmd execute result length ;; direction inquiry maybe too long
int MIN_COMPRESS_SBD_LEN;			// SBD compress minimum length
int MIN_COMPRESS_FTP_SIZE;			// ftp file compress minimum size
int MIN_CMD_RESULT_FTP;				// cmd result is too long ,send file by ftp instead of sbd
int MAX_SBD_NO;						// max SBD number						
int CMDFILES_MAX;						// max command files number				
int MAX_FTP_NUM;						// max ftp upload files number
int MAX_DLFTP_NUM;					// max downloadftp files number

int MAX_WAIT_TIME;					// max time :3
int SBD_Wait_Time;					// max upload sbd by lftp try time   :  5
int FTP_Wait_Time;					// max upload ftp by lftp try time   :  5
int DL_FTP_Wait_Time;				// max upload sbd by lftp try time   :  5

int Idle_loop_Time; // pthread loop time when it's idle  (seonds) = 5
int Busy_loop_Time;//pthread loop time when it's busy (mocro seconds)       = 500000

// parament set in static.conf
char OpenPort_IP[20];
char IP_PC104_1[20];
char IP_PC104_2[20];

int OpenPort_Check_Circle; //= 600

int MAX_Ping_Time; //= 3
int MAX_NOOP_Time; //= 3
int MAX_OpenPort_Reboot_Time; //= 2
// global variable 

time_t OpenPort_last_time;
int OpenPort_Reboot_Time;

int Modem_Check_Circle; //= 600

int MAX_AT_Time;  //= 5
int MAX_Modem_Reboot_Time; //= 2
// global variable
time_t Modem_last_time;

int Modem_Reboot_Time;

/// mutex 
#define NotOccupied 0  ///FTP/SBD file state,not occupied
#define Occupied 1  ///FTP/SBD file state,occupied
///		mutex locker
pthread_mutex_t SBD_mutex;
pthread_mutex_t FTP_mutex;
pthread_mutex_t DL_FTP_mutex;
pthread_mutex_t Config_mutex;
int Config_Occupy_Sign;


///about ip
#define  ETH_NAME   "eth3"
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN (1024*(EVENT_SIZE+100))

char IP_local_PC104[20];

/// PC104 OpenPort Modem No
#define OpenPort_1 1
#define OpenPort_2 2
#define Modem_1 1
#define Modem_2 2
#define PC104_1 1
#define PC104_2 2

int PC104_No;
int OpenPort_No;
int Modem_No;

int PC104_state;

#define PC104_Master	0	//It's master PC104.
#define PC104_Slave	1	//It's slave PC104.
#define PC104_Single_Master	2	//It's single master PC104.

#define OK 1
#define Error 0

typedef struct
{
	pthread_attr_t         attr;
	pthread_t              id;
	struct sched_param     param;
} Thread_Param;
typedef Thread_Param *Thread_P;

/// -------------------------- Data structure -------------------------------
//  download
typedef struct
{
	unsigned char CmdType;
	unsigned short CmdNo;
	char dest_source[PATHLENGTH];
}download_cmd;

//  dynamic configure 
typedef struct
{
	long nSBD;
	long nFTP;
}dynamic_config;

unsigned long nSBD;
unsigned long nFTP;

//  static configure
typedef struct
{	
	char dBackup[PATHLENGTH];
	char dConfigBackup[PATHLENGTH];
	char dShellBackup[PATHLENGTH];
	char dSrc_Cmd[PATHLENGTH];
	char dCmd[PATHLENGTH];
	char dDownload[PATHLENGTH];
	char dConfig[PATHLENGTH];	
	char dLog[PATHLENGTH];
	char dTmp[PATHLENGTH];
	char dCatlog[PATHLENGTH];	
	char dSBD[PATHLENGTH];
	char dFTP[PATHLENGTH];
	char dUpload[PATHLENGTH];
	char dUpdate[PATHLENGTH];
	char dFilelist[PATHLENGTH];
	char dM2S_filelist[PATHLENGTH];
	char dSJ_config[PATHLENGTH];	
}static_config;

//  include static & dynamic configure data
typedef struct 
{
		static_config static_conf;
		dynamic_config dynamic_conf;
		
}interface;

struct switchdata
{
	int state;
	time_t lastchangetime;
	int fastchangenum;
};

typedef struct
{
	int mtype;
  	char mtext[MSG_BUF_SIZE];//MSG_BUF_SIZE
}Msg_Info;

typedef struct 
{
	char prio;
	int comid;
	char str[1024];	
}MsgUp;

typedef struct 
{
	int comid;
	char str[64];	
}MsgDown,MsgUp_Post;

//// command execute result   S: sucess   F: fail
typedef struct
{
	unsigned short CmdNo;
	char result;
}execute_result;


////  upload command data info
typedef struct
{
	unsigned short CmdNo;
	unsigned char Priority;
	unsigned char mv_cp_flag;
}upload_cmd;

//// cmd execute result;
typedef struct
{
//		unsigned char CmdType;
	unsigned short CmdNo;
	char str[100];	
}cmd_msg;


/// -------------------------- function --------------------------------------
/////////functions in njkk_public.h
void verify_success(int status,const char *message_p);
int Install_Thread(Thread_P ThreadParam, void *ThreadFun, int ThreadPrio, sem_t *PSem);
int CreateDir(const char *PathName);

int Create_Msg_Queue(const char *MsgPath);
int send_msg(int msqid,int msgtype,char *msg,int size);
int recv_msg(int msqid,int *msgtype,char *msg,int size);
int recv_msg_NOWAIT_prio(int msqid,int *msgtype,char *msg,int size);
int show_msg_queue_stat(int msqid);
int Install_Msg_Queue();

int ReadConfig(char *conf_path,char *conf_name,char *config_buff,static_config *jtpz,dynamic_config *dtpz);
int Change_Config(char *conf_path,char *conf_name,char *config_buff,static_config *jtpz,dynamic_config *dtpz);
void Read_Static_Config();
void Read_dynamic_Config();
void Check_Config_File(const char *static_config_path,const char *dynamic_config_path);

void CheckPath(const char *dirpath);
int Check_IrdmFile(const char *SBDListName);
void check_script_file(static_config *jtpz,dynamic_config *dtpz);

int WriteLog(char *newlog);
/// get filename from fileDir
void getfilename(const char *filesoure,char *name);
int getfile_type(const char *filename,char *file_type);
int FILESIZE(unsigned char SENDFILE[100]);

/////added :20121115
//get variable safely in multi-thread
int getVar(pthread_mutex_t *mutex,int *var);

//set variable safely in multi-thread
int set_variable(pthread_mutex_t *mutex,int *var,int newVal);
//Increase variable by 1 safely in multi-thread
int IncVar(pthread_mutex_t *mutex,int *var);

//Decrease variable by 1 safely in multi-thread
int DecVar(pthread_mutex_t *mutex,int *var);

// get local PC104's IP address
int get_ip(char* buffer);

#endif

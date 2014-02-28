#ifndef _GETCHILDFILES_H
#define _GETCHILDFILES_H

/////////////////////// Add header files needed below //////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//linux file operation headers
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

//linux thread operation headers
#include <pthread.h>

//COM Operation headers
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h> 
#include "zlib.h"
#include "njkk_interface.h"

//////////////////// Add new macro variables definition below ////////////////////////////////

///\//Irdm Modem Operation

#define MAX_SBDNAME_LEN 128          

//#define MAX_SBD_LEN 1841
#define max_RcvSBD 1890
#define max_SendSBD 1960

///cmd file processing

#define CMDFILENAME_LEN_MAX  64     
#define MAX_CMD_LEN	100				
#define INT2STR_LEN	32
 
#define LongInt_LEN 12 
/// File name length and numbers
#define MAX_NAME_LEN 128

/// configure files location ; maybe change

#define SBD_File_Header  "M_"
#define FTP_File_Header  "F_"

///\////   1/2 select
int Comm_Channel;	//Communicate channel:0:OpenPort;1:Irduim Modem.
int Trans_Mode;//Transfer method in Modem channel:SBD or FTP;  0:SBD;1:FTP. 

int OpenPort_state;
int Modem_state;
int opposite_openport_state;
int opposite_modem_state;

char M2S_cmd[PATHLENGTH];
// SBD send flag, kill lftp to dead ,because lftp is killed once, lftp will try MAX_WAIT_TIME
///OpenPOrt transport state
int sbd_send_flag;
int ftp_send_flag;

/// Modem transport state
int sbd_rcv_flag;
int sbd_snd_flag;
int ftp_deliver_flag;

#define OPENPORT 0	///OpenPort or Modem state
#define MODEM    1  	///OpenPort or Modem state

#define SBDSTART 0	///SBD mode
#define FTPSTART 1  ///FTP mode

//////////////////// Add new global variables definition below //////////////////////////////

///  threads start flag 
int Th_channel_check_start;
int Th_sbd_check_start;
int Th_irdm_msg_start;
int Th_sbd_upload_start;
int Th_ftp_upload_start;

/// 	MSG ID  name
int MsgIDPLC;
int MsgIDIrdm;
int MsgIDPC104;
int MsgIDNAS;
int MsgIDCamera;
int MsgIDProc;

///  	Iridum modem  
int fd_serial;
int flag_close;
int MTqueued;
int MTMSN;
int MTLENGTH;
int MO;
int MT;
int MOMSN;

unsigned char SBDbuffer[max_RcvSBD];

///\ H/M/LSBD.dat flag
int HSBD_Occupy_Sign;
int MSBD_Occupy_Sign;
int LSBD_Occupy_Sign;
int HFTP_Occupy_Sign;
int MFTP_Occupy_Sign;
int LFTP_Occupy_Sign;
int DL_FTP_Occupy_Sign;

int HSBD_Num,MSBD_Num,LSBD_Num;
int HFTP_Num,MFTP_Num,LFTP_Num;
int DL_FTP_Num;

////SJ
#define FINISH 1
#define UNFINISH 0
int Slave_exe_result;

int Ethernet_state;
int CAN_state;
//////////////////// Irdm Modem Operation ////////////////////////////
int Checksum(unsigned char sbdmsg[max_RcvSBD],int len);
int decompress_mem(unsigned char compress_flag,unsigned char buffer[max_RcvSBD],int length);
int ReadMTBuf(unsigned char buffer[max_RcvSBD],unsigned char channel,int length);
int SERIAL_RX1(unsigned char buffer[max_RcvSBD]);
int SERIAL_RX2(unsigned char buffer[max_RcvSBD]);
int open_serial(int k);
int init_iridium(int fd_serial,char atcommand[40]);
int RCVmainfun();
int GainSystem(char systemcommand[80]);
void KillWvdial();

int Global_OpLock(pthread_mutex_t *OpLock, int *GlobalVar);

///\ get file numbers from filelist,to initialize HSBD_Num,MSBD_Num,LSBD_Num;
/// HFTP_Num,MFTP_Num,LFTP_Num;DownloadFileNum;
int get_filenum(char *fileaddr);

///  call lftp script to upload files
int uploadftp(char *file);
///  call lftp script to download files
int downloadftp(char *file);
int List_Adjust(char *file_list,static_config *jtpz,dynamic_config *dtpz);
int rsync_file(char *file_rsync,char *path,static_config *jtpz,dynamic_config *dtpz);
///seperate fileDir to filepath and filename
void sepFileDir(const char *filesoure,char *path,char *name);
int ping_test(char *ip_addr);
void CheckIrdmAllFile(static_config *jtpz,dynamic_config *dtpz);
void check_irdm_path();
void init_occupy_sign();
void init_BSD_FTP_num(static_config *jtpz,dynamic_config *dtpz);
void init_mutex();
void Irdm_Destory();
void Irdm_Init(static_config *jtpz,dynamic_config *dtpz);
/////////////////////// functions for OpenPort Command File Processing ////////////////////////

///\//////////////////////getChildFiles//////////////////////////////////
/// compare string like int numbers
int strIntcmp(const char *str1,const char *str2);
/// Bubble sort for Str2Int
void BubbleSortStr(char filename[CMDFILES_MAX][INT2STR_LEN],int n);
/// travel the directory,get children files
int trave_dir(const char* path,char filename[CMDFILES_MAX][INT2STR_LEN]);

///\/////////////////////checkFileGood////////////////////////////////////
/// get filesize
unsigned long getFileSize(const char *path);
/// function:check if command file is completely received
/// return val:0:yes;-1:no.
/// param1:filepath;	param2:filename.
int checkFileGood(const char* path,const char *filename);

///\///////////////////uncompress command files////////////////////////////
int uncomp_File(const char *path,const char *filename,char *uncomp_filename);
/////////////////////// end of functions for OpenPort Command File Processing ////////////////////////

#endif

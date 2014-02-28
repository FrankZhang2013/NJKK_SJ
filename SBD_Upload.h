#include "njkk_interface.h"
#include "Irdm_Interface.h"

#define MAX_CFG_BUF 512

int SBD_List_Adjust(char type,int num,static_config *jtpz,dynamic_config *dtpz);
int Modem_SBD_send_mainfun(unsigned char SENDFILE[100]);
int SBD_Modem_Upload(static_config *jtpz,dynamic_config *dtpz);

int OpenPort_SBD_send_mainfun(char *SBD_List_Name,int msqid);
int SBD_OpenPort_Compress(char *SBD_Name,int num,static_config *jtpz,dynamic_config *dtpz);
int SBD_OpenPort_Upload(static_config *jtpz,dynamic_config *dtpz,int msqid);
//void SBD_Upload(static_config *jtpz,dynamic_config *dtpz);

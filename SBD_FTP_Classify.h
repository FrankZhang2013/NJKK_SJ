#include "njkk_interface.h"
#include "Irdm_Interface.h"

int write_SBD(char *fileaddr,int nSBD,char *str,int len);

/// SBD Classify
int SBD_Classify(char pri,int feedbak_msg_type,char *str,int len,static_config *jtpz,dynamic_config *dtpz);
/*
void getfilename(char *filesoure,char *name);
void FTP_File_Gen(char *source,int CmdID,char *dest,dynamic_config dtpz,static_config jtpz);
*/
/// FTP Classify
int FTP_Classify(int msqid,char pri,int feedback_msg_type,char *str,int len,static_config *jtpz,dynamic_config *dtpz);

/// Download FTP Classify
int DL_FTP_Classify(char *str,int len,static_config *jtpz,dynamic_config *dtpz);

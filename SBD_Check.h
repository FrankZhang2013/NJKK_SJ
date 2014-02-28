#include "njkk_interface.h"
#include "Irdm_Interface.h"

#include "SBD_Cmd.h"

int SBD_Check(unsigned char *str,int length,int msqid,int msqid0,int msqid1,int msqid2,int msqid3,static_config *jtpz,dynamic_config *dtpz);
void SBDCmd_Read_Modem(static_config *jtpz,dynamic_config *dtpz);

int parser_cmd(char *filename,static_config *jtpz,dynamic_config *dtpz);
int SBDCmd_Read_OpenPort(static_config *jtpz,dynamic_config *dtpz);

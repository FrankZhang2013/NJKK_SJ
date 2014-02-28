#include "njkk_interface.h"
#include "Irdm_Interface.h"
#include "Irdm_main.h"

int FTP_Modem_OpenPort(char *fileaddr, int compress_flag,dynamic_config *dtpz,static_config *jtpz);

int  FTP_Uploadftp(int pc104_state,static_config *jtpz,dynamic_config *dtpz);

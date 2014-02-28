#include "Irdm_Interface.h"
#include "njkk_interface.h"

#include "SBD_Cmd.h"

#include <termios.h>
#include <unistd.h>



int AT_test();
void check_Modem_state(int msqid);

int NOOP_test(static_config *jtpz,dynamic_config *dtpz);
void ping(int msqid);
void NOOP(static_config *jtpz,dynamic_config *dtpz);
void ping_NOOP_test(int msqid,static_config *jtpz,dynamic_config *dtpz);
void check_OpenPort_state(int msqid,static_config *jtpz,dynamic_config *dtpz);
int ping_test(char *ip_addr);
// new added 
void check_modem(int msqid);
void check_openport(int msqid,static_config *jtpz,dynamic_config *dtpz);

void OpenPort_Modem_state_check(int msqid0,int msqid,static_config *jtpz,dynamic_config *dtpz);

int set_modem_openport_No(char* local_ip,static_config *jtpz,dynamic_config *dtpz);
void write_OpenPort_Modem_state(static_config *jtpz,dynamic_config *dtpz);

void poweron_channel_check(static_config *jtpz,dynamic_config *dtpz);

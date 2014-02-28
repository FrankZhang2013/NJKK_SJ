#ifndef _PC104_H
#define _PC104_H

//COM Operation headers
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h> 

#include "njkk_interface.h"
#include "Irdm_Interface.h"
///\////////////// define macro variables ///////////////////////////////////

///socket
#define PC104_1_Send_PORT  5432
#define PC104_1_Rcv_PORT   5433

////socket recvfrom 
#define MAXLENGTH  512

typedef struct
{
	char heart_head[10];
	int pc104_state;
	int slave_exe_result;
	int openport_state;
	int modem_state;
}heart;

///serial Port
#define COM0	0
#define COM1	1
///tcp and serial transfer bufsize
#define SENDBUF_SIZE	128
#define RECVBUF_SIZE	128
///NET and CAN link-state

//// rsync + inotify
#define  ETH_NAME   "eth3"
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN (1024*(EVENT_SIZE+100))

///\////////////// define struct variables /////////////////////////////////

///\////////////// function declaration ///////////////////////////////////

int CAN_state_check();
int network_state_check(static_config *jtpz,dynamic_config *dtpz);

int tcp_send_heart();
int tcp_recv_heart();

int udp_send_heart();
int set_PC104_State(char *rcv_msg);
int udp_recv_heart();
void CAN_send_heart();
void CAN_recv_heart();
#endif

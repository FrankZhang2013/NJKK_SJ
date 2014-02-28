#include "OpenPort_Modem_state_check.h"

int AT_test()
{
	int rt;
	fd_serial = open_serial(0);
	if (fd_serial <= 0)
	{
		printf("open modem COM error!\n");
		Modem_state = 0;
		rt = -1;
		return rt;
	}
//
	 struct termios newtio;
	 struct termios oldtio;
	 bzero(&newtio,sizeof(newtio));
	 tcgetattr(fd_serial,&oldtio);
//
	 cfsetispeed(&newtio,B19200);      /*娉㈢壒鐜囪缃负19200bps*/
	 cfsetospeed(&newtio,B19200);
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
	 
	 sleep(1);
	 char atcommand[10] = {0};
	 int length;
	 strcpy(atcommand,"atz\r");
	 length = strlen(atcommand);
	 write(fd_serial,atcommand,length+1);                          //向串口写at命令
	
	sleep(1);	
	char *ptr;
	char buffer[100] = {0};
	SERIAL_RX1(buffer);
	ptr=strstr((const char*)buffer, "OK");
	if ( ptr != NULL)
	{
		Modem_state = 1;
		rt = 0;
	}
	else
	{
		Modem_state = 0;
		rt = -1;
	}
	
	tcsetattr(fd_serial,TCSANOW,&oldtio);

	 flag_close=close(fd_serial);  
	 if(flag_close==-1)               
		printf("Close the Irdm failure\n");	
		
	return rt;
}
/*
void check_Modem_state(int msqid)
{
	int at_time=0;
	if (Modem_state == 1)
	{
		if (sbd_rcv_flag == 1 || sbd_snd_flag == 1 || ftp_deliver_flag == 1)
		{
			Modem_OK_Count = 0;
			return ;
		}
		else
		{		
			Modem_OK_Count++;
			Modem_Error_Count = 0;
			if (Modem_OK_Count < MAX_Modem_OK_Count)  // every 20minutes /check
			{
				return;
			}
			else
			{
				sbd_rcv_flag = 1;
				while (AT_test()!=0)
				{
					at_time++;
					if(at_time == MAX_AT_Time)
					{
						if (Modem_Reboot_Time == MAX_Modem_Reboot_Time)
						{
							Modem_state = 0;
							Comm_Channel = OPENPORT;
							Modem_OK_Count = 0;
							Modem_Error_Count++;
							sbd_rcv_flag = 0;	
							return;
						}
						else
						{
							send_msg(msqid,137,NULL,0);  // restart modem;
							return;
						}
					}
				}
				Modem_OK_Count = 0;
				Modem_state = 1;
				sbd_rcv_flag = 0;		
			}
		}		
	}
	else
	{
		Modem_Error_Count++;
		Modem_OK_Count = 0;
		if (Modem_Error_Count < MAX_Modem_Error_Count)  // every 20minutes /check
		{
			return;
		}
		else
		{
			sbd_rcv_flag = 1;
			while (AT_test()!=0)
			{
				at_time++;
				if(at_time == MAX_AT_Time)
				{
					if (Modem_Reboot_Time == MAX_Modem_Reboot_Time)
					{
						Modem_state = 0;
						Comm_Channel = OPENPORT;
						Modem_OK_Count = 0;
						Modem_Error_Count = 0;
						sbd_rcv_flag = 0;	
						return;
					}
					else
					{
						send_msg(msqid,137,NULL,0);  // restart modem;
						return;
					}
				}
			}
			Modem_OK_Count = 0;
			Modem_Error_Count = 0;
			Modem_state = 1;
			sbd_rcv_flag = 0;		
		}
	}
}
*/
int ping_test(char *IP_addr)
{	
	int rt;
	char ping_cmd[50] = {0};
	char *p;	
	sprintf(ping_cmd,"ping -w 5 -c 5 %s",IP_addr);	
	rt = system(ping_cmd);
	usleep(5000);
	if (rt == 0)
	{
		printf("ping check: OK!\n");		
		return 0;
	}
	else
	{
		printf("ping check: Error!\n");
		return -1;
	}
}

int NOOP_test(static_config *jtpz,dynamic_config *dtpz)
{
	char cmd[100] = "/root/njkk/testftp ";
	char result[100] = {0};
	
	execute_cmd(cmd, result,jtpz,dtpz);
	if (strstr(result,"200") != NULL)
	{
		OpenPort_state = 1;		
		return 0;
	}
	else
	{
		return -1;
	}
}
/*
void ping(int msqid)   // only ping test
{
	int rt;
	int ping_time = 0;
	time_t now;
	time(&now);
	while (ping_test() != 0)   // first:  ping BDE
	{
		ping_time++;
		if (ping_time == MAX_Ping_Time)
		{
			if (OpenPort_Reboot_Time == MAX_OpenPort_Reboot_Time)
			{
				OpenPort_Reboot_Time = 0;
				OpenPort_state = 0;
				OpenPort_OK_Count = 0;
				
				if (Modem_state == 1)
					Comm_Channel = MODEM;	
				return;
			}
			else
			{
				send_msg(msqid,139,NULL,0);     // restart OpenPort
				sleep(10);
				return;
			}							
		}
		usleep(5000);
	}
	OpenPort_state = 1;
	return;
}
*/
void NOOP(static_config *jtpz,dynamic_config *dtpz)
{
	int noop_time= 0;
	while (NOOP_test(jtpz,dtpz) != 0)   // ping is ok,then send noop:200 ok ,openport link is ok;
	{
		noop_time++;
		if (noop_time == MAX_NOOP_Time)
		{
			printf("noop 3 times error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			OpenPort_state = Error;	
			return;
		}
		usleep(5000);
	}
	OpenPort_state = OK;	
	Comm_Channel = OPENPORT;
	return;
}

void ping_NOOP_test(int msqid,static_config *jtpz,dynamic_config *dtpz)
{
	int rt;
	int ping_time = 0;
	int noop_time = 0;
	time_t now;
	time(&now);
	char IP_addr[20]={0};
	if (PC104_No == PC104_1)
		strcpy(IP_addr,IP_PC104_2);
	else
		strcpy(IP_addr,IP_PC104_1);
	/*
	while (ping_test(IP_addr) != 0)   // first:  ping BDE
	{
		ping_time++;
		if (ping_time == MAX_Ping_Time)
		{
			send_msg(msqid,139,NULL,0);     // restart OpenPort
			sleep(10);
			return;
		}
		usleep(5000);
	}
	*/
	while (NOOP_test(jtpz,dtpz) != 0)   // ping is ok,then send noop:200 ok ,openport link is ok;
	{
		noop_time++;
		if (noop_time == MAX_NOOP_Time)
		{
			OpenPort_last_time = now;		
			if (Modem_state == 1)
				Comm_Channel = MODEM;
			printf("---1111111111----OpenPort_state = Error!\n");	
			return;
		}
		usleep(5000);
	}
	OpenPort_state = 1;	
	Comm_Channel = OPENPORT;
	OpenPort_last_time = now;
	printf("---1111111111----OpenPort_state = OK!\n");
	return;
}
/*
void check_OpenPort_state(int msqid,static_config *jtpz,dynamic_config *dtpz)
{
	int rt;
	
	if ( (Comm_Channel == OPENPORT) && (OpenPort_state == 1))
	{
		if (sbd_send_flag == 1 || ftp_send_flag == 1)   // send sbd/ftp with openport lftp
		{
			OpenPort_state = 1;		
			OpenPort_OK_Count = 0;
			OpenPort_Error_Count=0;
			return;
		}	
		else
		{
			OpenPort_OK_Count++;
			OpenPort_Error_Count=0;
			if (OpenPort_OK_Count < MAX_OpenPort_OK_Count)
			{
				return;
			}
			else  			// time to check again (20 minutes)
			{
				ping_NOOP_test(msqid,jtpz,dtpz);
			}
		}		
	}
	else
	{	
		if (OpenPort_state == 0)
		{
			ping(msqid);
			return;
		}
		else
		{
			if (OpenPort_Error_Count < MAX_OpenPort_Error_Count)
			{
				return;
			}
			else  			// time to check again (20 minutes)
			{
				ping_NOOP_test(msqid,jtpz,dtpz);
			}			
		}
	}
}
*/
void check_modem(int msqid)
{
	int at_time=0;
	time_t now;
	time(&now);
	double differ_time = 0;
		
	if (Modem_state == 1)
	{
		if (sbd_rcv_flag == 1 || sbd_snd_flag == 1 || ftp_deliver_flag == 1)
		{
			Modem_last_time = now;
			return ;
		}
		else
		{		
			differ_time = difftime(now,Modem_last_time);
			if (differ_time < Modem_Check_Circle)  // every 20minutes /check
			{
				return;
			}
			else
			{
				sbd_rcv_flag = 1;
				while (AT_test()!=0)
				{
					at_time++;
					if(at_time == MAX_AT_Time)
					{
							send_msg(msqid,137,NULL,0);  // restart modem;
							sleep(10);						
							Modem_state = 0;
							Comm_Channel = OPENPORT;
							Modem_last_time = now;
							sbd_rcv_flag = 0;
							printf("----111111----Modem_state = Error!\n");	
							return;
					}
				}
				Modem_last_time = now;
				Modem_state = 1;
				sbd_rcv_flag = 0;
				printf("----111111----Modem_state = OK!\n");			
			}
		}		
	}
	else
	{
		differ_time = difftime(now,Modem_last_time);
		if (differ_time < Modem_Check_Circle)  // every 20minutes /check
		{
			return;
		}
		else
		{
			sbd_rcv_flag = 1;
			sbd_send_flag =1;
			while (AT_test()!=0)
			{
				at_time++;
				if(at_time == MAX_AT_Time)
				{
						send_msg(msqid,137,NULL,0);  // restart modem;
						sleep(10);					
						Modem_state = 0;
						Comm_Channel = OPENPORT;
						Modem_last_time = now;
						sbd_send_flag =0;
						sbd_rcv_flag = 0;
						printf("----111111----Modem_state = Error!\n");		
						return;
				}
			}
			Modem_last_time = now;
			Modem_state = 1;
			sbd_send_flag =0;
			sbd_rcv_flag = 0;
			printf("----111111----Modem_state = OK!\n");			
		}
	}
}	

void check_openport(int msqid,static_config *jtpz,dynamic_config *dtpz)
{
	int rt;
	double differ_time = 0;	
	time_t now;
	time(&now);
	
	if ( (Comm_Channel == OPENPORT) && (OpenPort_state == 1))
	{
		if (sbd_send_flag == 1 || ftp_send_flag == 1)   // send sbd/ftp with openport lftp
		{
			OpenPort_state = OK;		
			OpenPort_last_time = now;
			return;
		}	
		else
		{
			differ_time = difftime(now,OpenPort_last_time);

			if (differ_time < OpenPort_Check_Circle)
			{
				return;
			}
			else  			// time to check again (20 minutes)
			{
				ping_NOOP_test(msqid,jtpz,dtpz);
			}
		}		
	}
	else
	{			
		differ_time = difftime(now,OpenPort_last_time);
		if (differ_time < OpenPort_Check_Circle)
		{
			return;
		}
		else  			// time to check again (20 minutes)
		{
			ping_NOOP_test(msqid,jtpz,dtpz);
		}
	}
}	
								     //   Irdm ,PC104 
void OpenPort_Modem_state_check(int msqid0,int msqid,static_config *jtpz,dynamic_config *dtpz)
{

	check_openport(msqid,jtpz,dtpz);
	check_modem(msqid);
/*	
	printf("++++++++  OpenPort_Modem_state_check  ++++++\n");
	printf("OpenPort_state = %d\n",OpenPort_state);
	printf("Modem_state = %d\n",Modem_state);
	printf("Comm_channel=%d\n",Comm_Channel);
	printf("++++++++++++++++++++++++++++++++++++++++++++\n");
*/
}

void poweron_channel_check(static_config *jtpz,dynamic_config *dtpz)
{
	MTqueued = 0;
	MTMSN = 0;
	MTLENGTH = 0;
	MO = 0;
	MT = 0;
	MOMSN = 0;

	NOOP(jtpz,dtpz);
	AT_test();
	if ((OpenPort_state == Error) && (Modem_state == OK))
		Comm_Channel = MODEM;
			
	printf("+++++++++  poweron_channel_check  ++++++++++++++++++\n");
	printf("OpenPort_state = %d\n",OpenPort_state);
	printf("Modem_state = %d\n",Modem_state);
	printf("Comm_channel=%d\n",Comm_Channel);
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	
	time_t start;
	time(&start); 	// start time ;

	OpenPort_last_time = start;
	Modem_last_time = start; //= 600
}

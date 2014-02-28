///\//socket///////
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

///\//thread//////

#include "njkk_interface.h"
#include "SJ_heart.h"

char head[10]="abcdefg";
int CAN_state_check()
{
	
}

int network_state_check(static_config *jtpz,dynamic_config *dtpz)
{
	Ethernet_state_check();
	CAN_state_check();
}
int tcp_send_heart()
{
	/// send heart package
	heart  heart_package;
	memset(&heart_package,0,sizeof(heart_package));
	strncpy(heart_package.heart_head,head,strlen(head));
	heart_package.pc104_state=PC104_state;
	heart_package.slave_exe_result=Slave_exe_result;
	heart_package.openport_state=OpenPort_state;
	heart_package.modem_state=Modem_state;
	int SERV_PORT;
	
	char IP_addr[20]={0};
	if (PC104_No == PC104_1)
	{
		strcpy(IP_addr,IP_PC104_2);
		SERV_PORT = PC104_1_Send_PORT;
	}
	else
	{
		strcpy(IP_addr,IP_PC104_1);	
		SERV_PORT = PC104_1_Rcv_PORT;
	}
	int sockfd;
	if ( (sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1 )
	{
		printf("create sock errpr!\n");
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,IP_addr,&serv_addr.sin_addr);
//	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero),8);
// wait server to create 
//	sleep(1);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) 
	{	
		printf("tcp client connect error!\n");
		close(sockfd);
		Ethernet_state = Error;		
		return -1;
	}
	if ( (send(sockfd,&heart_package, sizeof(heart_package), 0)) == -1)
	{
		printf("tcp client send heart package error!\n");
		close(sockfd);
		Ethernet_state = Error;		   
		return -1;
	}
    close(sockfd);
    return 0;
}
int tcp_recv_heart()
{
	int SERV_PORT;
    int BACKLOG = 10;
	if (PC104_No == PC104_1)
		SERV_PORT = PC104_1_Rcv_PORT;
	else
		SERV_PORT = PC104_1_Send_PORT;
	 int sockfd,client_fd,ret; /*sock_fd锛氱洃鍚瑂ocket锛沜lient_fd锛氭暟鎹紶杈搒ocket */
	 struct sockaddr_in my_addr; /* 鏈満鍦板潃淇℃伅 */
	 struct sockaddr_in remote_addr; /* 瀹㈡埛绔湴鍧€淇℃伅 */
	 if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	 {
		printf("tcp server socket start error!\n");
		close(sockfd);
		Ethernet_state = Error;		   
		return -1;
	 }
	 bzero(&(my_addr.sin_zero),8);	 
	 my_addr.sin_family=AF_INET;
	 my_addr.sin_port=htons(SERV_PORT);
	 my_addr.sin_addr.s_addr = INADDR_ANY;

	// bind error: address already in use,,
	// solution: wait time ,then the address is valid
	unsigned int value = 0x1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&value,sizeof(value)); 

	 if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) 
	 {
		perror("++++++  tcp server bind error");
		close(sockfd);
		Ethernet_state = Error;			  
		return -1;		   
	 } 
	 if (listen(sockfd, BACKLOG) == -1) 
	 {
		perror("+++ tcp server listen error");
		close(sockfd);
		Ethernet_state = Error;			  
		return -1;	
	 }
	int  maxfd;
	maxfd = sockfd+1;
	char rcv_msg[MAXLENGTH]={0};
	struct timeval wait_time;
	wait_time.tv_sec = 5;
	wait_time.tv_usec = 0;
	fd_set rset;
	int result;
	int len;
	int length_rcv;
	int rt_set_pc104;
	while(1)
	{

		FD_SET(sockfd,&rset);	
		if ( (result = select(maxfd,&rset,NULL,NULL,&wait_time))<0 )
		{
			if (errno == EINTR)
				continue;
			else
			{
				printf("tcp server recieve: select error!\n");				
				WriteLog("tcp server recieve: select error!\n");
				close(sockfd);
				Ethernet_state = Error;
				return -1;
			}
		}
/// socket  is readable
		if (FD_ISSET(sockfd, &rset))
		{

			len = sizeof(struct sockaddr_in);
			if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &len)) == -1) 
			{
			    printf("Tcp Server accept error!\n");
				close(sockfd);
				Ethernet_state = Error;
			    return -1;
			}
			memset(rcv_msg,0,sizeof(rcv_msg));
			length_rcv = recv(client_fd, rcv_msg, MAXLENGTH, 0);
			if (length_rcv < 0)
			{
				printf("recv msg error, length <0\n");
				close(sockfd);
				Ethernet_state = Error;
				return -1;
			}
			else
			{
				printf("======= TCP recieve OK,start process heart package!\n");
				rt_set_pc104 = set_PC104_State(rcv_msg);
				Ethernet_state = OK;
				printf("======= TCP ,process heart package end!,Ethernet_state=%d\n",Ethernet_state);
				sleep(1);
				continue;
			//	return rt_set_pc104;
			}

		}
		printf("===== TCP recieve error: wait ? seconds ,no msg !\n");
		close(sockfd);
		Ethernet_state = Error;
		return -1;
	}
}

int udp_send_heart()
{
	int sockfd;
	struct sockaddr_in server_addr;
	char IP_addr[20]={0};
	ssize_t result_send;
	int SERV_PORT;

	if (PC104_No == PC104_1)
	{
		strcpy(IP_addr,IP_PC104_2);
		SERV_PORT = PC104_1_Send_PORT;
	}
	else
	{
		strcpy(IP_addr,IP_PC104_1);	
		SERV_PORT = PC104_1_Rcv_PORT;
	}	
	
/// UDP server parameter initial		
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,IP_addr,&server_addr.sin_addr);
/// create socket		
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
/// send heart package
	heart  heart_package;
	memset(&heart_package,0,sizeof(heart_package));
	strncpy(heart_package.heart_head,head,strlen(head));
	heart_package.pc104_state=PC104_state;
	heart_package.slave_exe_result=Slave_exe_result;
	result_send = sendto(sockfd, &heart_package, sizeof(heart_package), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (result_send == -1)
	{
		WriteLog("UDP client send heart package error");
		close(sockfd);		
		return -1;
	}
    close(sockfd);	
    return 0;
}
int set_PC104_State(char *rcv_msg)
{
	char *heart_msg = rcv_msg;
	char *p;
	heart heart_pkg;
	p = strstr(heart_msg,head);
	if (p != NULL)
	{
		memcpy(&heart_pkg,p,sizeof(heart_pkg));
		printf(" recieve heart_pkg.pc104_state = %d\n",heart_pkg.pc104_state);
		if (heart_pkg.pc104_state == PC104_Master && PC104_state != PC104_Slave)
		{
			if (PC104_No == PC104_1)
			{
				PC104_state = PC104_Master;
				Slave_exe_result = heart_pkg.slave_exe_result;
			}
			else
				PC104_state = PC104_Slave;
		}
		else if (heart_pkg.pc104_state == PC104_Master && PC104_state == PC104_Slave)
		{
			PC104_state = PC104_Slave;
		}
		else if (heart_pkg.pc104_state == PC104_Single_Master)
		{
			if (PC104_No == PC104_1)
			{
				PC104_state = PC104_Master;
				Slave_exe_result = heart_pkg.slave_exe_result;
			}
			else
				PC104_state = PC104_Slave;
		}
		else if (heart_pkg.pc104_state == PC104_Slave && PC104_state != PC104_Master)
		{
			if (PC104_No == PC104_1)
			{
				PC104_state = PC104_Master;
				Slave_exe_result = heart_pkg.slave_exe_result;
			}
			else
				PC104_state = PC104_Slave;				
		}
		else
		{
			PC104_state = PC104_Master;
			Slave_exe_result = heart_pkg.slave_exe_result;
		}
	}
	else
	{
		printf("recieve message error!\n");
		return -1;
	}
	opposite_openport_state = heart_pkg.openport_state;
	opposite_modem_state = heart_pkg.modem_state;
	printf("------- PC104_state=%d		opposite OpenPort=%d    opposite modem=%d\n",PC104_state,opposite_openport_state,opposite_modem_state);
	if (PC104_state == PC104_Master)
		printf("------ PC104_Master: get Slave exe result(FINISH=1) = %d\n",Slave_exe_result);
	return 0;
}
int udp_recv_heart()
{
	int sockfd;
	struct sockaddr_in serveradrr,cliaddr;	
	fd_set rset;
	int maxfd;
	int result;
	int len;
	int length_rcv;
	char rcv_msg[MAXLENGTH]={0};
	char *p;
	int SERV_PORT;
	int rt_set_pc104;
	
	if (PC104_No == PC104_1)
		SERV_PORT = PC104_1_Rcv_PORT;
	else
		SERV_PORT = PC104_1_Send_PORT;	
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	bzero(&serveradrr,sizeof(serveradrr));
/// UDP server parameter initial		
	serveradrr.sin_family = AF_INET;
	serveradrr.sin_addr.s_addr = htons(INADDR_ANY);
	serveradrr.sin_port = htons(SERV_PORT);
/// bind UDP server		
	bind(sockfd,(struct sockaddr*)&serveradrr,sizeof(serveradrr));

	maxfd = sockfd+1;
	struct timeval wait_time;
	wait_time.tv_sec = 2;
	wait_time.tv_usec = 0;
	while(1)
	{
		FD_SET(sockfd,&rset);
		
		if ( (result = select(maxfd,&rset,NULL,NULL,&wait_time))<0 )
		{
			if (errno == EINTR)
				continue;
			else
			{
				WriteLog("UDP server recieve: select error!\n");
				printf("UDP server recieve: select error!\n");
			}
		}
/// socket  is readable
		if (FD_ISSET(sockfd, &rset))
		{
			len = sizeof(cliaddr);
			length_rcv = recvfrom(sockfd,rcv_msg,MAXLENGTH,0,(struct sockaddr*)&cliaddr,&len);
			if (length_rcv < 0)
			{
				printf("recv msg error, length <0\n");
				return -1;
			}
			else
			{
				rt_set_pc104 = set_PC104_State(rcv_msg);
				return rt_set_pc104;
			}
		}
		return -1;
	}
}
void CAN_send_heart()
{
	sleep(4);
	int rt;
	int fd_serial;
	fd_serial = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NDELAY);

	if(fd_serial == -1)
	{ 
		perror("open /dev/ttyS1 failed!");
		return ;
	}	
	fcntl(fd_serial,F_SETFL,0);			
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
	heart  heart_package;
	memset(&heart_package,0,sizeof(heart_package));
	strncpy(heart_package.heart_head,head,strlen(head));
	heart_package.pc104_state=PC104_state;
	heart_package.slave_exe_result=Slave_exe_result;
	heart_package.openport_state=OpenPort_state;
	heart_package.modem_state=Modem_state;
	write(fd_serial,&heart_package,sizeof(heart_package));                          //向串口写at命令
	tcsetattr(fd_serial,TCSANOW,&oldtio);
	close(fd_serial);  
}
void CAN_recv_heart()
{
	int rt;
	int fd_serial; 
	fd_serial = open("/dev/ttyS1",O_RDWR|O_NOCTTY|O_NDELAY);

	if(fd_serial <= 0) 
	{
		perror("open /dev/ttyS0 failed!");
		close(fd_serial); 
		return;
	}
	fcntl(fd_serial,F_SETFL,0);
	
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
	 char rcv_msg[MAXLENGTH]={0};
	 int len_recv;

	tcsetattr(fd_serial,TCSANOW,&oldtio);

	memset(rcv_msg,0,sizeof(rcv_msg));	 
// do not zuse wait for msg from opposite
	len_recv = read(fd_serial,rcv_msg,MAXLENGTH);                      
	if (len_recv < 0)
	{
		PC104_state = PC104_Single_Master;
	}
	else
	{
		printf("++++++++ CAN recieve OK,start process!\n");
		rt = set_PC104_State(rcv_msg);
		if (rt<0)
		{
			PC104_state = PC104_Single_Master;
			CAN_state = Error;
		}
		CAN_state = OK;
		printf("CAN ,process heart package end ,CAN_state=%d\n",CAN_state);
	}
	close(fd_serial); 
	printf("++++++++++ CAN: PC104_state=%d\n",PC104_state);	
	//sleep(1);
} 

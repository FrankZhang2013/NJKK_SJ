#include "SBD_Cmd.h"
	
// send msg to Irdm the file: location
int file_upload_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	upload_cmd upload;
	memcpy(&upload, str, sizeof(upload));
	
	char source_file[100] = {0};
	char source_file_name[100]={0};
	memcpy(source_file, (str+sizeof(upload)),(length-sizeof(upload)));
	
	FILE* fp;
	char msg_send[100] = {0};
	int result = 0;
	int len;
	
	char dest_file[100] = {0};
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = upload.CmdNo;
	
	fp = fopen(source_file, "r");
	if (NULL == fp)
	{
		msg_feedback.result = 'F';
		send_msg(msqid, (cmd_type+1), (char*)&msg_feedback,sizeof(msg_feedback));
		return 0;
	}
	
	if (upload.Priority != 'H'  || upload.Priority != 'M' || upload.Priority != 'L')
	{
		msg_feedback.result = 'F';
		send_msg(msqid, (cmd_type+1), (char*)&msg_feedback,sizeof(msg_feedback));
		return 0;
	}
	
	if (upload.mv_cp_flag < 0 || upload.mv_cp_flag > 1)
	{
		msg_feedback.result = 'F';
		send_msg(msqid, (cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));
		return 0;
	}
	
	getfilename(source_file, source_file_name);
	
	char mv_cp_cmd[120] = {0};
	if (upload.mv_cp_flag == 1)   // mv
	{
		sprintf(mv_cp_cmd, "mv -f %s %s%s%d_%d_%s", source_file, jtpz->dFTP, FTP_File_Header,cmd_type, upload.CmdNo, source_file_name);
	}
	else if (upload.mv_cp_flag == 0)  // cp
	{
		sprintf(mv_cp_cmd, "cp -f %s %s%s%d_%d_%s", source_file, jtpz->dFTP, FTP_File_Header,cmd_type, upload.CmdNo, source_file_name);		
	}
	
	result = system(mv_cp_cmd);
	if (result != 0)
	{
		WriteLog("execute copy/cut error\n");
		return -1;
	}
	
	sprintf(dest_file, "%c%s%s%d_%d_%s", upload.Priority, jtpz->dFTP, FTP_File_Header,cmd_type, upload.CmdNo, source_file_name);
	len = strlen(dest_file);
	send_msg(msqid, cmd_type, dest_file, len);
	return 0;	
}

// send msg to Irdm: cmdtype(1B)+cmdNo(2B)+detinination sourcefile(string)
int file_download_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	char source_dest_file[100] = {0};
	char souce[100] = {0};
	int len ;
	int i;
	char msg_send[120] = {0};
	unsigned short CmdNo;
	memcpy(&CmdNo, str, sizeof(CmdNo));
	
	int rt;	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;
	printf("CmdNo = %d\n",CmdNo);
	
	memcpy(source_dest_file, (str+sizeof(CmdNo)),(length-sizeof(CmdNo)));

	char *delim = " ";
	char *p;
	char *q;
	p = strtok(source_dest_file, delim);  // download to PC104 route
	q = strtok(NULL, delim);   // source file	
	if (NULL == p)
	{
		msg_feedback.result = 'F';
		send_msg(msqid, (cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));
		return 0;
	}

	DIR *dp;
	dp = opendir(p);
	if (!dp)
	{

		msg_feedback.result = 'F';
		send_msg(msqid, (cmd_type+1), (char*)&msg_feedback,sizeof(msg_feedback));
		return 0;
	}	
	
	char destination[100] = {0};	
	sprintf(destination,"%s %s",p,q);
	printf("destination = %s  :length =%d\n",destination,len);
	len = strlen(destination);
	
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");	
	printf("Cmd_Type = %d\n",cmd_type);
	printf("CmdNo =%d\n",CmdNo);
	printf("destination = %s\n",destination);	
	
	memcpy(  msg_send,&cmd_type, 1);	
	memcpy( (msg_send+1),&CmdNo, sizeof(CmdNo));
	memcpy( (msg_send+3), destination, len);
	len += 3;
	msg_send[len] = '\n';
	len += 1;
//	printf("before classify msg_send=%s\n",msg_send);
	 Read_dynamic_Config(jtpz,dtpz);
	 unsigned int nFTP  = dtpz->nFTP;
	 nFTP += 1;
	 
	 char download_dir[MAX_NAME_LEN] = {0};
	 len = sprintf(download_dir,"%s%s%d.ftp",jtpz->dFTP ,FTP_File_Header,nFTP );
	 rt = write_SBD(download_dir,nFTP ,msg_send,len);
	 if (rt<0)
		return -1;
	 send_msg(msqid,cmd_type, download_dir, len);
	 
	 char str_nFTP[LongInt_LEN] = {0};
	 sprintf(str_nFTP,"%d",nFTP );
	 Change_Config(dynamic_configfile_path,"nFTP",str_nFTP,jtpz,dtpz);
	 
	return 0;
}

void execute_cmd(const char *cmd, char *result,static_config *jtpz,dynamic_config *dtpz)
{
/// 存储一行内容
	char buf_ps[MAX_LINE_LENGTH];
	memset(buf_ps,0,sizeof(buf_ps));
///重定向命令	
	char cmd_redirect[MAX_CMD_LEN+20];   
	memset(cmd_redirect,0,sizeof(cmd_redirect));
///内容过大，存储的文件名	
	char result_filename[CMDFILENAME_LEN_MAX] = {0};
	char too_long_msg[50] = {0};
/// 存储超过1960时所有的内容	
	char over_result[max_SendSBD+MAX_LINE_LENGTH+10] = {0};
	
	char *p;
	char *delim = " ";
	strcpy(cmd_redirect, cmd); // keep the old cmdline	
	p = strtok((char *)cmd, delim);   // cmd type  ; ls / cp /mv
	if (NULL == p)
	{
		strcpy(result, "F");		
		return;	
	}
	
	Read_dynamic_Config(jtpz,dtpz);
	unsigned int nFTP = dtpz->nFTP+1;	
	char result_file[CMDFILENAME_LEN_MAX] = {0};
	sprintf(result_file,"%s%d.txt",jtpz->dCatlog,nFTP);
	
	char redirect[MAX_CMD_LEN];	
	memset(redirect,0,sizeof(redirect));
	
	sprintf(redirect,"  1>%s 2>%s",result_file,result_file);
	strcat(cmd_redirect,redirect);
	printf("Linux command = %s\n",cmd_redirect);
	
	int rt;	
	rt = system(cmd_redirect);
	if (rt != 0)
	{
		strcat(result, "F");
		printf("system cmd error\n");
		remove(result_file);
		return;
	}	
	
	FILE* fp;

	fp = fopen(result_file,"r");
	if (NULL == fp)
	{
		printf("open the command result file error!\n");
		strcat(result, "F");
		return;
	}
	int result_len = 0;
	while (fgets(buf_ps,MAX_LINE_LENGTH,fp) != NULL)
	{
		printf("command result = %s\n",buf_ps);
		strcat(over_result,buf_ps);
		result_len = strlen(over_result);
		if (result_len > MIN_CMD_RESULT_FTP)
		{
			fclose(fp);
			sprintf(too_long_msg,"Too long,see more detail in %s",result_file);			
			strcat(result,too_long_msg);
			return;
		}
	}
	fclose(fp);
	
	char *q;
	if (result_len == 0)
	{
		strcat(result, "S");	
		printf("comamnd result is empty\n");
	}
	else                            // have result
	{
		q = strstr(over_result, (const char*)p);
		if (q != NULL)
		{
			strcat(result, "F");
			printf("result have cmd type: is error\n");
		}
		else
		{
			strcat(result,over_result);
		}
	}
	remove(result_file);
	
	char str_nFTP[LongInt_LEN] = {0};
	sprintf(str_nFTP,"%d",nFTP);
	Change_Config(dynamic_configfile_path,"nFTP",str_nFTP,jtpz,dtpz);
	return;		
}

void search_direction_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	char result[max_SendSBD+50] = {0};
	char tmp_result[max_SendSBD] = {0};
	char direction_addr[100] = {0};
	int len = 0;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	memcpy(direction_addr,(str+sizeof(CmdNo)),(length-sizeof(CmdNo)));
/*	
	direction msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
*/	
	memcpy(result, &CmdNo, sizeof(CmdNo));
	DIR *dp;
	dp = opendir(direction_addr);
	if (!dp)
	{
//		msg_feedback.CmdNo = CmdNo;
//		strcpy(msg_feedback.str,"direction not exist");

		strcat(result,"F");
		send_msg(msqid, (cmd_type+1), result, strlen(result));
		return;
	}
	char cmd[100] = {0};
	sprintf(cmd,"ls -F %s",(str+sizeof(CmdNo)));

	execute_cmd(cmd, tmp_result,jtpz,dtpz);
	len = strlen(tmp_result);
	memcpy((result+sizeof(CmdNo)), tmp_result, len);	
	len+=sizeof(CmdNo);
	send_msg(msqid, (cmd_type+1), result, len);
}

void file_copy_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	memcpy(&CmdNo,str,sizeof(CmdNo));

	char result[10] ={0};
	char cmd[100] = {0};
	char src_dest[100]={0};
	memcpy(src_dest,(str+sizeof(CmdNo)),(length-sizeof(CmdNo)));
	sprintf(cmd,"cp %s",src_dest);
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;	
		
	execute_cmd(cmd, result,jtpz,dtpz);
	
	memcpy(&msg_feedback.result ,result, 1); 
	
	send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));	
}

void file_delete_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	char src[100]={0};
	memcpy(src,(str+sizeof(CmdNo)),(length-sizeof(CmdNo)));	
	char cmd[100] = {0};
	sprintf(cmd,"rm -f -r %s",src);
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;
	
	char result[100] = {0};
	execute_cmd(cmd, result,jtpz,dtpz);
	
	memcpy(&msg_feedback.result ,result, 1); 
	send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));	
}

void system_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	char result[max_SendSBD] = {0};
	char send_result[max_SendSBD] = {0};
	int len=0;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	char cmd[100]={0};
	memcpy(cmd,(str+sizeof(CmdNo)),(length-sizeof(CmdNo)));
	execute_cmd(cmd, result,jtpz,dtpz);

	memcpy(send_result, &CmdNo, sizeof(CmdNo));
	len = strlen(result);
	memcpy( (send_result+sizeof(CmdNo)), result,len);
	len+=sizeof(CmdNo);
	send_msg(msqid, (cmd_type+1), send_result, len);
}


void get_file_path(const char*filesource, char path[100])
{
	char delim = '/';
	char *p = strrchr(filesource, delim);
	memcpy(path, filesource, (strlen(filesource)-strlen(p)+1));
}

void file_compress_cmd(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	char file_name[100]={0};
	char file[100] = {0};
	strcpy(file, (str+sizeof(CmdNo)));
	getfilename(file,file_name);
	
	char cmd[100] = {0};
	sprintf(cmd,"tar czf %s.tar.gz %s",file_name,file_name);//(str+sizeof(unsigned short))
	
	printf("+++++++++++++++ compress cmd = %s\n",cmd);
		
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;
	
	char path[100] = {0};
	char current_path[100] = {0};
	DIR *dp;
	get_file_path(file, path);

	getcwd(current_path, 100);
	dp = opendir(path);
	if (!dp)
	{
		msg_feedback.result = 'F';
		send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));
	}
	chdir(path);

	char result[10] = {0};
	execute_cmd(cmd, result,jtpz,dtpz);
	chdir(current_path);		
	//int len = strlen(result);
	memcpy(&msg_feedback.result ,result, 1); 
	if (msg_feedback.result == 'S')
	{
		remove(file);
	}	
	send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));		

}

void file_decompress_cmd(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	char file[100] = {0};
	strcpy(file, (str+sizeof(CmdNo)));
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;
	
	char file_name[100]={0};
	getfilename(file,file_name);
	
	char cmd[100] = {0};
	sprintf(cmd,"tar xzf %s",file_name);
	
	char path[100] = {0};
	char current_path[100] = {0};
	DIR *dp;
	get_file_path(file, path);

	getcwd(current_path, 100);
	dp = opendir(path);
	if (!dp)
	{
		msg_feedback.result = 'F'; 
		send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));
	}
	chdir(path);	
	
	char result[10] = {0};
	execute_cmd(cmd, result,jtpz,dtpz);
	chdir(current_path);		
	//int len = strlen(result);
	memcpy(&msg_feedback.result ,result, 1); 
	if (msg_feedback.result == 'S')
	{
		remove(file);
	}
	send_msg(msqid,(cmd_type+1), (char*)&msg_feedback, sizeof(msg_feedback));	
}
void check_software_version(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	char result[max_SendSBD] = {0};
	char tmp_result[max_SendSBD] = {0};
	char direction_addr[100] = {0};
	int len;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	memcpy(result, &CmdNo, sizeof(CmdNo));

	char cmd[100] = {0};
	sprintf(cmd,"ls  %s",jtpz->dUpdate);

	execute_cmd(cmd, tmp_result,jtpz,dtpz);
	len = strlen(tmp_result);
	memcpy( (result+sizeof(CmdNo)), tmp_result, len);	
	len+=sizeof(CmdNo);
	send_msg(msqid, (cmd_type+1), result, len);
}

void check_backup(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	unsigned short CmdNo;
	char result[max_SendSBD] = {0};
	char tmp_result[max_SendSBD] = {0};
	char direction_addr[100] = {0};
	int len;
	memcpy(&CmdNo,str,sizeof(CmdNo));
	memcpy(result, &CmdNo, sizeof(CmdNo));

	char cmd[100] = {0};
	sprintf(cmd,"ls  %s",jtpz->dBackup);

	execute_cmd(cmd, tmp_result,jtpz,dtpz);
	len = strlen(tmp_result);
	memcpy( (result+sizeof(CmdNo)), tmp_result, len);	
	len+=sizeof(CmdNo);
	send_msg(msqid, (cmd_type+1), result, len);
}
int update_software(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz)
{
	char source_file[PATHLENGTH] = {0};
	char souce[PATHLENGTH] = {0};
	int len;
	int i;
	char msg_send[PATHLENGTH] = {0};
	unsigned short CmdNo;
	memcpy(&CmdNo, str, sizeof(CmdNo));
	int rt;
	char destination[PATHLENGTH]={0};
	
	execute_result msg_feedback;
	memset(&msg_feedback, 0, sizeof(msg_feedback));
	msg_feedback.CmdNo = CmdNo;
	printf("CmdNo = %d\n",CmdNo);
	strcpy(source_file, (str+sizeof(CmdNo)));	
	len = sprintf(destination,"%s %s",jtpz->dUpdate,source_file);
	
	memcpy(  msg_send,&cmd_type, sizeof(cmd_type));	
	memcpy( (msg_send+sizeof(cmd_type)),&CmdNo, sizeof(CmdNo));
	memcpy( (msg_send+sizeof(cmd_type)+sizeof(CmdNo)), destination, len);

	 Read_dynamic_Config(jtpz,dtpz);
	 unsigned int nFTP  = dtpz->nFTP;
	 nFTP += 1;
	 
	 char download_dir[MAX_NAME_LEN] = {0};
	 len = sprintf(download_dir,"%s%s%d.ftp",jtpz->dFTP ,FTP_File_Header,nFTP );
	 rt = write_SBD(download_dir,nFTP ,msg_send,len);
	 if (rt<0)
		return -1;	 
	 char str_nFTP[LongInt_LEN] = {0};
	 sprintf(str_nFTP,"%d",nFTP );
	 Change_Config(dynamic_configfile_path,"nFTP",str_nFTP,jtpz,dtpz);
	 send_msg(msqid,cmd_type, download_dir, len);	 
	return 0;
}

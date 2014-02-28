#include "SBD_FTP_Classify.h"

int write_SBD(char *fileaddr,int nSBD,char *str,int len)  
{	
	FILE *fp;
	int tmp_len=0;
	if ((fp=fopen(fileaddr,"wb+"))==NULL)
	{
		printf("open the file fail!\n");
		return -1;
	}
	else
	{
	    tmp_len =  fwrite(str,sizeof(char),len,fp);
	    if (tmp_len != len)
	    {
			printf("write length is not equal with length given\n");
			return -1;
		}
	    fflush(fp);	
		fsync(fileno(fp));	
		fclose(fp);
    }
    return 0;
}	


int SBD_Classify(char pri,int feedbak_msg_type,char *str,int len,static_config *jtpz,dynamic_config *dtpz)
{
	    FILE *Fd_SBD_List;

		 char sbd_buf[MSG_BUF_SIZE+10] = {0};
		 char fileaddr[PATHLENGTH]={0};
		 char del_fileaddr[PATHLENGTH] = {0};
		 char file_list[PATHLENGTH] = {0};
		 int rt;
/// read nSBD	
		 Read_dynamic_Config(jtpz,dtpz);
		 unsigned int tmp_nSBD = dtpz->nSBD;
	 
		 int len_sbd_bud = 0;
		 int len_fileaddr = 0;
////////////////////////unsigned msg_type = feedbak_msg_type-1;//////////////////////////////////////////////////////
		 unsigned char msg_type = feedbak_msg_type;
		 memcpy(sbd_buf,&msg_type,sizeof(msg_type));
		 memcpy( (sbd_buf+sizeof(msg_type)),str,len);
		 len_sbd_bud = len + 1;
		 		  	 
		if (pri == 'H')
		{
  			 if( (HSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&HSBD_Occupy_Sign) == 0) )
   			 {
				HSBD_Occupy_Sign = Occupied;
				sprintf(file_list,"%sHSBD_List.dat",jtpz->dFilelist);
				Fd_SBD_List=fopen(file_list, "r");
				if (NULL == Fd_SBD_List)
				{
					Fd_SBD_List=fopen(file_list, "w");
				}
				fclose(Fd_SBD_List);
				
				tmp_nSBD += 1;
				len_fileaddr = sprintf(fileaddr,"%s%s%d.sbd",jtpz->dSBD,SBD_File_Header,tmp_nSBD);
				rt =  write_SBD(fileaddr,tmp_nSBD,sbd_buf,len_sbd_bud);	
				if (rt<0)
					return -1;
				 printf("----------------- write sbd file sucessfully   ==== %s\n",fileaddr);
				 
				 fileaddr[len_fileaddr] = '\n';	
				 
				 char str_nSBD[LongInt_LEN] = {0};
				 sprintf(str_nSBD,"%d",tmp_nSBD);
				 printf("############################ str_nSBD = %s\n",str_nSBD);
				
				Fd_SBD_List=fopen(file_list, "a");	
							
				fputs(fileaddr,Fd_SBD_List);
				fflush(Fd_SBD_List);	
				fsync(fileno(Fd_SBD_List));
				fclose(Fd_SBD_List);
				
				HSBD_Num++;									
				Change_Config(dynamic_configfile_path,"nSBD",str_nSBD,jtpz,dtpz);
				HSBD_Occupy_Sign = NotOccupied;					
													
				WriteLog("classify HSBD sucessfully\n");						
				return 0;
			  }
		}
		else if (pri == 'M')
		{
  			 if( (MSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&MSBD_Occupy_Sign) == 0) )
   			 {		  
				MSBD_Occupy_Sign = Occupied;
				sprintf(file_list,"%sMSBD_List.dat",jtpz->dFilelist);
				Fd_SBD_List = fopen(file_list, "r");
				if (NULL == Fd_SBD_List)
				{	
					Fd_SBD_List=fopen(file_list, "w");
				}
				fclose(Fd_SBD_List);
				
				tmp_nSBD += 1;
				 len_fileaddr = sprintf(fileaddr,"%s%s%d.sbd",jtpz->dSBD,SBD_File_Header,tmp_nSBD);
				 rt = write_SBD(fileaddr,tmp_nSBD,sbd_buf,len_sbd_bud);
				if (rt<0)
					return -1;
				 printf("--------------------------- write sbd file sucessfully   ==== %s\n",fileaddr);
				 
				 fileaddr[len_fileaddr] = '\n';	
				 
				 char str_nSBD[LongInt_LEN] = {0};
				 sprintf(str_nSBD,"%d",tmp_nSBD);
				  printf("############################ str_nSBD = %s\n",str_nSBD);
							
				Fd_SBD_List=fopen(file_list, "a");	
							
				fputs(fileaddr,Fd_SBD_List);
				fflush(Fd_SBD_List);	
				fsync(fileno(Fd_SBD_List));
				fclose(Fd_SBD_List);

				MSBD_Num++;		
				Change_Config(dynamic_configfile_path,"nSBD",str_nSBD,jtpz,dtpz);				
				MSBD_Occupy_Sign = NotOccupied;	
								
				WriteLog("classify MSBD sucessfully\n");
				return 0;
			  }
		}
		else if (pri == 'L')
		{
  			 if( (LSBD_Occupy_Sign == NotOccupied)&&(Global_OpLock(&SBD_mutex,&LSBD_Occupy_Sign) == 0) )
   			 {
				LSBD_Occupy_Sign = Occupied;
				sprintf(file_list,"%sLSBD_List.dat",jtpz->dFilelist);
				Fd_SBD_List=fopen(file_list, "r");
				if (NULL == Fd_SBD_List)
				{	
					Fd_SBD_List=fopen(file_list, "w");
				}
				fclose(Fd_SBD_List);
				
				tmp_nSBD += 1;
				 len_fileaddr = sprintf(fileaddr,"%s%s%d.sbd",jtpz->dSBD,SBD_File_Header,tmp_nSBD);
				 rt = write_SBD(fileaddr,tmp_nSBD,sbd_buf,len_sbd_bud);	 
				 if (rt<0)
					return -1;
				 printf("--------------------------- write sbd file sucessfully   ==== %s\n",fileaddr);
				 
				 fileaddr[len_fileaddr] = '\n';	
				 
				 char str_nSBD[LongInt_LEN] = {0};
				 sprintf(str_nSBD,"%d",tmp_nSBD);
				  printf("############################ str_nSBD = %s\n",str_nSBD);
							
				Fd_SBD_List=fopen(file_list, "a");	
						
				fputs(fileaddr,Fd_SBD_List);
				fflush(Fd_SBD_List);	
				fsync(fileno(Fd_SBD_List));
				fclose(Fd_SBD_List);
				
				LSBD_Num++;
				Change_Config(dynamic_configfile_path,"nSBD",str_nSBD,jtpz,dtpz);				
				LSBD_Occupy_Sign = NotOccupied;	
																
				WriteLog("classify LSBD sucessfully\n");
				return 0;
			 }
		}
		return -1;
}

int FTP_Classify(int msqid,char pri,int feedback_msg_type,char *str,int len,static_config *jtpz,dynamic_config *dtpz)
{
	   FILE *Fd_HFTP_List, *Fd_MFTP_List, *Fd_LFTP_List;
	   char file_list[PATHLENGTH] ={0};
	   char file_addr[PATHLENGTH];
	   memset(file_addr,0,sizeof(file_addr));
	   char *str_pri;
	   int M2S_flag = 0;
	   if (len > PATHLENGTH)
			return 0;	
		
		int length = strlen(str);	
		memcpy(file_addr,str,length);
		length = strlen(file_addr);
		file_addr[length] = '\n';
		printf("length=%d	 file_addr -->str= %s",length,file_addr);
		
		if (pri == 'H')
			str_pri = "H";
		else if (pri == 'M')
			str_pri = "M";
		else
			str_pri = "L";
							
		sprintf(file_list,"%s%sFTP_List.dat",jtpz->dFilelist,str);				
		if (pri == 'H')
		{
  			 if( (HFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&HFTP_Occupy_Sign) == 0) )
   			  {	 
			 
				HFTP_Occupy_Sign = Occupied;
				Fd_HFTP_List = fopen(file_list, "r");
				if (NULL == Fd_HFTP_List)
				{
					Fd_HFTP_List=fopen(file_list, "w");
				}
				fclose(Fd_HFTP_List);

				Fd_HFTP_List = fopen(file_list, "a");				
				fputs(file_addr,Fd_HFTP_List);
				fflush(Fd_HFTP_List);	
				fsync(fileno(Fd_HFTP_List));
				fclose(Fd_HFTP_List);

				if (M2S_flag == 0)	
					HFTP_Num++;
				HFTP_Occupy_Sign = NotOccupied;
				 
				WriteLog("classify HFTP sucessfully\n");
				return 0;
			  }
		}
		else if (pri == 'M')
		{
  			 if( (MFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&MFTP_Occupy_Sign)==0) )
   			  {
				MFTP_Occupy_Sign = Occupied;			   

				Fd_MFTP_List = fopen(file_list, "r");
				if (NULL == Fd_MFTP_List)	
				{
					Fd_MFTP_List = fopen(file_list, "w");
				}
				fclose(Fd_MFTP_List);
			
				Fd_MFTP_List = fopen(file_list, "a");				
				fputs(file_addr,Fd_MFTP_List);
				fflush(Fd_MFTP_List);	
				fsync(fileno(Fd_MFTP_List));				
				fclose(Fd_MFTP_List);

				if (M2S_flag == 0)	
					MFTP_Num++;
				MFTP_Occupy_Sign = NotOccupied;
				
				WriteLog("classify MFTP sucessfully\n");
				return 0;
			}
		}
		else if (pri == 'L')
		{
  			 if( (LFTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&FTP_mutex,&LFTP_Occupy_Sign)==0) )
   			  {	   
				LFTP_Occupy_Sign = Occupied;
				Fd_LFTP_List=fopen(file_list, "r");
				if (NULL == Fd_LFTP_List)	
				{
					Fd_LFTP_List=fopen(file_list, "w");
				}
				fclose(Fd_LFTP_List);
		
				Fd_LFTP_List=fopen(file_list, "a");				
				fputs(file_addr,Fd_LFTP_List);
				fflush(Fd_LFTP_List);	
				fsync(fileno(Fd_LFTP_List));					
				fclose(Fd_LFTP_List);
				
				if (M2S_flag == 0)				
					LFTP_Num++;
				LFTP_Occupy_Sign = NotOccupied;
				 
				WriteLog("classify LFTP sucessfully\n");
				return 0;
			}
		}
		return -1;
}

int DL_FTP_Classify(char *str,int len,static_config *jtpz,dynamic_config *dtpz)
{
	FILE* fd_DL_FTP_List;
	char file_list[PATHLENGTH] = {0};
	char file_addr[PATHLENGTH] = {0};
	if (len > PATHLENGTH)
		return 0;		
	int length = strlen(str);	
	int M2S_flag=0;
	memcpy(file_addr,str,length);
	length = strlen(file_addr);
	file_addr[length] = '\n';
	printf("length=%d	 file_addr -->str= %s",length,file_addr);
	
	sprintf(file_list,"%sDL_FTP_List.dat",jtpz->dFilelist);
	
	if( (DL_FTP_Occupy_Sign == NotOccupied)&&(Global_OpLock(&DL_FTP_mutex,&DL_FTP_Occupy_Sign)==0) )
   	{
		DL_FTP_Occupy_Sign = Occupied;

		fd_DL_FTP_List=fopen(file_list, "r");
		if (fd_DL_FTP_List == NULL)	
			fd_DL_FTP_List = fopen(file_list, "w");
		fclose(fd_DL_FTP_List);
		
		fd_DL_FTP_List=fopen(file_list, "a");	
		fputs(file_addr,fd_DL_FTP_List);
		fflush(fd_DL_FTP_List);	
		fsync(fileno(fd_DL_FTP_List));		
		fclose(fd_DL_FTP_List);
		
		if (M2S_flag == 0)		
			DL_FTP_Num++;	
		DL_FTP_Occupy_Sign = NotOccupied;
	
		WriteLog("classify DLFTP sucessfully\n");
		return 0;
	}
	return -1;
}

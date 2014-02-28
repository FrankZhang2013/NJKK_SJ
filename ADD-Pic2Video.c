#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

int FFmpegCmd(char *VideoName)
{
	if (NULL == VideoName)
	{
		return -1;
	}
	char Cmd[100] ="ffmpeg -f image2 -i %d.jpg  ";
	strncat(Cmd,VideoName,strlen(VideoName));
	
	if (system(Cmd) != 0)
	{
		return -1;
	}
	return 0;
}

int FFmpeg2Video(const char* PicturesDir)
{
	if (NULL == PicturesDir)
	{
		return -1;
	}
	if (access(PicturesDir,0) != 0)
	{
		return -1;
	}
	char old_dirent[256] = {0};
	if (getcwd(old_dirent,256) == NULL)
	{
		return -1;
	}
	if (chdir(PicturesDir) == -1)
	{
		return -1;
	}
	if (FFmpegCmd("Video.mpg") == -1)
	{
		return -1;
	}
	chdir(old_dirent);
	return 0;
}

int PreProcess(char* PicList,const char* PicturesDir)
{
	if ( (NULL == PicList) || (NULL == PicturesDir) )
	{
		return -1;
	}
	if (access(PicList,0) != 0 || access(PicturesDir,0) != 0)
	{
		return -1;
	}
	FILE* fp;
	char file_path[100] = {0};
	int pic_count = 1;
	char cmd[256] = {0};
	fp = fopen(PicList,"r");
	if (NULL == fp)
	{
		return -1;
	}
	while (fgets(file_path,100,fp) != NULL)
	{
		if (file_path[strlen(file_path)-1] == '\n')
		{
			file_path[strlen(file_path)-1] = '\0';
		}		
		if (access(file_path,0) != 0)
		{
			memset(file_path,0,sizeof(file_path));
			continue;
		}		

		snprintf(cmd,256,"cp %s %s%d.jpg",file_path,PicturesDir,pic_count++);
		system(cmd);
		memset(cmd,'\0',sizeof(cmd));
		memset(file_path,0,sizeof(file_path));
	}
	fclose(fp);
	return 0;
}
int ClearList(char* PicList,const char* PicturesDir)
{
	char delete_pics_cmd[100] = {0};
	if (NULL == PicList)
	{
		return -1;
	}
	else
	{
		remove(PicList);
		snprintf(delete_pics_cmd,100,"rm %s*.jpg",PicturesDir);
		system(delete_pics_cmd);
		return 0;
	}
}
int main()
{
	PreProcess("/home/cc/addNewFun/list.txt","/home/cc/addNewFun/Picture/");
	FFmpeg2Video("/home/cc/addNewFun/Picture/");
	ClearList("/home/cc/addNewFun/list.txt","/home/cc/addNewFun/Picture/");
	return 0;
}

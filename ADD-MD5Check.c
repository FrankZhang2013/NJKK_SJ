#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int MD5sumCmd(const char* fileName,char* result)
{
	FILE* pPipe = NULL;
	char cmd[100]={0};
	char RstBuff[100]={0};
	int offset = 0;	
	char* tmp = result;
	
	snprintf(cmd,100,"md5sum -t %s",fileName);
	pPipe = popen(cmd,"r");
	if (NULL == pPipe)
	{
		return -1;
	}
	while (fgets(RstBuff,100,pPipe) != NULL)
	{
		strncat(tmp,RstBuff,strlen(RstBuff));
		offset = strlen(RstBuff);
		tmp += offset;
		memset(RstBuff,0,100);
	}
	return 0;
}
int MD5Check(const char* fileName)
{
	if (NULL == fileName)
	{
		return -1;
	}
	if (access(fileName,0) != 0)
	{
		return -1;
	}
	char MD5Value[100] = {0};
	char *NameMd5 ;
	NameMd5 = strrchr(fileName,'_');
	if (NULL == NameMd5)
	{
		return -1;
	}
	NameMd5++;
	
	if (MD5sumCmd(fileName,MD5Value) == 0)
	{
		if (strncmp(NameMd5,MD5Value,32) == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}
void Testcase(const char* fileName)
{
	if (MD5Check(fileName) == 0)
	{
		printf("file:%s MD5 value is right!\n",fileName);
	}
	else
	{
		printf("file:%s MD5 value is wrong!\n",fileName);
	}
}

int FileMD5Add(const char* fileName)
{
	if (NULL == fileName)
	{
		return -1;
	}
	if (access(fileName,0) != 0)
	{
		return -1;
	}
	char MD5Value[100] = {0};
	char FileNewName[100] = {0};
	snprintf(FileNewName,100,"%s_",fileName);
	if (MD5sumCmd(fileName,MD5Value) == 0)
	{
		strncat(FileNewName+strlen(fileName)+1,MD5Value,32);
		rename(fileName,FileNewName);
		
		/*
		 * test case: check MD5 value
		 */
		Testcase((const char*)FileNewName);
		
		return 0;
	}
	else
	{
		return -1;
	}
}

int main()
{
	FileMD5Add("test.txt");

	return 0;
}

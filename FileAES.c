#include "FileAES.h"

int key[32] = {0x1,0x2,0x3,0x4d,0x56,0x10,0x11,0x12,
				0x1,0x2,0x3,0x4d,0x56,0x10,0x11,0x12,
				0x1,0x2,0x3,0x4d,0x56,0x10,0x11,0x12,
				0x1,0x2,0x3,0x4d,0x56,0x10,0x11,0x12};


int encry_file(char* file,char* encry_file_name)
{
	if (NULL == file)
	{
		return -1;
	}
	FILE* fp;
    fp = fopen(file,"r");
    if (NULL == fp)
    {
		return -1;
	}
	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
    int total_size = len;
    unsigned char *encrypt = NULL ;
    unsigned char decrypt[16+1] = {0};
    struct crypto_aes_ctx ctx;
    crypto_aes_expand_key(&ctx,(u8 *)key,32);
    if (len % 16 != 0)
    {
		total_size = (len/16 + 1)*16;
	}
 	encrypt = (char*)malloc(total_size);   
 	rewind(fp);
 	fread(encrypt,len,1,fp);
 	int i;
 	for (i=len; i<total_size; i++)
 	{
		encrypt[i] = 0;
	}
    fclose(fp);
    
    fp = fopen("2.txt","w");
    for (i=0; i<total_size; i += 16)
    {
		memset(decrypt,'\0',sizeof(decrypt));
		aes_encrypt(&ctx,decrypt,(char*)(encrypt+i));
	//	printf("加密前:%s\n加密后:",encrypt+i);
		fwrite(decrypt,16,1,fp);
	}
    fclose(fp);
    rename("2.txt",encry_file_name);
    return 0;
}

int decry_file(char* file,char* decry_file_name)
{
	if (NULL == file)
	{
		return -1;
	}
	
	unsigned char *decrypt_tmp = NULL ;
    unsigned char decrypt2[16+1] = {0}; 
    
    struct crypto_aes_ctx ctx;
    FILE* fp = NULL;
	crypto_aes_expand_key(&ctx,(u8 *)key,32);
	fp = fopen(file,"r");
    fseek(fp,0,SEEK_END);
    int len = ftell(fp);
    if (len % 16 != 0)
    {
		return -1;
	}
 	decrypt_tmp = (char*)malloc(len); 
 	rewind(fp);
 	fread(decrypt_tmp,len,1,fp);
 	fclose(fp);
 	
 	fp = fopen("3.txt","w");
 	int i;
    for (i=0; i<len; i += 16)
    {
		memset(decrypt2,'\0',sizeof(decrypt2));
		aes_decrypt(&ctx,decrypt2,(char*)(decrypt_tmp+i));
	//	printf("加密前:%s\n加密后:",encrypt+i);
		fwrite(decrypt2,16,1,fp);
	}	
    fclose(fp);
    rename("3.txt",decry_file_name);
}

int main()
{
    encry_file("1.txt","1.txt");
    
	decry_file("1.txt","1.txt");
	
	FILE* fp = NULL;
	
    fp = fopen("1.txt","r");
    fseek(fp,0,SEEK_END);
   int len = ftell(fp);
   unsigned char* decrypt_tmp;
 	decrypt_tmp = (char*)malloc(len); 
 	memset(decrypt_tmp,'\0',len);
 	rewind(fp);
 	fread(decrypt_tmp,len,1,fp);
 	fclose(fp); 
    printf("%d\n",strlen(decrypt_tmp));

#if 0 
    int i;
    for(i=0;i<16;i++)
    {
        printf("%02x",decrypt[i]);
    }
    printf("\n");
//
    aes_decrypt(&ctx,decrypt2,decrypt);
    printf("解密前:");
    for(i=0;i<16;i++)
    {
        printf("%02x",decrypt[i]);
    }
    printf("\n");
    printf("解密后:%s\n",decrypt2);
 
    printf("\n");
 #endif
    return 0;
	
}

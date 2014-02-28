#include "njkk_interface.h"
#include "Irdm_Interface.h"

int file_upload_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz);

int file_download_cmd(char *str, unsigned char cmd_type, int length,int msqid, static_config *jtpz,dynamic_config *dtpz);

void execute_cmd(const char *cmd, char *result,static_config *jtpz,dynamic_config *dtpz);

void search_direction_cmd(char *str, unsigned char cmd_type, int length, int msqid, static_config *jtpz,dynamic_config *dtpz);

void file_copy_cmd(char *str, unsigned char cmd_type, int length, int msqid, static_config *jtpz,dynamic_config *dtpz);

void file_delete_cmd(char *str, unsigned char cmd_type, int length, int msqid, static_config *jtpz,dynamic_config *dtpz);

void system_cmd(char *str, unsigned char cmd_type, int length, int msqid, static_config *jtpz,dynamic_config *dtpz);

void get_file_path(const char*filesource, char path[100]);

void file_compress_cmd(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz);

void file_decompress_cmd(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz);

void check_software_version(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz);
void check_backup(char *str, unsigned char cmd_type, int msqid, static_config *jtpz,dynamic_config *dtpz);
int update_software(char *str, unsigned char cmd_type, int length, int msqid, static_config *jtpz,dynamic_config *dtpz);

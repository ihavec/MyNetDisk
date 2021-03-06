#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <sys/sendfile.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/mman.h>


typedef struct{
	union tmp1{
		int cmd_type;
		int ret_type;
	}type;
	int uid;//用户ID
	union tmp2{
		char filename[128];//文件名
		char param[128];
		char user_name[128];//用户名
	}u1;//u1联合体
	union tmp3{
		char dir[128];
		char passwd_encry[128];
	}u2;//u2联合体
}message_t;

typedef struct{
	char type;
	size_t size;
	char filename[64];
}file_info_t;

void my_log(int uid, char *command, char *param);//插入一行到Ftplog表
int clac_md5(char *, char *);//计算md5
void dupli_remove(char *, char c);//去重
char *get_next(char *, char *, char);//获得在字符c之前的下一个字符串(开头有c先去掉)
void msg_print(message_t *);//打印信息
void get_rand_str(char *, int);//生成长度为num的随机字符串
void strip(char *);//删除字符串头尾的空格
void split(const char *, const char *, char **, int*);//字符串s按照separation里的字符分割,存储在store中
int find(const char *, const char);//在字符串s中查找字符c,找到返回1,否则返回0

#endif

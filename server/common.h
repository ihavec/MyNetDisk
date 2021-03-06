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
	int uid;//�û�ID
	union tmp2{
		char filename[128];//�ļ���
		char param[128];
		char user_name[128];//�û���
	}u1;//u1������
	union tmp3{
		char dir[128];
		char passwd_encry[128];
	}u2;//u2������
}message_t;

typedef struct{
	char type;
	size_t size;
	char filename[64];
}file_info_t;

void my_log(int uid, char *command, char *param);//����һ�е�Ftplog��
int clac_md5(char *, char *);//����md5
void dupli_remove(char *, char c);//ȥ��
char *get_next(char *, char *, char);//������ַ�c֮ǰ����һ���ַ���(��ͷ��c��ȥ��)
void msg_print(message_t *);//��ӡ��Ϣ
void get_rand_str(char *, int);//���ɳ���Ϊnum������ַ���
void strip(char *);//ɾ���ַ���ͷβ�Ŀո�
void split(const char *, const char *, char **, int*);//�ַ���s����separation����ַ��ָ�,�洢��store��
int find(const char *, const char);//���ַ���s�в����ַ�c,�ҵ�����1,���򷵻�0

#endif

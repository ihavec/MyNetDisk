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
#include <openssl/md5.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/mman.h>


typedef struct{
	union tmp1{
		int cmd_type;
		int ret_type;
	}type;
	int uid;
	union tmp2{
		char filename[128];
		char param[128];
		char user_name[128];
	}u1;
	union tmp3{
		char dir[128];
		char passwd_encry[128];
	}u2;
}message_t;

typedef struct{
	char type;
	size_t size;
	char filename[64];
}file_info_t;


int clac_md5(char *, char *);
void msg_print(message_t *);
void get_rand_str(char *, int);
void strip(char *);
void split(const char *, const char *, char **, int*);
int find(const char *, const char);


#endif

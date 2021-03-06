#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "common.h"

typedef struct{
	char signin_flag;
	int uid;
	char user_name[64];
//	char passwd_encry[100];
	char dir[128];
}user_info_t;

typedef struct{
	char addr[16];
	char port[8];
}config_t;


typedef struct{
	int sfd;
	struct sockaddr_in ser;
	user_info_t info;
}factory_t, *pfactory_t;

typedef struct{
	factory_t fac;
	message_t msg;
}pthread_stru;

void get_pthread_stru(pthread_stru**, pfactory_t, message_t*);
void *pthread_send_recv(void *);
void get_config(config_t *, const char *);
void factory_init(pfactory_t, config_t *);
int message_handle(message_t *, char *);
void command_handle(pfactory_t, message_t *);

#endif

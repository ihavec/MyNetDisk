#ifndef __PASSWD_H__
#define __PASSWD_H__

#include "work_que.h"
#include "mysql_operate.h"
#include "crypt.h"

int sign_up(pNode );//ע��
int sign_in(pNode );//��¼
int query_by_username(char *, char **);//��Userinfo����username����,�ɹ�����0(���ݿ��д��ڴ��û�)
int add_user(message_t *);//�����û���Userinfo
void get_salt(char *,char *);//��ȡsaltֵ
void disconnect(int );//�Ͽ�����,ɾ���û�������

#endif

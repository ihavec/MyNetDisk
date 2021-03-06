#ifndef __PASSWD_H__
#define __PASSWD_H__

#include "work_que.h"
#include "mysql_operate.h"
#include "crypt.h"

int sign_up(pNode );//注册
int sign_in(pNode );//登录
int query_by_username(char *, char **);//在Userinfo中用username查找,成功返回0(数据库中存在此用户)
int add_user(message_t *);//增加用户到Userinfo
void get_salt(char *,char *);//获取salt值
void disconnect(int );//断开连接,删除用户链表结点

#endif

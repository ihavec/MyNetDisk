#ifndef __WORK_LIST_H__
#define __WORK_LIST_H__

#include "common.h"

typedef struct lnode{
	int sockfd;
	struct lnode *pNext;
}task_t, *ptask_t;//任务结点

typedef struct{
	ptask_t head, tail;
}task_list_t, *ptask_list_t;//任务链表

//每个连接用户的信息
typedef struct node{
	char signin;//登录状态
	int sockfd;
	int send_recv_fd;
	int uid;//ID
	struct node *pNext;//下个结点
}user_t;

//连接用户列表
typedef struct{
	user_t *head, *tail;//首尾
	int conn_user_num;//连接的用户数
	int signin_user_num;//登录的用户数
	pthread_mutex_t mutex;//锁
}user_list;//用户链表

void user_list_init(user_list *);//初始化用户链表
void user_signin(user_list *, int, int );//用户登录
void user_signout(user_list *, int );
void user_list_insert(user_list *, user_t *);//插入链表
void user_list_delete(user_list *, int, user_t **);//删除
int user_list_serch_byuid(user_list *, int, user_t **);//用UID找结点的前一个结点
int user_list_serch_byfd(user_list *, int, user_t **);//用fd找结点的前一个结点

void task_list_init(ptask_list_t);
void task_list_insert(ptask_list_t, ptask_t);
void task_list_delete(ptask_list_t, int);
int task_list_serch_byfd(ptask_list_t, int, ptask_t *);
void task_list_destory(ptask_list_t);
void update(ptask_list_t *, int);

#endif

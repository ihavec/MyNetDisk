#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__

#include "common.h"

typedef struct Node{
	int new_fd;//套接口描述符
	struct Node *pNext;//下一个结点
	message_t msg;//message_t结构体
}Node_t, *pNode;//结点

typedef struct{
	pNode head, tail;//队头,尾
	int size;//队长
	int capacity;//队列能力
	pthread_mutex_t mutex;//队列锁
}trantask_que_t;//长传输队列结构体

typedef struct{
	pNode head, tail;
	int size;
	int capacity;
	pthread_mutex_t mutex;
}othertask_que_t;//短命令队列结构体

void other_que_init(othertask_que_t *, int);
void tran_que_init(trantask_que_t *, int);//初始化
int empty_other(othertask_que_t *);
int empty_tran(trantask_que_t *);//判空
int insert_othertask(othertask_que_t *, pNode);
int insert_trantask(trantask_que_t *, pNode);//入队
int get_othertask(othertask_que_t *, pNode*);
int get_trantask(trantask_que_t *, pNode*);//出队 队列指针 | 结点指针(传出参数)

#endif

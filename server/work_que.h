#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__

#include "common.h"

typedef struct Node{
	int new_fd;//�׽ӿ�������
	struct Node *pNext;//��һ�����
	message_t msg;//message_t�ṹ��
}Node_t, *pNode;//���

typedef struct{
	pNode head, tail;//��ͷ,β
	int size;//�ӳ�
	int capacity;//��������
	pthread_mutex_t mutex;//������
}trantask_que_t;//��������нṹ��

typedef struct{
	pNode head, tail;
	int size;
	int capacity;
	pthread_mutex_t mutex;
}othertask_que_t;//��������нṹ��

void other_que_init(othertask_que_t *, int);
void tran_que_init(trantask_que_t *, int);//��ʼ��
int empty_other(othertask_que_t *);
int empty_tran(trantask_que_t *);//�п�
int insert_othertask(othertask_que_t *, pNode);
int insert_trantask(trantask_que_t *, pNode);//���
int get_othertask(othertask_que_t *, pNode*);
int get_trantask(trantask_que_t *, pNode*);//���� ����ָ�� | ���ָ��(��������)

#endif

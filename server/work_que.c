#include "work_que.h"

//��������г�ʼ��
void other_que_init(othertask_que_t *p, int capacity){
	p->head = p->tail = NULL;
	p->capacity = capacity;
	p->size = 0;
	pthread_mutex_init(&p->mutex, NULL);
}

//������г�ʼ��
void tran_que_init(trantask_que_t *p, int capacity){//���нṹ��ָ�� | ��������(����)
	p->head = p->tail = NULL;//��β�ÿ�
	p->capacity = capacity;//������ʼ��
	p->size = 0;//��С��0
	pthread_mutex_init(&p->mutex, NULL);//��ʼ��������
}

int empty_other(othertask_que_t *p){
	return !p->size;
}

int empty_tran(trantask_que_t *p){
	return !p->size;
}

int insert_othertask(othertask_que_t *p, pNode pinsert){
	if (p->size == p->capacity){
		return 1;
	}
	if (empty_other(p)){
		p->head = p->tail = pinsert;
	}
	else{
		p->tail->pNext = pinsert;
		p->tail = pinsert;
	}
	p->tail->pNext = NULL;
	p->size++;
	return 0;
}

int insert_trantask(trantask_que_t *p, pNode pinsert){
	if (p->size == p->capacity){
		return 1;
	}
	if (empty_tran(p)){
		p->head = p->tail = pinsert;
	}
	else{
		p->tail->pNext = pinsert;
		p->tail = pinsert;
	}
	p->tail->pNext = NULL;
	p->size++;
	return 0;
}

int get_othertask(othertask_que_t *p, pNode *pget){
	if (empty_other(p)){
		return 1;
	}
	if (p->size==1){
		*pget = p->head;
		p->head = p->tail = NULL;
	}
	else{
		*pget = p->head;
		p->head = p->head->pNext;
	}
	p->size--;
	return 0;
}

//����
int get_trantask(trantask_que_t *p, pNode *pget){//����ָ�� | ���ָ��(��������)
	if (empty_tran(p)){
		return 1;
	}
	if (p->size==1){
		*pget = p->head;
		p->head = p->tail = NULL;
	}
	else{
		*pget = p->head;
		p->head = p->head->pNext;
	}
	p->size--;
	return 0;
}

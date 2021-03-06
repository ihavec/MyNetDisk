#include "work_que.h"

//短命令队列初始化
void other_que_init(othertask_que_t *p, int capacity){
	p->head = p->tail = NULL;
	p->capacity = capacity;
	p->size = 0;
	pthread_mutex_init(&p->mutex, NULL);
}

//传输队列初始化
void tran_que_init(trantask_que_t *p, int capacity){//队列结构体指针 | 队列能力(传入)
	p->head = p->tail = NULL;//首尾置空
	p->capacity = capacity;//能力初始化
	p->size = 0;//大小置0
	pthread_mutex_init(&p->mutex, NULL);//初始化队列锁
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

//出队
int get_trantask(trantask_que_t *p, pNode *pget){//队列指针 | 结点指针(传出参数)
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

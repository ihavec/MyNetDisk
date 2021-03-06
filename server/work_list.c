#include "work_list.h"

extern int cur_index;
extern int g_time_out;
extern ptask_list_t *p_task_list;

//初始化
void user_list_init(user_list *p){//用户链表指针
	p->head = p->tail = NULL;//头尾置空
	p->conn_user_num = 0;//0
	p->signin_user_num = 0;//0
	pthread_mutex_init(&p->mutex, NULL);//初始化锁
}

//增加用户连接信息(用户连接)
void user_list_insert(user_list *p, user_t *pinsert){
	if (p->conn_user_num == 0){
		p->head = p->tail = pinsert;
	}
	else{
		p->tail->pNext = pinsert;
		pinsert->pNext = NULL;
	}
	p->conn_user_num++;
}

//用户登录
void user_signin(user_list *p, int sockfd, int uid){//用户链表 | socket描述符 | UID
	user_t *ptmp;
	if (0 == user_list_serch_byfd(p, sockfd, &ptmp)){//用fd找结点的前一个结点,成功
		if (NULL == ptmp){//为第一个结点
			p->head->uid = uid;//填入uid
			p->head->signin = 1;//登录状态为1
		}
		else{
			ptmp->pNext->uid = uid;//填入uid
			ptmp->pNext->signin = 1;//登录状态为1
		}
		p->signin_user_num++;//登录用户数+1
	}
}
//用户登出,不删链表结点
void user_signout(user_list *p, int uid){//用户链表 | UID
	user_t *ptmp;
	if (0 == user_list_serch_byuid(p, uid, &ptmp)){//用uid找结点的前一个结点,成功
		if (NULL == ptmp){
			p->head->uid = 0;
			p->head->signin = 0;//登录状态为0
		}
		else{
			ptmp->pNext->uid = 0;
			ptmp->pNext->signin = 0;//登录状态为0
		}
		p->signin_user_num--;//登录用户数-1
	}
}

//用户断开连接,删链表结点
void user_list_delete(user_list *p, int sockfd, user_t **pout){
	user_t *ppre, *ptmp = NULL;
	if (0 == user_list_serch_byfd(p, sockfd, &ppre)){
		p->conn_user_num--;
		if (NULL == ppre){
			if (0 == p->conn_user_num){
				p->tail = NULL;
			}
			ptmp = p->head;
			p->head = ptmp->pNext;
		}
		else{
			ptmp = ppre->pNext;
			//删除最后一个结点要改变尾指针
			if (NULL == ptmp->pNext){
				p->tail = ppre;
			}
			ppre->pNext = ptmp->pNext;
		}
	}
	*pout = ptmp;
}

//通过uid查找用户信息的前一个结点的指针(返回0表示找到,如果是第一个,传出参数为NULL)
int user_list_serch_byuid(user_list *p, int uid, user_t **pout){//用户链表 | UID | 指向用户结点指针(传出参数)
	user_t *ppre, *pcur = p->head;//ppre指向前一个结点,pcur指向当前结点
	while (pcur){
		if (uid == pcur->uid){
			//说明是第一个结点
			if (pcur == p->head){//是第一个,传出参数为NULL
				*pout = NULL;
			}
			else{
				*pout = ppre;
			}
			return 0;//成功返回0
		}
		else{
			ppre = pcur;
			pcur = pcur->pNext;
		}
	}
	*pout = NULL;
	return -1;//失败-1
}
//通过fd查找用户信息的前一个结点的指针(返回0表示找到,如果是第一个,传出参数为NULL)
int user_list_serch_byfd(user_list *p, int sockfd, user_t **pout){//用户链表 | socket描述符 | 指向用户结点指针(传出参数)
	user_t *ppre, *pcur = p->head;//ppre指向前一个结点,pcur指向当前结点
	while (pcur){
		if (sockfd == pcur->sockfd){
			//说明是第一个结点
			if (pcur == p->head){//是第一个,传出参数为NULL
				*pout = NULL;
			}
			else{
				*pout = ppre;
			}
			return 0;//成功返回0
		}
		else{
			ppre = pcur;
			pcur = pcur->pNext;
		}
	}
	*pout = NULL;
	return -1;//失败-1
}

//初始化
void task_list_init(ptask_list_t p){
	p->head = p->tail = NULL;
}

//增加任务
void task_list_insert(ptask_list_t p, task_t *pinsert){
	if (NULL == p->head){
		p->head = p->tail = pinsert;
	}
	else{
		p->tail->pNext = pinsert;
		pinsert->pNext = NULL;
	}
}

//删除任务结点
void task_list_delete(ptask_list_t p, int sockfd){
	task_t *ppre, *ptmp = NULL;
	if (0 == task_list_serch_byfd(p, sockfd, &ppre)){
		if (NULL == ppre){
			ptmp = p->head;
			p->head = ptmp->pNext;
			if (NULL == p->head){
				p->tail = NULL;
			}
		}
		else{
			ptmp = ppre->pNext;
			//删除最后一个结点要改变尾指针
			if (NULL == ptmp->pNext){
				p->tail = ppre;
			}
			ppre->pNext = ptmp->pNext;
		}
		free(ptmp);
	}
}

//通过fd查找任务结点的前一个结点的指针(返回0表示找到,如果是第一个,传出参数为NULL)
int task_list_serch_byfd(ptask_list_t p, int sockfd, task_t **pout){
	task_t *ppre, *pcur = p->head;
	while (pcur){
		if (sockfd == pcur->sockfd){
			//说明是第一个结点
			if (pcur == p->head){
				*pout = NULL;
			}
			else{
				*pout = ppre;
			}
			return 0;
		}
		else{
			ppre = pcur;
			pcur = pcur->pNext;
		}
	}
	*pout = NULL;
	return -1;
}

//销毁任务链表
void task_list_destory(ptask_list_t p){
	ptask_t pcur = p->head, ptmp;
	while (pcur){
		ptmp = pcur;
		pcur = pcur->pNext;
		free(ptmp);
	}
	p->head = p->tail = NULL;
}

//更新
void update(ptask_list_t *p_task_list, int sockfd)//任务链表指针数组 | socket描述符
{
	ptask_t ptmp;
	for (int i = 0; i < g_time_out; i++){
		if(0 == task_list_serch_byfd(p_task_list[i], sockfd, &ptmp)){//找到,存入ptmp
			task_list_delete(p_task_list[i], sockfd);//删除任务结点
		}
	}
	ptmp = (ptask_t)calloc(1, sizeof(task_t));//创建任务结点
	ptmp->sockfd = sockfd;
	task_list_insert(p_task_list[cur_index], ptmp);//插入当前任务队列
}

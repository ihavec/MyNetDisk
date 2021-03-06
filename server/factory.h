#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "common.h"
#include "work_que.h"
#include "work_list.h"
#include "trans_file.h"
#include "command.h"

typedef struct{
	char addr[16];//IP地址
	char port[8];//端口
	char pthread_tran_num[8];//长传输线程数
	char pthread_other_num[8];//短命令线程数
	char tran_capacity[8];//长传输队列能力
	char other_capacity[8];//短命令队列能力
	char max_sockfd_num[8];//最大socket数
	char server[32];//服务器IP
	char user[32];//用户名
	char passwd[32];//密码
	char database[32];//数据库名
	char timeout[8];//超时
	char filepath[64];//文件目录
}config_t;//配置结构体

typedef struct{
	short start_flag;//开始标志
	unsigned pthread_tran_num;//长传输线程数
	unsigned pthread_other_num;//短命令线程数
	unsigned max_sockfd_num;//最大socket数
	int *fsockfd;//socket描述符数组
	pthread_t *parr;//线程ID数组
	user_list users;//用户链表
	ptask_list_t *task;//任务链表指针数组
	pthread_cond_t tran_cond;//长传输条件变量
	pthread_cond_t other_cond;//短命令条件变量
	trantask_que_t tran_que;//长传输队列
	othertask_que_t other_que;//短命令队列
}factory_t, *pfactory_t;

void get_config(config_t *, const char *);//获取配置信息
void *pthreadfun_tran(void *);
void *pthreadfun_other(void *);
void factory_init(pfactory_t, config_t *);
void factory_start(pfactory_t);
int socket_bind(config_t *);
int is_send_recvfd(int);
void dectect_fd();

#endif

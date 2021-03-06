#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "common.h"
#include "work_que.h"
#include "work_list.h"
#include "trans_file.h"
#include "command.h"

typedef struct{
	char addr[16];//IP��ַ
	char port[8];//�˿�
	char pthread_tran_num[8];//�������߳���
	char pthread_other_num[8];//�������߳���
	char tran_capacity[8];//�������������
	char other_capacity[8];//�������������
	char max_sockfd_num[8];//���socket��
	char server[32];//������IP
	char user[32];//�û���
	char passwd[32];//����
	char database[32];//���ݿ���
	char timeout[8];//��ʱ
	char filepath[64];//�ļ�Ŀ¼
}config_t;//���ýṹ��

typedef struct{
	short start_flag;//��ʼ��־
	unsigned pthread_tran_num;//�������߳���
	unsigned pthread_other_num;//�������߳���
	unsigned max_sockfd_num;//���socket��
	int *fsockfd;//socket����������
	pthread_t *parr;//�߳�ID����
	user_list users;//�û�����
	ptask_list_t *task;//��������ָ������
	pthread_cond_t tran_cond;//��������������
	pthread_cond_t other_cond;//��������������
	trantask_que_t tran_que;//���������
	othertask_que_t other_que;//���������
}factory_t, *pfactory_t;

void get_config(config_t *, const char *);//��ȡ������Ϣ
void *pthreadfun_tran(void *);
void *pthreadfun_other(void *);
void factory_init(pfactory_t, config_t *);
void factory_start(pfactory_t);
int socket_bind(config_t *);
int is_send_recvfd(int);
void dectect_fd();

#endif

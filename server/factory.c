#include "factory.h"

extern user_list *p_user_list;
extern ptask_list_t *p_task_list;
extern int cur_index;
extern int g_time_out;

char *filepath;
char *server;
char *user;
char *password;
char *database;

void get_config(config_t *p1, const char *configfile){//传出配置结构体 | 配置文件
	int off[12] = {16, 8, 8, 8, 8, 8, 8, 32, 32, 32, 32, 8};
	memset(p1, 0, sizeof(config_t));//p1清0
	char buf[2][32], line[64], *p = (char *)p1;
	char *s[2]; s[0] = buf[0]; s[1] = buf[1];//s的两指针分别指向buf的两行
	memset(buf, 0, sizeof(buf));//buf清0
	int i = 0, len = 2;
	FILE *f = fopen(configfile, "r");//只读打开配置文件
	while (fgets(line, sizeof(line), f)){
		split(line, ": \t", s, &len);//划分每行配置文件,存入指针数组s
		printf("%s\n",buf[1]);//打印冒号后的数据
		memcpy(p, buf[1], strlen(buf[1]));//拷贝数据到p所指空间,即p1指的config_t结构体
		p += off[i++];
		memset(buf[1], 0, sizeof(buf[1]));//清空buf[1]
	}
	filepath = p1->filepath;//文件存储真实路径
	server = p1->server;//存server属性
	user = p1->user;//存用户名
	password = p1->passwd;//存密码
	database = p1->database;//存数据库名
	fclose(f);//关闭文件
}

//factory初始化
void factory_init(pfactory_t p, config_t *pconf){//指向factory结构体 | 指向config_t结构体
	memset(p, 0, sizeof(factory_t));
	p->pthread_tran_num = atoi(pconf->pthread_tran_num);
	p->pthread_other_num = atoi(pconf->pthread_other_num);
	p->max_sockfd_num = atoi(pconf->max_sockfd_num);
	p->fsockfd = (int *)calloc(p->max_sockfd_num, sizeof(int));//fsockfd数组
	p->parr = (pthread_t *)calloc(p->pthread_tran_num+p->pthread_other_num, sizeof(pthread_t)); //线程ID数组
	user_list_init(&p->users);//用户链表数组初始化

	//初始化tmp个任务列表(就是超时断开时间)
	int tmp = atoi(pconf->timeout);
	p->task = (ptask_list_t*)calloc(tmp, sizeof(ptask_list_t));//任务链表指针数组
	ptask_list_t ptask;//任务链表指针
	for (int i = 0; i < tmp; i++){
		ptask = (ptask_list_t)calloc(1, sizeof(task_list_t));//任务链表
		p->task[i] = ptask;
	}

	pthread_cond_init(&p->tran_cond, NULL);
	pthread_cond_init(&p->other_cond, NULL);
	tran_que_init(&p->tran_que, atoi(pconf->tran_capacity));
	other_que_init(&p->other_que, atoi(pconf->other_capacity));
}

//factory开始,创建线程池
void factory_start(pfactory_t p){
	p->start_flag = 1;
	for (unsigned i = 0; i < p->pthread_tran_num; i++){
		pthread_create(p->parr+i, NULL, pthreadfun_tran, p);
	}
	for (unsigned i = p->pthread_tran_num; i < p->pthread_tran_num+p->pthread_other_num;i++){
		pthread_create(p->parr+i, NULL, pthreadfun_other, p);
	}
}

//线程函数,传输文件
void *pthreadfun_tran(void *p1){
	pfactory_t p = (pfactory_t)p1;
	pNode pnode;
	trantask_que_t *pq = &p->tran_que;
	while(1){
		pthread_mutex_lock(&pq->mutex);
		if (empty_tran(pq)){
			pthread_cond_wait(&p->tran_cond, &pq->mutex);
		}
		get_trantask(pq, &pnode);//出队,结点存入pnode
		pthread_mutex_unlock(&pq->mutex);
		trans_file(pnode);//传输文件
		free(pnode);
	}
}

//线程函数,短命令
void *pthreadfun_other(void *p1){
	pfactory_t p = (pfactory_t)p1;
	pNode pnode;
	othertask_que_t *pq = &p->other_que;
	while(1){
		pthread_mutex_lock(&pq->mutex);
		if (empty_other(pq)){
			pthread_cond_wait(&p->other_cond, &pq->mutex);
		}
		get_othertask(pq, &pnode);
		pthread_mutex_unlock(&pq->mutex);
		command_handle(pnode);
		free(pnode);
	}
}

//绑定socket
int socket_bind(config_t *p){
	int sfd = socket(AF_INET, SOCK_STREAM, 0);//生成socket描述符
	int reuse = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, 4);//允许重用本地地址和端口int
	struct sockaddr_in ser;
	memset(&ser, 0, sizeof(ser));
	int len = sizeof(ser);
	ser.sin_family = AF_INET;
	ser.sin_port = htons(atoi(p->port));
	ser.sin_addr.s_addr = inet_addr(p->addr);
	bind(sfd, (struct sockaddr *)&ser, len);//绑定一个端口号和 IP 地址，使套接口与指定的端口号和 IP 地址相关联成功则返回 0，失败返回 - 1
	return sfd;
}

//判断是不是传输文件的fd,是就存起来
int is_send_recvfd(int fd){
	int uid;
	recv(fd, &uid, sizeof(int), 0);//从fd接收uid
	if (uid > 1000){//是传输文件的fd
		user_t *ptmp;
		if (0 == user_list_serch_byuid(p_user_list, uid, &ptmp)){//通过uid查找用户信息的前一个结点的指针(返回0表示找到,如果是第一个,传出参数为NULL)
			if (NULL == ptmp){
				p_user_list->head->send_recv_fd = fd;//存储fd
			}
			else{
				ptmp->pNext->send_recv_fd = fd;//存储fd
			}
		}
		return 1;
	}
	return 0;//不是传输文件的fd
}

//
void dectect_fd(){
	ptask_list_t ptask = p_task_list[(cur_index+1)%g_time_out];//指向任务链表数组中的下一条链表
	ptask_t ptmp = ptask->head;//取该任务链表的头结点
	while (ptmp){//若头结点存在,循环删除
		printf("delete\n");//打印
		disconnect(ptmp->sockfd);//断开连接,删用户链表结点
		ptmp = ptmp->pNext;//指向下一个
	}
	task_list_destory(ptask);//销毁任务链表
	cur_index = (cur_index+1)%g_time_out;//当前标识后移
}

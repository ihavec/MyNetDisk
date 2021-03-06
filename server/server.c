#include "factory.h"

user_list *p_user_list;
ptask_list_t *p_task_list;
int g_time_out;//超时时间
int g_epfd;
int cur_index;

int main(int argc, char **argv){
	config_t config;
	factory_t fac;//定义factory变量fac
	get_config(&config, argv[1]);
	factory_init(&fac, &config);//factory初始化,从配置文件读取
	p_user_list = &fac.users;
	p_task_list = fac.task;
	g_time_out = atoi(config.timeout);
	factory_start(&fac);//factory开始,创建线程池

	int sfd = socket_bind(&config);//绑定socket
	g_epfd = epoll_create(1);//创建epoll句柄
	struct epoll_event event, *evs;
	evs = (struct epoll_event *)calloc(fac.max_sockfd_num, sizeof(struct epoll_event));//创建struct epoll_event数组,大小为max_sockfd_num
	event.events = EPOLLIN;
	event.data.fd = sfd;
	epoll_ctl(g_epfd, EPOLL_CTL_ADD, sfd, &event);//注册sfd

	pNode pnode;
	message_t *pmsg;
	trantask_que_t *pq1 = &fac.tran_que;//长传输队列
	othertask_que_t *pq2 = &fac.other_que;//短命令队列

	int ready_fd_num, i, new_fd, len;
	user_t *pl, *ptmp;
	ptask_t pt;

	listen(sfd, 10);//使服务器的这个端口和 IP 处于监听状态
	while (1){
		ready_fd_num = epoll_wait(g_epfd, evs, p_user_list->conn_user_num+2, 1000);//等待事件发生,最多等conn_user_num+2个
		//printf("ready=%d,%d\n", ready_fd_num, p_user_list->conn_user_num+2);
		for (i = 0; i < ready_fd_num; i++){
			if (evs[i].data.fd == sfd){//若sfd发生
				printf("hello\n");
				new_fd = accept(sfd, NULL, NULL);//接收连接请求
				if (1 == is_send_recvfd(new_fd)){//如果是用来传输文件的连接,就不监控
					//update(p_task_list, new_fd);
				}
				else{//如果是短命令连接
					if (fac.max_sockfd_num <= p_user_list->conn_user_num+2){//如果监控的描述符数量不够了，就重新分配
						evs = (struct epoll_event *)realloc(evs, fac.max_sockfd_num * 2);//重新分配2倍的最大数量
						fac.max_sockfd_num *= 2;
					}
					pt = (ptask_t)calloc(1, sizeof(task_t));//申请任务结点空间
					pt->sockfd = new_fd;//存new_fd
					task_list_insert(fac.task[cur_index], pt);//插入任务链表

					//有客户端连上就插入链表
					ptmp = (user_t *)calloc(1, sizeof(user_t));//申请用户结点空间
					ptmp->sockfd = new_fd;
					pthread_mutex_lock(&p_user_list->mutex);//加锁
					user_list_insert(p_user_list, ptmp);//插入用户链表,内含new_fd
					pthread_mutex_unlock(&p_user_list->mutex);//解锁

					event.data.fd = new_fd;
					epoll_ctl(g_epfd, EPOLL_CTL_ADD, new_fd, &event);//注册new_fd
				}
				printf("user_num=%d\n", p_user_list->conn_user_num);//打印连接用户数
			}
			
			for (pl = p_user_list->head; pl != NULL; pl = pl->pNext){//遍历已登录用户表(也就是已连接的客户端)
				if (evs[i].data.fd == pl->sockfd){//若此用户的sockfd发生
					pnode = (pNode)calloc(1,sizeof(Node_t));
					pmsg = &pnode->msg;
					len = recv(pl->sockfd, pmsg, sizeof(message_t), 0);//从socket接收信息到pmsg
					if (0 == len){//如果接收到信息为空
						disconnect(pl->sockfd);//断开连接,删用户链表结点
						continue;
					}
					//更新客户操作时间
					update(p_task_list, pl->sockfd);

					msg_print(pmsg);//打印信息
					if (pmsg->type.cmd_type == 5 || pmsg->type.cmd_type == 6){//若命令类型为puts或gets
						pnode->new_fd = pl->sockfd;
						pthread_mutex_lock(&pq1->mutex);//加锁
						insert_trantask(pq1, pnode);//将pnode插入传输队列pq1
						pthread_mutex_unlock(&pq1->mutex);//解锁
						pthread_cond_signal(&fac.tran_cond);//激发某个线程
					}
					else{//若命令类型为短命令
						pnode->new_fd = pl->sockfd;
						pthread_mutex_lock(&pq2->mutex);//加锁
						insert_othertask(pq2, pnode);//将pnode插入短命令队列pq2
						pthread_mutex_unlock(&pq2->mutex);//解锁
						pthread_cond_signal(&fac.other_cond);//激发某个线程
					}
				}
			}
		}
		dectect_fd();
	}
}

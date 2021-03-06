//#include "factory.h"
#include "passwd.h"
#include "command.h"
#include "trans_file.h"

static int s_command_cnt = 14;
static char *s_command[14]={"signup", "signin", "cd", "ls", "puts", "gets", "remove", "pwd", "mkdir", "rmdir", "rename", "logout", "quit", "clear"};

void get_config(config_t *p1, const char *configfile){
	int off[1] = {16};
	memset(p1, 0, sizeof(config_t));
	char buf[2][32], line[64], *p = (char *)p1;
	char *s[2]; s[0] = buf[0]; s[1] = buf[1];
	memset(buf, 0, sizeof(buf));
	int i = 0, len = 2;
	FILE *f = fopen(configfile, "r");
	while (fgets(line, sizeof(line), f)){
		split(line, ": ", s, &len);
		memcpy(p, buf[1], sizeof(buf[1])-1);
		p += off[i++];
		memset(buf[1], 0, sizeof(buf[1]));
	}
	fclose(f);
}

void factory_init(pfactory_t p, config_t *pconfig){
	memset(p, 0, sizeof(factory_t));
	p->sfd = socket(AF_INET,SOCK_STREAM,0);
	p->ser.sin_family = AF_INET;
	p->ser.sin_port = htons(atoi(pconfig->port));
	p->ser.sin_addr.s_addr = inet_addr(pconfig->addr);
}

//这里需要用堆内存来存储线程需要用到的信息,因为栈空间会被释放
void get_pthread_stru(pthread_stru** out, pfactory_t pfac, message_t *pmsg){//传出参数 | pfactory_t结构体指针 | message_t结构体
	pthread_stru *p = (pthread_stru*)calloc(1,sizeof(pthread_stru));
	memcpy(&p->fac, pfac, sizeof(factory_t));
	memcpy(&p->msg, pmsg, sizeof(message_t));
	*out = p;
}



//上传下载文件的线程函数
void *pthread_send_recv(void *p1){
	pthread_stru *p = (pthread_stru *)p1;
	pfactory_t pfac = &p->fac;
	message_t *pmsg = &p->msg;
	if (6 == pmsg->type.cmd_type){//若为gets
		int fd = open(pmsg->u1.param, O_RDONLY);//只读打开此文件
		if (-1 != fd){//若打开成功,表示文件存在
			printf("File exists\n");
			printf("%s@ftp:~/%s$ ", pfac->info.user_name, pfac->info.dir+1);
			pthread_exit(NULL);
		}
	}
	int send_recv_fd;
	send_recv_fd = socket(AF_INET,SOCK_STREAM,0);//创建socket描述符
	int ret = connect(send_recv_fd, (struct sockaddr *)&pfac->ser, sizeof(struct sockaddr));//连接
	if(-1==ret){
		perror("connect");
	}
	send(send_recv_fd, &pmsg->uid, 4, 0);//发送uid
	if (5 == pmsg->type.cmd_type){//若为puts
		client_puts(pfac, pmsg, send_recv_fd);
	}
	else{
		client_gets(pfac, pmsg, send_recv_fd);
	}
	printf("%s@ftp:~/%s$ ", pfac->info.user_name, pfac->info.dir+1);
	fflush(stdout);
	close(send_recv_fd);
	free(p);
	pthread_exit(NULL);
}

//获取输入信息存入message_t结构体
int message_handle(message_t *pmsg, char *input){//message_t结构体(传出参数) | 传入字符串
	if (!*input){
		return -1;
	}
	memset(pmsg, 0, sizeof(message_t));//清0
	char cmd[32]={0};
	int i=0, flag = 0;
	while (*input && *input != ' '){//存储命令,遇到空格结束
		cmd[i++] = *input++;
	}
	for (int j = 0; j < s_command_cnt; j++){//最多14种命令
		if (strcmp(s_command[j], cmd)==0){
			pmsg->type.cmd_type = j+1;//存入命令种类+1
			flag = 1;
			break;
		}
	}
	if (!flag){
		return -1;
	}
	i = 0;
	while (*input == ' '){//跳过空格
		input++;
	}
	while (*input){//存储参数
		pmsg->u1.param[i++] = *input++;
	}
	return 0;
}

void command_handle(pfactory_t pfac, message_t *pmsg){
	int flag = 0;
	if (pmsg->type.cmd_type < 15 && pmsg->type.cmd_type > 1 && pmsg->type.cmd_type != 5 && pmsg->type.cmd_type != 6){//若为短命令
		flag = 1;
	}
	if (1 == pmsg->type.cmd_type){//若为登录
		if (0 == pfac->info.signin_flag){
			sign_up(pfac, pmsg);
		}
		else{
			printf("请先登出\n");
		}
	}
	else if (2 == pmsg->type.cmd_type){//若为注册
		if (0 == pfac->info.signin_flag){
			sign_in(pfac, pmsg);
		}
		else{
			printf("请先登出\n");
		}
	}
	else if (14 == pmsg->type.cmd_type){//若为登出
			system("clear");
	}
	else{
		if (1 == pfac->info.signin_flag){//若已登录
			pmsg->uid = pfac->info.uid;
			strcpy(pmsg->u2.dir, pfac->info.dir);
			pthread_stru *tmp;
			pthread_t pthid;
			switch(pmsg->type.cmd_type){
				case 3:
					change_dir(pfac, pmsg);//cd
					break;
				case 4:
					list_file(pfac, pmsg);//ls
					break;
				case 5://puts
				case 6://gets
					get_pthread_stru(&tmp, pfac, pmsg);//用tmp存pfac和pmsg
					pthread_create(&pthid, NULL, pthread_send_recv, tmp);
					break;
				case 7:
					remove_file(pfac, pmsg);
					break;
				case 8:
					printf("%s\n", pfac->info.dir);
					break;
				case 9:
					make_dir(pfac, pmsg);
					break;
				case 10:
					remove_dir(pfac, pmsg);
					break;
				case 11:
					re_name(pfac, pmsg);
					break;
				case 12:
					send(pfac->sfd, pmsg, sizeof(message_t), 0);
					log_out();
					break;
				case 13:
					quit2();
					break;
				default:
					printf("command not found\n");
					break;
			}
		}
		else{
			printf("请登录后再操作\n");
		}
	}
	if (flag){
		printf("%s@ftp:~/%s$ ", pfac->info.user_name, pfac->info.dir+1);	
	}
}


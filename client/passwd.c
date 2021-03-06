#include "passwd.h"
#include "factory.h"

void quit();

//返回0表示登录成功，其他表示错误码
int sign_in(pfactory_t pfac, message_t *pmsg){
	int flag = 0;
	char passwd[64];
	printf("请输入用户名:");
	fgets(pmsg->u1.user_name, 64, stdin);
	pmsg->u1.user_name[strlen(pmsg->u1.user_name)-1]=0;
	//先发送用户名
	send(pfac->sfd, pmsg, sizeof(message_t)-128, 0);
	memset(pmsg, 0, sizeof(message_t));
	//接收返回值
	if( 0 == recv(pfac->sfd, pmsg, sizeof(message_t)-128, 0)){
		quit();
	}
	//返回值为0表示存在此用户
	if (0 == pmsg->type.ret_type){
		printf("用户存在\n");
		flag = 1;
	}
	else if(-1 == pmsg->type.ret_type){
		printf("重复登录:\n");
		return -1;
	}
	for (int i = 0; i < 3; i++){
		strcpy(passwd,getpass("请输入密码:"));
		if (flag){
			//如果用户存在,user_name的后64位存储查找到的salt值
			strcpy(pmsg->u2.passwd_encry, crypt(passwd, pmsg->u1.user_name+64));
			//表明是登录
			pmsg->type.cmd_type = 2;
			send(pfac->sfd, pmsg, sizeof(message_t), 0);
			if (0 == recv(pfac->sfd, pmsg, 2*sizeof(int), 0)){
				quit();
			}
			//返回0表示密码正确
			if (pmsg->type.ret_type==0){
				pfac->info.uid = pmsg->uid;
				pfac->info.signin_flag = 1;
				strcpy(pfac->info.dir, "/");
				strcpy(pfac->info.user_name, pmsg->u1.user_name);
				system("clear");
				printf("登录成功\n");
				return 0;
			}
			else{
				printf("用户名或密码错误\n");
			}
		}
		else{
				printf("用户名或密码错误\n");
		}
	}

	return -2;
}

int sign_up(pfactory_t pfac, message_t *pmsg){
	char passwd[64], salt[20]={0},tmp[10]={0};
	while (1){
		printf("请输入用户名:");
		fgets(pmsg->u1.user_name, 64, stdin);
		pmsg->u1.user_name[strlen(pmsg->u1.user_name)-1]=0;
		//输入quit表示退出
		if (strcmp(pmsg->u1.user_name, "quit")==0){
			return -1;
		}	
		send(pfac->sfd, pmsg, sizeof(message_t)-128, 0);
		//接收返回值
		if(0 == recv(pfac->sfd, &pmsg->type.ret_type, sizeof(int), 0)){
			quit();
		}
		//返回值为0表示可以用此用户名
		if (pmsg->type.ret_type == 0){
			break;
		}
		else{
			printf("用户名已存在\n");
		}
	}
	strcpy(passwd,getpass("请输入密码:"));
	get_rand_str(tmp, 8);
	//用户名数组的后64位用来存放salt
	sprintf(salt,"$6$%s", tmp);
	//存放加密后的密码
	strcpy(pmsg->u2.passwd_encry, crypt(passwd, salt));
	//这里没有接收返回值,假设一定成功
	pmsg->type.cmd_type = 1;
	send(pfac->sfd, pmsg, sizeof(message_t), 0);
	if (0 == recv(pfac->sfd, &pmsg->type.ret_type, sizeof(int), 0)){
		quit();
	}
	if (pmsg->type.ret_type == 0){
		printf("注册成功\n");
	}
	else{
		printf("注册失败\n");
	}
	return 0;
}

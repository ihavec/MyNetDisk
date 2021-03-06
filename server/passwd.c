#include "passwd.h"
#include "work_list.h"

extern  user_list *p_user_list;
extern  int g_epfd;

//获取salt
void get_salt(char *salt,char *passwd){//存salt | 密文
	int i,j;
	//取出salt,i 记录密码字符下标,j 记录$出现次数
	for(i=0,j=0;passwd[i] && j != 3;++i)
	{
		if(passwd[i] == '$')
			++j;
	}
	strncpy(salt,passwd,i-1);
}

//在Userinfo中用username查找,成功返回0(数据库中存在此用户)
int query_by_username(char *user_name, char **out){//用户名 | 存uid和salt(传出参数) 
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	query_t qt;
	qt.conn = conn;
	if (out){//接收结果
		qt.len = 2;
		qt.out = out;
	}
	else{//不接收结果
		qt.len = 0;
		qt.out = NULL;
	}
	char cond[64];
	sprintf(cond,"user_name='%s'", user_name);//拼接条件
	if (-3==sql_query(&qt, "uid, salt", "Userinfo", cond)){//query_t结构体(传出参数) | 列名串 | 表名 | 条件
		printf("不存在\n");
		sql_close(conn);
		return 1;
	}
	sql_close(conn);//关闭连接
	return 0;
}

//增加用户到Userinfo
int add_user(message_t *pmsg){
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	msg_print(pmsg);//打印信息
	int ret;
	char value[256], salt[20];
	get_salt(salt, pmsg->u2.passwd_encry);//获取salt值
	sprintf(value,"'%s','%s','%s'",pmsg->u1.user_name,salt,pmsg->u2.passwd_encry);//拼接待插入值  user_name | salt | 密文  
	if(0==sql_insert(conn, "Userinfo", "user_name,salt,passwd", value)){//插入一行到数据库
		ret = 0;//成功
	}
	else{
		ret = 1;//失败
	}
	sql_close(conn);//关闭连接
	return ret;
}

//注册
int sign_up(pNode pnode){
	message_t *pmsg = &pnode->msg;
	int return_value;
	if (pmsg->u2.passwd_encry[0]==0){//密文为空,查询阶段
		if (query_by_username(pmsg->u1.user_name, NULL)==0){//找到
			return_value = 1;
		}
		else{//未找到
			return_value = 0;
		}
		send(pnode->new_fd, &return_value, 4, 0);//发送给new_fd查找结果
	}
	else{//注册阶段
		return_value = add_user(pmsg);//增加用户到Userinfo
		send(pnode->new_fd, &return_value, 4, 0);//发送操作结果
	}
	return 0;
}

//登录
int sign_in(pNode pnode){
	message_t *pmsg = &pnode->msg;
	if (pmsg->u2.passwd_encry[0]==0){//密文为空,查询阶段
		char *s[2], uid[10];
		s[0] = uid;
		s[1] = pmsg->u1.user_name+64;
		if (query_by_username(pmsg->u1.user_name, s)==0){//按用户名在数据库中查找uid和salt,返回0成功
			pmsg->uid = atoi(uid);
			user_t *ptmp;
			if(0 == user_list_serch_byuid(p_user_list, pmsg->uid, &ptmp)){//通过uid在用户链表中查找用户信息的前一个结点的指针(返回0表示找到)
				//表示该用户已经登录;
				pmsg->type.ret_type = -1;//已登录
			}
			else{
				pmsg->type.ret_type = 0;//未登录
			}
		}
		else{
			pmsg->type.ret_type = -2;//数据库中无用户
		}
		send(pnode->new_fd, pmsg, sizeof(message_t)-128, 0);//传送message_t前3项
	}
	else{//密文不空,登录阶段,验证密码
		printf("密码验证:\n");
		MYSQL *conn;
		sql_conn(&conn);//连接数据库
		char passwd_encry[128], cond[16], *s = passwd_encry;//s指向密文串
		query_t qt = {1, &s, conn};
		sprintf(cond,"uid=%d", pmsg->uid);//拼接条件
		if (0==sql_query(&qt, "passwd", "Userinfo", cond)){//query_t结构体(传出参数) | 列名串 | 表名 | 条件
			if (strcmp(passwd_encry, pmsg->u2.passwd_encry)==0){//验证成功
				printf("可以登录\n");
				pmsg->type.ret_type = 0;//登录成功0
			
				//记录日志
				my_log(pmsg->uid, "signin", "");
				//更新用户登录信息表
				user_signin(p_user_list, pnode->new_fd, pmsg->uid);
			}
			else{
				pmsg->type.ret_type = 1;//登录失败1
			}
		}
		else{
			pmsg->type.ret_type = 1;//登录失败1
		}
		send(pnode->new_fd, pmsg, 8, 0);//传送message_t前两项
	}
	return 0;
}

//登出,不删链表结点
void log_out(int uid){
	user_signout(p_user_list, uid);
}

//断开连接,删用户链表结点
void disconnect(int sockfd){//socket描述符
	user_t *ptmp;
	pthread_mutex_lock(&p_user_list->mutex);//用户链表加锁
	user_list_delete(p_user_list, sockfd, &ptmp);//删除用户链表结点(断开连接)
	pthread_mutex_unlock(&p_user_list->mutex);//用户链表解锁
	free(ptmp);//释放用户结点
	epoll_ctl(g_epfd, EPOLL_CTL_DEL, sockfd, NULL);//解除socket注册
	close(sockfd);//关闭socket
}

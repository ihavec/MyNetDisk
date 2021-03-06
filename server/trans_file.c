#include "trans_file.h"
#include "command.h"
#include "work_list.h"

extern char *filepath;
extern user_list *p_user_list;

int trans_file(pNode p){
	msg_print(&p->msg);//打印信息
	char s[2][5] = {"gets", "puts"};//s存"gets"和"puts"
	//插入一行到Ftplog
	my_log(p->msg.uid, s[p->msg.type.cmd_type%2], p->msg.u1.param);//UID | s[命令种类] | Node_t.msg.u1.param(参数)
	if (p->msg.uid == -1){//无uid
		printf("不传输\n");
		return -1;
	}
	user_t *ptmp;//用户结点
	int fd;
	if (0 == user_list_serch_byuid(p_user_list, p->msg.uid, &ptmp)){//通过uid查找用户信息的前一个结点的指针(返回0表示找到,如果是第一个,传出参数为NULL)
		if (NULL == ptmp){
			fd = p_user_list->head->send_recv_fd;//send_recv_fd赋值给fd
		}
		else{
			fd = ptmp->pNext->send_recv_fd;//send_recv_fd赋值给fd
		}
	}
	if (p->msg.type.cmd_type == 5){//cmd_type为5
		server_recvfile(p, fd);//接收文件
	}
	else {
		server_sendfile(p, fd);//否则接收文件
	}
	close(fd);//关闭socket
	return 0;
}

//服务端发送文件
int server_sendfile(pNode pnode, int send_fd){//队列结点指针 | 发送到的socket描述符
	message_t *pmsg = &pnode->msg;
	char realname[128], size[20], *s[2] = {realname, size};//s[0]指向md5字符串,s[1]指向size字符串
	MYSQL *conn;
	int pre_code, ret, fd;
	printf("send_fd = %d\n", send_fd);
	off_t filesize, offset;
	msg_print(pmsg);//打印信息
	sql_conn(&conn);//连接数据库
	//根据绝对路径查找目录,将主键存入ret_code,成功返回0
	find_dir(conn, pmsg, &pre_code);//连接描述符 | message_t结构体 | 指向precode(传出参数)
	//query_t结构体,存储查找结果
	query_t qt = {2, s, conn};//项数 | 指针数组 | 连接描述符
	msg_print(pmsg);//打印信息
	char cond[128];
	sprintf(cond, "precode=%d and filename='%s' and uid=%d and filetype='f'", pre_code, pmsg->u1.filename, pmsg->uid);//拼接条件,precode && filename && uid && filetype

	//数据库中查找，线性存储入query_t结构体(qt->out,即s),成功返回0,失败-1
	ret = sql_query(&qt, "md5, size", "Dirinfo", cond);//query_t结构体(传出参数) | 列名串 | 表名 | 条件
	//发送查找是否成功
	send(send_fd, &ret, 4, 0);//socket描述符 | 成功为0,失败负数 | 大小4B(int) | 0
	if (0 == ret){//查找成功
		char realpath[128];
		//拼接真实路径
		sprintf(realpath, "%s/%s", filepath, realname);//真实文件存储路径 | md5码
		offset = atol(pmsg->u1.filename+64);//偏移
		filesize = atol(size) - offset;//求文件大小
		printf("文件大小:%ld  %ld\n",filesize, offset);
		fd = open(realpath, O_RDONLY);//只读打开待传送文件
		//发送文件大小
		send(send_fd, &filesize, 8, 0);//socket描述符 | 文件大小 | 大小8B(long) | 0
		//发送文件
		if(filesize==sendfile(send_fd, fd, &offset, filesize)){//接收的描述符 | 发送的描述符 | 发送的偏移 | 发送的大小
			printf("sendfile success\n");
			return 0;
		}
		else{
			printf("sendfile failed \n");
		}
	}
	return -1;
}

//服务端接收文件
int server_recvfile(pNode pnode, int recv_fd){//队列结点指针 | 发送到的socket描述符
	message_t *pmsg = &pnode->msg;
	int ret;
	off_t filesize, download_size = 0;
	char md5[40] = {0};
	strcpy(md5, pmsg->u1.filename+64);
	MYSQL *conn;
	sql_conn(&conn);
	int pre_code;
	find_dir(conn, pmsg, &pre_code);
	printf("precode=%d\n", pre_code);
	//说明已经有同名文件了
	if (0 == find_file(conn,pre_code, pmsg->u1.param, 'f', pmsg->uid, NULL)){
		printf("有相同文件\n");
		printf("send_fd = %d\n", recv_fd);
		ret = -1;
		send(recv_fd, &ret, 4, 0);
		sql_close(conn);
		return -1;
	}

	char size[20], *s = size;
	query_t qt = {1, &s, conn};
	char cond[64];
	sprintf(cond, "md5='%s'", md5);
	ret = sql_query(&qt, "size", "Dirinfo", cond);//query_t结构体(传出参数) | 列名串 | 表名 | 条件

	//说明服务端有这个文件
	if (0 == ret){
		filesize = atol(size);
		send(recv_fd, &ret, 4, 0);
	}
	else{
		//告诉客户端文件不存在
		ret = -2;
		send(recv_fd, &ret, 4, 0);
		printf("接收文件\n");
		char file[128];
		int len;
		//md5码当做文件名
		sprintf(file, "%s/%s", filepath, md5);
		printf("文件名：%s\n", file);
		int fd = open(file, O_RDWR|O_CREAT, 0664);
		printf("creat fd = %d\n", fd);
		//接收文件大小
		recv(recv_fd, &filesize, 8, 0);
		printf("文件大小:%ld\n", filesize);
		ftruncate(fd, filesize);
		//接收并存储文件
		if (filesize < 100 * 1024 * 1024){
			char buf[4096];
			while (download_size != filesize){
				//退出机制的时候要考虑download_size不等于size的情况(没传完客户端退出可以考虑直接删掉已接收的文件)
				len = recv(recv_fd, buf, sizeof(buf), 0);
				if (0 == len){
					break;
				}
				write(fd, buf, len);
				download_size += len;
			}
		}
		//大于100M使用mmap
		else{
			char *p = (char *)mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, fd, 0);
			if ((char*)-1==p){
				perror("mmap");
			}
			while (download_size != filesize){
				len = recv(recv_fd, p+download_size, 4096, 0);
				if (0 == len){
					break;
				}
				download_size += len;
			}
			munmap(p, filesize);
		}
	}
	if (ret == 0 || download_size == filesize){
	char *item = "precode,filename,filetype,uid,md5,size";
	char value[256];
	sprintf(value, "%d,'%s','f',%d,'%s',%ld", pre_code, pmsg->u1.param, pmsg->uid, md5, filesize);
	ret = sql_insert(conn, "Dirinfo", item, value);
	}
	else{
		ret = -1;
	}
	send(recv_fd, &ret, 4, 0);
	sql_close(conn);
	return 0;
}

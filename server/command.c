#include "command.h"
#include "passwd.h"

extern char *filepath;
//定义静态变量,存储命令
static char *s_command[14]={"signup", "signin", "cd", "ls", "puts", "gets", "remove", "pwd", "mkdir", "rmdir", "rename", "logout", "quit", "clear"};

int command_handle(pNode p){//传入队列结点,执行短命令(不包括puts和gets)
	message_t *pmsg = &p->msg;
	printf("type=%d\n", pmsg->type.cmd_type);//打印命令种类
	if (pmsg->type.cmd_type > 2){
		my_log(pmsg->uid, s_command[pmsg->type.cmd_type-1], pmsg->u1.param);//插入一行到Ftplog表
	}
	switch(pmsg->type.cmd_type){
		case 1:
			sign_up(p);//注册,增加用户到Userinfo
			break;
		case 2:
			sign_in(p);//登录
			break;
		case 3:
			change_dir(p);//cd
			break;
		case 4:
			list_file(p);//列出文件
			break;
		case 7:
			remove_f(p);//删除文件
			break;
		case 9:
			make_dir(p);//创建目录
			break;
		case 10:
			remove_dir(p);//删除目录
			break;
		case 11:
			re_name(p);//重命名
			break;
		case 12:
			disconnect(p->new_fd);//断开连接,删用户链表结点
			break;
		case 13:
			log_out(p->msg.uid);//登出,不删链表结点
			break;
		default:
			break;
	}
	return 0;
}

//cd
void change_dir(pNode pnode){//传入队列结点
	message_t *pmsg = &pnode->msg;
	if (strcmp(pmsg->u1.param, "..") == 0){//参数为"..",即返回上层目录
		//当前为根目录则什么也不做
		if (strcmp(pmsg->u2.dir, "/") != 0){//不为根目录时
			int len = strlen(pmsg->u2.dir);
			while (pmsg->u2.dir[--len] != '/')//往前移到'/'处
				;
			if (len != 0){//不是只剩根目录时
				pmsg->u2.dir[len] = 0;//截断
			}
			else {
				pmsg->u2.dir[len+1] = 0;//只剩根目录的话要保留'/'
			}
		}
		pmsg->type.ret_type = 0;//登录状态
	}
	else{ 
		//绝对路径直接拷贝
		if (pmsg->u1.param[0] == '/'){//参数为绝对路径(以'/'开头)
			strcpy(pmsg->u2.dir, pmsg->u1.param);//拷贝绝对路径
		}
		//相对路径要进行拼接
		else{
			sprintf(pmsg->u2.dir, "%s/%s", pmsg->u2.dir, pmsg->u1.param);//拼接成绝对路径存入dir
		}
		printf("dir = %s\n", pmsg->u2.dir);//打印路径
		int ret;
		MYSQL *conn;
		sql_conn(&conn);//连接数据库
		ret = find_dir(conn, pmsg, NULL);//根据虚拟目录的绝对路径查找文件,将主键存入第3个参数,成功返回0
		sql_close(conn);//关闭数据库
		if (0 == ret){
			pmsg->type.ret_type = 0;//成功
		}
		else{
			pmsg->type.ret_type = 1;//失败
		}
	}
	send(pnode->new_fd, pmsg, sizeof(message_t), 0);//发送操作结果
}

//根据虚拟目录的绝对路径查找文件,将主键存入ret_code,成功返回0
int find_dir(MYSQL *conn, message_t *pmsg, int *ret_code){//连接描述符 | message_t结构体 | 指向code(传出参数)
	char filename[64], *s = pmsg->u2.dir+1;//s存路径
	int code = 0;//根目录code为0
	int ret, flag = 0;
	while (*s){//循环进入下一级目录
		s = get_next(s, filename, '/');//取下一个'/'前的字符串,存入filename
	    //在Dirinfo中根据filename查找code,找到返回0,失败-1
		ret = query_code_by_filename(conn, filename, pmsg->uid, &code);//连接描述符 | filename | uid | 传入该文件的precode,传出该文件的code(主键)(传入传出参数)
		if (0 != ret){//目录不存在
			flag = 1;
			break;
		}
	}
	if (0 == flag){//目录存在
		if (ret_code){
			*ret_code = code;//存储主键
		}
		dupli_remove(pmsg->u2.dir, '/');//去重
		printf("pmsg->u2.dir=%s\n", pmsg->u2.dir);//打印路径
		return 0;
	}
	return -1;//失败-1
}

//在Dirinfo中根据filename查找code,找到返回0,失败-1
int query_code_by_filename(MYSQL *conn, char *filename, int uid, int *code){//连接描述符 | filename | uid | 传入该文件的precode,传出该文件的code(主键)(传入传出参数)
	char new_code[10], *s = new_code, cond[128];
	query_t qt = {1, &s, conn};
	//拼接条件
	sprintf(cond, "filename='%s' and uid=%d and precode=%d and filetype='d'", filename, uid, *code);//filename && uid && precode && filetype
	//数据库查找,s所指字符串(即new_code)存结果code
	if(0==sql_query(&qt, "code", "Dirinfo", cond)){//query_t结构体(传出参数) | 列名串 | 表名 | 条件
		*code  = atoi(new_code);//字符串转整型
		return 0;
	}
	return -1;
}

//查找Dirinfo中是否有此文件(如果文件类型填NULL,则查找全部类型)
int find_file(MYSQL *conn, int pre_code, char *filename, char filetype, int uid, query_t *p){//连接描述符 | pre_code | filename | filetype | uid | 传出结果
	query_t qt = {0, NULL, conn};
	if (NULL == p){//若p为空,则不存结果
		p = &qt;
	}
	char filecond[128];
	sprintf(filecond, "precode=%d and filename='%s' and uid=%d", pre_code, filename, uid);//拼接文件信息
	if (0 != filetype){//如果文件类型不为空
		sprintf(filecond, "%s and filetype='%c'", filecond, filetype);//拼接上类型
	}
	int ret = sql_query(p, "code,filetype", "Dirinfo", filecond);//query_t结构体(传出参数) | 列名串 | 表名 | 条件
	printf("查找文件结果:%d\n", ret);
	return ret;//成功为0,失败为负
}

//创建文件夹,成功发送并返回0,失败发送并返回-1
int make_dir(pNode pnode){
	int pre_code;
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	int ret = -2;
	//查找到当前文件夹
	if (0 == find_dir(conn, &pnode->msg, &pre_code)){//根据虚拟目录的绝对路径查找文件,将主键存入第3个参数,成功返回0
		//查看是否有重名文件
		if (0 == find_file(conn, pre_code, pnode->msg.u1.param, 'd', pnode->msg.uid, NULL)){//查找Dirinfo中是否有此文件(如果文件类型填NULL,则查找全部类型)
			ret = -1;//有重名文件-1
			send(pnode->new_fd, &ret, 4, 0);//有重名文件,发送结果-1
			sql_close(conn);//关闭数据库
			return ret;
		}
		char value[128];
		sprintf(value,"%d,'%s','d',%d", pre_code,pnode->msg.u1.param, pnode->msg.uid);//拼接插入值, precode | 参数(文件名) | 'd'(目录) | uid
		sql_insert(conn, "Dirinfo", "precode,filename,filetype,uid", value);//向Dirinfo插入一行
		ret = 0;
	}
	send(pnode->new_fd, &ret, 4, 0);//发送结果0
	sql_close(conn);//关闭连接
	return ret;
}

//重命名
void re_name(pNode pnode){
	message_t *pmsg = &pnode->msg;
	int pre_code;
	char oldname[64], newname[64], *s = pmsg->u1.param;
	s = get_next(s, oldname, ' ');//获得在字符' '之前的下一个字符串(开头有' '先去掉),即取旧名字
	get_next(s, newname, ' ');//获得在字符' '之前的下一个字符串(开头有' '先去掉),即取新名字

	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	int ret = -2;
	//查找到当前文件夹
	if (0 == find_dir(conn, &pnode->msg, &pre_code)){//根据虚拟目录的绝对路径查找文件,将主键存入第3个参数,成功返回0
		//查看是否有这个文件
		char code[10], *t = code;//t指向code
		query_t qt = {1, &t, conn};
		if (0 != find_file(conn, pre_code, oldname, 0, pnode->msg.uid, &qt)){//查找Dirinfo中是否有此文件(如果文件类型填NULL,则查找全部类型)
			ret = -1;//未找到
			send(pnode->new_fd, &ret, 4, 0);//文件名错误,发送-1
			sql_close(conn);//关闭连接
			return;
		}
		char cond[32], item[64];
		sprintf(cond,"code=%d", atoi(code));//code(主键)写入cond
		sprintf(item, "filename='%s'", newname);//文件名写入item
		sql_update(conn, "Dirinfo", item, cond);//改名
		ret = 0;//改名成功,发送0
	}
	send(pnode->new_fd, &ret, 4, 0);//路径错误,发送-2
	sql_close(conn);//关闭连接
}

//列出当前目录所有文件
void list_file(pNode pnode){
	file_info_t file;
	int pre_code;
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	sql_conn(&conn);//连接数据库
	find_dir(conn, &pnode->msg, &pre_code);//根据虚拟目录的绝对路径查找文件,将主键存入第3个参数,成功返回0
	printf("code=%d\n", pre_code);//打印本文件code
	char query_item[128];
	sprintf(query_item, "select filetype,size,filename from Dirinfo where uid=%d and precode=%d", pnode->msg.uid, pre_code);//填入搜索条件:搜索precode为code的所有文件
	printf("query_item=%s\n", query_item);//打印查找项目
	mysql_query(conn, query_item);//搜索
	res = mysql_use_result(conn);//存结果
	if(res){
		while((row = mysql_fetch_row(res)) !=NULL){//循环取一行
			file.type = row[0][0];//存类型第一个字符
			if (file.type == 'f'){//若为文件
				file.size = atoi(row[1]);
			}
			else{//若为目录
				file.size = 0;
			}
			strcpy(file.filename, row[2]);//存文件名
			send(pnode->new_fd, &file, sizeof(file_info_t), 0);//发送找到的文件信息
		}
	}
	file.type = -1;
	send(pnode->new_fd, &file, sizeof(file_info_t), 0);//type为-1表示结束
	sql_close(conn);//关闭连接
}

//删除当前目录及目录内所有文件
void remove_dir(pNode pnode){
	message_t *pmsg = &pnode->msg;
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	int ret = 0, pre_code, code;//ret初始为0
	char code_t[10], type[2], *s[2] = {code_t, type};//s[0]指向code_t,s[1]指向type
	query_t qt = {2, s, conn};
	if (0 == find_dir(conn, pmsg, &pre_code)){//根据虚拟目录的绝对路径查找文件,将主键存入第三个参数,成功返回0;查到该目录的父目录
		if (0 == find_file(conn, pre_code, pmsg->u1.param, 0, pmsg->uid, &qt)){	//查找Dirinfo中是否有此文件(如果文件类型填NULL,则查找全部类型);查到该目录文件
			printf("文件类型%c\n", type[0]);
			if (type[0] == 'f'){//若类型为文件
				ret = -1;//令ret为-1
			}
			code = atoi(code_t);
			printf("code = %d\n", code);//打印该目录的code
		}
		else{
			ret = -2;//令ret为-2
		}
	}
	if (0 == ret){//若类型为目录
		remove_all(code);//递归删除
	}
	sql_close(conn);//关闭连接
	send(pnode->new_fd, &ret, 4, 0);//发送操作结果
	return;
}

//删除文件(传入路径和filename)
void remove_f(pNode pnode){
	message_t *pmsg = &pnode->msg;
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	int ret = 0, pre_code, code;
	char code_t[10], type[2], *s[2] = {code_t, type};//s[0]指向code_t,s[1]指向type
	query_t qt = {2, s, conn};
	if (0 == find_dir(conn, pmsg, &pre_code)){//根据虚拟目录的绝对路径查找文件,将主键存入第三个参数,成功返回0
		if (0 == find_file(conn, pre_code, pmsg->u1.param, 0, pmsg->uid, &qt)){	//查找Dirinfo中是否有此文件(如果文件类型填NULL,则查找全部类型),存入qt
			printf("文件类型\n");
			if (type[0] == 'd'){//若为目录
				ret = -1;//ret=-1
			}
			code = atoi(code_t);//填入code
			printf("code = %d\n", code);//打印code
		}
		else{
			ret = -2;//无此文件,ret=-2
		}
	}
	if(0 == ret){//若找到此文件,且类型为文件
		remove_file(code);//删除文件
	}
	sql_close(conn);//关闭连接
	send(pnode->new_fd, &ret, 4, 0);//发送操作结果
}

//递归删除目录
void remove_all(int pre_code){
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	printf("i am remove all\n");//打印提示
	MYSQL_RES *res;
	MYSQL_ROW row;
	char item[64];
	int code; char type;
	sprintf(item, "select code,filetype from Dirinfo where precode=%d ", pre_code);//拼接条件
	if (0 == mysql_query(conn, item)){//查找precode为pre_code的所有文件
		printf("i coming\n");//打印提示
		res = mysql_use_result(conn);//存结果
		if (res){
			while ((row = mysql_fetch_row(res)) != NULL){//循环取一行
				code = atoi(row[0]);//存code
				type = row[1][0];//存type
				//删除文件
				if ('f' == type){//若类型为文件
					remove_file( code);//删除文件
				}
				//删除目录
				else{
					remove_all( code);//若为目录,则递归删除
				}
			}
		}
		mysql_free_result(res);//释放res
	}
	char cond[20];
	sprintf(cond,"code=%d", pre_code);//拼接条件
	printf("cond=%s\n", cond);//打印precode
	
	sql_delete(conn, "Dirinfo", cond);//最后删除本目录
	sql_close(conn);//关闭连接
}

//删除文件
void remove_file(int code){
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	char md5[2][40]={{0}}, *s[2] = {md5[0], md5[1]};//s[0]指向md5[0],s[1]指向md5[1]
	query_t qt = {2, s, conn};
	char cond[20];
	sprintf(cond,"code=%d", code);//拼接条件
	if(0 == sql_query(&qt, "md5", "Dirinfo", cond)){//query_t结构体(传出参数) | 列名串 | 表名 | 条件
		if (md5[1][0] == 0){
			char filename[100];
			sprintf(filename,"%s/%s", filepath, md5[0]);//拼接文件实际存储路径
			unlink(filename);//删除一个文件的目录项并减少它的链接数
		}
	}
	sql_delete(conn, "Dirinfo", cond);//删除该行
	sql_close(conn);//关闭连接
}


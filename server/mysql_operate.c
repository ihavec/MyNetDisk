#include "mysql_operate.h"

extern char *server;
extern char *user;
extern char *password;
extern char *database;

//修改数据库,改某格数据
int sql_update(MYSQL *conn, char *tab, char *item, char *cond){//连接的描述符 | 表名 | 修改后的值 | 条件
	if (!item || !tab){
		return -1;
	}
	char query_item[256];
	sprintf(query_item, "update %s set %s where %s", tab, item, cond);//拼接命令,存入query_item
	puts(query_item);//打印命令行
	int t;
	t=mysql_query(conn,query_item);//执行命令
	if(t)
	{
		return -2;
	}
	return 0;
}
//删除某行
int sql_delete(MYSQL *conn, char *tab, char *cond){//连接的描述符 | 表名 | 条件
	if (!tab || !cond){
		return -1;
	}
	char query_item[256];
	sprintf(query_item,"delete from %s where %s", tab, cond);//拼接命令,存入query_item
	int t;
	t=mysql_query(conn,query_item);//执行命令
	if(t)
	{
		return -1;
	}
	return 0;
}

//插入数据库
int sql_insert(MYSQL *conn, char *tab, char *item, char *value){//连接的描述符 | 表名 | 列名串 | 要插入的值串
	if (!tab || !value){
		return -1;
	}
	char query_item[256];
	sprintf(query_item,"insert into %s(%s) values(%s)", tab, item, value);//拼接命令,存入query_item
	printf("%s\n", query_item);
	int t;
	t=mysql_query(conn,query_item);//执行命令
	if(t){
		return -2;
	}
	printf("插入成功\n");
	return 0;
}

//数据库中查找，线性存储入query_t结构体,成功返回0,失败为负
int sql_query(query_t *p, char *item, char *tab, char *cond){//query_t结构体(传出参数) | 列名串 | 表名 | 条件
	if (!item || !tab){
		return -1;
	}
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query_item[256];
	sprintf(query_item,"SELECT %s FROM %s", item, tab);//拼接命令,存入query_item
	if (cond){
		sprintf(query_item,"%s WHERE %s", query_item, cond);//拼接命令,存入query_item
	}
	printf("条件%s\n", query_item);
	int t,i;
	t = mysql_query(p->conn, query_item);//执行命令
	if (t){	
		//查找操作失败(可能是不存在这个表，item之类)
		printf("查找失败\n");
		return -2;
	}
	res = mysql_use_result(p->conn);//获得结果存入MYSQL_RES结构体res
	if (res){
		printf("查找成功(可能是空集)\n");
		i = 0;
		row = mysql_fetch_row(res);//取一行存入MYSQL_ROW结构体row
		if (NULL==row){//空集
			printf("空集\n");
			return -3;
		}
		else{//非空集
			do{
				for (t = 0; t < mysql_num_fields(res); t++){//列数
					if (i>=p->len){//填入的数据项达到len
						p->len = i;
						mysql_free_result(res);//释放res
						return 0;
					}
					if(row[t]){
						strcpy(p->out[i++], row[t]);//拷贝某行t列数据到out
					}
					else{
						i++;
					}
				}
			}while ((row = mysql_fetch_row(res))!=NULL);//取一行存入MYSQL_ROW结构体row
			p->len = i;
			mysql_free_result(res);//释放res
		}
	}
	else {
		//出错
		return -4;
	}
	return 0;
}

//连接数据库
int sql_conn(MYSQL **conn){
	printf("数据库%s\n", server);//打印server属性
	*conn = mysql_init(NULL);//初始化连接
				           //conn描述符 | server IP | 用户名 | 密码 | 数据库名 | 0 | NULL | 0
	if (!mysql_real_connect(*conn,server,user,password,database,0,NULL,0)){//连接数据库
		return -1;
	}
	return 0;
}

//断开数据库连接
void sql_close(MYSQL *conn){
	mysql_close(conn);//关闭连接
}

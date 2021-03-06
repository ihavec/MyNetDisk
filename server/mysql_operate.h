#ifndef __MYSQL_OPERATE_H__
#define __MYSQL_OPERATE_H__

#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

typedef struct{
	int len;//out����
	char **out;//ָ������
	MYSQL *conn;//���ӵ�������
}query_t;

int sql_query(query_t *, char *, char *, char *);
int sql_insert(MYSQL *, char *, char *, char *);
int sql_update(MYSQL *, char *, char *, char *);
int sql_delete(MYSQL *, char *, char *);

int sql_conn(MYSQL **);
void sql_close(MYSQL *);

#endif

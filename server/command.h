#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "common.h"
#include "work_que.h"
#include "passwd.h"


int command_handle(pNode);//传入队列结点,执行短命令(不包括puts和gets)
void change_dir(pNode);//cd
int find_file(MYSQL *, int, char *, char, int, query_t *);//查找数据库中是否有此文件(如果文件类型填NULL,则查找全部类型)
int find_dir(MYSQL *, message_t *, int *);//根据虚拟目录的绝对路径查找文件,将主键存入第3个参数,成功返回0
int query_code_by_filename(MYSQL *, char *, int, int *);//在Dirinfo中根据filename查找code,找到返回0,失败-1
int make_dir(pNode);//创建文件夹,成功发送并返回0,失败发送并返回-1
void remove_dir(pNode);//删除当前目录及目录内所有文件
void remove_f(pNode );//删除文件(传入路径和filename)
void remove_all(int);//递归删除目录
void remove_file(int);//删除文件
void list_file(pNode);//列出当前目录所有文件
void re_name(pNode);//重命名

void log_out(int);//登出,不删链表结点

#endif

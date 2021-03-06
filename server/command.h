#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "common.h"
#include "work_que.h"
#include "passwd.h"


int command_handle(pNode);//������н��,ִ�ж�����(������puts��gets)
void change_dir(pNode);//cd
int find_file(MYSQL *, int, char *, char, int, query_t *);//�������ݿ����Ƿ��д��ļ�(����ļ�������NULL,�����ȫ������)
int find_dir(MYSQL *, message_t *, int *);//��������Ŀ¼�ľ���·�������ļ�,�����������3������,�ɹ�����0
int query_code_by_filename(MYSQL *, char *, int, int *);//��Dirinfo�и���filename����code,�ҵ�����0,ʧ��-1
int make_dir(pNode);//�����ļ���,�ɹ����Ͳ�����0,ʧ�ܷ��Ͳ�����-1
void remove_dir(pNode);//ɾ����ǰĿ¼��Ŀ¼�������ļ�
void remove_f(pNode );//ɾ���ļ�(����·����filename)
void remove_all(int);//�ݹ�ɾ��Ŀ¼
void remove_file(int);//ɾ���ļ�
void list_file(pNode);//�г���ǰĿ¼�����ļ�
void re_name(pNode);//������

void log_out(int);//�ǳ�,��ɾ������

#endif

#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "common.h"
#include "work_que.h"

int trans_file(pNode );
int server_sendfile(pNode, int);
int server_recvfile(pNode, int);

#endif

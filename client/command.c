#include "command.h"

extern pfactory_t g_pfac;

//cd命令，支持绝对路径，相对路径和..，参数为空则到根目录
void change_dir(pfactory_t pfac, message_t *pmsg){
	if (0==pmsg->u1.param[0]){
		strcpy(pmsg->u1.param, "/");
	}
	send(pfac->sfd, pmsg, sizeof(message_t), 0);
	if(0==recv(pfac->sfd, pmsg, sizeof(message_t), 0)){
		quit();
	}
	if (0 == pmsg->type.ret_type){
		strcpy(pfac->info.dir, pmsg->u2.dir);
		printf("~%s\n", pmsg->u2.dir);
	}
	else{
		printf("No such file or directory\n");
	}
}

void list_file(pfactory_t pfac, message_t *pmsg){
	file_info_t file;
	send(pfac->sfd, pmsg, sizeof(message_t), 0);
	do{
		if(0 == recv(pfac->sfd, &file, sizeof(file_info_t), MSG_WAITALL)){
			quit();
		}
		if (file.type != -1){
			char type;
			type = file.type == 'd' ? 'd':'-';
			printf("%c%15s%10ld\n", type, file.filename, file.size);
		}
	}while(file.type != -1);
}

void remove_file(pfactory_t pfac, message_t *pmsg){
	int ret;
	send(pfac->sfd, pmsg, sizeof(message_t), 0);
	if(0 ==recv(pfac->sfd, &ret, 4, 0)){
		quit();
	}
	if(0 == ret){
		;
	}
	else if (-1 == ret){
		printf("cannot remove '%s': Is a directory\n", pmsg->u1.param);
	}
	else{
		printf("No such file or directory\n");
	}
}

void log_out(){
	g_pfac->info.signin_flag = 0;
	g_pfac->info.uid = 0;
	close(g_pfac->sfd);
}


//退出
void quit(){
	log_out();//登出
	printf("Server disconnected\n");
	exit(-1);
}

void quit2(){
	log_out();
	printf("Bye\n");
	exit(0);
}

//只在当前目录下新建
void make_dir(pfactory_t pfac, message_t *pmsg){
	char *p = pmsg->u1.param;
	int i = 0, ret;
	while (p[i]){
		if (p[i] == '/'){
			break;
		}
		i++;
	}
	if (i==strlen(p)){
		send(pfac->sfd, pmsg, sizeof(message_t), 0);
	}
	else{
		printf("No such file or directory\n");
		return;
	}
	if (0 == recv(pfac->sfd, &ret, 4, 0)){
		quit();
	}
	if (-1 == ret){
		printf("File Exists\n");
	}
	else if(-2 == ret){
		printf("Mkdir failed\n");
	}
}

void remove_dir(pfactory_t pfac, message_t *pmsg){
	int ret;
	if(-1==send(pfac->sfd, pmsg, sizeof(message_t), 0)){
		exit(0);
	}
	if(0 == recv(pfac->sfd, &ret, 4, 0)){
		quit();
	}
	if(0 == ret){
		;
	}
	else if (-1 == ret){
		printf("failed to remove '%s': Not a directory\n", pmsg->u1.param);
	}
	else{
		printf("No such file or directory\n");
	}
}

void re_name(pfactory_t pfac, message_t *pmsg){
	int ret;
	ret=send(pfac->sfd, pmsg, sizeof(message_t), 0);
	if(0 == recv(pfac->sfd, &ret, 4, 0)){
		quit();
	}
	if (0 != ret){
		printf("No such file or directory\n");
	}
}

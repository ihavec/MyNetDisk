#include "command.h"

//客户端接收文件
int client_gets(pfactory_t pfac, message_t *pmsg, int recv_fd){
	struct stat state;
	state.st_size = 0;
	int ret;
	char filename[64];
	sprintf(filename,".%s.swp", pmsg->u1.param);
	int fd = open(filename, O_RDONLY);
	if (-1 != fd){
		fstat(fd, &state);
		state.st_size = state.st_size >> 12 << 12;
		sprintf(pmsg->u1.param+64,"%ld", state.st_size);
		close(fd);
	}
	send(pfac->sfd, pmsg, sizeof(message_t), 0);
	if( 0 == recv(recv_fd, &ret, 4, 0)){
		quit();
	}
	if (0 != ret){
		printf("No such file or directory\n");
		return -2;
	}
	off_t size, download_size = 0, len;
	if (0 == recv(recv_fd, &size, 8, 0)){
		quit();
	}
	fd = open(filename, O_RDWR|O_CREAT, 0664);
	if (-1==fd){
		printf("打开文件失败:\n");
	}
	ftruncate(fd, size+state.st_size);
	if (size < 100 * 1024 * 1024){
		char buf[4096];
		lseek(fd, state.st_size, SEEK_SET);
		while (download_size != size){
			len = recv(recv_fd, buf, sizeof(buf), 0);
			if (0 == len){
				break;
			}
			write(fd, buf, len);
			download_size += len;
		}
	}
	else{
		//lseek(fd, state.st_size, SEEK_SET);
		char *p = (char *)mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, state.st_size);
		while (download_size != size){
			len = recv(recv_fd, p+download_size, 4096, 0);
			if (0 == len){
				break;
			}
			download_size += len;
		}
		munmap(p, size);
	}
	if (download_size == size){
		rename(filename, pmsg->u1.param);
		printf("gets success\n");
		close(fd);
	}
	else{
		ftruncate(fd, download_size+state.st_size);
		printf("gets failed\n");
		close(fd);
	}
	if (0 == len){
		quit();
	}
	return 0;
}

//客户端发送文件
int client_puts(pfactory_t pfac, message_t *pmsg, int send_fd){
	int ret, fd;
	fd = open(pmsg->u1.param, O_RDONLY);//只读打开该文件
	if (-1 == fd){//若打不开,表示无此文件
		printf("No such file or directory\n");
		pmsg->uid = -1;
		send(pfac->sfd, pmsg, sizeof(message_t), 0);//发送信息
		return -1;
	}
	char md5[40] = {0};
	clac_md5(pmsg->u1.param, md5);//计算md5
	strcpy(pmsg->u1.filename+64, md5);//存储md5
	send(pfac->sfd, pmsg, sizeof(message_t), 0);//发送信息
	if (0 == recv(send_fd, &ret, 4, 0)){
		quit();
	}
	//本目录下有同名文件
	if(-1 == ret){
		close(fd);
		printf("File exists\n");
		return -2;
	}
	//服务器里没有这个文件
	else if (-2 == ret){
		struct stat state;
		off_t filesize;
		fstat(fd, &state);
		filesize = state.st_size;
		send(send_fd, &filesize, 8, 0);
		if (filesize != sendfile(send_fd, fd, NULL, filesize)){
			printf("puts failed\n");
			close(fd);
			return -1;
		}
	}
	//传完了或者服务器有这个文件,秒传
	close(fd);
	if (0 == recv(send_fd, &ret, 4, 0)){
		quit(0);
	}
	if (0 == ret){
		printf("puts success\n");
	}
	else{
		printf("puts failed\n");
	}
	return 0;
}

#include "common.h"


int clac_md5(char *filename, char *md5)
{
	MD5_CTX ctx;
	unsigned char outmd[16] = {0};
	char buff[1024] = {0};
	int i, len;
	
	int fd = open(filename, O_RDONLY);
	if (-1 == fd){
		printf("No such file\n");
		return 1;
	}

	MD5_Init(&ctx);
	while ((len=read(fd, buff, 1024)) > 0){
		MD5_Update(&ctx, buff, len);
		memset(buff, 0, 1024);
	}
	MD5_Final(outmd,&ctx);
	for(i=0; i<16; i++)
	{
		sprintf(md5, "%s%02X", md5, outmd[i]);
	}
	return 0;
}

void msg_print(message_t  *pmsg){
	printf("type=%d\n", pmsg->type.cmd_type);
	printf("uid=%d\n", pmsg->uid);
	printf("u1=%s\n", pmsg->u1.param);
	printf("u2=%s\n", pmsg->u2.dir);
}
//生成长度为num的随机字符串
void get_rand_str(char *s,int num){
	//定义随机生成字符串表
	char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int i,lstr;
	char ss[2] = {0};
	lstr = strlen(str);
	srand(time(NULL));
	for(i = 1; i <= num; i++){
		sprintf(ss,"%c",str[(rand()%lstr)]);
		strcat(s,ss);
	}
}

//删除字符串头尾的空格
void strip(char *str){
	char *p = str;
	while (*str==' '){
		str++;
	}
	while (*str){
		*p++ = *str++;
	}
	int n = 0;
	str--;
	while (*str==' '){
		str--;
		n++;
	}
	p[strlen(p)-n]=0;
}
//字符串s按照separation里的字符分割,存储在store中
void split(const char *s, const char *separation, char **store, int *len){
	if (!s){
		return;
	}
	int i = 0,j = 0, flag = 0;
	while (*s && find(separation, *s)){
		s++;
	}
	while (*s){
		while (*s && !find(separation, *s)){
			store[i][j++] = *s++;
		}
		while (*s && find(separation, *s)){
			flag = 1;
			s++;
		}
		if (flag){
			i++;
			if (i >= *len){
				break;
			}
			j = 0;	
		}
	}
	*len = i;
}

//在字符串s中查找字符c,找到返回1,否则返回0
int find(const char *s, const char c){
	if (!s){
		return 0;
	}
	while (*s && *s != c){
		s++;
	}
	if (*s){
		return 1;
	}
	return 0;
}

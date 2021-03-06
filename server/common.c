#include "common.h"
#include <mysql/mysql.h>

int sql_insert(MYSQL *, char *, char *, char *);
int sql_conn(MYSQL **);
int sql_close(MYSQL *);

//插入一行到Ftplog表
void my_log(int uid, char *command, char *param){//UID | 命令串 | 参数串
	MYSQL *conn;
	sql_conn(&conn);//连接数据库
	char value[256];
	sprintf(value, "%d,'%s','%s'", uid, command, param);//拼接行值
	sql_insert(conn, "Ftplog", "uid,command,parameter",value);//插入一行到Ftplog
	sql_close(conn);//关闭数据库连接
}

//计算md5
int clac_md5(char *filename, char *md5)//文件名 | md5码(传出参数)
{
	MD5_CTX ctx;
	unsigned char outmd[16] = {0};
	char buff[1024] = {0};
	int i, len;
	
	int fd = open(filename, O_RDONLY);//以只读打开文件
	if (-1 == fd){//若打开失败
		printf("No such file\n");
		return 1;
	}

	MD5_Init(&ctx);//初始化md5
	while ((len=read(fd, buff, 1024)) > 0){//循环读取文件
		MD5_Update(&ctx, buff, len);
		memset(buff, 0, 1024);
	}
	MD5_Final(outmd,&ctx);
	for(i=0; i<16; i++)
	{
		sprintf(md5, "%s%02X", md5, outmd[i]);//存入md5
	}
	return 0;
}

//去重
void dupli_remove(char *str, char c){//待去重字符串 | 去重字符
	printf("strdir=%s\n", str);
	int i = 0, j = 0, flag;
	while (str[i]){
		flag = 0;
		while(str[i]==c){//遇到连续c时,后移
			i++;
			flag = 1;
		}
		if (flag){
			i--;
			str[j++] = str[i++];//只记录连续c的最后一个
		}
		while(str[i] && str[i] != c){//不为c时,后移
			str[j++] = str[i++];
		}
	}
	str[j] = 0;
	printf("strdir=%s\n", str);//打印结果
}

//获得在字符c之前的下一个字符串(开头有c先去掉)
char *get_next(char *str, char *store, char c){//待查找字符串 | 存储结果 | 一般为'/'
	while (*str == c){//开头为c,跳过
		str++;
	}
	while (*str && *str!=c){//不为c,前进
		*store++ = *str++;
	}
	*store = 0;//字符串结束符
	while (*str == c){//为c,跳过
		str++;
	}
	return str;
}
//打印信息
void msg_print(message_t  *pmsg){
	printf("type=%d\n", pmsg->type.cmd_type);//打印命令类型
	printf("uid=%d\n", pmsg->uid);//打印用户ID
	printf("u1=%s\n", pmsg->u1.param);//打印参数
	printf("u2=%s\n", pmsg->u2.dir);//打印路径
}
//生成长度为num的随机字符串
void get_rand_str(char *s,int num){
	//定义随机生成字符串表
	char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"'<>?";
	int i,lstr;
	char ss[2] = {0};
	lstr = strlen(str);
	srand(time(NULL));
	for(i = 1; i <= num; i++){
		sprintf(ss,"%c",str[(rand()%lstr)]);//在str里随机取一个字符存入ss
		strcat(s,ss);//拼接到s尾部
	}
}

//删除字符串头尾的空格
void strip(char *str){
	char *p = str;//p记录str首
	while (*str==' '){//跳过头部空格
		str++;
	}
	while (*str){//移到尾部
		*p++ = *str++;
	}
	int n = 0;
	str--;
	while (*str==' '){//n记录尾部空格数
		str--;
		n++;
	}
	p[strlen(p)-n]=0;//截断
}
//字符串s按照separation里的字符分割,存储在store中
void split(const char *s, const char *separation, char **store, int *len){//待划分字符串s | 划分字符separation | 指针数组,存储划分后的字符串 | 划分行数
	if (!s){
		return;
	}
	int i = 0,j = 0, flag = 0;
	while (*s && find(separation, *s)){//若s首字符与separation首字符匹配,跳过
		s++;
	}
	while (*s){
		while (*s && !find(separation, *s)){//存入第i行
			flag = 1;
			store[i][j++] = *s++;
		}
		while (*s && find(separation, *s)){//跳过
			s++;
		}
		if (flag){
			i++;
			if (i >= *len){//行数超出len
				break;
			}
			j = 0;	
		}
	}
	*len = i;
}

//在字符串s中查找字符c,找到返回1,否则返回0
int find(const char *s, const char c){//字符串 | 要找的字符
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

#include "factory.h"

pfactory_t g_pfac;

int main(int argc, char **argv){
	if (argc < 1){
		printf("error args\n");
		return -1;
	}
	config_t config;
	get_config(&config ,argv[1]);
	factory_t fac;
	factory_init(&fac, &config);
	g_pfac = &fac;
	int len = sizeof(struct sockaddr_in);
	char input[256];
	message_t msg;
	int flag = 0;
	while (1){
		if(!connect(fac.sfd, (struct sockaddr *)&fac.ser, len)){
			send(fac.sfd, &flag, 4, 0);
			while(1){
				fgets(input, sizeof(input), stdin);
				input[strlen(input)-1]=0;
				if(!message_handle(&msg, input)){
					command_handle(&fac, &msg);
				}
				else{
					printf("commad not found\n");
					if (fac.info.signin_flag){
						printf("%s@ftp:~/%s$ ", fac.info.user_name, fac.info.dir+1);
					}
				}
			}
		}
		else{
			perror("connect");
			sleep(3);
		}
	}
}

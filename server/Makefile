OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
server:$(OBJS)
	gcc  -o $@ $^ -pthread -I /usr/include/mysql -lmysqlclient -lcrypto
CC:=gcc
CFLAGS:=-Wall -g
clean:
	rm -rf server $(OBJS)

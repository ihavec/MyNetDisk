OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
client:$(OBJS)
	gcc -o $@ $^ -pthread -lcrypt -lcrypto
CC:=gcc
CFLAGS:=-Wall -g
clean:
	rm -rf client $(OBJS)
